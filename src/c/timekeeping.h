#pragma once
#include <pebble.h>

void update_time(void);
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void timeDate_draw(Layer *window_layer, GRect bounds);
