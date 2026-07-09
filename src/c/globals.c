#include "globals.h"
#include "health.h"
#include "drawing.h"

/*******************************************
 * Definitions for all extern globals
 *******************************************/
ClaySettings settings;

Window *s_main_window;
TextLayer *s_time_layer, *s_date_layer, *s_beat_layer;

#if defined(PBL_HEALTH)
TextLayer *s_hr_layer, *s_step_layer, *s_td_layer;
GBitmap *s_football_bitmap;
BitmapLayer *s_football_layer;
Layer *hr_icon, *step_ladder;
bool noHR = true;
#endif

GBitmap *s_logo_bitmap, *s_beat_team_bitmap, *s_bt_bitmap, *s_batt_crg_bitmap, *s_batt_empty_bitmap, *s_batt_low_bitmap, *s_bag_bitmap;
BitmapLayer *s_logo_layer, *s_beat_team_layer, *s_bt_layer, *s_batt_layer, *s_bag_layerf, *s_bag_layerb;
Layer *rect_layer, *horizontal_line, *beat_team_layer, *rect_beat_layer;
#ifdef PBL_RECT
  Layer *vertical_line;
#endif

GFont s_font;

int16_t s_prev_y = 0;
bool s_bt_connected = false;
bool s_animation = false;
BatteryChargeState s_battery_state;
bool s_bt_history = true;
int16_t s_batt_history = 0;
int32_t current_time_integer;

uint16_t beat_spot;
uint16_t beat_primary;

#ifdef PBL_ROUND
  float rect_h = 0.66;
  float date_h = 0.84;
  float vert_2 = 0.93;
  float hor_1 = 0.45;
  float hor_2 = 0.55;
  float time_h = 0.62;
  #if PBL_DISPLAY_HEIGHT > 180
    uint16_t time_w = 75;
    uint16_t time_x = 150;
    uint16_t time_y = 70;
    uint16_t icon_bump = 10;
    uint16_t hr_thick = 2;
    bool hr_w = 0;
    uint16_t stepx1 = 16;
    uint16_t stepx2 = 95;
    uint16_t stepy = 50;
  #else
    uint16_t time_w = 60;
    uint16_t time_x = 120;
    uint16_t time_y = 50;
    uint16_t icon_bump = 8;
    uint16_t hr_thick = 1;
    bool hr_w = 1;
    uint16_t stepx1 = 12;
    uint16_t stepx2 = 67;
    uint16_t stepy = 37;
  #endif
#else
  float rect_h = 0.72;
  float date_w = 0.81;
  uint16_t icon_bump = 5;
  float time_h = 0.70;
  #if PBL_DISPLAY_HEIGHT > 180
    float date_h = 0.72;
    uint16_t time_w = 92;
    uint16_t time_x = 160;
    uint16_t time_y = 70;
    float vert_1 = 0.82;
    float vert_2 = 0.90;
    float hor_1 = 0.83;
    float hor_2 = 0.92;
    uint16_t hr_thick = 2;
    bool hr_w = 0;
    uint16_t stepx1 = 16;
    uint16_t stepx2 = 95;
    uint16_t stepy = 50;
  #else
    float date_h = 0.74;
    uint16_t time_w = 72;
    uint16_t time_x = 120;
    uint16_t time_y = 50;
    float vert_1 = 0.85;
    float vert_2 = 0.93;
    float hor_1 = 0.86;
    float hor_2 = 0.97;
    uint16_t hr_thick = 1;
    bool hr_w = 1;
    uint16_t stepx1 = 12;
    uint16_t stepx2 = 67;
    uint16_t stepy = 37;
  #endif
#endif

#if PBL_DISPLAY_HEIGHT > 180
  uint16_t bitmap_size = 160;
#else
  uint16_t bitmap_size = 115;
#endif

void prv_default_settings() {
  settings.DisconnectVibration = 3;
  settings.ReconnectVibration = 1;
  settings.LowBatteryPercent = 30;
  settings.LowBatteryVibration = 1;
  settings.EmptyBatteryPercent = 10;
  settings.EmptyBatteryVibration = 2;
  settings.DisplayTeam = 0;
  settings.FavoriteTeam = 1;
  settings.BeatTeam = 0;
  settings.animationSensitivity = 1200;
  settings.quietTimeBool = false;
  settings.quietTimeStart = 2330;
  settings.quietTimeEnd = 630;
  settings.animationsBatt = 0;
  settings.animationsCustom = 30;
  settings.stepsBool = false;
  settings.hrBool = false;
  settings.stepsGoalBool = false;
  settings.stepsGoal = 10000;
  settings.hardcodeRival = false;
  settings.donate = false;
  settings.bagBool = false;
}

void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void prv_load_settings() {
  prv_default_settings();
  // Only load if the saved struct matches current size
  // (protects against corrupt data or struct layout changes)
  if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == sizeof(ClaySettings)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
  // Bounds-check team indices before they're used to index TEAMS[]
  if (settings.FavoriteTeam >= NUM_TEAMS) settings.FavoriteTeam = 108;
  if (settings.BeatTeam >= NUM_TEAMS) settings.BeatTeam = 26;
}

void prv_update_display() {
  // Only update if window exists
  if (!s_main_window) return;

  // Update beat_primary if DisplayTeam changed
  beat_primary = settings.DisplayTeam;

  // Update beat team layer position
  if (rect_beat_layer) {
    GRect new_frame = GRect(beat_spot, -40 + beat_primary, 44, 40);
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
  
  if (settings.hardcodeRival){
    settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
  }

  if (settings.DisplayTeam > 1) {
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.BeatTeam].color});
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);

    #if defined(PBL_HEALTH)
    text_layer_set_text_color(s_hr_layer, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    text_layer_set_text_color(s_step_layer, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    
    multiline_set_all_colors(hr_icon, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    multiline_set_all_colors(step_ladder, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    #endif
    
    if (beat_team_layer) {
      RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);
      if (beat_data) {
        beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color};
        layer_mark_dirty(beat_team_layer);
      }
    }
  } else {
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.FavoriteTeam].color});
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
    
    #if defined(PBL_HEALTH)
    text_layer_set_text_color(s_hr_layer, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    text_layer_set_text_color(s_step_layer, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    
    multiline_set_all_colors(hr_icon, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    multiline_set_all_colors(step_ladder, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    #endif
    
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
  
  if(settings.bagBool) {
      // Prevent a memory leak by only creating the bitmap if it doesn't already exist
      if(!s_bag_bitmap) {
          s_bag_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAG);
      }
  
      // Set the bitmap on the active layer, and explicitly clear it from the inactive layer
      if (settings.DisplayTeam > 1) {
          bitmap_layer_set_bitmap(s_bag_layerb, s_bag_bitmap);
          bitmap_layer_set_bitmap(s_bag_layerf, NULL); 
      } else {
          bitmap_layer_set_bitmap(s_bag_layerf, s_bag_bitmap);
          bitmap_layer_set_bitmap(s_bag_layerb, NULL); 
      }
  } else {
      // Clear the bitmap from BOTH layers before destroying it
      bitmap_layer_set_bitmap(s_bag_layerb, NULL);
      bitmap_layer_set_bitmap(s_bag_layerf, NULL);
  
      // Safely destroy and nullify the pointer
      if(s_bag_bitmap) {
          gbitmap_destroy(s_bag_bitmap);
          s_bag_bitmap = NULL; 
      }
  }
  
  #if defined(PBL_HEALTH)
    health_handler();
  #endif
}
