#pragma once
#include <pebble.h>

#if defined(PBL_HEALTH)
void heartRateHandler(void);
void stepHandler(void);
void health_handler(void);
void health_draw(Layer *window_layer, GRect bounds);
#endif
