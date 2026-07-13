#include "sensors.h"
#include "globals.h"
#include "animation.h"
#include "drawing.h"
#include "timekeeping.h"

/*********************/
/* Accelerometer     */
/*********************/
void sensor_accel_data_handler(AccelData *data, uint32_t num_samples) {
  if (num_samples == 0 || data == NULL) return;
  // Use the last sample in the batch for current reading
  int16_t curr_y = data[num_samples - 1].y;
  int16_t delta = curr_y - s_prev_y;

  if (abs(delta) > settings.animationSensitivity &&
      !s_animation &&
      settings.animationSensitivity != 0 &&
      (settings.animationsBatt == 0 ||
        (settings.animationsBatt == 1 && s_batt_history < 1) ||
        (settings.animationsBatt == 2 && s_batt_history < 2) ||
        (settings.animationsBatt == 3 && s_battery_state.charge_percent > settings.animationsCustom)
      ) &&
      (!settings.quietTimeBool ||
        (current_time_integer <= settings.quietTimeStart && current_time_integer >= settings.quietTimeEnd)
      )
     ) {
      // Detected sudden Y movement and play animation
      s_animation = true;
      if(settings.animationDelay){
        app_timer_register(1000, timer_callback, NULL);
      }
      else{
        animation_beat_team_layer();
      }
    }
  s_prev_y = curr_y;
}

/******************/
/* Bluetooth      */
/******************/
void sensor_connection_handler(bool connected) {
  s_bt_connected = connected;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Bluetooth: %d History: %d", s_bt_connected, s_bt_history);

  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), connected);

  // optional: give feedback when connection state changes
  if (connected && !s_bt_history) {
    // connected
    switch (settings.ReconnectVibration) {
      case 0: break;
      case 1: vibes_short_pulse(); break;
      case 2: vibes_long_pulse(); break;
      case 3: vibes_double_pulse(); break;
    }

    s_bt_history = true;
  } else if (!connected && s_bt_history) {
    // disconnected
    switch (settings.DisconnectVibration) {
      case 0: break;
      case 1: vibes_short_pulse(); break;
      case 2: vibes_long_pulse(); break;
      case 3: vibes_double_pulse(); break;
    }
    s_bt_history = false;
  }
}

void sensor_bluetooth_draw(Layer *window_layer, GRect bounds){
  // Create Bluetooth GBitmap from resource
  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);

  #if PBL_DISPLAY_HEIGHT > 180
    //182
    s_bt_layer = drawing_bitmap_set(bounds.size.w * hor_2 - icon_bump, bounds.size.h * vert_2, 10, 14, s_bt_bitmap, window_layer);
  #else
    s_bt_layer = drawing_bitmap_set(bounds.size.w * hor_2 - icon_bump, bounds.size.h * vert_2 + 3, 5, 7, s_bt_bitmap, window_layer);
  #endif
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), s_bt_connected);
}

/****************/
/* Battery      */
/****************/
void sensor_battery_handler(BatteryChargeState state) {
  s_battery_state = state;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery: %d History: %d", state.charge_percent, s_batt_history);

  // optional: vibrate on low battery threshold or update UI
  if (!state.is_charging && state.charge_percent <= settings.LowBatteryPercent && state.charge_percent > settings.EmptyBatteryPercent) {
    // warn briefly
    if (s_batt_history == 0) {
      switch (settings.LowBatteryVibration) {
        case 0: break;
        case 1: vibes_short_pulse(); break;
        case 2: vibes_long_pulse(); break;
        case 3: vibes_double_pulse(); break;
      }
      s_batt_history = 1;
    }
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_low_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
  } else if (!state.is_charging && state.charge_percent <= settings.EmptyBatteryPercent) {
    if (s_batt_history == 1) {
      switch (settings.EmptyBatteryVibration) {
        case 0: break;
        case 1: vibes_short_pulse(); break;
        case 2: vibes_long_pulse(); break;
        case 3: vibes_double_pulse(); break;
      }
      bitmap_layer_set_bitmap(s_batt_layer, s_batt_empty_bitmap);
      layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
      s_batt_history = 2;
    }
  } else if (state.is_charging) {
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_crg_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
    s_batt_history = 0;
  } else {
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), true);
    s_batt_history = 0;
  }
}

void sensor_battery_draw(Layer *window_layer, GRect bounds){
  // Create Battery GBitmap from resource
  s_batt_low_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOWBATT);
  s_batt_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EMPTYBATT);
  s_batt_crg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBATT);
  
  #if PBL_DISPLAY_HEIGHT > 180
    //168
    s_batt_layer = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump + 14), bounds.size.h * vert_2, 8, 14, s_batt_low_bitmap, window_layer);
  #else
    s_batt_layer = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump + 7), bounds.size.h * vert_2 + 3, 4, 7, s_batt_low_bitmap, window_layer);
  #endif
}