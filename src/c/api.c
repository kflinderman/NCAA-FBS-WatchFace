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

// Games, records, and rankings never need to be buffered at the same time -
// the JS sends them fully sequentially (all game chunks, then all record
// chunks, then all ranking chunks), and each type is parsed and cleared as
// soon as its chunk count is reached. So one shared buffer replaces what
// used to be three separate 2048-byte buffers, saving ~4KB of .bss - which
// matters a lot on aplite's 24KB app RAM budget.
#define CFBD_JSON_MAX 2048
static char cfbd_json_buf[CFBD_JSON_MAX] = {0};
static int cfbd_json_pos = 0;

typedef enum {
  CFBD_STAGE_NONE = 0,
  CFBD_STAGE_GAMES,
  CFBD_STAGE_RECORDS,
  CFBD_STAGE_RANKINGS,
  CFBD_STAGE_DONE
} CFBDStage;
static CFBDStage cfbd_stage = CFBD_STAGE_NONE;

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

/**
 * Minimal JSON helpers
 * -----------------------------------------------------------------------
 * The Pebble SDK has no bundled JSON library, and the payloads here are
 * flat, known-shape arrays of objects (no nested arrays within an object,
 * no escaped quotes in values we care about), so a small hand-rolled
 * scanner is enough - no need for a general-purpose parser or malloc.
 */

// Find the start of the next "{...}" object after `from`. Returns NULL if
// none found. Writes the object's closing '}' position into *end.
static const char *json_next_object(const char *from, const char **end) {
  const char *start = strchr(from, '{');
  if (!start) return NULL;

  int depth = 0;
  const char *p = start;
  for (; *p; p++) {
    if (*p == '{') depth++;
    else if (*p == '}') {
      depth--;
      if (depth == 0) {
        *end = p;
        return start;
      }
    }
  }
  return NULL; // unbalanced / truncated - treat as no object
}

// Extract a string value for "key":"value" within [obj, obj_end]. Copies
// into out (size out_size), NUL-terminated, truncating if needed. Returns
// true if the key was found (even if value was empty).
static bool json_get_string(const char *obj, const char *obj_end, const char *key,
                             char *out, size_t out_size) {
  out[0] = '\0';

  char needle[48];
  snprintf(needle, sizeof(needle), "\"%s\"", key);

  const char *key_pos = strstr(obj, needle);
  if (!key_pos || key_pos > obj_end) return false;

  const char *colon = strchr(key_pos, ':');
  if (!colon || colon > obj_end) return false;

  // Skip whitespace after the colon
  const char *p = colon + 1;
  while (*p == ' ' || *p == '\t') p++;

  if (*p == 'n') {
    // null
    return true;
  }
  if (*p != '"') return false; // not a string value

  p++; // skip opening quote
  const char *val_start = p;
  while (*p && *p != '"' && p <= obj_end) p++;
  if (*p != '"') return false;

  size_t len = (size_t)(p - val_start);
  if (len >= out_size) len = out_size - 1;
  memcpy(out, val_start, len);
  out[len] = '\0';
  return true;
}

// Extract an integer value for "key":123 (or "key":null -> out_present=false)
// within [obj, obj_end]. Returns true if the key was found and had a
// numeric value; false if missing or null.
static bool json_get_int(const char *obj, const char *obj_end, const char *key, int *out) {
  *out = 0;

  char needle[48];
  snprintf(needle, sizeof(needle), "\"%s\"", key);

  const char *key_pos = strstr(obj, needle);
  if (!key_pos || key_pos > obj_end) return false;

  const char *colon = strchr(key_pos, ':');
  if (!colon || colon > obj_end) return false;

  const char *p = colon + 1;
  while (*p == ' ' || *p == '\t') p++;

  if (*p == 'n') return false; // null

  *out = atoi(p);
  return true;
}

// Find a team's slot in API_DATA[] by name (exact match). Returns NULL if
// the team isn't in the current test-bed roster.
static API_Info *api_find_team(const char *name) {
  for (size_t i = 0; i < API_DATA_COUNT; i++) {
    if (API_DATA[i].name && strcmp(API_DATA[i].name, name) == 0) {
      return &API_DATA[i];
    }
  }
  return NULL;
}

/**
 * Parse the games JSON array: [{startDate,homeTeam,homePoints,awayTeam,awayPoints}, ...]
 * For each game, if either side matches a team in API_DATA[], fill in that
 * team's opponent id/score/gametime fields.
 */
static void parse_games_json(const char *json) {
  const char *cursor = json;
  const char *obj, *obj_end;
  int games_matched = 0;

  while ((obj = json_next_object(cursor, &obj_end)) != NULL) {
    char home_team[64], away_team[64];
    int home_points = 0, away_points = 0;
    bool has_home_points = json_get_int(obj, obj_end, "homePoints", &home_points);
    bool has_away_points = json_get_int(obj, obj_end, "awayPoints", &away_points);
    json_get_string(obj, obj_end, "homeTeam", home_team, sizeof(home_team));
    json_get_string(obj, obj_end, "awayTeam", away_team, sizeof(away_team));

    API_Info *home_info = home_team[0] ? api_find_team(home_team) : NULL;
    API_Info *away_info = away_team[0] ? api_find_team(away_team) : NULL;

    if (home_info) {
      API_Info *opp = away_info;
      home_info->vs_id = opp ? opp->id : home_info->vs_id;
      home_info->score = has_home_points ? (uint16_t)home_points : home_info->score;
      home_info->vs_score = has_away_points ? (uint16_t)away_points : home_info->vs_score;
      games_matched++;
    }
    if (away_info) {
      API_Info *opp = home_info;
      away_info->vs_id = opp ? opp->id : away_info->vs_id;
      away_info->score = has_away_points ? (uint16_t)away_points : away_info->score;
      away_info->vs_score = has_home_points ? (uint16_t)home_points : away_info->vs_score;
      games_matched++;
    }

    cursor = obj_end + 1;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "parse_games_json: matched %d team-sides", games_matched);
}

/**
 * Parse the records JSON array:
 * [{team, total:{games,wins}, postseason:{games,wins,losses}}, ...]
 */
static void parse_records_json(const char *json) {
  const char *cursor = json;
  const char *obj, *obj_end;
  int records_matched = 0;

  while ((obj = json_next_object(cursor, &obj_end)) != NULL) {
    char team[64];
    json_get_string(obj, obj_end, "team", team, sizeof(team));

    API_Info *info = team[0] ? api_find_team(team) : NULL;
    if (info) {
      int wins = 0, ps_games = 0, ps_wins = 0, ps_losses = 0;
      // "wins" and "games" each appear twice (once under "total", once
      // under "postseason"). json_get_string/json_get_int always return
      // the first match in the searched range, so total.wins is read from
      // the full object here, and the postseason.* fields are read below
      // from a range starting at "postseason" so they can't match total's.
      if (json_get_int(obj, obj_end, "wins", &wins)) {
        info->wins = (uint16_t)wins;
      }

      const char *postseason_pos = strstr(obj, "\"postseason\"");
      if (postseason_pos && postseason_pos < obj_end) {
        if (json_get_int(postseason_pos, obj_end, "games", &ps_games)) {
          info->postseasonGames = (uint16_t)ps_games;
        }
        if (json_get_int(postseason_pos, obj_end, "wins", &ps_wins)) {
          info->postseasonWins = (uint16_t)ps_wins;
        }
        if (json_get_int(postseason_pos, obj_end, "losses", &ps_losses)) {
          info->postseasonLosses = (uint16_t)ps_losses;
        }
      }
      records_matched++;
    }

    cursor = obj_end + 1;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "parse_records_json: matched %d teams", records_matched);
}

/**
 * Parse the rankings JSON array: [{rank, school}, ...]
 */
static void parse_rankings_json(const char *json) {
  const char *cursor = json;
  const char *obj, *obj_end;
  int rankings_matched = 0;

  while ((obj = json_next_object(cursor, &obj_end)) != NULL) {
    char school[64];
    json_get_string(obj, obj_end, "school", school, sizeof(school));

    API_Info *info = school[0] ? api_find_team(school) : NULL;
    if (info) {
      int rank = 0;
      if (json_get_int(obj, obj_end, "rank", &rank)) {
        info->ranking = (uint16_t)rank;
      }
      rankings_matched++;
    }

    cursor = obj_end + 1;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "parse_rankings_json: matched %d teams", rankings_matched);
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
    
    // Reset stage tracking and the shared buffer. Start at whichever stage
    // actually has chunks coming (a type with 0 chunks is skipped entirely
    // rather than waiting on a chunk count that will never arrive).
    cfbd_chunks_received_games = 0;
    cfbd_chunks_received_records = 0;
    cfbd_chunks_received_rankings = 0;
    cfbd_json_pos = 0;
    memset(cfbd_json_buf, 0, CFBD_JSON_MAX);

    if (cfbd_total_chunks_games > 0) {
      cfbd_stage = CFBD_STAGE_GAMES;
    } else if (cfbd_total_chunks_records > 0) {
      cfbd_stage = CFBD_STAGE_RECORDS;
    } else if (cfbd_total_chunks_rankings > 0) {
      cfbd_stage = CFBD_STAGE_RANKINGS;
    } else {
      cfbd_stage = CFBD_STAGE_DONE;
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "CFBD metadata: %d game chunks, %d record chunks, %d ranking chunks",
      cfbd_total_chunks_games, cfbd_total_chunks_records, cfbd_total_chunks_rankings);
    return;
  }

  // Game chunks - only meaningful while we're in the games stage
  Tuple *game_index = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_CHUNK_INDEX);
  Tuple *game_data = dict_find(iterator, MESSAGE_KEY_CFBD_GAMES_CHUNK_DATA);
  
  if (cfbd_stage == CFBD_STAGE_GAMES && game_index && game_data) {
    const char *chunk = game_data->value->cstring;
    int len = strlen(chunk);
    if (cfbd_json_pos + len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_json_buf, chunk);
      cfbd_json_pos += len;
      cfbd_chunks_received_games++;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Game chunk %d/%d", cfbd_chunks_received_games, cfbd_total_chunks_games);
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "CFBD games buffer overflow - truncating");
    }

    if (cfbd_chunks_received_games >= cfbd_total_chunks_games) {
      parse_games_json(cfbd_json_buf);
      memset(cfbd_json_buf, 0, CFBD_JSON_MAX);
      cfbd_json_pos = 0;
      cfbd_stage = (cfbd_total_chunks_records > 0) ? CFBD_STAGE_RECORDS
                 : (cfbd_total_chunks_rankings > 0) ? CFBD_STAGE_RANKINGS
                 : CFBD_STAGE_DONE;
    }
  }

  // Record chunks - only meaningful while we're in the records stage
  Tuple *record_index = dict_find(iterator, MESSAGE_KEY_CFBD_RECORDS_CHUNK_INDEX);
  Tuple *record_data = dict_find(iterator, MESSAGE_KEY_CFBD_RECORDS_CHUNK_DATA);
  
  if (cfbd_stage == CFBD_STAGE_RECORDS && record_index && record_data) {
    const char *chunk = record_data->value->cstring;
    int len = strlen(chunk);
    if (cfbd_json_pos + len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_json_buf, chunk);
      cfbd_json_pos += len;
      cfbd_chunks_received_records++;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Record chunk %d/%d", cfbd_chunks_received_records, cfbd_total_chunks_records);
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "CFBD records buffer overflow - truncating");
    }

    if (cfbd_chunks_received_records >= cfbd_total_chunks_records) {
      parse_records_json(cfbd_json_buf);
      memset(cfbd_json_buf, 0, CFBD_JSON_MAX);
      cfbd_json_pos = 0;
      cfbd_stage = (cfbd_total_chunks_rankings > 0) ? CFBD_STAGE_RANKINGS : CFBD_STAGE_DONE;
    }
  }

  // Ranking chunks - only meaningful while we're in the rankings stage
  Tuple *ranking_index = dict_find(iterator, MESSAGE_KEY_CFBD_RANKINGS_CHUNK_INDEX);
  Tuple *ranking_data = dict_find(iterator, MESSAGE_KEY_CFBD_RANKINGS_CHUNK_DATA);
  
  if (cfbd_stage == CFBD_STAGE_RANKINGS && ranking_index && ranking_data) {
    const char *chunk = ranking_data->value->cstring;
    int len = strlen(chunk);
    if (cfbd_json_pos + len < CFBD_JSON_MAX - 1) {
      strcat(cfbd_json_buf, chunk);
      cfbd_json_pos += len;
      cfbd_chunks_received_rankings++;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Ranking chunk %d/%d", cfbd_chunks_received_rankings, cfbd_total_chunks_rankings);
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "CFBD rankings buffer overflow - truncating");
    }

    if (cfbd_chunks_received_rankings >= cfbd_total_chunks_rankings) {
      parse_rankings_json(cfbd_json_buf);
      memset(cfbd_json_buf, 0, CFBD_JSON_MAX);
      cfbd_json_pos = 0;
      cfbd_stage = CFBD_STAGE_DONE;
    }
  }

  if (cfbd_stage == CFBD_STAGE_DONE) {
    APP_LOG(APP_LOG_LEVEL_INFO, "All CFBD data received and parsed!");

    settings.cfbd.last_full_sync_ts = time(NULL);
    settings.cfbd.api_data_valid = true;
    settings.cfbd.api_calls_this_month += 3;
    globals_prv_save_settings();
    globals_prv_update_display();

    cfbd_stage = CFBD_STAGE_NONE; // avoid re-firing on stray follow-up messages
  }
}