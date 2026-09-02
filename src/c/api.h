#pragma once
#include <pebble.h>

typedef enum {
  CFBD_TEAM_DATA_GAMES = 0,
  CFBD_TEAM_DATA_RECORDS = 1
} CFBDTeamDataType;

void api_request_cfbd_full_sync(void);
void api_format_2digits(char *buf, int val);
void api_request_cfbd_light_sync(void);
void api_cfbd_callback(DictionaryIterator *iterator, void *context);
bool api_should_full_sync(void);
bool api_should_light_sync(void);
uint8_t api_calls_percent_used(void);
bool api_calls_nearing_limit(void);
void api_score_display(void);
void api_icon_draw(Layer *window_layer, GRect bounds);