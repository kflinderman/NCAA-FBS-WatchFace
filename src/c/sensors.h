#pragma once
#include <pebble.h>

void accel_data_handler(AccelData *data, uint32_t num_samples);
void connection_handler(bool connected);
void bluetooth_draw(Layer *window_layer, GRect bounds);
void battery_handler(BatteryChargeState state);
void battery_draw(Layer *window_layer, GRect bounds);
