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
  uint8_t FavoriteTeam;
  uint8_t BeatTeam;
  uint16_t animationSensitivity;
  bool quietTimeBool;
  uint16_t quietTimeStart;
  uint16_t quietTimeEnd;
  uint8_t animationsBatt;
  uint8_t animationsCustom;
  bool healthQuiet;
  bool stepsBool;
  bool hrBool;
  bool stepsGoalBool;
  uint16_t stepsGoal;
  uint8_t hardcodeRival;
  bool bagBool;
  bool animationDelay;
  bool countdownBool;
  uint8_t countdownTime;
  uint32_t countdownCustomDate;
  uint16_t countdownCustomTime;
  uint8_t countdownDisplay;
  bool api;
  char api_key[128];
  bool api_quiet;
  bool scoreDisplayBool;
  uint16_t scoreUpdate;
  uint8_t scoreLocation;
  bool opponentBool;
  uint8_t opponentSelect;
  uint16_t customOpponent;
  bool weatherBool;
  bool weatherQuiet;
  bool weatherUnits;
  bool rankingBool;
  bool winBool;
  bool confBool;
  bool bowlBool;
  bool champBool;
  CFBDState cfbd;
} ClaySettings;

extern ClaySettings settings;

/*******************************************
 * UI element pointers (owned/created in main.c,
 * used by other modules)
 *******************************************/
extern Window *s_main_window;

typedef enum {
  LAYER_KIND_TEXT,
  LAYER_KIND_BITMAP,
  LAYER_KIND_GBITMAP,
  LAYER_KIND_GENERIC
} LayerKind;

typedef enum {
  TEXT_LAYER_TIME,
  TEXT_LAYER_DATE,
  TEXT_LAYER_BEAT,
  TEXT_LAYER_WEATHER,
  TEXT_LAYER_CONDITIONS,
  TEXT_LAYER_HOME,
  TEXT_LAYER_AWAY,
  TEXT_LAYER_DAY,
  TEXT_LAYER_HOUR,
  TEXT_LAYER_COUNTDOWN,
  TEXT_LAYER_SCORE,
  TEXT_LAYER_RANK,
  #if defined(PBL_HEALTH)
  TEXT_LAYER_HR,
  TEXT_LAYER_STEP,
  TEXT_LAYER_TD,
  #endif
  NUM_TEXT_LAYERS
} TextLayerID;
extern TextLayer* s_text_layers[NUM_TEXT_LAYERS];

#if defined(PBL_HEALTH)
extern Layer *hr_icon, *step_ladder;
extern bool noHR;
#endif

typedef enum {
  GBITMAP_LAYER_LOGO,
  GBITMAP_LAYER_BEAT_TEAM,
  GBITMAP_LAYER_BT,
  GBITMAP_LAYER_BATT_CRG,
  GBITMAP_LAYER_BATT_EMPTY,
  GBITMAP_LAYER_BATT_LOW,
  GBITMAP_LAYER_BAG,
  GBITMAP_LAYER_API_LOW,
  GBITMAP_LAYER_API_EMPTY,
  GBITMAP_LAYER_WIN,
  GBITMAP_LAYER_BOWL,
  GBITMAP_LAYER_CHAMP,
  #if defined(PBL_HEALTH)
  GBITMAP_LAYER_FOOTBALL,
  #endif
  NUM_GBITMAP_LAYERS
} GBitmapLayerID;
extern GBitmap* s_gbitmap_layers[NUM_GBITMAP_LAYERS];

typedef enum {
  BITMAP_LAYER_LOGO,
  BITMAP_LAYER_BEAT_TEAM,
  BITMAP_LAYER_BT,
  BITMAP_LAYER_BATT,
  BITMAP_LAYER_BAG,
  BITMAP_LAYER_BAGB,
  BITMAP_LAYER_API,
  BITMAP_LAYER_WIN,
  BITMAP_LAYER_TROPHY,
  #if defined(PBL_HEALTH)
  BITMAP_LAYER_FOOTBALL,
  #endif
  NUM_BITMAP_LAYERS
} BitmapLayerID;
extern BitmapLayer* s_bitmap_layers[NUM_BITMAP_LAYERS];

typedef enum {
  LAYER_RECT,
  LAYER_HOR,
  LAYER_BEAT_TEAM,
  LAYER_BEAT_RECT,
  LAYER_RANK_RECT,
  #ifdef PBL_RECT
  LAYER_VERT,
  #endif
  NUM_GENERIC_LAYERS
} LayerID;
extern Layer* s_layers[NUM_GENERIC_LAYERS];

/*
typedef enum {
  GFONT_FONT,
  GFONT_WICON,
  NUM_GFONT
} GFontID;
extern GFont s_gfont[NUM_GFONT];
*/
extern GFont s_font, s_wIcon;

/*******************************************
 * Sensor / state variables
 *******************************************/
extern int16_t s_prev_y;
extern bool s_bt_connected;
extern bool s_animation;
extern bool after_time;
extern bool gametime;
extern BatteryChargeState s_battery_state;
extern bool s_bt_history;
extern int16_t s_batt_history;
extern int32_t current_time_integer;
extern int16_t temperatureValue;
extern int16_t conditionValue;

/*******************************************
	* CFBD score state — NOT persisted (live
	* data refreshed from the API, unlike
	* ClaySettings which is user configuration)
 *******************************************/
//extern char scoreHomeTeam[32];
//extern char scoreAwayTeam[32];
//extern int16_t scoreHomePoints;
//extern int16_t scoreAwayPoints;
//extern bool scoreCompleted;
//extern bool scoreValid; // false until the first successful response arrives

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
void globals_prv_default_settings(void);
void globals_prv_save_settings(void);
void globals_prv_load_settings(void);
void globals_prv_update_display(void);
