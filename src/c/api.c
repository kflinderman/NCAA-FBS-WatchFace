/*
Champ stuff
GET /records
if total.wins > 6
winning season
if last regular season game contains Championship + week
Conf Champ
if postseason.games = 1 && postseason.wins = 1 (need to figure out other playoff teams not getting this)
Bowl win
if postseason.games > 1 && postseason.loses != 1 (need to figure out other playoff teams not getting this)
Champion
*/

#include <pebble.h>
#include "api.h"
#include "globals.h"
#include "drawing.h"
#include "outbox_queue.h"
#include "timekeeping.h"

/**
 * CFBD sync protocol (team-by-team, split by sync type)
 * -----------------------------------------------------------------------
 * Full sync (every 24h) fetches calendar (year, next season kickoff) PLUS
 * records + rankings for the current week - records/rankings don't change
 * fast enough to need their own more-frequent sync, so they ride along
 * with the daily full sync instead of the (now games-only) light sync.
 *
 * Light sync fetches only this week's games, on its own cadence
 * (settings.cfbd.last_light_sync_ts, independent of full sync's timing).
 *
 * Each sync type, once JS has fetched its data, walks TEAMS[] one team
 * at a time exactly like before - but now there are two independent walk
 * types (CFBD_TEAM_DATA_GAMES, CFBD_TEAM_DATA_RECORDS), each updating only
 * the fields it's responsible for:
 *   1. JS finishes fetching -> sends CFBD_LIGHT_SYNC_READY (games) or
 *      CFBD_RECORDS_SYNC_READY (records+rankings).
 *   2. C starts (or, if a walk of the other type is already running,
 *      defers) a walk of that type: team cursor to 0, sends
 *      REQUEST_CFBD_TEAM_DATA with CFBD_TEAM_INDEX + CFBD_TEAM_NAME +
 *      CFBD_TEAM_DATA_TYPE for TEAMS[0].
 *   3. JS looks team_name up in whichever cache matches the requested
 *      type (already-fetched, not re-fetched) and replies with just that
 *      type's fields - opponent/score/gametime for games, or
 *      rank/wins/postseason for records.
 *   4. C applies whichever fields are present (their presence alone
 *      tells it what to apply - no separate branch needed) to
 *      TEAMS[team_index], then requests the next index, repeating
 *      until TEAMS_COUNT is reached, then starts whatever walk was
 *      deferred in step 2, if any.
 *
 * This means at most one small AppMessage dictionary (well under the
 * existing 512-byte inbox/outbox) is ever in flight for CFBD data - no
 * static JSON buffer of any size is needed, which is what actually fixes
 * the aplite .bss overflow: the old approach's problem was trying to hold
 * whole (or large chunks of) games/records/rankings payloads in RAM at
 * once, and this protocol never does that at all.
 */

#define MAX_DISPLAYABLE_SCORE 99

static int cfbd_current_team_index = -1; // -1 = no team walk in progress
static CFBDTeamDataType cfbd_current_sync_type; // only meaningful while cfbd_current_team_index >= 0

// If a sync-type's "ready" signal arrives while the OTHER type's walk is
// still running, it's remembered here and started once the running walk
// finishes, rather than corrupting the in-progress walk's state.
static bool cfbd_pending_games_walk = false;
static bool cfbd_pending_records_walk = false;

// In-flight guard covering a light sync's entire lifecycle - set when
// api_request_cfbd_light_sync() sends the request, and only cleared once
// the resulting games walk fully completes in cfbd_team_walk_complete()
// (not merely when JS's ready-signal arrives - see that function's
// comments for why the walk's completion, not the ready-signal, is the
// right place to release this).
static bool cfbd_light_sync_pending = false;

static void cfbd_team_walk_complete(CFBDTeamDataType type);

/**
 * Look up a team by name in the full TEAMS[] roster (used to resolve an
 * opponent name to a logo/color entry for drawing - separate from
 * TEAMS[], which only holds the teams actively tracked on the watch).
 * Returns NULL if the opponent isn't in the curated TEAMS[] roster (this
 * is expected/common - most opponents won't be).
 */
static const Team *teams_find_by_name(const char *name) {
  if (!name || !name[0]) return NULL;
  if (strcmp(name, "NA") == 0) return NULL; // never resolve a placeholder opponent

  for (size_t i = 0; i < TEAMS_COUNT; i++) {
    if (!TEAMS[i].name) continue;
    if (strcmp(TEAMS[i].name, "NA") == 0) continue; // skip placeholder roster slots
    if (strcmp(TEAMS[i].name, name) == 0) return &TEAMS[i];
  }
  return NULL;
}

// Threshold (percent) at which api_calls_nearing_limit() reports true -
// matches the "like the battery indicator" idea from the header comment
// above. 90% leaves a reasonable buffer before actually hitting the cap.
#define CFBD_API_CALLS_WARNING_PERCENT 90

/**
 * Applies CFBD_API_CALLS_USED/CFBD_API_CALLS_LIMIT from an incoming
 * message to settings.cfbd, if present. JS is the source of truth for
 * these - it's the one making real HTTP calls and correcting its own
 * count against CFBD's GET /info - so the watch always just mirrors
 * whatever JS last reported rather than estimating locally. Called from
 * both the calendar (full sync) and light-sync-ready handlers, since JS
 * reports current usage after every sync, not just full syncs.
 */
static void apply_api_usage_from_message(DictionaryIterator *iterator) {
  Tuple *used_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_API_CALLS_USED);
  Tuple *limit_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_API_CALLS_LIMIT);

  if (used_tuple) {
    settings.cfbd.api_calls_this_month = (uint16_t)used_tuple->value->int32;
  }
  if (limit_tuple) {
    settings.cfbd.api_calls_monthly_limit = (uint16_t)limit_tuple->value->int32;
  }

  if (used_tuple || limit_tuple) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "CFBD API usage: %d/%d",
            settings.cfbd.api_calls_this_month, settings.cfbd.api_calls_monthly_limit);
    #endif
  }
}

/**
 * Percent of this month's call budget used so far, 0-100. Returns 0 if the
 * limit isn't known yet (no full sync has completed since app install).
 * For a future UI indicator (battery-style icon) to consume.
 */
uint8_t api_calls_percent_used(void) {
  if (settings.cfbd.api_calls_monthly_limit == 0) return 0;
  uint32_t percent = ((uint32_t)settings.cfbd.api_calls_this_month * 100)
    / settings.cfbd.api_calls_monthly_limit;
  return (uint8_t)(percent > 100 ? 100 : percent);
}

/**
 * True once usage crosses CFBD_API_CALLS_WARNING_PERCENT of the monthly
 * limit. For a future UI indicator to consume - see api_calls_percent_used.
 */
bool api_calls_nearing_limit(void) {
  return api_calls_percent_used() >= CFBD_API_CALLS_WARNING_PERCENT;
}

/**
 * Builds the REQUEST_CFBD_TEAM_DATA message for whichever team
 * cfbd_current_team_index currently points at, tagged with
 * cfbd_current_sync_type so JS knows whether to compute games fields or
 * records/rankings fields for this team.
 */
static void build_request_team_data(DictionaryIterator *iter) {
  uint16_t team_index = (uint16_t)cfbd_current_team_index;
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_TEAM_DATA, 1);
  dict_write_uint16(iter, MESSAGE_KEY_CFBD_TEAM_INDEX, team_index);
  dict_write_cstring(iter, MESSAGE_KEY_CFBD_TEAM_NAME, TEAMS[team_index].name);
  dict_write_uint8(iter, MESSAGE_KEY_CFBD_TEAM_DATA_TYPE, (uint8_t)cfbd_current_sync_type);
}

/**
 * Queues REQUEST_CFBD_TEAM_DATA for TEAMS[cfbd_current_team_index],
 * asking JS for that one team's data (games or records/rankings,
 * depending on cfbd_current_sync_type). Assumes cfbd_current_team_index
 * is valid (checked by the caller before setting it).
 */
static void request_team_data(void) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting CFBD data for team %d/%d (%s), type %d",
          cfbd_current_team_index + 1, (int)TEAMS_COUNT, TEAMS[cfbd_current_team_index].name,
          cfbd_current_sync_type);
  #endif

  outbox_queue_send(build_request_team_data);
}

/**
 * Starts a team walk of the given type, unless one is already running -
 * in that case the request is remembered and started once the running
 * walk completes (see cfbd_team_walk_complete), rather than clobbering
 * its in-progress state.
 */
static void start_team_walk(CFBDTeamDataType type) {
  if (cfbd_current_team_index >= 0) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD team walk (type %d) deferred - type %d walk in progress",
            type, cfbd_current_sync_type);
    #endif
    if (type == CFBD_TEAM_DATA_GAMES) {
      cfbd_pending_games_walk = true;
    } else {
      cfbd_pending_records_walk = true;
    }
    return;
  }

  if (TEAMS_COUNT == 0) {
    cfbd_team_walk_complete(type);
    return;
  }

  cfbd_current_sync_type = type;
  cfbd_current_team_index = 0;
  request_team_data();
}

/**
 * Called once all of TEAMS[] has been walked for the given sync type.
 * Updates whichever timestamp that type owns, then starts any walk that
 * got deferred while this one was running.
 */
static void cfbd_team_walk_complete(CFBDTeamDataType type) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "CFBD team walk complete (type %d) - all %d teams updated",
          type, (int)TEAMS_COUNT);
  #endif

  cfbd_current_team_index = -1;

  if (type == CFBD_TEAM_DATA_GAMES) {
    settings.cfbd.last_light_sync_ts = time(NULL);
    // Only now - once the games walk has actually finished applying data
    // to every tracked team - is it safe to let api_should_light_sync()
    // fire another request. Clearing this back when CFBD_LIGHT_SYNC_READY
    // first arrived left a window open: if the games walk got deferred
    // (a records walk was still running), globals_prv_update_display()
    // a few lines below would see the guard already released and
    // last_light_sync_ts still 0, and fire a second, premature
    // REQUEST_CFBD_LIGHT_SYNC right as the deferred walk was about to
    // start on its own.
    cfbd_light_sync_pending = false;
  } else {
    settings.cfbd.last_full_sync_ts = time(NULL);
  }
  settings.cfbd.api_data_valid = true;
  globals_prv_save_settings();
  globals_prv_update_display();

  if (cfbd_pending_games_walk) {
    cfbd_pending_games_walk = false;
    start_team_walk(CFBD_TEAM_DATA_GAMES);
  } else if (cfbd_pending_records_walk) {
    cfbd_pending_records_walk = false;
    start_team_walk(CFBD_TEAM_DATA_RECORDS);
  }
}

static void build_request_full_sync(DictionaryIterator *iter) {
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_FULL_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
}

uint8_t api_update_status_indicator() {
  // Only one status GBitmap is ever resident at a time (destroyed and
  // recreated here as the state changes) instead of both API_LOW and
  // API_EMPTY being preloaded permanently at startup.
  if (api_calls_percent_used() >= 99) {
    if (s_gbitmap_layers[GBITMAP_LAYER_API]) {
      gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_API]);
    }
    s_gbitmap_layers[GBITMAP_LAYER_API] = gbitmap_create_with_resource(RESOURCE_ID_APIEMPTY);
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_API], s_gbitmap_layers[GBITMAP_LAYER_API]);
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), false);
    return 0;
  } else if (api_calls_nearing_limit()) {
    if (s_gbitmap_layers[GBITMAP_LAYER_API]) {
      gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_API]);
    }
    s_gbitmap_layers[GBITMAP_LAYER_API] = gbitmap_create_with_resource(RESOURCE_ID_APILOW);
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_API], s_gbitmap_layers[GBITMAP_LAYER_API]);
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), false);
    return 1;
  } else {
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), true);
    return 2;
  }
}

void api_request_cfbd_full_sync(void) {
  if (!settings.api || settings.api_key[0] == '\0') {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD full sync skipped: API disabled or no key");
    #endif
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), true);
    return;
  }
  else if (api_update_status_indicator() == 0){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD full sync skipped: API calls used for the month");
    #endif
    return;
  }

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD full sync (calendar)");
  #endif
  outbox_queue_send(build_request_full_sync);


}

static void build_request_light_sync(DictionaryIterator *iter) {
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_LIGHT_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
}

// cfbd_light_sync_pending (declared near the top of the file, alongside
// the other sync-state statics) guards this: globals_prv_update_display()
// can call api_request_cfbd_light_sync() from more than one place in the
// same invocation (countdown block + score block), and update_display()
// itself gets invoked from several other call sites (settings changed,
// sync complete). Without this flag, all of those can independently
// decide "no valid data yet" and each fire their own
// REQUEST_CFBD_LIGHT_SYNC before the first one's response has come back,
// causing JS to run two (or more) concurrent games walks racing each
// other.

void api_request_cfbd_light_sync(void) {
  if (!settings.api || settings.api_key[0] == '\0') {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: API disabled or no key");
    #endif
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), true);
    return;
  }
  if (cfbd_light_sync_pending) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD light sync skipped: already in flight");
    #endif
    return;
  }
  else if (api_update_status_indicator() == 0){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: API calls used for the month");
    #endif
    return;
  }

  // Need calendar data from a prior full sync so JS can determine the
  // current week itself (it keeps season/week dates in its own cache).
  //if (!settings.cfbd.api_data_valid) {
  //APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: no prior data");
  //return;
  //}

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD light sync");
  #endif
  cfbd_light_sync_pending = true;
  outbox_queue_send(build_request_light_sync);
}

bool api_should_full_sync(void) {
  time_t now = time(NULL);

  if(settings.api_quiet && !timekeeping_is_quiet_time()){
    return false;
  }
  
  // Never synced, or more than 24 hours since last full sync
  if (settings.api && (!settings.cfbd.api_data_valid ||
                       (now - settings.cfbd.last_full_sync_ts >= 86400))) {
    return true;
  }

  // Also check: if we've crossed into next season, force a sync
  time_t next_game_ts = settings.cfbd.next_season_first_game_ts;
  if (next_game_ts > 0 && now >= next_game_ts && !settings.cfbd.api_data_valid) {
    return true;
  }

  return false;
}

bool api_should_light_sync(void) {
  
  if(settings.api_quiet && !timekeeping_is_quiet_time()){
    return false;
  }
  
  if (cfbd_light_sync_pending) {
    return false;
  }

  time_t now = time(NULL);

  // Light sync (games only) currently weekly, same as before the
  // records/rankings split - now tracked against its own timestamp
  // instead of full sync's, since the two are no longer coupled. Worth
  // revisiting: games likely need fresher data than records/rankings did
  // (e.g. on game day), so this interval may want to come down now that
  // it's decoupled - left at 7 days for now since that wasn't explicitly
  // asked to change.
  if (settings.api && (!settings.cfbd.api_data_valid ||
                       (now - settings.cfbd.last_light_sync_ts >= (settings.scoreUpdate * 60)))) {
    return true;
  }
  return false;
}

void api_cfbd_callback(DictionaryIterator *iterator, void *context) {
  // Full sync response: calendar data (year, next season kickoff).
  Tuple *year_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_YEAR);
  if (year_tuple) {
    settings.cfbd.current_season_year = year_tuple->value->uint16;

    Tuple *next_season_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_NEXT_SEASON_TS);
    if (next_season_tuple) {
      settings.cfbd.next_season_first_game_ts = next_season_tuple->value->uint32;
    }

    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD calendar received: year %d, next season ts %lu",
            settings.cfbd.current_season_year, (unsigned long)settings.cfbd.next_season_first_game_ts);
    #endif

    apply_api_usage_from_message(iterator);

    apply_api_usage_from_message(iterator);

    globals_prv_save_settings();
    return;
  }

  // Light sync: JS is ready to serve per-team game lookups. Kick off (or
  // defer, if a records walk is already running) a games-type team-by-team
  // walk. cfbd_light_sync_pending stays true through this - it's only
  // cleared once the games walk actually completes in
  // cfbd_team_walk_complete(), not here, so a deferred walk can't get
  // raced by another premature light-sync request in the meantime.
  Tuple *games_ready_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_LIGHT_SYNC_READY);
  if (games_ready_tuple) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD games ready - requesting %d teams", (int)TEAMS_COUNT);
    #endif

    apply_api_usage_from_message(iterator);
    globals_prv_save_settings();

    start_team_walk(CFBD_TEAM_DATA_GAMES);
    return;
  }

  // Full sync also fetches records+rankings (moved here from light sync -
  // they don't change fast enough to need their own cadence, so they ride
  // along with the daily full sync instead). Same pattern as above, just
  // a records-type walk instead of games-type.
  Tuple *records_ready_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_RECORDS_SYNC_READY);
  if (records_ready_tuple) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD records/rankings ready - requesting %d teams", (int)TEAMS_COUNT);
    #endif

    apply_api_usage_from_message(iterator);
    globals_prv_save_settings();

    start_team_walk(CFBD_TEAM_DATA_RECORDS);
    return;
  }

  // Per-team response: apply this team's data to TEAMS[team_index],
  // then move on to the next team (or finish). Which fields are present
  // depends on which walk type is running - a games-type response won't
  // include rank/wins/postseason, and a records-type response won't
  // include opponent/score/gametime - so we just apply whatever's
  // actually there rather than branching on type explicitly.
  Tuple *team_index_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_INDEX);
  if (team_index_tuple) {
    uint16_t team_index = team_index_tuple->value->uint16;

    if (cfbd_current_team_index < 0 || team_index != (uint16_t)cfbd_current_team_index) {
      #if defined(DEBUG)
      APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD team data for index %d ignored - expected %d",
              team_index, cfbd_current_team_index);
      #endif
      return;
    }

    if (team_index >= TEAMS_COUNT) {
      #if defined(DEBUG)
      APP_LOG(APP_LOG_LEVEL_ERROR, "CFBD team data index %d out of range", team_index);
      #endif
      cfbd_team_walk_complete(cfbd_current_sync_type);
      return;
    }

    Team *info = &TEAMS[team_index];

    Tuple *team_opponent_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_OPPONENT);
    Tuple *score_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_SCORE);
    Tuple *vs_score_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_VS_SCORE);
    Tuple *gametime_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_GAMETIME);
    Tuple *completed_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_COMPLETED);
    Tuple *rank_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_RANK);
    Tuple *wins_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_WINS);
    Tuple *ps_games_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_GAMES);
    Tuple *ps_wins_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_WINS);
    Tuple *ps_losses_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_LOSSES);

    // Only a games-type response includes the opponent field at all (even
    // an empty-string opponent for a bye week) - a records-type response
    // has no game to report, so vs_id is left untouched rather than wiped
    // by a records-only sync.
    if (team_opponent_tuple) {
      const char *opponent_name = team_opponent_tuple->value->cstring;
      if (opponent_name && opponent_name[0]) {
        const Team *opp = teams_find_by_name(opponent_name);
        // vs_id only means something if the opponent is itself in the
        // curated TEAMS[] roster (needed to draw their logo/colors) -
        // most opponents won't be, and that's fine; score/record fields
        // below still apply regardless. Falls through to the sentinel
        // below if not found, so a stale opponent from a prior week's
        // data can't linger. -1 (not 0) is the "no opponent" sentinel,
        // since 0 is a real, valid TEAMS[] index (Clemson).
        info->vs_id = opp ? (int16_t)(opp - TEAMS) : -1;
      } else {
        // Bye week - no game this week, so no opponent to show.
        info->vs_id = -1;
      }
    }

    if (score_tuple) info->score = (uint16_t)score_tuple->value->int32;
    if (vs_score_tuple) info->vs_score = (uint16_t)vs_score_tuple->value->int32;
    if (gametime_tuple) info->gametime = (uint32_t)gametime_tuple->value->int32;
    if (completed_tuple) info->completed = (completed_tuple->value->int32 != 0);
    if (rank_tuple) info->ranking = (uint16_t)rank_tuple->value->int32;
    if (wins_tuple) info->wins = (uint16_t)wins_tuple->value->int32;
    if (ps_games_tuple) info->postseasonGames = (uint16_t)ps_games_tuple->value->int32;
    if (ps_wins_tuple) info->postseasonWins = (uint16_t)ps_wins_tuple->value->int32;
    if (ps_losses_tuple) info->postseasonLosses = (uint16_t)ps_losses_tuple->value->int32;

    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "CFBD team %d (%s) type %d updated: vsd=%d score=%d-%d rank=%d wins=%d",
            team_index, info->name, cfbd_current_sync_type, info->vs_id, info->score, info->vs_score,
            info->ranking, info->wins);
    #endif

    uint16_t next_index = team_index + 1;
    if (next_index < TEAMS_COUNT) {
      cfbd_current_team_index = next_index;
      request_team_data();
    } else {
      cfbd_team_walk_complete(cfbd_current_sync_type);
    }
  }
}

void format_2digits(char *buf, int val) {
  buf[0] = '0' + ((val / 10) % 10);
  buf[1] = '0' + (val % 10);
}

void api_score_display() {
  // 1. Direct pointers replace s_temp_buffer1 and s_temp_buffer2
  const char *home_str;
  const char *away_str;

  // 2. Only s_temp_buffer3 needs memory for formatted score "XX|YY\0"
  static char s_score_buffer[6];

  if (TEAMS[settings.FavoriteTeam].vs_id == -1) {
    home_str = "BYE";
    away_str = "WEEK";
    // Copy constant string directly instead of snprintf
    memcpy(s_score_buffer, "00|00", 6);
  } else {
    // Zero-copy! Point directly to existing string constants
    home_str = TEAMS[settings.FavoriteTeam].shortname;
    away_str = TEAMS[TEAMS[settings.FavoriteTeam].vs_id].shortname;

    // Clamp scores cleanly
    int score1 = TEAMS[settings.FavoriteTeam].score;
    int score2 = TEAMS[settings.FavoriteTeam].vs_score;
    if (score1 > MAX_DISPLAYABLE_SCORE) score1 = MAX_DISPLAYABLE_SCORE;
    if (score2 > MAX_DISPLAYABLE_SCORE) score2 = MAX_DISPLAYABLE_SCORE;

    // Manual string building for "XX|YY" without snprintf
    format_2digits(&s_score_buffer[0], score1);
    s_score_buffer[2] = '|';
    format_2digits(&s_score_buffer[3], score2);
    s_score_buffer[5] = '\0';
  }
  text_layer_set_text(s_text_layers[TEXT_LAYER_HOME], home_str);
  text_layer_set_text(s_text_layers[TEXT_LAYER_AWAY], away_str);
  
  text_layer_set_text(s_text_layers[TEXT_LAYER_SCORE], s_score_buffer);
}

void api_icon_draw(Layer *window_layer, GRect bounds){
  GRect logo_bounds = layer_get_bounds(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));

  // No GBitmap created here - api_update_status_indicator() creates
  // whichever status icon actually applies, so only one (or none) is
  // ever resident instead of both APILOW and APIEMPTY permanently.
  s_gbitmap_layers[GBITMAP_LAYER_API] = NULL;

  #if PBL_DISPLAY_HEIGHT > 180
  //168
  s_bitmap_layers[BITMAP_LAYER_API] = drawing_bitmap_set((bounds.size.w * hor_2) / 1000 - (icon_bump + 19), (bounds.size.h * vert_2) / 1000 + 3, 8, 14, NULL, window_layer);
  #else
  s_bitmap_layers[BITMAP_LAYER_API] = drawing_bitmap_set((bounds.size.w * hor_2) / 1000 - (icon_bump + 10), (bounds.size.h * vert_2) / 1000 + 3, 4, 7, NULL, window_layer);
  //+ 14
  #endif

  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), true);

  //Create Ranking resources
  #ifdef PBL_ROUND
  s_layers[LAYER_RANK_RECT] = layer_create_with_data(GRect((logo_bounds.size.w / 2) - 20, 0, 40, 25), sizeof(RoundRectData));
  #else
  s_layers[LAYER_RANK_RECT] = layer_create_with_data(GRect(0, 0, 40, 25), sizeof(RoundRectData));
  #endif
  RoundRectData *rect_beat_data = (RoundRectData *)layer_get_data(s_layers[LAYER_RANK_RECT]);
  rect_beat_data->fill_color = GColorWhite;
  layer_set_update_proc(s_layers[LAYER_RANK_RECT], drawing_round_rect_update_proc);
  layer_add_child(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]), s_layers[LAYER_RANK_RECT]);
  s_text_layers[TEXT_LAYER_RANK] = drawing_text_set(0, -4, 40, 25, GColorBlack, "#00", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter, s_layers[LAYER_RANK_RECT]);
  

  layer_set_hidden(s_layers[LAYER_RANK_RECT], true);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_RANK]), true);

  //Create supurlitive resources
  s_gbitmap_layers[GBITMAP_LAYER_WIN] = gbitmap_create_with_resource(RESOURCE_ID_WIN);
  // No GBitmap created here for the trophy slot - the bowlBool block in
  // globals.c creates whichever of BOWL/CHAMP actually applies, so only
  // one (or none) is ever resident instead of both permanently.
  s_gbitmap_layers[GBITMAP_LAYER_TROPHY] = NULL;

  #ifdef PBL_ROUND
  s_bitmap_layers[BITMAP_LAYER_WIN] = drawing_bitmap_set(logo_bounds.size.w - 50, logo_bounds.size.h - 20, 12, 12, s_gbitmap_layers[GBITMAP_LAYER_WIN], bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  s_bitmap_layers[BITMAP_LAYER_TROPHY] = drawing_bitmap_set(logo_bounds.size.w - 30, logo_bounds.size.h - 20, 12, 12, NULL, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  #else
  s_bitmap_layers[BITMAP_LAYER_WIN] = drawing_bitmap_set(logo_bounds.size.w - 30, 0, 12, 12, s_gbitmap_layers[GBITMAP_LAYER_WIN], bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  s_bitmap_layers[BITMAP_LAYER_TROPHY] = drawing_bitmap_set(logo_bounds.size.w - 30, logo_bounds.size.h - 20, 12, 12, NULL, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  #endif
  
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_WIN]), true);
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_TROPHY]), true);
}