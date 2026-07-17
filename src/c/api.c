/*
GET /calendar current year
if within any dates
GET /games current week
else
GET /calendar last year
if within dates
GET /games current week
else
GET /games postseason week 1


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


Do I make an API indicator? Like the battery level, but of # of calls in a month? Not sure how I track this
*/
// src/c/api_cfbd.c
#include <pebble.h>
#include "api.h"
#include "globals.h"

#define CFBD_JSON_MAX 2048
static char cfbd_games_json[CFBD_JSON_MAX] = {0};
static char cfbd_records_json[CFBD_JSON_MAX] = {0};
static char cfbd_rankings_json[CFBD_JSON_MAX] = {0};

static int cfbd_json_pos_games = 0;
static int cfbd_json_pos_records = 0;
static int cfbd_json_pos_rankings = 0;

static int cfbd_total_chunks_games = 0;
static int cfbd_total_chunks_records = 0;
static int cfbd_total_chunks_rankings = 0;

static int cfbd_chunks_received_games = 0;
static int cfbd_chunks_received_records = 0;
static int cfbd_chunks_received_rankings = 0;


/**
 * Debug helper: log a long string in safe-sized pieces, since APP_LOG
 * truncates long messages once you factor in its own file/line/level
 * prefix eating into the line budget.
 */
static void log_json_chunks(const char *label, const char *json) {
  int len = strlen(json);
  int chunk_size = 100; // conservative given APP_LOG's own prefix overhead
  int chunk_index = 0;

  APP_LOG(APP_LOG_LEVEL_INFO, "%s: %d bytes total", label, len);

  for (int i = 0; i < len; i += chunk_size) {
    char chunk_buf[101];
    int remaining = len - i;
    int this_chunk = remaining < chunk_size ? remaining : chunk_size;
    memcpy(chunk_buf, json + i, this_chunk);
    chunk_buf[this_chunk] = '\0';
    APP_LOG(APP_LOG_LEVEL_INFO, "%s[%d]: %s", label, chunk_index, chunk_buf);
    chunk_index++;
  }
}

void api_request_cfbd_full_sync(void) {
  if (!settings.api || settings.api_key[0] == '\0') {
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD full sync skipped: API disabled or no key");
    return;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD full sync");
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_FULL_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
  app_message_outbox_send();
}

void api_request_cfbd_light_sync(void) {
  if (!settings.api || settings.api_key[0] == '\0') {
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: API disabled or no key");
    return;
  }

  // Need to know current year/week from previous sync
  if (!settings.cfbd.api_data_valid) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "CFBD light sync skipped: no prior data");
    return;
  }

  // Calculate current week from current_season_year
  // (This would require maintaining week state; simplified here)
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Requesting CFBD light sync");
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_CFBD_LIGHT_SYNC, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
  dict_write_uint16(iter, MESSAGE_KEY_cfbd_year, settings.cfbd.current_season_year);
  // Note: week would need to be calculated; this is a placeholder
  dict_write_uint8(iter, MESSAGE_KEY_cfbd_week, 1);
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
  // Metadata
  Tuple *metadata_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_TOTAL_CHUNKS);
  if (metadata_tuple) {
    cfbd_total_chunks_games = metadata_tuple->value->uint16;
    
    Tuple *records_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_RECORDS_TOTAL_CHUNKS);
    Tuple *rankings_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_RANKINGS_TOTAL_CHUNKS);
    
    if (records_tuple) cfbd_total_chunks_records = records_tuple->value->uint16;
    if (rankings_tuple) cfbd_total_chunks_rankings = rankings_tuple->value->uint16;
    
    // Reset all buffers
    cfbd_chunks_received_games = 0;
    cfbd_chunks_received_records = 0;
    cfbd_chunks_received_rankings = 0;
    cfbd_json_pos_games = 0;
    cfbd_json_pos_records = 0;
    cfbd_json_pos_rankings = 0;
    
    memset(cfbd_games_json, 0, CFBD_JSON_MAX);
    memset(cfbd_records_json, 0, CFBD_JSON_MAX);
    memset(cfbd_rankings_json, 0, CFBD_JSON_MAX);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD metadata: %d game chunks, %d record chunks, %d ranking chunks",
      cfbd_total_chunks_games, cfbd_total_chunks_records, cfbd_total_chunks_rankings);
    return;
  }

  // Game chunks
  Tuple *game_index = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_CHUNK_INDEX);
  Tuple *game_data = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_CHUNK_DATA);
  
  if (game_index && game_data) {
    const char *chunk = game_data->value->cstring;
    int len = strlen(chunk);
    if (cfbd_json_pos_games + len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_games_json, chunk);
      cfbd_json_pos_games += len;
      cfbd_chunks_received_games++;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Game chunk %d/%d", cfbd_chunks_received_games, cfbd_total_chunks_games);
    }
  }

  // Record chunks
  Tuple *record_index = dict_find(iterator, MESSAGE_KEY_CFBD_RECORDS_CHUNK_INDEX);
  Tuple *record_data = dict_find(iterator, MESSAGE_KEY_CFBD_RECORDS_CHUNK_DATA);
  
  if (record_index && record_data) {
    const char *chunk = record_data->value->cstring;
    int len = strlen(chunk);
    if (cfbd_json_pos_records + len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_records_json, chunk);
      cfbd_json_pos_records += len;
      cfbd_chunks_received_records++;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Record chunk %d/%d", cfbd_chunks_received_records, cfbd_total_chunks_records);
    }
  }

  // Ranking chunks
  Tuple *ranking_index = dict_find(iterator, MESSAGE_KEY_CFBD_RANKINGS_CHUNK_INDEX);
  Tuple *ranking_data = dict_find(iterator, MESSAGE_KEY_CFBD_RANKINGS_CHUNK_DATA);
  
  if (ranking_index && ranking_data) {
    const char *chunk = ranking_data->value->cstring;
    int len = strlen(chunk);
    if (cfbd_json_pos_rankings + len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_rankings_json, chunk);
      cfbd_json_pos_rankings += len;
      cfbd_chunks_received_rankings++;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Ranking chunk %d/%d", cfbd_chunks_received_rankings, cfbd_total_chunks_rankings);
    }
  }

  // Check if all three types are complete
  bool all_games_received = (cfbd_total_chunks_games == 0) || (cfbd_chunks_received_games >= cfbd_total_chunks_games);
  bool all_records_received = (cfbd_total_chunks_records == 0) || (cfbd_chunks_received_records >= cfbd_total_chunks_records);
  bool all_rankings_received = (cfbd_total_chunks_rankings == 0) || (cfbd_chunks_received_rankings >= cfbd_total_chunks_rankings);

  if (all_games_received && all_records_received && all_rankings_received) {
    APP_LOG(APP_LOG_LEVEL_INFO, "All CFBD data received!");
    
    //log_json_chunks("games", cfbd_games_json);
    //log_json_chunks("records", cfbd_records_json);
    //log_json_chunks("rankings", cfbd_rankings_json);
    
    // TODO: Parse all three JSON buffers
    // Parse cfbd_games_json
    // Parse cfbd_records_json
    // Parse cfbd_rankings_json
    // Populate api_info array
    
    settings.cfbd.last_full_sync_ts = time(NULL);
    settings.cfbd.api_data_valid = true;
    settings.cfbd.api_calls_this_month += 3;
    globals_prv_save_settings();
    globals_prv_update_display();
  }
}
