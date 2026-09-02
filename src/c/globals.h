#pragma once
#include <pebble.h>
#include "teams.h"


/*********************/
/* DEBUG Variables   */
/*********************/

//#define TESTING

#ifndef PBL_PLATFORM_APLITE
//#define DEBUG
#endif




/*******************************************/
/* ClaySettings — persisted user settings  */
/*******************************************/
#define SETTINGS_KEY 1
#define NUM_TEAMS 154
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

extern Window *s_main_window;


/*********************/
/* Layer Arrays      */
/*********************/
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

typedef enum {
  GBITMAP_LAYER_LOGO,
  GBITMAP_LAYER_BT,
  GBITMAP_LAYER_BATT,
  GBITMAP_LAYER_API,
  #ifndef PBL_PLATFORM_APLITE
  GBITMAP_LAYER_BEAT_TEAM,
  GBITMAP_LAYER_BAG,
  GBITMAP_LAYER_WIN,
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


/***************************************/
/* Sensor / state variables            */
/***************************************/

#if defined(PBL_HEALTH)
extern Layer *hr_icon, *step_ladder;
extern bool noHR;
#endif
extern GFont s_font, s_wIcon;
extern char s_time_text[6], s_countdown_text[6], s_score_text[6], s_home_text[5], s_away_text[5], s_day_text[5], s_hour_text[5];
extern int16_t s_prev_y;
extern bool s_bt_connected, s_animation, s_favorite_team_data_missing, after_time, gametime, s_bt_history;
extern BatteryChargeState s_battery_state;
extern uint8_t beat_spot, beat_primary;
extern int16_t s_batt_history;
extern int32_t current_time_integer;
#ifndef PBL_PLATFORM_APLITE
extern int16_t temperatureValue;
extern int16_t conditionValue;
#endif

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
    #define VERT_3    720
    #define VERT_4    850
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
    #define VERT_3    710
    #define VERT_4    860
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
    #define VERT_3    790
    #define VERT_4    940
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
    #define VERT_3    770
    #define VERT_4    920
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

/************************/
/* Structural items     */
/************************/
typedef struct {
  uint16_t x1, y1, x2, y2, width;
  GColor color;
} LinePoints;

typedef struct {
  GColor fill_color;
} RoundRectData;


/*****************/
/* Functions     */
/*****************/
void globals_what2show(const char *leftText, const char *rightText, const char *mainText, bool extras, bool Ishow);
void globals_prv_default_settings(void);
void globals_prv_save_settings(void);
void globals_prv_load_settings(void);
void globals_prv_update_display(void);
void globals_prv_save_team_data(void);
void globals_prv_load_team_data(void);
