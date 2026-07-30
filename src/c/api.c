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

Full sync takes 8s
*/

// src/c/api_cfbd.c
#include <pebble.h>
#include "api.h"
#include "globals.h"
#include "drawing.h"

/**
 * CFBD sync protocol (team-by-team)
 * -----------------------------------------------------------------------
 * Full sync fetches calendar data only (year, next season kickoff) - see
 * CFBD_YEAR / CFBD_NEXT_SEASON_TS handling below.
 *
 * Light sync fetches this week's games/records/rankings ONCE on the JS
 * side (three CFBD API calls total, cached in JS memory), then the two
 * sides walk API_DATA[] together one team at a time:
 *   1. JS finishes fetching -> sends CFBD_LIGHT_SYNC_READY.
 *   2. C resets its team cursor to 0 and sends REQUEST_CFBD_TEAM_DATA with
 *      CFBD_TEAM_INDEX + CFBD_TEAM_NAME for API_DATA[0].
 *   3. JS looks team_name up in its already-fetched (not re-fetched) light
 *      sync data and replies with one small AppMessage of that team's
 *      opponent/score/rank/record fields.
 *   4. C applies those fields to API_DATA[team_index], then requests the
 *      next index, repeating until API_DATA_COUNT is reached.
 *
 * This means at most one small AppMessage dictionary (well under the
 * existing 512-byte inbox/outbox) is ever in flight for CFBD data - no
 * static JSON buffer of any size is needed, which is what actually fixes
 * the aplite .bss overflow: the old approach's problem was trying to hold
 * whole (or large chunks of) games/records/rankings payloads in RAM at
 * once, and this protocol never does that at all.
 */

static int cfbd_current_team_index = -1; // -1 = no light sync in progress

/**
 * Look up a team by name in the full TEAMS[] roster (used to resolve an
 * opponent name to a logo/color entry for drawing - separate from
 * API_DATA[], which only holds the teams actively tracked on the watch).
 * Returns NULL if the opponent isn't in the curated TEAMS[] roster (this
 * is expected/common - most opponents won't be).
 */
static const Team *teams_find_by_name(const char *name) {
  if (!name || !name[0]) return NULL;
  for (size_t i = 0; i < TEAMS_COUNT; i++) {
    if (TEAMS[i].name && strcmp(TEAMS[i].name, name) == 0) {
      return &TEAMS[i];
    }
  }
  return NULL;
}

// Threshold (percent) at which api_calls_nearing_limit() reports true -
// matches the "like the battery indicator" idea from the header comment
// above. 90% leaves a reasonable buffer before actually hitting the cap.
#define CFBD_API_CALLS_WARNING_PERCENT 20
//#define CFBD_API_CALLS_WARNING_PERCENT 90

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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "CFBD API usage: %d/%d",
      settings.cfbd.api_calls_this_month, settings.cfbd.api_calls_monthly_limit);
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


void debug_dump_api_info(const API_Info *array, size_t count) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "=== DUMPING %u API_INFO RECORDS ===", (unsigned int)count);

  for (size_t i = 0; i < count; i++) {
    const API_Info *item = &array[i];

    // Single line log per item to avoid log buffer overflow
    APP_LOG(APP_LOG_LEVEL_DEBUG, 
            "[%03u] %s | ID:%u VS:%d | Score:%u-%u | W:%u | GT:%lu | Rank:%u PS-G:%u PS-W:%u PS-L:%u",
            (unsigned int)i,
            item->name ? item->name : "NULL",
            item->id,
            item->vs_id,
            item->score,
            item->vs_score,
            item->wins,
            (unsigned long)item->gametime,
            item->ranking, 
            item->postseasonGames, 
            item->postseasonWins, 
            item->postseasonLosses);

    // OPTIONAL: Add extra fields if needed
    /*
    APP_LOG(APP_LOG_LEVEL_DEBUG, 
            "      -> Rank:%u PS-G:%u PS-W:%u PS-L:%u",
            item->ranking, 
            item->postseasonGames, 
            item->postseasonWins, 
            item->postseasonLosses);
    */
    
    // Give the Pebble logging system breathing room every 20 records
    if (i > 0 && i % 20 == 0) {
      psleep(10); 
    }
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "=== END DUMP ===");
}


/**
 * Sends REQUEST_CFBD_TEAM_DATA for API_DATA[team_index], asking JS for
 * that one team's opponent/score/rank/record. Assumes team_index is valid
 * (checked by the caller).
 */
static void request_team_data(uint16_t team_index) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting CFBD data for team %d/%d (%s)",
    team_index + 1, (int)API_DATA_COUNT, API_DATA[team_index].name);

  DictionaryIterator *iter;
  if (app_message_outbox_begin(&iter) != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to begin outbox for team data request");
    return;
  }
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_TEAM_DATA, 1);
  dict_write_uint16(iter, MESSAGE_KEY_CFBD_TEAM_INDEX, team_index);
  dict_write_cstring(iter, MESSAGE_KEY_CFBD_TEAM_NAME, API_DATA[team_index].name);
  app_message_outbox_send();
}

/**
 * Called once all of API_DATA[] has been filled in for this light sync.
 */
static void cfbd_light_sync_complete(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "CFBD light sync complete - all %d teams updated", (int)API_DATA_COUNT);

  //debug_dump_api_info(API_DATA, API_DATA_COUNT);
  
  cfbd_current_team_index = -1;
  settings.cfbd.last_full_sync_ts = time(NULL);
  settings.cfbd.api_data_valid = true;
  globals_prv_save_settings();
  globals_prv_update_display();
}

void api_request_cfbd_full_sync(void) {
  if (!settings.api || settings.api_key[0] == '\0') {
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD full sync skipped: API disabled or no key");
    layer_set_hidden(bitmap_layer_get_layer(s_api_layer), true);
    return;
  }
  else if (api_calls_percent_used() >= 99){
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD full sync skipped: API calls used for the month");
    bitmap_layer_set_bitmap(s_api_layer, s_api_empty_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_api_layer), false);
    return;
  }
  else{
    if (api_calls_nearing_limit()){
      bitmap_layer_set_bitmap(s_api_layer, s_api_low_bitmap);
      layer_set_hidden(bitmap_layer_get_layer(s_api_layer), false);
    }
    else{
      layer_set_hidden(bitmap_layer_get_layer(s_api_layer), true);
    }
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD full sync (calendar)");

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_FULL_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
  app_message_outbox_send();
}

void api_request_cfbd_light_sync(void) {
  if (!settings.api || settings.api_key[0] == '\0') {
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: API disabled or no key");
    layer_set_hidden(bitmap_layer_get_layer(s_api_layer), true);
    return;
  }
  else if (api_calls_percent_used() >= 99){
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD full sync skipped: API calls used for the month");
    bitmap_layer_set_bitmap(s_api_layer, s_api_empty_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_api_layer), false);
    s_batt_history = 0;
    return;
  }
  else{
    if (api_calls_nearing_limit()){
      bitmap_layer_set_bitmap(s_api_layer, s_api_low_bitmap);
      layer_set_hidden(bitmap_layer_get_layer(s_api_layer), false);
    }
    else{
      layer_set_hidden(bitmap_layer_get_layer(s_api_layer), true);
    }
  }

  // Need calendar data from a prior full sync so JS can determine the
  // current week itself (it keeps season/week dates in its own cache).
  //if (!settings.cfbd.api_data_valid) {
    //APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: no prior data");
    //return;
  //}

  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD light sync");

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_LIGHT_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
  app_message_outbox_send();
}

bool api_should_full_sync(void) {
  time_t now = time(NULL);

  // Never synced, or more than 24 hours since last full sync
  if (!settings.cfbd.api_data_valid ||
      (now - settings.cfbd.last_full_sync_ts > 86400)) {
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
  time_t now = time(NULL);

  // Light sync weekly (every 7 days)
  if (settings.cfbd.api_data_valid &&
      (now - settings.cfbd.last_full_sync_ts < 604800)) {
    return false;
  }
  return true;
}

void api_cfbd_callback(DictionaryIterator *iterator, void *context) {
  // Full sync response: calendar data only (year, next season kickoff).
  Tuple *year_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_YEAR);
  if (year_tuple) {
    settings.cfbd.current_season_year = year_tuple->value->uint16;

    Tuple *next_season_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_NEXT_SEASON_TS);
    if (next_season_tuple) {
      settings.cfbd.next_season_first_game_ts = next_season_tuple->value->uint32;
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD calendar received: year %d, next season ts %lu",
      settings.cfbd.current_season_year, (unsigned long)settings.cfbd.next_season_first_game_ts);

    apply_api_usage_from_message(iterator);

    globals_prv_save_settings();
    return;
  }

  // Light sync: JS has fetched (once) and cached games/records/rankings
  // for the current week and is ready to serve per-team lookups. Kick off
  // the team-by-team walk starting at API_DATA[0].
  Tuple *ready_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_LIGHT_SYNC_READY);
  if (ready_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD light sync ready - requesting %d teams", (int)API_DATA_COUNT);

    apply_api_usage_from_message(iterator);
    globals_prv_save_settings();

    if (API_DATA_COUNT == 0) {
      cfbd_light_sync_complete();
      return;
    }

    cfbd_current_team_index = 0;
    request_team_data(0);
    return;
  }

  // Per-team response: apply this team's data to API_DATA[team_index],
  // then move on to the next team (or finish).
  Tuple *team_index_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_INDEX);
  Tuple *team_opponent_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_OPPONENT);
  if (team_index_tuple && team_opponent_tuple) {
    uint16_t team_index = team_index_tuple->value->uint16;

    if (cfbd_current_team_index < 0 || team_index != (uint16_t)cfbd_current_team_index) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD team data for index %d ignored - expected %d",
        team_index, cfbd_current_team_index);
      return;
    }

    if (team_index >= API_DATA_COUNT) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "CFBD team data index %d out of range", team_index);
      cfbd_light_sync_complete();
      return;
    }

    API_Info *info = &API_DATA[team_index];
    const char *opponent_name = team_opponent_tuple->value->cstring;

    Tuple *score_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_SCORE);
    Tuple *vs_score_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_VS_SCORE);
    Tuple *gametime_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_GAMETIME);
    Tuple *rank_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_RANK);
    Tuple *wins_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_WINS);
    Tuple *ps_games_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_GAMES);
    Tuple *ps_wins_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_WINS);
    Tuple *ps_losses_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_LOSSES);

    if (opponent_name && opponent_name[0]) {
      const Team *opp = teams_find_by_name(opponent_name);
      // vs_id only means something if the opponent is itself in the
      // curated TEAMS[] roster (needed to draw their logo/colors) - most
      // opponents won't be, and that's fine; score/record fields below
      // still apply regardless. Falls through to the sentinel below if
      // not found, so a stale opponent from a prior week's data can't
      // linger. -1 (not 0) is the "no opponent" sentinel, since 0 is a
      // real, valid TEAMS[] index (Clemson).
      info->vs_id = opp ? (int16_t)(opp - TEAMS) : -1;
    } else {
      // Bye week - no game this week, so no opponent to show.
      info->vs_id = -1;
    }

    if (score_tuple) info->score = (uint16_t)score_tuple->value->int32;
    if (vs_score_tuple) info->vs_score = (uint16_t)vs_score_tuple->value->int32;
    if (gametime_tuple) info->gametime = (uint32_t)gametime_tuple->value->int32;
    if (rank_tuple) info->ranking = (uint16_t)rank_tuple->value->int32;
    if (wins_tuple) info->wins = (uint16_t)wins_tuple->value->int32;
    if (ps_games_tuple) info->postseasonGames = (uint16_t)ps_games_tuple->value->int32;
    if (ps_wins_tuple) info->postseasonWins = (uint16_t)ps_wins_tuple->value->int32;
    if (ps_losses_tuple) info->postseasonLosses = (uint16_t)ps_losses_tuple->value->int32;

    APP_LOG(APP_LOG_LEVEL_DEBUG, "CFBD team %d (%s) updated: opp=%s score=%d-%d rank=%d wins=%d",
      team_index, info->name, opponent_name ? opponent_name : "", info->score, info->vs_score,
      info->ranking, info->wins);

    uint16_t next_index = team_index + 1;
    if (next_index < API_DATA_COUNT) {
      cfbd_current_team_index = next_index;
      request_team_data(next_index);
    } else {
      cfbd_light_sync_complete();
    }
  }
}

void api_score_display() {
  static char s_temp_buffer[8];
  snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%s", TEAMS[settings.FavoriteTeam].name);
  text_layer_set_text(s_home_layer, s_temp_buffer);
  snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%s", TEAMS[API_DATA[settings.FavoriteTeam].vs_id].name);
  text_layer_set_text(s_away_layer, s_temp_buffer);
  
  int score1, score2;
  
  if (API_DATA[settings.FavoriteTeam].score > 99) score1 = 99;
  else score1 = API_DATA[settings.FavoriteTeam].score;
  
  if (API_DATA[settings.FavoriteTeam].vs_score > 99) score2 = 99;
  else score2 = API_DATA[settings.FavoriteTeam].vs_score;
  
  snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%02d:%02d", score1, score2);
  text_layer_set_text(s_countdown_layer, s_temp_buffer);
}

void api_icon_draw(Layer *window_layer, GRect bounds){
  // Create Battery GBitmap from resource
  s_api_low_bitmap = gbitmap_create_with_resource(RESOURCE_ID_APILOW);
  s_api_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_APIEMPTY);
  
  #if PBL_DISPLAY_HEIGHT > 180
    //168
    s_api_layer = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump + 19), bounds.size.h * vert_2 + 3, 8, 14, s_api_low_bitmap, window_layer);
  #else
    s_api_layer = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump + 10), bounds.size.h * vert_2 + 3, 4, 7, s_api_low_bitmap, window_layer);
    //+ 14
  #endif
  
    
  layer_set_hidden(bitmap_layer_get_layer(s_api_layer), true);
}
