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

// DECLARE the variables here
extern const Team TEAMS[];
extern const size_t TEAMS_COUNT;
