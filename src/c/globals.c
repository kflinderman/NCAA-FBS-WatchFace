#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "weather.h"
#include "display.h"
#include "timekeeping.h"
#include "animation.h"
#include "api.h"


#ifdef PBL_PLATFORM_APLITE

#else
#define DEBUG
#endif


/*******************************************
 * Definitions for all extern globals
 *******************************************/
ClaySettings settings;

Window *s_main_window;
TextLayer* s_text_layers[NUM_TEXT_LAYERS];

#if defined(PBL_HEALTH)
Layer *hr_icon, *step_ladder;
bool noHR = true;
#endif

GBitmap* s_gbitmap_layers[NUM_GBITMAP_LAYERS];
BitmapLayer* s_bitmap_layers[NUM_BITMAP_LAYERS];
Layer* s_layers[NUM_GENERIC_LAYERS];

//GFont s_gfont[NUM_GFONT];
GFont s_font, s_wIcon;

int16_t s_prev_y = 0;
bool s_bt_connected = false;
bool s_animation = false;
bool after_time = true;
bool gametime = false;
BatteryChargeState s_battery_state;
bool s_bt_history = true;
int16_t s_batt_history = 0;
int32_t current_time_integer;
int16_t temperatureValue = 0;
int16_t conditionValue = 0;

//char scoreHomeTeam[32] = "";
//char scoreAwayTeam[32] = "";
//int16_t scoreHomePoints = 0;
//int16_t scoreAwayPoints = 0;
//bool scoreCompleted = false;
//bool scoreValid = false;

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
  settings.healthQuiet = false;
  settings.stepsBool = false;
  settings.hrBool = false;
  settings.stepsGoalBool = false;
  settings.stepsGoal = 10000;
  settings.hardcodeRival = 0;
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
  settings.watchUpdate = 1;
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
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- UPDATE DISPLAY --------");
  #endif

  // Only update if window exists
  if (!s_main_window) return;

  // API Check if empty
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "API Sync");
  #endif
  if (api_should_full_sync()) {
    api_request_cfbd_full_sync();
  }

  // Update beat_primary if DisplayTeam changed
  beat_primary = settings.DisplayTeam;

  // Update beat team layer position
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "BEAT Location");
  #endif
  if (s_layers[LAYER_BEAT_RECT]) {
    GRect new_frame = GRect(beat_spot, -40 + beat_primary, 44, 40);
    layer_set_frame(s_layers[LAYER_BEAT_RECT], new_frame);
    layer_mark_dirty(s_layers[LAYER_BEAT_RECT]);
  }

  // Update favorite team logo
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Favorite Team");
  #endif
  if (s_gbitmap_layers[GBITMAP_LAYER_LOGO]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_LOGO]);
  }
  if (s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]);
  }

  if (settings.hardcodeRival == 1){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - Rival");
    #endif
    settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
  }
  else if (settings.hardcodeRival == 2){
    if (API_DATA[settings.FavoriteTeam].vs_id == -1){
      if (settings.opponentSelect == 1){
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (Rival)");
        #endif
        settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
      }
      else if (settings.opponentSelect == 2){
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (Custom)");
        #endif
        settings.BeatTeam = settings.customOpponent;
      }
      else{
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (NCAA)");
        #endif
        settings.BeatTeam = 1; //77
      }
    }
    else{
      #if defined(DEBUG)
      APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - API");
      #endif
      settings.BeatTeam = API_DATA[settings.FavoriteTeam].vs_id;
    }
  }

  // 1. Resolve primary (displayed) and secondary team indices
  uint8_t primary_idx   = (settings.DisplayTeam > 1) ? settings.BeatTeam : settings.FavoriteTeam;
  uint8_t secondary_idx = (settings.DisplayTeam > 1) ? settings.FavoriteTeam : settings.BeatTeam;

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Display - %s", (settings.DisplayTeam > 1) ? "BEAT" : "Favorite");
  #endif

  // 2. Main Window Background
  window_set_background_color(s_main_window, (GColor){.argb = TEAMS[primary_idx].color});

  // 3. Update Logos
  /*
  if (s_gbitmap_layers[GBITMAP_LAYER_LOGO]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_LOGO]);
  }
  if (s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]);
  }
  */
  s_gbitmap_layers[GBITMAP_LAYER_LOGO]      = gbitmap_create_with_resource(TEAMS[primary_idx].logo_res_id);
  s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM] = gbitmap_create_with_resource(TEAMS[secondary_idx].logo_res_id);

  // 4. Setup Accent / Icon Colors
  GColor primary_icon_color = (GColor){.argb = TEAMS[primary_idx].icon_color};
  display_setupBag(primary_icon_color);

  #if defined(PBL_HEALTH)
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Health Colors");
  #endif
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_HR], primary_icon_color);
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_STEP], primary_icon_color);

  drawing_multiline_set_all_colors(hr_icon, primary_icon_color);
  drawing_multiline_set_all_colors(step_ladder, primary_icon_color);
  #endif

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Colors");
  #endif
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_WEATHER], primary_icon_color);
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_CONDITIONS], primary_icon_color);

  // 5. Update Secondary Badge Fill
  if (s_layers[LAYER_BEAT_TEAM]) {
    RoundRectData *beat_data = (RoundRectData *)layer_get_data(s_layers[LAYER_BEAT_TEAM]);
    if (beat_data) {
      beat_data->fill_color = (GColor){.argb = TEAMS[secondary_idx].color};
      layer_mark_dirty(s_layers[LAYER_BEAT_TEAM]);
    }
  }

  if (s_bitmap_layers[BITMAP_LAYER_LOGO]) {
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_LOGO], s_gbitmap_layers[GBITMAP_LAYER_LOGO]);
  }
  if (s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM]) {
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM], s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]);
  }


  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather");
  #endif
  weather_update();

  bool timeTrue = true;
  if (settings.countdownBool){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Countdown");
    #endif
    // Shares api_should_light_sync() with the score block below (rather
    // than its own separate !api_data_valid check) plus the in-flight
    // guard in api_request_cfbd_light_sync() - previously these two call
    // sites could both independently see "no valid data yet" on the same
    // update_display() pass and fire two REQUEST_CFBD_LIGHT_SYNC in a row.
    if (api_should_light_sync()) {
      api_request_cfbd_light_sync();
    }
    after_time = timekeeping_countdown();
    if ((!after_time  || !settings.scoreDisplayBool) && settings.countdownDisplay != 1){
      animation_hide_text(false, true, true);
      timeTrue = false;
    }
  }

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_DEBUG, "After Time: %d", after_time);
  #endif
  if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time))){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Score");
    #endif
    //OK This is the only time I'm not confident in after_time.  I also need to make a new variable for when it's checked the gametime since I should have that information.
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

  if (settings.rankingBool){
    if (API_DATA[settings.FavoriteTeam].ranking <= 25 && API_DATA[settings.FavoriteTeam].ranking > 0){

      static char s_rank_buffer[3];
      snprintf(s_rank_buffer, sizeof(s_rank_buffer), "#%d", API_DATA[settings.FavoriteTeam].ranking);
      text_layer_set_text(s_text_layers[TEXT_LAYER_RANK], s_rank_buffer);

      layer_set_hidden(s_layers[LAYER_RANK_RECT], false);
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_RANK]), false);
    }
    else{
      layer_set_hidden(s_layers[LAYER_RANK_RECT], true);
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_RANK]), true);
    }
  }

  if (settings.winBool){
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_WIN]), API_DATA[settings.FavoriteTeam].wins > 6);
  }


  if (settings.bowlBool){
    GBitmap *target_gbitmap = NULL;
    if(API_DATA[settings.FavoriteTeam].postseasonGames >= 1 && API_DATA[settings.FavoriteTeam].postseasonWins == 1){
      target_gbitmap = s_gbitmap_layers[GBITMAP_LAYER_BOWL];
    }
    else if (API_DATA[settings.FavoriteTeam].postseasonLosses < 1 && API_DATA[settings.FavoriteTeam].postseasonWins == 3){
      target_gbitmap = s_gbitmap_layers[GBITMAP_LAYER_CHAMP];
    }

    // Single Pass UI Update
    if (target_gbitmap != NULL) {
      bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_TROPHY], target_gbitmap);
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_TROPHY]), false);
    } else {
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_TROPHY]), true);
    }
  }

  #if defined(PBL_HEALTH)
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Health");
  #endif
  health_handler();
  #endif
}
