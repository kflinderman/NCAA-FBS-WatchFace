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
  int16_t vs_id; // index into TEAMS[]; -1 means no opponent (bye week, or opponent not in TEAMS[])
  uint8_t score;
  uint8_t vs_score;
  #ifndef PBL_PLATFORM_APLITE
  uint8_t ranking;
  uint8_t wins;
  uint8_t postseasonGames;
  uint8_t postseasonWins;
  uint8_t postseasonLosses;
  #endif
  bool completed;
  unsigned long gametime;
  const char *name;
  const char *shortname;
} Team;

#define TEAM_DATA_KEY 2
#define MAX_CACHED_FAVORITE_TEAMS 5
#define TEAM_CACHE_EMPTY_SLOT 0xFF

typedef struct {
  uint8_t team_index; // TEAM_CACHE_EMPTY_SLOT if this slot is unused
  int16_t vs_id;
  uint8_t score;
  uint8_t vs_score;
  #ifndef PBL_PLATFORM_APLITE
  uint8_t ranking;
  uint8_t wins;
  uint8_t postseasonGames;
  uint8_t postseasonWins;
  uint8_t postseasonLosses;
  #endif
  bool completed;
  unsigned long gametime;
} PersistedTeamData;

typedef struct {
  PersistedTeamData slots[MAX_CACHED_FAVORITE_TEAMS];
} PersistedTeamCache;
	
typedef struct {
  uint32_t next_season_first_game_ts;
  uint16_t current_season_year;
  uint32_t last_full_sync_ts;
  uint32_t last_light_sync_ts;
  uint16_t api_calls_this_month;
  uint16_t api_calls_monthly_limit;
  bool api_data_valid;
} CFBDState;

// DECLARE the variables here
extern Team TEAMS[];
extern const size_t TEAMS_COUNT;
extern CFBDState cfbd_state;