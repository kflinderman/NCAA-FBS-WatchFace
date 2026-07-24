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
  s_bag_layerf = bitmap_set(0, 0, bitmap_size, bitmap_size, s_bag_bitmap, bitmap_layer_get_layer(s_logo_layer));

  #if defined(PBL_HEALTH)
    // Create Health Layers
    #if PBL_DISPLAY_HEIGHT > 180
    s_hr_layer = text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
    #else
    s_hr_layer = text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
    #endif
    hr_icon = multiline_layer_create(bounds, window_layer);
    multiline_add_segment(hr_icon, GPoint(bounds.size.w - 23 + (6*hr_w), 30), GPoint(bounds.size.w - 20 + (5*hr_w), 30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    multiline_add_segment(hr_icon, GPoint(bounds.size.w - 20 + (5*hr_w), 30), GPoint(bounds.size.w - 17 + (4*hr_w), 35), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    multiline_add_segment(hr_icon, GPoint(bounds.size.w - 17 + (4*hr_w), 35), GPoint(bounds.size.w - 11 + (2*hr_w), 25), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    multiline_add_segment(hr_icon, GPoint(bounds.size.w - 11 + (2*hr_w), 25), GPoint(bounds.size.w - 8 + (hr_w),    30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    multiline_add_segment(hr_icon, GPoint(bounds.size.w - 8 + (hr_w), 30),    GPoint(bounds.size.w - 5,             30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    
    #if PBL_DISPLAY_HEIGHT > 180
    s_step_layer = text_set(bounds.size.w / 2 - stepx2, (bounds.size.h * time_h) - 20, 50, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentLeft, window_layer);
    #else
    s_step_layer = text_set(bounds.size.w / 2 - stepx2, (bounds.size.h * time_h) - 20, 50, 16, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
    #endif
    uint16_t gaps = ((bounds.size.h * time_h) - stepy - 25) / 3;
    uint16_t gaps2 = gaps / 5;
    step_ladder = multiline_layer_create(bounds, window_layer);
    //multiline_add_segment(step_ladder, GPoint((bounds.size.w / 2 - stepx2) + (stepx1 / 2), (bounds.size.h * time_h) - 27), GPoint((bounds.size.w / 2 - stepx2) + (stepx1 / 2), stepy), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  
    for (uint16_t x = 0; x < 4; x++) {
      multiline_add_segment(step_ladder, GPoint(bounds.size.w / 2 - stepx2, stepy + (gaps * x)), GPoint((bounds.size.w / 2 - stepx2) + stepx1, stepy + (gaps * x)), 2, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
      
      if (x < 3){
        for (uint16_t y = 1; y < 6; y++) {
          multiline_add_segment(step_ladder, GPoint((bounds.size.w / 2 - stepx2) + 3, stepy + (gaps * x) + (gaps2 * y)), GPoint((bounds.size.w / 2 - stepx2) + stepx1 - 3, stepy + (gaps * x) + (gaps2 * y)), 1, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
        }
      }
    }
    
    s_football_bitmap = gbitmap_create_with_resource(RESOURCE_ID_football);
    #if PBL_DISPLAY_HEIGHT > 180
      s_football_layer = bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 4, stepy + gaps * 4 - 6, 12, 12, s_football_bitmap, window_layer);
      #ifdef PBL_RECT
        s_td_layer = text_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 10, stepy - (14+21), 25, 21, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "TD!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
      #else
        s_td_layer = text_set((bounds.size.w / 2 - stepx2) - 15, (bounds.size.w / 2) - 55, 10, 80, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "T D !", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
      #endif
      layer_set_hidden(text_layer_get_layer(s_td_layer), true);
    #else
      s_football_layer = bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 4, stepy + gaps * 4 - 4, 8, 8, s_football_bitmap, window_layer);
      #ifdef PBL_RECT
        s_td_layer = text_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 10, stepy - (10+17), 25, 17, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "TD!", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
      #else
        s_td_layer = text_set((bounds.size.w / 2 - stepx2) - 15, (bounds.size.w / 2) - 35, 10, 50, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "T D !", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
      #endif
      layer_set_hidden(text_layer_get_layer(s_td_layer), true);
    #endif
  #endif
  
  // Create beat_team_layer with per-layer color data
  layer_set_update_proc(beat_team_layer, round_rect_update_proc);
  layer_add_child(window_layer, beat_team_layer);
  s_beat_team_layer = bitmap_set((bounds.size.w - bitmap_size) / 2, bounds.size.h * 0.025, bitmap_size, bitmap_size, s_beat_team_bitmap, beat_team_layer);
  s_bag_layerb = bitmap_set(0, 0, bitmap_size, bitmap_size, s_bag_bitmap, bitmap_layer_get_layer(s_beat_team_layer));

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
  //182
  s_bt_layer = bitmap_set(bounds.size.w * hor_2 - icon_bump, bounds.size.h * vert_2, 10, 14, s_bt_bitmap, window_layer);
#else
  s_bt_layer = bitmap_set(bounds.size.w * hor_2 - icon_bump, bounds.size.h * vert_2 + 3, 5, 7, s_bt_bitmap, window_layer);
#endif
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), s_bt_connected);

  // Create Battery GBitmap from resource
  s_batt_low_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOWBATT);
  s_batt_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EMPTYBATT);
  s_batt_crg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBATT);
#if PBL_DISPLAY_HEIGHT > 180
  //168
  s_batt_layer = bitmap_set(bounds.size.w * hor_2 - (icon_bump + 14), bounds.size.h * vert_2, 8, 14, s_batt_low_bitmap, window_layer);
#else
  s_batt_layer = bitmap_set(bounds.size.w * hor_2 - (icon_bump + 7), bounds.size.h * vert_2 + 3, 4, 7, s_batt_low_bitmap, window_layer);
#endif

  battery_handler(battery_state_service_peek());

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
  if(s_bag_bitmap) {
    gbitmap_destroy(s_bag_bitmap);
    s_bag_bitmap = NULL; 
  }

#if PBL_DISPLAY_HEIGHT > 180
  fonts_unload_custom_font(s_font);
#endif

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
  multiline_layer_destroy(hr_icon);
  multiline_layer_destroy(step_ladder);
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
