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
  #if defined(PBL_HEALTH)
  bool healthQuiet;
  bool stepsBool;
  bool hrBool;
  bool stepsGoalBool;
  uint16_t stepsGoal;
  #endif
  uint8_t hardcodeRival;
  bool animationDelay;
  bool countdownBool;
  uint8_t countdownTime;
  uint32_t countdownCustomDate;
  uint16_t countdownCustomTime;
  uint8_t countdownDisplay;
  bool api;
  char api_key[65];
  bool api_quiet;
  bool scoreDisplayBool;
  uint16_t scoreUpdate;
  uint8_t scoreLocation;
  bool opponentBool;
  uint8_t opponentSelect;
  uint16_t customOpponent;
  #ifndef PBL_PLATFORM_APLITE
  bool weatherBool;
  bool weatherQuiet;
  bool weatherUnits;
  bool bagBool;
  bool rankingBool;
  bool winBool;
  bool confBool;
  bool bowlBool;
  bool champBool;
  #endif
  uint8_t watchUpdate;
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
  TEXT_LAYER_HOME,
  TEXT_LAYER_AWAY,
  //TEXT_LAYER_DAY,
  //TEXT_LAYER_HOUR,
  //TEXT_LAYER_COUNTDOWN,
  //TEXT_LAYER_SCORE,
  #ifndef PBL_PLATFORM_APLITE
  TEXT_LAYER_WEATHER,
  TEXT_LAYER_CONDITIONS,
  TEXT_LAYER_RANK,
  #endif
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
  GBITMAP_LAYER_BT,
  // BATT was 3 separate GBitmaps (BATT_CRG/BATT_EMPTY/BATT_LOW) all kept
  // resident in RAM at once even though only one battery state is ever
  // shown - now a single slot that's created/destroyed as the state
  // changes (see sensor_battery_handler in sensors.c).
  GBITMAP_LAYER_BATT,
  // Same consolidation: API status was 2 GBitmaps (API_LOW/API_EMPTY),
  // now 1 slot swapped on demand (see api_update_status_indicator in api.c).
  GBITMAP_LAYER_API,
  #ifndef PBL_PLATFORM_APLITE
  GBITMAP_LAYER_BEAT_TEAM,
  GBITMAP_LAYER_BAG,
  GBITMAP_LAYER_WIN,
  // Same consolidation: the postseason badge was 2 GBitmaps
  // (BOWL/CHAMP), now 1 slot swapped on demand (see the bowlBool block
  // in globals.c).
  GBITMAP_LAYER_TROPHY,
  #endif
  #if defined(PBL_HEALTH)
  GBITMAP_LAYER_FOOTBALL,
  #endif
  NUM_GBITMAP_LAYERS
} GBitmapLayerID;
extern GBitmap* s_gbitmap_layers[NUM_GBITMAP_LAYERS];

typedef enum {
  BITMAP_LAYER_LOGO,
  BITMAP_LAYER_BT,
  BITMAP_LAYER_BATT,
  BITMAP_LAYER_API,
  #ifndef PBL_PLATFORM_APLITE
  BITMAP_LAYER_BEAT_TEAM,
  BITMAP_LAYER_BAG,
  BITMAP_LAYER_BAGB,
  BITMAP_LAYER_WIN,
  BITMAP_LAYER_TROPHY,
  #endif
  #if defined(PBL_HEALTH)
  BITMAP_LAYER_FOOTBALL,
  #endif
  NUM_BITMAP_LAYERS
} BitmapLayerID;
extern BitmapLayer* s_bitmap_layers[NUM_BITMAP_LAYERS];

typedef enum {
  LAYER_RECT,
  LAYER_HOR,
  LAYER_BEAT_RECT,
  #ifndef PBL_PLATFORM_APLITE
  LAYER_BEAT_TEAM,
  LAYER_RANK_RECT,
  #endif
  LAYER_SCORE_I,
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
#ifndef PBL_PLATFORM_APLITE
extern int16_t temperatureValue;
extern int16_t conditionValue;
#endif

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

extern uint8_t beat_spot, beat_primary;

/*******************************************
 * Layout constants — values differ by shape/size,
 * set once in main.c's init of these externs.
 * Declared here so animation.c (unobstructed-area
 * repositioning) and main.c (initial layout) agree.
 *******************************************/

#ifdef PBL_ROUND
  #define RECT_H    660
  #define DATE_H    840
  #define VERT_2    930
  #define HOR_1     450
  #define HOR_2     550
  #define TIME_H    620
  #if PBL_DISPLAY_HEIGHT > 180
    #define TIME_W    75
    #define TIME_X    155
    #define TIME_Y    70
    #define ICON_BUMP 9
    #define HR_THICK  2
    #define HR_W      0
    #define STEPX1    16
    #define STEPX2    95
    #define STEPY     50
  #else
    #define TIME_W    60
    #define TIME_X    120
    #define TIME_Y    50
    #define ICON_BUMP 7
    #define HR_THICK  1
    #define HR_W      1
    #define STEPX1    12
    #define STEPX2    67
    #define STEPY     37
  #endif
#else
  #define RECT_H    720
  #define DATE_W    810
  #define TIME_H    700
  #if PBL_DISPLAY_HEIGHT > 180
    #define DATE_H    720
    #define TIME_W    92
    #define TIME_X    160
    #define TIME_Y    70
    #define VERT_1    820
    #define VERT_2    900
    #define HOR_1     830
    #define HOR_2     920
    #define HR_THICK  2
    #define HR_W      0
    #define STEPX1    16
    #define STEPX2    95
    #define STEPY     50
    #define ICON_BUMP 1
  #else
    #define DATE_H    740
    #define TIME_W    72
    #define TIME_X    120
    #define TIME_Y    50
    #define VERT_1    850
    #define VERT_2    930
    #define HOR_1     860
    #define HOR_2     970
    #define HR_THICK  1
    #define HR_W      1
    #define STEPX1    12
    #define STEPX2    67
    #define STEPY     37
    #define ICON_BUMP 4
  #endif
#endif

#if PBL_DISPLAY_HEIGHT > 180
#define BITMAP_SIZE 160
#else
#define BITMAP_SIZE 115
#endif

/*
#ifdef PBL_ROUND
  extern const uint16_t rect_h, date_h, vert_2, hor_1, hor_2, time_h;
#else
  extern const uint16_t rect_h, date_w, time_h, date_h, vert_1, vert_2, hor_1, hor_2;
#endif
extern const uint8_t icon_bump, time_w, time_x, time_y, hr_thick, stepx1, stepx2, stepy, bitmap_size;
extern const bool hr_w;
*/

/*******************************************
 * Settings persistence + display application
 * (settings.c)
 *******************************************/
void globals_prv_default_settings(void);
void globals_prv_save_settings(void);
void globals_prv_load_settings(void);
void globals_prv_update_display(void);