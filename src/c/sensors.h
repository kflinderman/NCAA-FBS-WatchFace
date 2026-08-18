#pragma once
#include <pebble.h>

#ifndef PBL_PLATFORM_APLITE
void sensor_accel_data_handler(AccelData *data, uint32_t num_samples);
#endif
void sensor_connection_handler(bool connected);
void sensor_bluetooth_draw(Layer *window_layer, GRect bounds);
void sensor_battery_handler(BatteryChargeState state);
void sensor_battery_draw(Layer *window_layer, GRect bounds);
