// teams.h
#pragma once

#include <pebble.h>

// Struct to represent a team
typedef struct {
  uint32_t logo_res_id;
  uint8_t color;
  #if defined(PBL_COLOR)
  uint8_t icon_color;
  #endif
  uint8_t rival;
  const char *name;
  const char *shortname;
} Team;

typedef struct {
  int16_t vs_id; // index into TEAMS[]; -1 means no opponent (bye week, or opponent not in TEAMS[])
  uint8_t score;
  uint8_t vs_score;
  uint8_t ranking;
  uint8_t wins;
  uint8_t postseasonGames;
  uint8_t postseasonWins;
  uint8_t postseasonLosses;
  bool completed;
  unsigned long gametime;
  const char *name;
} API_Info;
	
typedef struct {
  uint32_t next_season_first_game_ts;
  uint16_t current_season_year;
  uint32_t last_full_sync_ts; // full sync: calendar + records + rankings
  uint32_t last_light_sync_ts; // light sync: games only - own cadence, no longer tied to last_full_sync_ts
  // Both fields are set directly from what JS reports (see
  // CFBD_API_CALLS_USED/CFBD_API_CALLS_LIMIT in api_cfbd_callback) - JS is
  // the source of truth since it's the one actually making HTTP calls,
  // and it tracks/corrects its own count against CFBD's GET /info. The
  // watch just mirrors whatever JS last reported, on every full and light
  // sync. uint16_t (not uint8_t) because the free tier alone is ~1000
  // calls/month - uint8_t would silently wrap well before that.
  uint16_t api_calls_this_month;
  uint16_t api_calls_monthly_limit;
  bool api_data_valid;
} CFBDState;

// DECLARE the variables here
extern const Team TEAMS[];
extern const size_t TEAMS_COUNT;
extern API_Info API_DATA[];
extern const size_t API_DATA_COUNT;
extern CFBDState cfbd_state;
