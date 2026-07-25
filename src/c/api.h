// src/c/api_cfbd.h
#pragma once
#include <pebble.h>

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