// src/c/api_cfbd.h
#pragma once
#include <pebble.h>

// Request full CFBD sync (heavy operation, calendar + games + records + rankings)
// Typically called on app launch or manual user refresh
void api_request_cfbd_full_sync(void);

// Request light CFBD sync (just this week's games + rankings)
// Lightweight operation for daily refreshes
void api_request_cfbd_light_sync(void);

// Process incoming CFBD data chunks from JS
void api_cfbd_callback(DictionaryIterator *iterator, void *context);

// Helper: check if we should do a full sync (based on timestamp + API call budget)
bool api_should_full_sync(void);

// Helper: check if light sync is needed (e.g., weekly refresh)
bool api_should_light_sync(void);