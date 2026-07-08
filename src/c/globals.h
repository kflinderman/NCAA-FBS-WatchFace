#pragma once
#include <pebble.h>
#include "teams.h"
#include "structure.h"

#define SETTINGS_KEY 1
#define NUM_TEAMS 154

/*******************************************
 * ClaySettings — persisted user settings
 *******************************************/
typedef struct ClaySettings {
  uint8_t DisconnectVibration;
  uint8_t ReconnectVibration;
  uint8_t LowBatteryPercent;
  uint8_t LowBatteryVibration;
  uint8_t EmptyBatteryPercent;
  uint8_t EmptyBatteryVibration;
  uint8_t DisplayTeam;
  uint16_t FavoriteTeam;
  uint16_t BeatTeam;
  uint16_t animationSensitivity;
  bool quietTimeBool;
  uint16_t quietTimeStart;
  uint16_t quietTimeEnd;
  uint8_t animationsBatt;
  uint8_t animationsCustom;
  bool stepsBool;
  bool hrBool;
  bool stepsGoalBool;
  uint16_t stepsGoal;
  bool hardcodeRival;
  bool donate;
  bool bagBool;
} ClaySettings;

extern ClaySettings settings;

/*******************************************
 * UI element pointers (owned/created in main.c,
 * used by other modules)
 *******************************************/
extern Window *s_main_window;
extern TextLayer *s_time_layer, *s_date_layer, *s_beat_layer;

#if defined(PBL_HEALTH)
extern TextLayer *s_hr_layer;
extern TextLayer *s_step_layer;
extern GBitmap *s_football_bitmap;
extern BitmapLayer *s_football_layer;
//extern Layer *hr1, *hr2, *hr3, *hr4, *hr5;
//extern Layer *step1, *step2, *step3, *step4, *step5, *step6;
extern Layer *hr_icon;
extern Layer *step_ladder;
extern bool noHR;
#endif

GBitmap *s_logo_bitmap, *s_beat_team_bitmap, *s_bt_bitmap, *s_batt_crg_bitmap, *s_batt_empty_bitmap, *s_batt_low_bitmap, *s_bag_bitmap;
BitmapLayer *s_logo_layer, *s_beat_team_layer, *s_bt_layer, *s_batt_layer, *s_bag_layerf, *s_bag_layerb;;
Layer *rect_layer, *horizontal_line, *beat_team_layer, *rect_beat_layer;

#ifdef PBL_RECT
  extern Layer *vertical_line;
#endif

extern GFont s_font;

/*******************************************
 * Sensor / state variables
 *******************************************/
extern int16_t s_prev_y;
extern bool s_bt_connected;
extern bool s_animation;
extern BatteryChargeState s_battery_state;
extern bool s_bt_history;
extern int16_t s_batt_history;
extern int32_t current_time_integer;

extern uint16_t beat_spot;
extern uint16_t beat_primary;

/*******************************************
 * Layout constants — values differ by shape/size,
 * set once in main.c's init of these externs.
 * Declared here so animation.c (unobstructed-area
 * repositioning) and main.c (initial layout) agree.
 *******************************************/
#ifdef PBL_ROUND
  extern float rect_h, date_h, vert_2, hor_1, hor_2, time_h;
#else
  extern float rect_h, date_w, time_h, date_h, vert_1, vert_2, hor_1, hor_2;
#endif
extern uint16_t icon_bump;
extern uint16_t time_w, time_x, time_y;
extern uint16_t hr_thick;
extern bool hr_w;
extern uint16_t stepx1, stepx2, stepy;
extern uint16_t bitmap_size;

/*******************************************
 * Settings persistence + display application
 * (settings.c)
 *******************************************/
void prv_default_settings(void);
void prv_save_settings(void);
void prv_load_settings(void);
void prv_update_display(void);
