#include <pebble.h>
#include "teams.h"
#include "structure.h"

// Persistent storage key
#define SETTINGS_KEY 1
#define NUM_TEAMS 154

/***********************************/
/* NCAA FBS Watchface              */
/*                                 */
/* Fill this in later              */
/*                                 */
/* #0 Table of Contents:           */
/*     1. Variables                */
/*     2. Settings                 */
/*     3. Time                     */
/*     4. Communication            */
/*     5. Drawing Functions        */
/*     6. Animation                */     
/*     7. Accelerometer            */
/*     8. Bluetooth                */
/*     9. Battery                  */
/*     10. Main Window              */
/*     11. Main Function           */
/***********************************/

/*
Task List:
---Release---
-Keizelpay
  - Code
-Weather
  - Weather Icons Black
  - Weather Icons White
  - Code
-Battery Saving mode
  - Code
-Health steps
  - Football Icon
  - Hash marks
  - Code
-Health Heartrate
  - Code
-Paper Bag
  - Code
-Hardcoded Rival
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


/*****************/
/* #1. Variables */
/*****************/

// Static pointers to UI elements
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_beat_layer;
static GBitmap *s_logo_bitmap;
static GBitmap *s_beat_team_bitmap;
static GBitmap *s_bt_bitmap;
static GBitmap *s_batt_crg_bitmap;
static GBitmap *s_batt_empty_bitmap;
static GBitmap *s_batt_low_bitmap;
static BitmapLayer *s_logo_layer;
static BitmapLayer *s_beat_team_layer;
static BitmapLayer *s_bt_layer;
static BitmapLayer *s_batt_layer;
static Layer *rect_layer;
static Layer *horizontal_line;
static Layer *beat_team_layer;
static Layer *rect_beat_layer;
#ifdef PBL_RECT
  static Layer *vertical_line;
#endif


static GFont s_font;
static void animate_beat_team_layer();

// Watch Sensor Variables
static int16_t s_prev_y = 0;
static bool s_bt_connected = false;
static bool s_animation = false;
static BatteryChargeState s_battery_state;
bool s_bt_history = true;
bool s_batt_history = false;

// Threshold in milli-g's.
//static int ACCEL_Y_THRESHOLD = 800;

// Different Watch Positions
static uint16_t beat_spot;
static uint16_t beat_primary;
#ifdef PBL_ROUND
  static float rect_h = 0.66;
  static float date_h = 0.84;
  static float vert_2 = 0.93;
  static float hor_1 = 0.45;
  static float hor_2 = 0.55;
  #ifdef PBL_PLATFORM_GABBRO
    static uint16_t time_w = 75;
    static float time_h = 0.62;
    static uint16_t time_x = 150;
    static uint16_t time_y = 70;
    static uint16_t icon_bump = 10;
  #else
    static uint16_t time_w = 60;
    static float time_h = 0.62;
    static uint16_t time_x = 120;
    static uint16_t time_y = 50;
    static uint16_t icon_bump = 8;
  #endif
#else
  static float rect_h = 0.72;
  static float date_w = 0.81;
  static float date_h = 0.74;
  static uint16_t icon_bump = 5;
  #ifdef PBL_PLATFORM_EMERY
    static uint16_t time_w = 90;
    static float time_h = 0.70;
    static uint16_t time_x = 160;
    static uint16_t time_y = 70;
    static float vert_1 = 0.82;
    static float vert_2 = 0.88;
    static float hor_1 = 0.83;
    static float hor_2 = 0.92;
  #else
    static uint16_t time_w = 70;
    static float time_h = 0.70;
    static uint16_t time_x = 120;
    static uint16_t time_y = 50;
    static float vert_1 = 0.85;
    static float vert_2 = 0.93;
    static float hor_1 = 0.86;
    static float hor_2 = 0.97;
  #endif
#endif

#if PBL_DISPLAY_HEIGHT > 180
  static uint16_t bitmap_size = 160;
#else
  static uint16_t bitmap_size = 115;
#endif


/****************/
/* #1. Settings */
/****************/

// Define our settings struct
typedef struct ClaySettings {
  int32_t DisconnectVibration;
  int32_t ReconnectVibration;
  int32_t LowBatteryPercent;
  int32_t LowBatteryVibration;
  int32_t EmptyBatteryPercent;
  int32_t EmptyBatteryVibration;
  int32_t DisplayTeam;
  int32_t FavoriteTeam;
  int32_t BeatTeam;
  int animationSensitivity;
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

// Set default settings
static void prv_default_settings() {
  settings.DisconnectVibration = 3;
  settings.ReconnectVibration = 1;
  settings.LowBatteryPercent = 30;
  settings.LowBatteryVibration = 1;
  settings.EmptyBatteryPercent = 10;
  settings.EmptyBatteryVibration = 2;
  settings.DisplayTeam = 0;
  settings.FavoriteTeam = 108; 
  settings.BeatTeam = 26;
  settings.animationSensitivity = 800;
}

// Save settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Read settings from persistent storage
static void prv_load_settings() {
  prv_default_settings();
  // Only load if the saved struct matches current size
  // (protects against corrupt data or struct layout changes)
  if (persist_exists(SETTINGS_KEY) && 
      persist_get_size(SETTINGS_KEY) == sizeof(ClaySettings)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
  // Bounds-check team indices before they're used to index TEAMS[]
  if (settings.FavoriteTeam >= NUM_TEAMS) settings.FavoriteTeam = 108;
  if (settings.BeatTeam >= NUM_TEAMS) settings.BeatTeam = 26;
}

// Apply settings to UI elements
static void prv_update_display() {
  // Bounds check team indices
  if (settings.FavoriteTeam >= NUM_TEAMS) settings.FavoriteTeam = 108;
  if (settings.BeatTeam >= NUM_TEAMS) settings.BeatTeam = 26;
  
  // Only update if window exists
  if (!s_main_window) return;
  
  // Update beat_primary if DisplayTeam changed
  beat_primary = settings.DisplayTeam;
  
  // Update beat team layer position
  if (rect_beat_layer) {
    GRect new_frame = GRect(beat_spot, -10 - beat_primary, 44, 40);
    layer_set_frame(rect_beat_layer, new_frame);
    layer_mark_dirty(rect_beat_layer);
  }
  
  // Update favorite team logo
  if (s_logo_bitmap) {
    gbitmap_destroy(s_logo_bitmap);
  }
  if (s_beat_team_bitmap) {
    gbitmap_destroy(s_beat_team_bitmap);
  }
  
  if(settings.DisplayTeam > 1){
    // Update window background color to favorite team's color
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.BeatTeam].color});
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
  
  
    // Update beat team layer color
    if (beat_team_layer) {
      RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);
      if (beat_data) {
        beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color};
        layer_mark_dirty(beat_team_layer);
      }
    }
  }
  else{
    // Update window background color to favorite team's color
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.FavoriteTeam].color});
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
  
  
    // Update beat team layer color
    if (beat_team_layer) {
      RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);
      if (beat_data) {
        beat_data->fill_color = (GColor){.argb = TEAMS[settings.BeatTeam].color};
        layer_mark_dirty(beat_team_layer);
      }
    }
  }
  
  if (s_logo_layer) {
    bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  }
  if (s_beat_team_layer) {
    bitmap_layer_set_bitmap(s_beat_team_layer, s_beat_team_bitmap);
  }
  
}

/************/
/* #3. Time */
/************/

// Updates the time TextLayer
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes uint16_to a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);

  // Write the month and day uint16_to a buffer with a newline
  static char s_date_buffer[10];
  #ifdef PBL_ROUND
    strftime(s_date_buffer, sizeof(s_date_buffer), "%b %e", tick_time);
  #else
    strftime(s_date_buffer, sizeof(s_date_buffer), "%b\n%e", tick_time);
  #endif
  
  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, s_date_buffer);
}

// Handles time ticks (every minute)
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
    app_message_outbox_send();
  }
  
}

/*********************/
/* #4. Communication */
/*********************/


// AppMessage received handler
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  /*
  // Check for weather data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  if (temp_tuple && conditions_tuple) {
    static char temperature_buffer[8];
    static char conditions_buffer[32];
    static char weather_layer_buffer[32];

    int temp_value = (int)temp_tuple->value->int32;

    // Convert to Fahrenheit if setting is enabled
    if (settings.TemperatureUnit) {
      temp_value = (temp_value * 9 / 5) + 32;
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°F", temp_value);
    } else {
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°C", temp_value);
    }

    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature_buffer, conditions_buffer);
  }
  */

  // Check for Clay settings data
  
  uint32_t claysettings_id[] = {
    MESSAGE_KEY_DisconnectVibration,
    MESSAGE_KEY_ReconnectVibration,
    MESSAGE_KEY_LowBatteryPercent,
    MESSAGE_KEY_LowBatteryVibration,
    MESSAGE_KEY_EmptyBatteryPercent,
    MESSAGE_KEY_EmptyBatteryVibration,
    MESSAGE_KEY_DisplayTeam,
    MESSAGE_KEY_FavoriteTeam,
    MESSAGE_KEY_BeatTeam,
    MESSAGE_KEY_animationSensitivity,
  };

  // 3. The simplified loop
  bool settings_changed = false;
  
  for (uint16_t x = 0; x < 10; x++){
    Tuple *temp_t = dict_find(iterator, claysettings_id[x]);
    if (temp_t) {
      
      int32_t value = 0;
      
      if (temp_t->type == TUPLE_CSTRING) {
        // Use a safer string-to-int conversion
        const char *str = temp_t->value->cstring;
        
        // Manual parsing instead of strtol
        value = 0;
        for (int i = 0; str[i] != '\0'; i++) {
          if (str[i] >= '0' && str[i] <= '9') {
            value = value * 10 + (str[i] - '0');
          }
        }
      } else if (temp_t->type == TUPLE_INT) {
        value = temp_t->value->int32;
      }
      
      // Directly assign to settings struct
      switch(x) {
        case 0: settings.DisconnectVibration = value; break;
        case 1: settings.ReconnectVibration = value; break;
        case 2: settings.LowBatteryPercent = value; break;
        case 3: settings.LowBatteryVibration = value; break;
        case 4: settings.EmptyBatteryPercent = value; break;
        case 5: settings.EmptyBatteryVibration = value; break;
        case 6: settings.DisplayTeam = value; break;
        case 7: settings.FavoriteTeam = value; break;
        case 8: settings.BeatTeam = value; break;
        case 9: settings.animationSensitivity = value; break;
      }
      
      settings_changed = true;
    }
  }
  
  // Save and apply if any settings were changed
  if(settings_changed){
  //if (dc_vibe_t || rc_vibe_t || lb_pct_t || lb_vibe_t || eb_pct_t || eb_vibe_t || dp_team_t || fv_team_t || bt_team_t || ver_t) {
    prv_save_settings();
    prv_update_display();

    // Refetch weather if the temperature unit changed so the display updates
    /*
    if (temp_unit_t) {
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
      app_message_outbox_send();
    }
    */
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/*************************/
/* #5. Drawing Functions */
/*************************/

//{{REVIEW}}


static void line_update_proc(Layer *layer, GContext *ctx) {
  LinePoints *points = (LinePoints *)layer_get_data(layer);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(points->x1, points->y1), GPoint(points->x2, points->y2));
}

Layer* line_draw(GRect bounds, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Layer *window_layer) {
  Layer *line_layer = layer_create_with_data(bounds, sizeof(LinePoints));
  LinePoints *points = (LinePoints *)layer_get_data(line_layer);
  points->x1 = x1;
  points->y1 = y1;
  points->x2 = x2;
  points->y2 = y2;
  layer_set_update_proc(line_layer, line_update_proc);
  layer_add_child(window_layer, line_layer);
  return line_layer;
}

BitmapLayer* bitmap_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GBitmap *bitmap, Layer *window){
  BitmapLayer *s_bitmap_layer = bitmap_layer_create(GRect(x, y, w, h));
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bitmap_layer, bitmap);
  layer_add_child(window, bitmap_layer_get_layer(s_bitmap_layer));

  return s_bitmap_layer;
}

TextLayer* text_set (uint16_t x, uint16_t y, uint16_t w, uint16_t h, GColor text_color, const char *initial_text, GFont font_handle, GTextAlignment alignment, Layer *window) {
  TextLayer *text_layer = text_layer_create(GRect(x, y, w, h));

  text_layer_set_text_color(text_layer, text_color);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_font(text_layer, font_handle);
  text_layer_set_text_alignment(text_layer, alignment);
  text_layer_set_text(text_layer, initial_text);
  layer_add_child(window, text_layer_get_layer(text_layer));

  return text_layer;
}

static void round_rect_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  uint16_t stroke_radius = 8;
  // If layer has RoundRectData, use its fill_color; otherwise default to white
  RoundRectData *data = (RoundRectData *)layer_get_data(layer);
  GColor fill = (data != NULL) ? data->fill_color : GColorWhite;
  graphics_context_set_fill_color(ctx, fill);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, stroke_radius, GCornersAll);
  graphics_draw_round_rect(ctx, bounds, stroke_radius);
}

/*****************/
/* #6. Animation */
/*****************/

//{{REVIEW}}

// Animation complete handler
static void beat_team_animation_stopped(Animation *animation, bool finished, void *context) {
  static bool returning = false;
  Layer *layer = (Layer *)context;

  if (!returning) {
    returning = true;
    // Wait 1 second, then animate back
    app_timer_register(1000, (AppTimerCallback)animate_beat_team_layer, layer);
  } else {
    returning = false; // Reset for next cycle
    s_animation = false;
  }
}

static void animate_beat_team_layer() {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GRect beat_from, beat_to;
  GRect rect_from, rect_to;
  static bool returning = false;

  if (!returning) {
    beat_from = GRect(-bounds.size.w, 0, bounds.size.w, bounds.size.h / 2 + 50);
    beat_to   = GRect(0, 0, bounds.size.w, bounds.size.h / 2 + 50);

    rect_from = GRect(beat_spot, -40 + beat_primary, 44, 40);
    rect_to   = GRect(beat_spot, -10 - beat_primary, 44, 40);
  } else {
    beat_from = GRect(0, 0, bounds.size.w, bounds.size.h / 2 + 50);
    beat_to   = GRect(-bounds.size.w, 0, bounds.size.w, bounds.size.h / 2 + 50);

    rect_from = GRect(beat_spot, -10 - beat_primary, 44, 40);
    rect_to   = GRect(beat_spot, -40 + beat_primary, 44, 40);
  }

  // Animate beat_team_layer
  PropertyAnimation *anim_beat = property_animation_create_layer_frame(beat_team_layer, &beat_from, &beat_to);
  animation_set_duration((Animation*)anim_beat, 1000);
  animation_set_handlers((Animation*)anim_beat, (AnimationHandlers){
    .stopped = beat_team_animation_stopped
  }, beat_team_layer);
  animation_schedule((Animation*)anim_beat);

  // Animate rect_beat_layer
  PropertyAnimation *anim_rect = property_animation_create_layer_frame(rect_beat_layer, &rect_from, &rect_to);
  animation_set_duration((Animation*)anim_rect, 1000);
  animation_schedule((Animation*)anim_rect);

  returning = !returning;
}

// Unobstructed area handlers
static void prv_unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {
}

static void layermove(GRect tmp_bounds, int diff, Layer *tmp_layer, float origin, uint16_t bump, float bmp_ratio) {
  GRect move_frame = layer_get_frame(tmp_layer);
  move_frame.origin.y = (tmp_bounds.size.h * origin + bump) - diff * bmp_ratio;
  layer_set_frame(tmp_layer, move_frame);
}

static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  Layer *root = window_get_root_layer(s_main_window);
  GRect obsBounds = layer_get_unobstructed_bounds(root);
  GRect unBounds = layer_get_bounds(root);
  
  // Reposition to fit in the available space
  int bound_diff = unBounds.size.h - obsBounds.size.h;
  
  layermove(unBounds, bound_diff, text_layer_get_layer(s_time_layer), time_h, 0, 1);
  layermove(unBounds, bound_diff, text_layer_get_layer(s_date_layer), date_h, 0, 1);
  layermove(unBounds, bound_diff, rect_layer, rect_h, 0, 1);
  layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_logo_layer), 0.025, 0, 0.5);
  layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_beat_team_layer), 0.025, 0, 0.5);
  layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bt_layer), vert_2, 3, 1);
  layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_batt_layer), vert_2, 3, 1);
  
  #ifdef PBL_RECT
    LinePoints *points = (LinePoints *)layer_get_data(vertical_line);
    points->y1 = unBounds.size.h * vert_1 - bound_diff;
    points->y2 = unBounds.size.h * vert_2 - bound_diff;
    layer_mark_dirty(vertical_line);
  #endif
  
  LinePoints *points2 = (LinePoints *)layer_get_data(horizontal_line);
  points2->y1 = unBounds.size.h * vert_2 - bound_diff;
  points2->y2 = unBounds.size.h * vert_2 - bound_diff;
  layer_mark_dirty(horizontal_line);
}

static void prv_unobstructed_did_change(void *context) {
  Layer *root = window_get_root_layer(s_main_window);
  GRect full_bounds = layer_get_bounds(root);
  GRect bounds = layer_get_unobstructed_bounds(root);
  bool obstructed = !grect_equal(&full_bounds, &bounds);

  if (obstructed) {
  }
  else {
  }
}

/*********************/
/* #7. Accelerometer */
/*********************/

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  if (num_samples == 0 || data == NULL) return;
  // Use the last sample in the batch for current reading
  int16_t curr_y = data[num_samples - 1].y;
  int16_t delta = curr_y - s_prev_y;
  
  if (abs(delta) > settings.animationSensitivity && !s_animation && settings.animationSensitivity != 0) {
    // Detected sudden Y movement and play animation
    s_animation = true;
    animate_beat_team_layer();
  }
  s_prev_y = curr_y;
}

/*****************/
/* #8. Bluetooth */
/*****************/

static void connection_handler(bool connected) {
  s_bt_connected = connected;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Bluetooth: %d History: %d", s_bt_connected, s_bt_history);
  
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), connected);
  
  // optional: give feedback when connection state changes
  if (connected && !s_bt_history) {
    // connected
    switch(settings.ReconnectVibration) {
      case 0: break;
      case 1: vibes_short_pulse(); break;
      case 2: vibes_long_pulse(); break;
      case 3: vibes_double_pulse(); break;
    }
    
    s_bt_history = true;
  } else if (!connected && s_bt_history) {
    // disconnected
    switch(settings.DisconnectVibration) {
      case 0: break;
      case 1: vibes_short_pulse(); break;
      case 2: vibes_long_pulse(); break;
      case 3: vibes_double_pulse(); break;
    }
    s_bt_history = false;
  }
}

static bool is_bt_connected() {
  return s_bt_connected;
}

/***************/
/* #9. Battery */
/***************/

static void battery_handler(BatteryChargeState state) {
  s_battery_state = state;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery: %d History: %d", state.charge_percent, s_batt_history);
  
  
  // optional: vibrate on low battery threshold or update UI
  if (!state.is_charging && state.charge_percent <= settings.LowBatteryPercent && state.charge_percent > settings.EmptyBatteryPercent ) {
    // warn briefly
    if(!s_batt_history){
    switch(settings.LowBatteryVibration) {
      case 0: break;
      case 1: vibes_short_pulse(); break;
      case 2: vibes_long_pulse(); break;
      case 3: vibes_double_pulse(); break;
    }
      s_batt_history = true;
    }
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_low_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
  }
  else if(!state.is_charging && state.charge_percent <= settings.EmptyBatteryPercent){
    switch(settings.EmptyBatteryVibration) {
      case 0: break;
      case 1: vibes_short_pulse(); break;
      case 2: vibes_long_pulse(); break;
      case 3: vibes_double_pulse(); break;
    }
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_empty_bitmap);  
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
    s_batt_history = false;
  }
  else if(state.is_charging){
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_crg_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
    s_batt_history = false;
  }
  else{
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), true);
    s_batt_history = false;
  }
}

static int get_battery_level() {
  return s_battery_state.charge_percent;
}

static bool is_battery_charging() {
  return s_battery_state.is_charging;
}

/*******************/
/* #10. Main Window */
/*******************/

//{{REVIEW}}

// Loads the main window's UI elements
static void main_window_load(Window *window) {
  // Get window information
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Here's where I increased the size of the moving box FYI. Started at bounds.size.h / 2 + 50
  beat_team_layer = layer_create_with_data(GRect(-bounds.size.w, 0, bounds.size.w + 10, bounds.size.h / 2 + 60), sizeof(RoundRectData));
  RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);
    
  if(settings.DisplayTeam > 1){
    // Set the team Colors
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.BeatTeam].color}); // Set to first team's color
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color}; // Set to second team's color
  
    // Create GBitmap from resource
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
  }
  else{
    // Set the team Colors
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.FavoriteTeam].color}); // Set to first team's color
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.BeatTeam].color}; // Set to second team's color
  
    // Create GBitmap from resource
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
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

  #if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_54));
    s_time_layer = text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
  #else
    s_time_layer = text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GTextAlignmentCenter, window_layer);
  #endif
  
  
  #ifdef PBL_RECT
    // Create the TextLayer for the time and date
    s_date_layer = text_set(bounds.size.w * date_w, bounds.size.h * date_h, 26, 32, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  
    vertical_line = line_draw(bounds, bounds.size.w * hor_1, bounds.size.h * vert_1, bounds.size.w * hor_1, bounds.size.h * vert_2, window_layer);
  #else
	#ifdef PBL_PLATFORM_GABBRO
	  // Create the TextLayer for the time and date
      s_date_layer = text_set(bounds.size.w / 2 - 22, bounds.size.h * date_h, 46, 21, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #else
      // Create the TextLayer for the time and date
      s_date_layer = text_set(bounds.size.w / 2 - 20, bounds.size.h * date_h, 42, 17, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #endif
  #endif
  horizontal_line = line_draw(bounds, bounds.size.w * hor_1, bounds.size.h * vert_2, bounds.size.w  * hor_2, bounds.size.h * vert_2, window_layer);
  
  // Create Bluetooth GBitmap from resource
  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);
  s_bt_layer = bitmap_set(bounds.size.w  * hor_2 - icon_bump, bounds.size.h * vert_2 + 3, 5, 7, s_bt_bitmap, window_layer);
  layer_set_hidden(bitmap_layer_get_layer(s_bt_layer), is_bt_connected());
  
  // Create Battery GBitmap from resource
  s_batt_low_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOWBATT);
  s_batt_empty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_EMPTYBATT);
  s_batt_crg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBATT);
  s_batt_layer = bitmap_set(bounds.size.w  * hor_2 - (icon_bump + 7), bounds.size.h * vert_2 + 3, 4, 7, s_batt_low_bitmap, window_layer);
  
  if (!is_battery_charging() && get_battery_level() <= 30 && get_battery_level() > 10 ) {
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_low_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
  }
  else if(!is_battery_charging() && get_battery_level() <= 10){
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_empty_bitmap);  
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
  }
  else if(is_battery_charging()){
    bitmap_layer_set_bitmap(s_batt_layer, s_batt_crg_bitmap);
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), false);
  }
  else{
    layer_set_hidden(bitmap_layer_get_layer(s_batt_layer), true);
  }
  
  // Make sure the time and date are displayed from the start
  update_time();
  
  // Apply saved settings
  prv_update_display();
  
  // Apply correct layout in case Quick View is already active
  prv_unobstructed_change(0, NULL);

  // Subscribe to unobstructed area events
  UnobstructedAreaHandlers handlers = {
    .will_change = prv_unobstructed_will_change,
    .change = prv_unobstructed_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe(handlers, NULL);
}

// Unloads the main window's UI elements
static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_beat_layer);

  // Destroy GBitmap
  gbitmap_destroy(s_logo_bitmap);
  gbitmap_destroy(s_beat_team_bitmap);
  gbitmap_destroy(s_bt_bitmap);
  gbitmap_destroy(s_batt_crg_bitmap);
  gbitmap_destroy(s_batt_empty_bitmap);
  gbitmap_destroy(s_batt_low_bitmap);
  
  #if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
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

  // Unsubscribe from TickTimerService
  tick_timer_service_unsubscribe();

}

/**********************/
/* #11. Main Function */
/**********************/

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
  accel_data_service_subscribe(1, accel_data_handler);
  
  // Subscribe to bluetooth connection updates and set initial state
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = connection_handler
  });
  connection_handler(connection_service_peek_pebble_app_connection());
  
  // Subscribe to battery state changes and initialize
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Batt Start");
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  //animate_beat_team_layer();

  
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
  // Destroy Window
  window_destroy(s_main_window);
  // Unsubscribe from accelerometer
  accel_data_service_unsubscribe();
  // Unsubscribe bluetooth
  bluetooth_connection_service_unsubscribe();
  // Unsubscribe battery
  battery_state_service_unsubscribe();
}

// App entry int
int main(void) {
  init();
  app_event_loop();
  deinit();
}