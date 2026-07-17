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
  uint16_t vs_id;
  uint16_t score;
  uint16_t vs_score;
  uint32_t gametime;
  uint16_t ranking;
  bool w_season;
  bool conf_champ;
  bool bowl_w;
  bool champion;
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
extern CFBDState cfbd_state;