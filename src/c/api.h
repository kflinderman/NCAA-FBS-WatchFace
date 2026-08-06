#pragma once
#include <pebble.h>

typedef enum {
  CFBD_TEAM_DATA_GAMES = 0,
  CFBD_TEAM_DATA_RECORDS = 1
} CFBDTeamDataType;

// Request full CFBD sync (calendar only: year, next season kickoff)
// Typically called on app launch or manual user refresh
void api_request_cfbd_full_sync(void);

// Request light CFBD sync (this week's games/records/rankings, applied to
// API_DATA[] one team at a time via a follow-up per-team exchange)
// Lightweight operation for weekly refreshes
void api_request_cfbd_light_sync(void);

// Process incoming CFBD messages from JS (calendar, light-sync-ready
// signal, and per-team data responses)
void api_cfbd_callback(DictionaryIterator *iterator, void *context);

// Helper: check if we should do a full sync (based on timestamp + API call budget)
bool api_should_full_sync(void);

// Helper: check if light sync is needed (e.g., weekly refresh)
bool api_should_light_sync(void);

// Percent of this month's CFBD call budget used so far (0-100). 0 if the
// limit isn't known yet (no full sync completed since app install).
uint8_t api_calls_percent_used(void);

// True once usage crosses the warning threshold (see
// CFBD_API_CALLS_WARNING_PERCENT in api.c) - for a battery-style "nearing
// limit" UI indicator.
bool api_calls_nearing_limit(void);

void api_score_display(void);

void api_icon_draw(Layer *window_layer, GRect bounds);