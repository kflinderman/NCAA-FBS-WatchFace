#include "timekeeping.h"
#include "globals.h"
#include "health.h"

// Updates the time TextLayer
void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Time handler
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);

  // Month/day handler
  static char s_date_buffer[10];
  #ifdef PBL_ROUND
    strftime(s_date_buffer, sizeof(s_date_buffer), "%b %e", tick_time);
  #else
    strftime(s_date_buffer, sizeof(s_date_buffer), "%b\n%e", tick_time);
  #endif
  text_layer_set_text(s_date_layer, s_date_buffer);

  // Convert current time to HHMM format
  // tick_time->tm_hour is 0-23
  current_time_integer = (tick_time->tm_hour * 100) + tick_time->tm_min;

  #if defined(PBL_HEALTH)
    health_handler();
  #endif
}

// Handles time ticks (every minute)
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get weather update every 30 minutes
  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
    app_message_outbox_send();
  }
}
