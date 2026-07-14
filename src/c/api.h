#pragma once
#include <pebble.h>

// Sends a request up to PebbleKit JS asking it to fetch the current
// score for the relevant team from the CFBD API. Mirrors the
// REQUEST_WEATHER pattern in timekeeping.c. No-op if api/scoreDisplayBool
// are off, or if no API key has been configured.
void api_request_score(void);

// Called from communication.c's inbox_received_callback when a
// score response arrives from JS. Parses the trimmed score fields
// out of the AppMessage dictionary.
void api_score_callback(DictionaryIterator *iterator, void *context);