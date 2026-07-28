/*
Task List:
---Release---
-Weather
  - Code
-Football API Integration
  - API
  - Code
-Champ Designation
  - Icons
    - Nat Champ
    - Conf Champ
    - Winning Season
    - Bowl Win
  - Code
*/

#include <pebble.h>
#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "animation.h"
#include "sensors.h"
#include "timekeeping.h"
#include "communication.h"
#include "weather.h"
#include "display.h"

/***********************************/
/* NCAA FBS Watchface              */
/***********************************/

// Loads the main window's UI elements
static void main_window_load(Window *window) {
  // Get window information
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- DRAWING FUNCTIONS --------");
  
  s_logo_layer = drawing_bitmap_set((bounds.size.w - bitmap_size) / 2, bounds.size.h * 0.025, bitmap_size, bitmap_size, s_logo_bitmap, window_layer);
  s_bag_layerf = drawing_bitmap_set(0, 0, bitmap_size, bitmap_size, s_bag_bitmap, bitmap_layer_get_layer(s_logo_layer));

  #if defined(PBL_HEALTH)
    APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Health");
    health_draw(window_layer, bounds);
  #endif
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Weather");
  weather_draw(window_layer, bounds);

  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Team to Beat");
  display_beatteam(window_layer, bounds);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing 'BEAT' Textbox");
  display_beat_textbox(window_layer, bounds);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Bottom Box");
  display_main_time_layer(window_layer, bounds);

  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Date and Time");
  timeDate_draw(window_layer, bounds);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Bluetooth");
  sensor_bluetooth_draw(window_layer, bounds);

  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Battery");
  sensor_battery_draw(window_layer, bounds);

  APP_LOG(APP_LOG_LEVEL_INFO, "-------- INFORMATION FILL --------");
  // Apply saved settings
  APP_LOG(APP_LOG_LEVEL_INFO, "Applying Saved Settings");
  globals_prv_update_display();

  // Make sure the time and date are displayed from the start
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Time");
  update_time();

  // Apply correct layout in case Quick View is already active
  APP_LOG(APP_LOG_LEVEL_INFO, "Apply Correct Layout w/ Quick View");
  animation_prv_unobstructed_change(0, NULL);
}

// Unloads the main window's UI elements
static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_beat_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_conditions_layer);
  #if defined(PBL_HEALTH)
    text_layer_destroy(s_hr_layer);
    text_layer_destroy(s_step_layer);
  #endif

  // Destroy GBitmap
  gbitmap_destroy(s_logo_bitmap);
  gbitmap_destroy(s_beat_team_bitmap);
  gbitmap_destroy(s_bt_bitmap);
  gbitmap_destroy(s_batt_crg_bitmap);
  gbitmap_destroy(s_batt_empty_bitmap);
  gbitmap_destroy(s_batt_low_bitmap);
  if(s_bag_bitmap) {
    gbitmap_destroy(s_bag_bitmap);
    s_bag_bitmap = NULL; 
  }

  #if PBL_DISPLAY_HEIGHT > 180
    fonts_unload_custom_font(s_font);
  #endif
  fonts_unload_custom_font(s_wIcon);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_logo_layer);
  bitmap_layer_destroy(s_beat_team_layer);
  bitmap_layer_destroy(s_bt_layer);
  bitmap_layer_destroy(s_batt_layer);
  bitmap_layer_destroy(s_bag_layerf);
  bitmap_layer_destroy(s_bag_layerb);

  // Destroy Layers
  layer_destroy(rect_layer);
  layer_destroy(horizontal_line);
  layer_destroy(beat_team_layer);
  layer_destroy(rect_beat_layer);

  #ifdef PBL_RECT
    layer_destroy(vertical_line);
  #endif

  #if defined(PBL_HEALTH)
    gbitmap_destroy(s_football_bitmap);
    bitmap_layer_destroy(s_football_layer);
    drawing_multiline_layer_destroy(hr_icon);
    drawing_multiline_layer_destroy(step_ladder);
  #endif

  // Unsubscribe from TickTimerService
  tick_timer_service_unsubscribe();
}

// Initializes the app
static void init() {
  // Load settings before creating UI
  globals_prv_load_settings();

  // Create main Window element
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- SUBSCRIBE --------");
  
  // Subscribe to unobstructed area events
  APP_LOG(APP_LOG_LEVEL_INFO, "Quick View");
  animation_subscribe_unobstructed_area();
  
  // Register with TickTimerService
  APP_LOG(APP_LOG_LEVEL_INFO, "Tick Handler");
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to continuous accelerometer data for Y-movement detection
  APP_LOG(APP_LOG_LEVEL_INFO, "Accelerometer");
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  accel_data_service_subscribe(5, sensor_accel_data_handler);

  // Subscribe to bluetooth connection updates and set initial state
  APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth");
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = sensor_connection_handler
  });
  sensor_connection_handler(connection_service_peek_pebble_app_connection());

  // Subscribe to battery state changes and initialize
  APP_LOG(APP_LOG_LEVEL_INFO, "Battery Level");
  battery_state_service_subscribe(sensor_battery_handler);
  sensor_battery_handler(battery_state_service_peek());

  // Register AppMessage callbacks
  APP_LOG(APP_LOG_LEVEL_INFO, "Communication");
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 512;
  const int outbox_size = 512;
  app_message_open(inbox_size, outbox_size);
}

// Deinitializes the app
static void deinit() {
  window_destroy(s_main_window);
  accel_data_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
