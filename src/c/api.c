#include <pebble.h>
#include "api.h"
#include "globals.h"
#include "drawing.h"
#include "outbox_queue.h"
#include "timekeeping.h"

/**********************/
/* Constants & State  */
/**********************/

#define MAX_DISPLAYABLE_SCORE 99
#define CFBD_API_CALLS_WARNING_PERCENT 90

// Sync state tracking variables
static int cfbd_current_team_index = -1; // -1 = no team in progress
static CFBDTeamDataType cfbd_current_sync_type; // only meaningful while cfbd_current_team_index >= 0
static bool cfbd_pending_games_walk = false;
static bool cfbd_pending_records_walk = false;
static bool cfbd_light_sync_pending = false;

// Forward declaration
static void cfbd_team_walk_complete(CFBDTeamDataType type);

/**********************/
/* Private Helpers    */
/**********************/

// Helper function to look up a team pointer by string name matching
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

// Extract API call quota and usage numbers from incoming message
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

// Calculate current monthly API usage percentage
uint8_t api_calls_percent_used(void) {
  if (settings.cfbd.api_calls_monthly_limit == 0) return 0;
  uint32_t percent = ((uint32_t)settings.cfbd.api_calls_this_month * 100)
    / settings.cfbd.api_calls_monthly_limit;
  return (uint8_t)(percent > 100 ? 100 : percent);
}

// Check if monthly API usage has reached warning threshold
bool api_calls_nearing_limit(void) {
  return api_calls_percent_used() >= CFBD_API_CALLS_WARNING_PERCENT;
}

// Construct dictionary request for team data
static void build_request_team_data(DictionaryIterator *iter) {
  uint16_t team_index = (uint16_t)cfbd_current_team_index;
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_TEAM_DATA, 1);
  dict_write_uint16(iter, MESSAGE_KEY_CFBD_TEAM_INDEX, team_index);
  dict_write_cstring(iter, MESSAGE_KEY_CFBD_TEAM_NAME, TEAMS[team_index].name);
  dict_write_uint8(iter, MESSAGE_KEY_CFBD_TEAM_DATA_TYPE, (uint8_t)cfbd_current_sync_type);
}

// Queue request to fetch data for current active team
static void request_team_data(void) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting CFBD data for team %d/%d (%s), type %d",
          cfbd_current_team_index + 1, (int)TEAMS_COUNT, TEAMS[cfbd_current_team_index].name,
          cfbd_current_sync_type);
  #endif

  outbox_queue_send(build_request_team_data);
}

// Begin stepping through all teams to fetch data sequentially
static void start_team_walk(CFBDTeamDataType type) {
  // If a walk is already active, defer this request
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

// Complete active team walk sequence, persist updated data, and check pending walks
static void cfbd_team_walk_complete(CFBDTeamDataType type) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "CFBD team walk complete (type %d) - all %d teams updated",
          type, (int)TEAMS_COUNT);
  #endif

  cfbd_current_team_index = -1;

  if (type == CFBD_TEAM_DATA_GAMES) {
    settings.cfbd.last_light_sync_ts = time(NULL);
    cfbd_light_sync_pending = false;
  } else {
    settings.cfbd.last_full_sync_ts = time(NULL);
  }
  settings.cfbd.api_data_valid = true;
  globals_prv_save_settings();
  globals_prv_save_team_data();
  s_favorite_team_data_missing = false;
  globals_prv_update_display();

  // Trigger any deferred team walks queued during sync
  if (cfbd_pending_games_walk) {
    cfbd_pending_games_walk = false;
    start_team_walk(CFBD_TEAM_DATA_GAMES);
  } else if (cfbd_pending_records_walk) {
    cfbd_pending_records_walk = false;
    start_team_walk(CFBD_TEAM_DATA_RECORDS);
  }
}

// Construct dictionary request for full sync
static void build_request_full_sync(DictionaryIterator *iter) {
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_FULL_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
}

// Construct dictionary request for light sync
static void build_request_light_sync(DictionaryIterator *iter) {
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_LIGHT_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
}

/**********************/
/* Global Functions   */
/**********************/

// Update and render status icon layer based on API quota remaining
uint8_t api_update_status_indicator() {
  uint8_t status;

  if (s_gbitmap_layers[GBITMAP_LAYER_API]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_API]);
  }

  // Check quota levels (0 = depleted, 1 = warning, 2 = normal)
  if (api_calls_percent_used() >= 99) {
    s_gbitmap_layers[GBITMAP_LAYER_API] = gbitmap_create_with_resource(RESOURCE_ID_APIEMPTY);
    status = 0;
  }
  else if (api_calls_nearing_limit()) {
    s_gbitmap_layers[GBITMAP_LAYER_API] = gbitmap_create_with_resource(RESOURCE_ID_APILOW);
    status = 1;
  }
  else {
    if (s_gbitmap_layers[GBITMAP_LAYER_API]) {
      s_gbitmap_layers[GBITMAP_LAYER_API] = NULL;
    }
    status = 2;
  }

  if (status < 2) bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_API], s_gbitmap_layers[GBITMAP_LAYER_API]);
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), status == 2);

  return status;
}

// Request full API calendar/season sync if prerequisites pass
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

// Request light API score sync if prerequisites pass
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

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD light sync");
  #endif
  cfbd_light_sync_pending = true;
  outbox_queue_send(build_request_light_sync);
}

// Evaluate conditions to check if full sync is required
bool api_should_full_sync(void) {
  time_t now = time(NULL);

  if(settings.api_quiet && !timekeeping_is_quiet_time()){
    return false;
  }

  if (settings.api && s_favorite_team_data_missing) {
    return true;
  }

  // Never synced, or more than 24 hours since last full sync
  if (settings.api && (!settings.cfbd.api_data_valid ||
                       (now - settings.cfbd.last_full_sync_ts >= 86400))) {
    return true;
  }

  // Check if we've crossed into next season to force sync
  time_t next_game_ts = settings.cfbd.next_season_first_game_ts;
  if (next_game_ts > 0 && now >= next_game_ts && !settings.cfbd.api_data_valid) {
    return true;
  }

  return false;
}

// Evaluate conditions to check if light sync is required
bool api_should_light_sync(void) {
  if(settings.api_quiet && !timekeeping_is_quiet_time()){
    return false;
  }
  
  if (cfbd_light_sync_pending) {
    return false;
  }

  if (settings.api && s_favorite_team_data_missing) {
    return true;
  }

  time_t now = time(NULL);

  if (settings.api && (!settings.cfbd.api_data_valid || (now - settings.cfbd.last_light_sync_ts >= (settings.scoreUpdate * 60)))) {
    return true;
  }
  return false;
}

// Main incoming AppMessage dictionary parser for API data
void api_cfbd_callback(DictionaryIterator *iterator, void *context) {
  // Full sync response: calendar data (year, next season kickoff)
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
    globals_prv_save_settings();
    return;
  }

  // Games dataset ready, start going through team data
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

  // Records dataset ready, start going through team standings/rankings
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

  // Incoming data payload for a specific team index
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
    Tuple *score_tuple         = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_SCORE);
    Tuple *vs_score_tuple      = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_VS_SCORE);
    Tuple *gametime_tuple      = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_GAMETIME);
    Tuple *completed_tuple     = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_COMPLETED);
    #ifndef PBL_PLATFORM_APLITE
    Tuple *rank_tuple      = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_RANK);
    Tuple *wins_tuple      = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_WINS);
    Tuple *ps_games_tuple  = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_GAMES);
    Tuple *ps_wins_tuple   = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_WINS);
    Tuple *ps_losses_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_TEAM_PS_LOSSES);
    #endif

    if (team_opponent_tuple) {
      const char *opponent_name = team_opponent_tuple->value->cstring;
      if (opponent_name && opponent_name[0]) {
        const Team *opp = teams_find_by_name(opponent_name);
        info->vs_id = opp ? (int16_t)(opp - TEAMS) : -1;
      } else {
        // Bye week - no opponent active
        info->vs_id = -1;
      }
    }

    if (score_tuple)     info->score     = (uint16_t)score_tuple->value->int32;
    if (vs_score_tuple)  info->vs_score  = (uint16_t)vs_score_tuple->value->int32;
    if (gametime_tuple)  info->gametime  = (uint32_t)gametime_tuple->value->int32;
    if (completed_tuple) info->completed = (completed_tuple->value->int32 != 0);
    #ifndef PBL_PLATFORM_APLITE
    if (rank_tuple)      info->ranking          = (uint16_t)rank_tuple->value->int32;
    if (wins_tuple)      info->wins             = (uint16_t)wins_tuple->value->int32;
    if (ps_games_tuple)  info->postseasonGames  = (uint16_t)ps_games_tuple->value->int32;
    if (ps_wins_tuple)   info->postseasonWins   = (uint16_t)ps_wins_tuple->value->int32;
    if (ps_losses_tuple) info->postseasonLosses = (uint16_t)ps_losses_tuple->value->int32;
    #endif

    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "CFBD team %d (%s) type %d updated: vsd=%d score=%d-%d rank=%d wins=%d",
            team_index, info->name, cfbd_current_sync_type, info->vs_id, info->score, info->vs_score,
            info->ranking, info->wins);
    #endif

    // Advance to next team or finish walk sequence
    uint16_t next_index = team_index + 1;
    if (next_index < TEAMS_COUNT) {
      cfbd_current_team_index = next_index;
      request_team_data();
    } else {
      cfbd_team_walk_complete(cfbd_current_sync_type);
    }
  }
}

// Fast formatting helper to format integers < 100 as 2-character ASCII strings
void api_format_2digits(char *buf, int val) {
  buf[0] = '0' + ((val / 10) % 10);
  buf[1] = '0' + (val % 10);
}

// Format team shortnames and score strings for UI text buffers
void api_score_display() {
  if (TEAMS[settings.FavoriteTeam].vs_id == -1) {
    snprintf(s_day_text, sizeof(s_home_text), "BYE");
    snprintf(s_hour_text, sizeof(s_away_text), "WEEK");
    memcpy(s_score_text, "00|00", 6);
  } else {
    strncpy(s_home_text, TEAMS[settings.FavoriteTeam].shortname, sizeof(s_home_text) - 1);
    strncpy(s_away_text, TEAMS[TEAMS[settings.FavoriteTeam].vs_id].shortname, sizeof(s_away_text) - 1);

    // Clamp score display values to 2 digits max
    int score1 = TEAMS[settings.FavoriteTeam].score;
    int score2 = TEAMS[settings.FavoriteTeam].vs_score;
    if (score1 > MAX_DISPLAYABLE_SCORE) score1 = MAX_DISPLAYABLE_SCORE;
    if (score2 > MAX_DISPLAYABLE_SCORE) score2 = MAX_DISPLAYABLE_SCORE;

    // Fast string construction for "XX|YY"
    api_format_2digits(&s_score_text[0], score1);
    s_score_text[2] = ' ';
    api_format_2digits(&s_score_text[3], score2);
    s_score_text[5] = '\0';
  }
}

// Initialize and setup UI resources for status icons, rankings, and trophies
void api_icon_draw(Layer *window_layer, GRect bounds){
  s_gbitmap_layers[GBITMAP_LAYER_API] = NULL;

  #if PBL_DISPLAY_HEIGHT > 180
  s_bitmap_layers[BITMAP_LAYER_API] = drawing_bitmap_set((bounds.size.w * HOR_2) / 1000 - (ICON_BUMP + 19), (bounds.size.h * VERT_2) / 1000 + 3, 8, 14, NULL, window_layer);
  #else
  s_bitmap_layers[BITMAP_LAYER_API] = drawing_bitmap_set((bounds.size.w * HOR_2) / 1000 - (ICON_BUMP + 10), (bounds.size.h * VERT_2) / 1000 + 3, 4, 7, NULL, window_layer);
  #endif

  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_API]), true);

  // Create ranking display layers for non-Aplite targets
  #ifndef PBL_PLATFORM_APLITE
  GRect logo_bounds = layer_get_bounds(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  
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

  // Setup win and postseason trophy graphics
  s_gbitmap_layers[GBITMAP_LAYER_WIN] = gbitmap_create_with_resource(RESOURCE_ID_WIN);
  s_gbitmap_layers[GBITMAP_LAYER_TROPHY] = NULL;

  #if PBL_DISPLAY_HEIGHT > 180
  uint8_t winW = 22, winH = 28, trophWH = 24;
  #else
  uint8_t winW = 11, winH = 14, trophWH = 12;
  #endif
  
  #ifdef PBL_ROUND
  s_bitmap_layers[BITMAP_LAYER_WIN] = drawing_bitmap_set(logo_bounds.size.w - (trophWH * 2) - (winW + 5), logo_bounds.size.h - (trophWH + 5), winW, winH, s_gbitmap_layers[GBITMAP_LAYER_WIN], bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  s_bitmap_layers[BITMAP_LAYER_TROPHY] = drawing_bitmap_set(logo_bounds.size.w - (trophWH * 2), logo_bounds.size.h - (trophWH + 5) + (winH - trophWH), trophWH, trophWH, NULL, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  #else
  s_bitmap_layers[BITMAP_LAYER_WIN] = drawing_bitmap_set(logo_bounds.size.w - (winW * 2), 0, winW, winH, s_gbitmap_layers[GBITMAP_LAYER_WIN], bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  s_bitmap_layers[BITMAP_LAYER_TROPHY] = drawing_bitmap_set(logo_bounds.size.w - (trophWH * 2), logo_bounds.size.h - (trophWH + 5), trophWH, trophWH, NULL, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  #endif
  
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_WIN]), true);
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_TROPHY]), true);
  #endif
}