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
#include "api.h"

/***********************************/
/* NCAA FBS Watchface              */
/***********************************/

void destroy_layers_by_kind(void **layers, LayerKind kind, size_t count) {
  if (!layers) return;
  for (size_t i = 0; i < count; ++i) {
    Layer *l = layers[i];
    if (!l) continue;

    switch (kind) {
      case LAYER_KIND_TEXT:
      text_layer_destroy((TextLayer*)l);
      break;
      case LAYER_KIND_BITMAP:
      bitmap_layer_destroy((BitmapLayer*)l);
      break;
      case LAYER_KIND_GBITMAP:
      gbitmap_destroy((GBitmap*)l);
      break;
      case LAYER_KIND_GENERIC:
      default:
      layer_destroy(l);
      break;
    }
    layers[i] = NULL;
  }
}

// Loads the main window's UI elements
static void main_window_load(Window *window) {
  // Get window information
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- DRAWING FUNCTIONS --------");
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Main Logo");
  #endif
  s_bitmap_layers[BITMAP_LAYER_LOGO] = drawing_bitmap_set((bounds.size.w - BITMAP_SIZE) / 2, (bounds.size.h * 25) / 1000, BITMAP_SIZE, BITMAP_SIZE, s_gbitmap_layers[GBITMAP_LAYER_LOGO], window_layer);
  
  #ifndef PBL_PLATFORM_APLITE
  s_bitmap_layers[BITMAP_LAYER_BAG] = drawing_bitmap_set(0, 0, BITMAP_SIZE, BITMAP_SIZE, s_gbitmap_layers[GBITMAP_LAYER_BAG], bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]));
  #endif
  
  #if defined(PBL_HEALTH)
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Health");
  #endif
  health_draw(window_layer, bounds);
  #endif

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Weather");
  #endif
  #ifndef PBL_PLATFORM_APLITE
  weather_draw(window_layer, bounds);
  #endif

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Team to Beat");
  #endif
  #ifndef PBL_PLATFORM_APLITE
  display_beatteam(window_layer, bounds);
  #endif

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing 'BEAT' Textbox");
  #endif
  display_beat_textbox(window_layer, bounds);

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Bottom Box");
  #endif
  display_main_time_layer(window_layer, bounds);

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Date and Time");
  #endif
  timeDate_draw(window_layer, bounds);
  
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Bluetooth");
  #endif
  sensor_bluetooth_draw(window_layer, bounds);

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing Battery");
  #endif
  sensor_battery_draw(window_layer, bounds);

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Drawing API");
  #endif
  api_icon_draw(window_layer, bounds);

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- INFORMATION FILL --------");
  // Apply saved settings
  APP_LOG(APP_LOG_LEVEL_INFO, "Applying Saved Settings");
  #endif
  globals_prv_update_display();
  //APP_LOG(APP_LOG_LEVEL_ERROR, "HEAP after settings: %d", (int)heap_bytes_free());

  // Make sure the time and date are displayed from the start
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Time");
  #endif
  update_time();

  // Apply correct layout in case Quick View is already active
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Apply Correct Layout w/ Quick View");
  #endif
  animation_prv_unobstructed_change(0, NULL);
}

// Unloads the main window's UI elements
static void main_window_unload(Window *window) {
  // Unsubscribe from TickTimerService
  tick_timer_service_unsubscribe();

  // Destroy TextLayers
  destroy_layers_by_kind((void**)s_text_layers, LAYER_KIND_TEXT, NUM_TEXT_LAYERS);

  // Destroy GBitmap
  destroy_layers_by_kind((void**)s_gbitmap_layers, LAYER_KIND_GBITMAP, NUM_GBITMAP_LAYERS);

  // Unload Fonts
  fonts_unload_custom_font(s_font);
  fonts_unload_custom_font(s_wIcon);

  // Destroy BitmapLayer
  destroy_layers_by_kind((void**)s_bitmap_layers, LAYER_KIND_BITMAP, NUM_BITMAP_LAYERS);

  // Destroy Layers
  destroy_layers_by_kind((void**)s_layers, LAYER_KIND_GENERIC, NUM_GENERIC_LAYERS);

  #if defined(PBL_HEALTH)
  drawing_multiline_layer_destroy(hr_icon);
  drawing_multiline_layer_destroy(step_ladder);
  #endif
}

// Initializes the app
static void init() {
  // Load settings before creating UI
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- LOAD SETTINGS --------");
  #endif
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

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- SUBSCRIBE --------");

  // Subscribe to unobstructed area events
  APP_LOG(APP_LOG_LEVEL_INFO, "Quick View");
  #endif
  animation_subscribe_unobstructed_area();

  // Register with TickTimerService
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Tick Handler");
  #endif
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to continuous accelerometer data for Y-movement detection
  #ifndef PBL_PLATFORM_APLITE
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Accelerometer");
  #endif
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  accel_data_service_subscribe(5, sensor_accel_data_handler);
  #endif

  // Subscribe to bluetooth connection updates and set initial state
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth");
  #endif
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = sensor_connection_handler
  });
  sensor_connection_handler(connection_service_peek_pebble_app_connection());

  // Subscribe to battery state changes and initialize
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Battery Level");
  #endif
  battery_state_service_subscribe(sensor_battery_handler);
  sensor_battery_handler(battery_state_service_peek());

  // Register AppMessage callbacks
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Communication");
  #endif
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 560;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

}

// Deinitializes the app
static void deinit() {
  //bluetooth_connection_service_unsubscribe();

  #ifndef PBL_PLATFORM_APLITE
  accel_data_service_unsubscribe();
  #endif
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  unobstructed_area_service_unsubscribe();

  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}