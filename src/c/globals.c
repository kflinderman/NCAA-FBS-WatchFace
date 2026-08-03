#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "weather.h"
#include "display.h"
#include "timekeeping.h"
#include "animation.h"
#include "api.h"

/*******************************************
 * Definitions for all extern globals
 *******************************************/
ClaySettings settings;

Window *s_main_window;
TextLayer *s_time_layer, *s_date_layer, *s_beat_layer, *s_weather_layer, *s_conditions_layer, *s_home_layer, *s_away_layer, *s_day_layer, *s_hour_layer, *s_countdown_layer, *s_score_layer;
int dummy = 0;

#if defined(PBL_HEALTH)
TextLayer *s_hr_layer, *s_step_layer, *s_td_layer;
GBitmap *s_football_bitmap;
BitmapLayer *s_football_layer;
Layer *hr_icon, *step_ladder;
bool noHR = true;
#endif

GBitmap *s_logo_bitmap, *s_beat_team_bitmap, *s_bt_bitmap, *s_batt_crg_bitmap, *s_batt_empty_bitmap, *s_batt_low_bitmap, *s_bag_bitmap, *s_api_low_bitmap, *s_api_empty_bitmap;
BitmapLayer *s_logo_layer, *s_beat_team_layer, *s_bt_layer, *s_batt_layer, *s_bag_layerf, *s_bag_layerb, *s_api_layer;
Layer *rect_layer, *horizontal_line, *beat_team_layer, *rect_beat_layer;
#ifdef PBL_RECT
  Layer *vertical_line;
#endif

GFont s_font, s_wIcon;

int16_t s_prev_y = 0;
bool s_bt_connected = false;
bool s_animation = false;
bool after_time = true;
BatteryChargeState s_battery_state;
bool s_bt_history = true;
int16_t s_batt_history = 0;
int32_t current_time_integer;
int16_t temperatureValue = 0;
int16_t conditionValue = 0;

char scoreHomeTeam[32] = "";
char scoreAwayTeam[32] = "";
int16_t scoreHomePoints = 0;
int16_t scoreAwayPoints = 0;
bool scoreCompleted = false;
bool scoreValid = false;

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
    uint16_t icon_bump = 9; //10;
    uint16_t hr_thick = 2;
    bool hr_w = 0;
    uint16_t stepx1 = 16;
    uint16_t stepx2 = 95;
    uint16_t stepy = 50;
  #else
    uint16_t time_w = 60;
    uint16_t time_x = 120;
    uint16_t time_y = 50;
    uint16_t icon_bump = 7;
    uint16_t hr_thick = 1;
    bool hr_w = 1;
    uint16_t stepx1 = 12;
    uint16_t stepx2 = 67;
    uint16_t stepy = 37;
  #endif
#else
  float rect_h = 0.72;
  float date_w = 0.81;
  //uint16_t icon_bump = 4;//5
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
    uint16_t icon_bump = 1;
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
    uint16_t icon_bump = 4;
  #endif
#endif

#if PBL_DISPLAY_HEIGHT > 180
  uint16_t bitmap_size = 160;
#else
  uint16_t bitmap_size = 115;
#endif

void globals_prv_default_settings() {
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
  settings.hardcodeRival = 0;
  settings.donate = false;
  settings.bagBool = false;
  settings.animationDelay = false;
  settings.countdownBool = false;
  settings.countdownTime = 0;
  settings.countdownCustomDate = 0;
  settings.countdownCustomTime = 0;
  settings.countdownDisplay = 1;
  settings.api = false;
  settings.api_quiet = false;
  settings.scoreDisplayBool = false;
  settings.scoreUpdate = 5;
  settings.scoreLocation = 1;
  settings.opponentBool = false;
  settings.opponentSelect = 0;
  settings.customOpponent = 0;
  settings.weatherBool = false;
  settings.weatherQuiet = false;
  settings.weatherUnits = 0;
  settings.rankingBool = false;
  settings.winBool = false;
  settings.confBool = false;
  settings.bowlBool = false;
  settings.champBool = false;
  settings.cfbd.next_season_first_game_ts = 0;
  settings.cfbd.current_season_year = 0;
  settings.cfbd.last_full_sync_ts = 0;
  settings.cfbd.last_light_sync_ts = 0;
  settings.cfbd.api_calls_this_month = 0;
  settings.cfbd.api_calls_monthly_limit = 0; // unknown until first full sync reports it
  settings.cfbd.api_data_valid = false;
}

void globals_prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void globals_prv_load_settings() {
  globals_prv_default_settings();
  // Only load if the saved struct matches current size
  // (protects against corrupt data or struct layout changes)
  if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == sizeof(ClaySettings)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
  // Bounds-check team indices before they're used to index TEAMS[]
  if (settings.FavoriteTeam >= NUM_TEAMS) settings.FavoriteTeam = 1;
  if (settings.BeatTeam >= NUM_TEAMS) settings.BeatTeam = 0;
}

void globals_prv_update_display() {
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- UPDATE DISPLAY --------");
  
  // Only update if window exists
  if (!s_main_window) return;
  
  // API Check if empty
  APP_LOG(APP_LOG_LEVEL_INFO, "API Sync");
  if (api_should_full_sync()) {
    api_request_cfbd_full_sync();
  }
  
  // Update beat_primary if DisplayTeam changed
  beat_primary = settings.DisplayTeam;

  // Update beat team layer position
  APP_LOG(APP_LOG_LEVEL_INFO, "BEAT Location");
  if (rect_beat_layer) {
    GRect new_frame = GRect(beat_spot, -40 + beat_primary, 44, 40);
    layer_set_frame(rect_beat_layer, new_frame);
    layer_mark_dirty(rect_beat_layer);
  }

  // Update favorite team logo
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Favorite Team");
  if (s_logo_bitmap) {
    gbitmap_destroy(s_logo_bitmap);
  }
  if (s_beat_team_bitmap) {
    gbitmap_destroy(s_beat_team_bitmap);
  }
  
  if (settings.hardcodeRival == 1){
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - Rival");
    settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
  }
  else if (settings.hardcodeRival == 2){
    if (API_DATA[settings.FavoriteTeam].vs_id == -1){
      if (settings.opponentSelect == 1){
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (Rival)");
        settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
      }
      else if (settings.opponentSelect == 2){
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (Custom)");
        settings.BeatTeam = settings.customOpponent;
      }
      else{
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (NCAA)");
        settings.BeatTeam = 1; //77
      }
    }
    else{
      APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - API");
      settings.BeatTeam = API_DATA[settings.FavoriteTeam].vs_id;
    }
  }

  if (settings.DisplayTeam > 1) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Display - BEAT");
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.BeatTeam].color});
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
    display_setupBag((GColor){.argb = TEAMS[settings.BeatTeam].icon_color});

    #if defined(PBL_HEALTH)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Health Colors");
    text_layer_set_text_color(s_hr_layer, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    text_layer_set_text_color(s_step_layer, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    
    drawing_multiline_set_all_colors(hr_icon, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    drawing_multiline_set_all_colors(step_ladder, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    #endif
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Colors");
    text_layer_set_text_color(s_weather_layer, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    text_layer_set_text_color(s_conditions_layer, (GColor){.argb = TEAMS[settings.BeatTeam].icon_color});
    
    if (beat_team_layer) {
      RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);
      if (beat_data) {
        beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color};
        layer_mark_dirty(beat_team_layer);
      }
    }
  } 
  else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Display - Favorite");
    window_set_background_color(s_main_window, (GColor){.argb = TEAMS[settings.FavoriteTeam].color});
    s_logo_bitmap = gbitmap_create_with_resource(TEAMS[settings.FavoriteTeam].logo_res_id);
    s_beat_team_bitmap = gbitmap_create_with_resource(TEAMS[settings.BeatTeam].logo_res_id);
    display_setupBag((GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    
    #if defined(PBL_HEALTH)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Health Colors");
    text_layer_set_text_color(s_hr_layer, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    text_layer_set_text_color(s_step_layer, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    
    drawing_multiline_set_all_colors(hr_icon, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    drawing_multiline_set_all_colors(step_ladder, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    #endif
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Colors");
    text_layer_set_text_color(s_weather_layer, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    text_layer_set_text_color(s_conditions_layer, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
    
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
  
  bool timeTrue = true;
  if (settings.countdownBool){
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Countdown");
    after_time = timekeeping_countdown();
    if (((!after_time && settings.scoreDisplayBool) || !settings.scoreDisplayBool) && settings.countdownDisplay != 1){
      animation_hide_text(false, true, true);
      timeTrue = false;
    }
  }
  
  if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time))){
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Score");
    if (api_should_light_sync() && after_time) {
      api_request_cfbd_light_sync();
    }
    api_score_display();
    if (settings.scoreLocation != 1){
      animation_hide_text(true, false, true);
      timeTrue = false;
    }
  }
  
  if (timeTrue){
    animation_hide_text(true, true, false);
  }
  
  #if defined(PBL_HEALTH)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Health");
    health_handler();
  #endif
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather");
  weather_update();
}
