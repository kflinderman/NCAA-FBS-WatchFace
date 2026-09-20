#pragma once
#include <pebble.h>

void display_setupBag(GColor bagColor);
void display_beat_textbox(Layer *window_layer, GRect bounds);
void display_main_time_layer(Layer *window_layer, GRect bounds);
#ifndef PBL_PLATFORM_APLITE
void display_beatteam(Layer *window_layer, GRect bounds);
#endif