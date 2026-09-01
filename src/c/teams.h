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

// Number of distinct FavoriteTeam selections whose data is cached on
// flash at once. Lets someone who roots for a handful of teams switch
// between them without triggering a fresh CFBD API call every time -
// only a team outside this set needs a new sync. Sized at 5 as a
// reasonable "teams someone actually swaps between" count; each cached
// entry is well under 20 bytes, so this whole cache is nowhere near
// Pebble's 256-byte per-key persist limit even at a much larger size.
#define MAX_CACHED_FAVORITE_TEAMS 5

// Sentinel marking a cache slot as unused. Safe against any real team
// index since TEAMS_COUNT (154) never reaches 255.
#define TEAM_CACHE_EMPTY_SLOT 0xFF

// One cached team's dynamic fields, persisted separately from the full
// TEAMS[] roster. TEAMS[] itself is RAM-only and gets rebuilt from
// scratch (back to its compiled-in defaults) every time the app process
// restarts - only entries in the cache below survive across launches.
// For any given team, only settings.FavoriteTeam's entry is ever read
// for display (see api.c/globals.c/timekeeping.c/health.c/weather.c) -
// every other TEAMS[] field read at draw time (name/color/logo_res_id/
// icon_color) is a static compiled-in constant that never needs
// persisting, so this is the only data that actually needs to survive
// a restart or a switch back to a previously-tracked team.
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

// The full persisted cache: slots[0] is always the most-recently-used
// team, slots[MAX_CACHED_FAVORITE_TEAMS - 1] the least-recently-used -
// so evicting on a cache-full new team is just dropping the last slot,
// and "using" a team (syncing it, or just switching back to it) is a
// move-to-front.
typedef struct {
  PersistedTeamData slots[MAX_CACHED_FAVORITE_TEAMS];
} PersistedTeamCache;

/*
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
  //const char *name;
} API_Info;
*/
	
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
extern Team TEAMS[];
extern const size_t TEAMS_COUNT;
//extern API_Info API_DATA[];
//extern const size_t API_DATA_COUNT;
extern CFBDState cfbd_state;