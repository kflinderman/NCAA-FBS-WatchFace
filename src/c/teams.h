// teams.h
#pragma once

#include <pebble.h>

// Struct to represent a team
typedef struct {
  uint32_t logo_res_id;
  uint8_t color;
  uint8_t icon_color;
  uint8_t rival;
  const char *name;
} Team;

typedef struct {
  const char *name;
  uint16_t id;
  int16_t vs_id; // index into TEAMS[]; -1 means no opponent (bye week, or opponent not in TEAMS[])
  uint16_t score;
  uint16_t vs_score;
  uint32_t gametime;
  uint16_t ranking;
  uint16_t wins;
  uint16_t postseasonGames;
  uint16_t postseasonWins;
  uint16_t postseasonLosses;
} API_Info;
	
typedef struct {
  uint32_t next_season_first_game_ts;
  uint16_t current_season_year;
  uint32_t last_full_sync_ts;
  uint8_t api_calls_this_month;
  bool api_data_valid;
} CFBDState;

// DECLARE the variables here
extern const Team TEAMS[];
extern const size_t TEAMS_COUNT;
extern API_Info API_DATA[];
extern const size_t API_DATA_COUNT;
extern CFBDState cfbd_state;