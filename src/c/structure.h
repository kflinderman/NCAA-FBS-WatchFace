// teams.h
#pragma once

#include <pebble.h>

typedef struct {
  uint16_t x1, y1, x2, y2, width;
  GColor color;
} LinePoints;

// Per-layer data for rounded-rect layers. Stores a fill color so each
// round rect can have its own color set at creation time.
typedef struct {
  GColor fill_color;
} RoundRectData;