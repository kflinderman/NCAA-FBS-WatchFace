#include <pebble.h>
#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "animation.h"
#include "sensors.h"
#include "timekeeping.h"
#include "communication.h"

/***********************************/
/* NCAA FBS Watchface              */
/***********************************/

// Loads the main window's UI elements
static void main_window_load(Window *window) {
  // Get window information
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Here's where I increased the size of the moving box FYI. Started at bounds.size.h / 2 + 50
  beat_team_layer = layer_create_with_data(GRect(-bounds.size.w - 10, 0, bounds.size.w + 10, bounds.size.h / 2 + 100), sizeof(RoundRectData));
  RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);

  if (settings.DisplayTeam > 1) {
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color};
  } else {
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.BeatTeam].color};
  }

  s_logo_layer = bitmap_set((bounds.size.w - bitmap_size) / 2, bounds.size.h * 0.025, bitmap_size, bitmap_size, s_logo_bitmap, window_layer);

  // Create beat_team_layer with per-layer color data
  layer_set_update_proc(beat_team_layer, round_rect_update_proc);
  layer_add_child(window_layer, beat_team_layer);
  s_beat_team_layer = bitmap_set((bounds.size.w - bitmap_size) / 2, bounds.size.h * 0.025, bitmap_size, bitmap_size, s_beat_team_bitmap, beat_team_layer);

#ifdef PBL_RECT
  beat_spot = 0;
#else
  beat_spot = bounds.size.w / 2 - 22;
#endif
  beat_primary = settings.DisplayTeam;

  // rect_beat_layer with its own color
  rect_beat_layer = layer_create_with_data(GRect(beat_spot, -40 + beat_primary, 45, 40), sizeof(RoundRectData));
  RoundRectData *rect_beat_data = (RoundRectData *)layer_get_data(rect_beat_layer);
  rect_beat_data->fill_color = GColorWhite; // default
  layer_set_update_proc(rect_beat_layer, round_rect_update_proc);
  layer_add_child(window_layer, rect_beat_layer);

  s_beat_layer = text_set(0, 10, 44, 30, GColorBlack, "BEAT", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter, rect_beat_layer);

  rect_layer = layer_create_with_data(GRect(0, bounds.size.h * rect_h, bounds.size.w, 100), sizeof(RoundRectData));

  RoundRectData *rect_data = (RoundRectData *)layer_get_data(rect_layer);
  rect_data->fill_color = GColorWhite; // default
  layer_set_update_proc(rect_layer, round_rect_update_proc);
  layer_add_child(window_layer, rect_layer);

#if PBL_DISPLAY_HEIGHT > 180
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_54));
  s_time_layer = text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
#else
  s_time_layer = text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GTextAlignmentCenter, window_layer);
#endif

#ifdef PBL_RECT
  // Create the TextLayer for the time and date
  #if PBL_DISPLAY_HEIGHT > 180
    s_date_layer = text_set(155, bounds.size.h * date_h, 35, 38, GColorBlack, "Dec 31", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
  #else
    s_date_layer = text_set(bounds.size.w * date_w, bounds.size.h * date_h, 26, 32, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  #endif

  vertical_line = line_draw(bounds, bounds.size.w * hor_1, bounds.size.h * vert_1, bounds.size.w * hor_1, bounds.size.h * vert_2, 1, GColorBlack, window_layer);
#else
  #if PBL_DISPLAY_HEIGHT > 180
    // Create the TextLayer for the time and date
    s_date_layer = text_set(bounds.size.w / 2 - 22, bounds.size.h * date_h, 46, 21, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
  #else
    // Create the TextLayer for the time and date
    s_date_layer = text_set(bounds.size.w / 2 - 20, bounds.size.h * date_h, 42, 17, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  #endif
#endif
  horizontal_line = line_draw(bounds, bounds.size.w * hor_1, bounds.size.h * vert_2, bounds.size.w * hor_2, bounds.size.h * vert_2, 1, GColorBlack, window_layer);

  // Create Bluetooth GBitmap from resource
  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);

#if PBL_DISPLAY_HEIGHT > 180
  s_bt_layer = bitmap_set(182, bounds.size.h * vert_2, 10, 14, s_bt_bitmap, window_layer);
#else
  s_bt_layer = bitmap_set(bounds.size.w * hor_2 - icon_bump, bounds.size.h * vert_2 + 3, 5, 7, s_bt_bitmap, window_layer);
#endif
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), s_bt_connected);

  // Create Battery GBitmap from resource
  s_batt_low_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOWBATT);
  s_batt_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EMPTYBATT);
  s_batt_crg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBATT);
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  s_batt_layer = bitmap_set(168, bounds.size.h * vert_2, 8, 14, s_batt_low_bitmap, window_layer);
#else
  s_batt_layer = bitmap_set(bounds.size.w * hor_2 - (icon_bump + 7), bounds.size.h * vert_2 + 3, 4, 7, s_batt_low_bitmap, window_layer);
#endif

  battery_handler(battery_state_service_peek());

#if defined(PBL_HEALTH)
  // Create Health Layers
  s_hr_layer = text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
  hr1 = line_draw(bounds, bounds.size.w - 23 + (6*hr_w), 30, bounds.size.w - 20 + (5*hr_w), 30, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  hr2 = line_draw(bounds, bounds.size.w - 20 + (5*hr_w), 30, bounds.size.w - 17 + (4*hr_w), 35, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  hr3 = line_draw(bounds, bounds.size.w - 17 + (4*hr_w), 35, bounds.size.w - 11 + (2*hr_w), 25, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  hr4 = line_draw(bounds, bounds.size.w - 11 + (2*hr_w), 25, bounds.size.w - 8 + (hr_w), 30, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  hr5 = line_draw(bounds, bounds.size.w - 8 + (hr_w), 30, bounds.size.w - 5, 30, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);

  s_step_layer = text_set(bounds.size.w / 2 - stepx2, (bounds.size.h * time_h) - 20, 40, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentLeft, window_layer);
  int gaps = ((bounds.size.h * time_h) - stepy - 25) / 4;
  step1 = line_draw(bounds, (bounds.size.w / 2 - stepx2) + (stepx1 / 2), (bounds.size.h * time_h) - 27, (bounds.size.w / 2 - stepx2) + (stepx1 / 2), stepy, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  step2 = line_draw(bounds, bounds.size.w / 2 - stepx2, stepy, (bounds.size.w / 2 - stepx2) + stepx1, stepy, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  step3 = line_draw(bounds, bounds.size.w / 2 - stepx2, stepy + gaps * 1, (bounds.size.w / 2 - stepx2) + stepx1, stepy + gaps * 1, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  step4 = line_draw(bounds, bounds.size.w / 2 - stepx2, stepy + gaps * 2, (bounds.size.w / 2 - stepx2) + stepx1, stepy + gaps * 2, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  step5 = line_draw(bounds, bounds.size.w / 2 - stepx2, stepy + gaps * 3, (bounds.size.w / 2 - stepx2) + stepx1, stepy + gaps * 3, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);
  step6 = line_draw(bounds, bounds.size.w / 2 - stepx2, stepy + gaps * 4, (bounds.size.w / 2 - stepx2) + stepx1, stepy + gaps * 4, hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, window_layer);

  s_football_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBATT);
  s_football_layer = bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 5, stepy + gaps * 4 - 5, 10, 10, s_football_bitmap, window_layer);
#endif

  // Apply saved settings
  prv_update_display();

  // Make sure the time and date are displayed from the start
  update_time();

  // Apply correct layout in case Quick View is already active
  prv_unobstructed_change(0, NULL);

  // Subscribe to unobstructed area events
  animation_subscribe_unobstructed_area();
}

// Unloads the main window's UI elements
static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_beat_layer);
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

#if PBL_DISPLAY_HEIGHT > 180
  fonts_unload_custom_font(s_font);
#endif

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_logo_layer);
  bitmap_layer_destroy(s_beat_team_layer);
  bitmap_layer_destroy(s_bt_layer);
  bitmap_layer_destroy(s_batt_layer);

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
  layer_destroy(hr1);
  layer_destroy(hr2);
  layer_destroy(hr3);
  layer_destroy(hr4);
  layer_destroy(hr5);
  layer_destroy(step1);
  layer_destroy(step2);
  layer_destroy(step3);
  layer_destroy(step4);
  layer_destroy(step5);
  layer_destroy(step6);
#endif

  // Unsubscribe from TickTimerService
  tick_timer_service_unsubscribe();
}

// Initializes the app
static void init() {
  // Load settings before creating UI
  prv_load_settings();

  // Create main Window element
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to continuous accelerometer data for Y-movement detection
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  accel_data_service_subscribe(5, accel_data_handler);

  // Subscribe to bluetooth connection updates and set initial state
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = connection_handler
  });
  connection_handler(connection_service_peek_pebble_app_connection());

  // Subscribe to battery state changes and initialize
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Batt Start");
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  // Register AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 256;
  const int outbox_size = 256;
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
