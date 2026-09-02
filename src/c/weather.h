#ifndef PBL_PLATFORM_APLITE

#pragma once
#include <pebble.h>

typedef struct {
  uint32_t black;
  uint32_t white;
} weatherIcons;

void weather_temp_update(void);
void weather_conditions_update(void);
void weather_update(void);
void weather_callback(DictionaryIterator *iterator, void *context);
void weather_draw(Layer *window_layer, GRect bounds);
#endif