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

// DECLARE the variables here
extern const Team TEAMS[];
extern const size_t TEAMS_COUNT;
