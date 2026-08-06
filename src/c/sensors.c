#include "sensors.h"
#include "globals.h"
#include "animation.h"
#include "drawing.h"
#include "timekeeping.h"


static void sensor_trigger_vibration(uint8_t type) {
  switch (type) {
    case 1: vibes_short_pulse(); break;
    case 2: vibes_long_pulse(); break;
    case 3: vibes_double_pulse(); break;
    default: break;
  }
}

/*********************/
/* Accelerometer     */
/*********************/
void sensor_accel_data_handler(AccelData *data, uint32_t num_samples) {
  if (num_samples == 0 || data == NULL) return;
  // Use the last sample in the batch for current reading
  int16_t curr_y = data[num_samples - 1].y;
  int16_t delta = curr_y - s_prev_y;

  if (settings.animationSensitivity != 0 &&
      !s_animation &&
      abs(delta) > settings.animationSensitivity &&
      (settings.animationsBatt == 0 || 
       (settings.animationsBatt == 1 && s_batt_history < 1) ||
       (settings.animationsBatt == 2 && s_batt_history < 2) ||
       (settings.animationsBatt == 3 && s_battery_state.charge_percent > settings.animationsCustom)) &&
      (!settings.quietTimeBool || !timekeeping_is_quiet_time())
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

  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BT]), connected);

  // optional: give feedback when connection state changes
  if (connected && !s_bt_history) {
    // connected
    sensor_trigger_vibration(settings.ReconnectVibration);

    s_bt_history = true;
  } else if (!connected && s_bt_history) {
    // disconnected
    sensor_trigger_vibration(settings.DisconnectVibration);
    s_bt_history = false;
  }
}

void sensor_bluetooth_draw(Layer *window_layer, GRect bounds){
  // Create Bluetooth GBitmap from resource
  s_gbitmap_layers[GBITMAP_LAYER_BT] = gbitmap_create_with_resource(RESOURCE_ID_BT);

  #if PBL_DISPLAY_HEIGHT > 180
    //182
    s_bitmap_layers[BITMAP_LAYER_BT] = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump + 9), bounds.size.h * vert_2, 10, 14, s_gbitmap_layers[GBITMAP_LAYER_BT], window_layer);
  #else
    s_bitmap_layers[BITMAP_LAYER_BT] = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump + 5), bounds.size.h * vert_2 + 3, 5, 7, s_gbitmap_layers[GBITMAP_LAYER_BT], window_layer);
    //+ 9
  #endif
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BT]), s_bt_connected);
}

/****************/
/* Battery      */
/****************/
void sensor_battery_handler(BatteryChargeState state) {
  s_battery_state = state;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery: %d History: %d", state.charge_percent, s_batt_history);
  GBitmap *target_gbitmap = NULL;

  // Charging State
  if (state.is_charging) {
    target_gbitmap = s_gbitmap_layers[GBITMAP_LAYER_BATT_CRG];
    s_batt_history = 0;
  } 
  // Empty Battery State
  else if (state.charge_percent <= settings.EmptyBatteryPercent) {
    target_gbitmap = s_gbitmap_layers[GBITMAP_LAYER_BATT_EMPTY];
    
    // Only vibrate on new state entry (0 -> 2 or 1 -> 2)
    if (s_batt_history < 2) {
      sensor_trigger_vibration(settings.EmptyBatteryVibration);
      s_batt_history = 2;
    }
  } 
  // Low Battery State
  else if (state.charge_percent <= settings.LowBatteryPercent) {
    target_gbitmap = s_gbitmap_layers[GBITMAP_LAYER_BATT_LOW];
    
    // Only vibrate on new state entry (0 -> 1)
    if (s_batt_history < 1) {
      sensor_trigger_vibration(settings.LowBatteryVibration);
      s_batt_history = 1;
    }
  } 
  // Normal Battery State
  else {
    s_batt_history = 0;
  }

  // Single Pass UI Update
  if (target_gbitmap != NULL) {
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BATT], target_gbitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BATT]), false);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BATT]), true);
  }
}

void sensor_battery_draw(Layer *window_layer, GRect bounds){
  // Create Battery GBitmap from resource
  s_gbitmap_layers[GBITMAP_LAYER_BATT_LOW] = gbitmap_create_with_resource(RESOURCE_ID_LOWBATT);
  s_gbitmap_layers[GBITMAP_LAYER_BATT_EMPTY] = gbitmap_create_with_resource(RESOURCE_ID_EMPTYBATT);
  s_gbitmap_layers[GBITMAP_LAYER_BATT_CRG] = gbitmap_create_with_resource(RESOURCE_ID_FULLBATT);
  
  #if PBL_DISPLAY_HEIGHT > 180
    //168
    s_bitmap_layers[BITMAP_LAYER_BATT] = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump - 4), bounds.size.h * vert_2, 8, 14, s_gbitmap_layers[GBITMAP_LAYER_BATT_LOW], window_layer);
  #else
    s_bitmap_layers[BITMAP_LAYER_BATT] = drawing_bitmap_set(bounds.size.w * hor_2 - (icon_bump - 2), bounds.size.h * vert_2 + 3, 4, 7, s_gbitmap_layers[GBITMAP_LAYER_BATT_LOW], window_layer);
    //+ 2
  #endif
}
