#if defined(PBL_HEALTH)
#pragma once
#include <pebble.h>

void health_heartRateHandler(void);
void health_stepHandler(void);
void health_handler(void);
void health_draw(Layer *window_layer, GRect bounds);
#endif
