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

// In-memory buffer for JSON reassembly (games are chunked)
#define CFBD_JSON_MAX 8192
static char cfbd_games_json[CFBD_JSON_MAX] = {0};
static int cfbd_json_pos = 0;
static int cfbd_total_chunks = 0;
static int cfbd_chunks_received = 0;

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
  // Metadata chunk
  APP_LOG(APP_LOG_LEVEL_INFO, "Checking for CFBD Chunks");
  Tuple *metadata_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_TOTAL_CHUNKS);
  if (metadata_tuple) {
    cfbd_total_chunks = metadata_tuple->value->uint16;
    cfbd_chunks_received = 0;
    cfbd_json_pos = 0;
    memset(cfbd_games_json, 0, CFBD_JSON_MAX);
    
    Tuple *year_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_YEAR);
    Tuple *ts_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_NEXT_SEASON_TS);
    
    if (year_tuple) {
      settings.cfbd.current_season_year = year_tuple->value->uint16;
    }
    if (ts_tuple) {
      settings.cfbd.next_season_first_game_ts = ts_tuple->value->uint32;
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD metadata: %d chunks, year %d",
      cfbd_total_chunks, settings.cfbd.current_season_year);
    return;
  }

  // Data chunk
  Tuple *chunk_index_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_CHUNK_INDEX);
  Tuple *chunk_data_tuple = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_CHUNK_DATA);
  
  if (chunk_index_tuple && chunk_data_tuple) {
    const char *chunk_data = chunk_data_tuple->value->cstring;
    int chunk_len = strlen(chunk_data);
    
    // Append to buffer
    if (cfbd_json_pos + chunk_len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_games_json, chunk_data);
      cfbd_json_pos += chunk_len;
      cfbd_chunks_received++;
      
      APP_LOG(APP_LOG_LEVEL_DEBUG, "CFBD chunk %d/%d, buffer now %d bytes",
        chunk_index_tuple->value->uint8, cfbd_total_chunks, cfbd_json_pos);
    }

    // All chunks arrived, parse and apply
    if (cfbd_chunks_received >= cfbd_total_chunks) {
      APP_LOG(APP_LOG_LEVEL_INFO, "All CFBD chunks received, parsing...");
      // TODO: Parse cfbd_games_json and populate team_cfbd[] array
      
      settings.cfbd.last_full_sync_ts = time(NULL);
      settings.cfbd.api_data_valid = true;
      settings.cfbd.api_calls_this_month += 3;  // Rough estimate: calendar, games, records, rankings
      
      globals_prv_save_settings();
      globals_prv_update_display();
    }
  }
}