#pragma once
#include <pebble.h>


void update_time(void);
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void timer_callback(void *data);
void timeDate_draw(Layer *window_layer, GRect bounds);
bool timekeeping_countdown(void);
bool timekeeping_is_quiet_time(void);