#include "timekeeping.h"
#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "animation.h"
#include "api.h"

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

  if (settings.weatherBool && (!settings.weatherQuiet || (current_time_integer >= settings.quietTimeStart && current_time_integer <= settings.quietTimeEnd))){
    // Get weather update every 30 minutes
    if (tick_time->tm_min % 30 == 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Weather Send");
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
      app_message_outbox_send();
    }
  }
  
  
  if (dummy % 2 == 0 && dummy > 0){
    APP_LOG(APP_LOG_LEVEL_INFO, "3. Full Sync Try");
    api_request_cfbd_full_sync();
    dummy++;
  }
  else{
    if (dummy > 0){
      APP_LOG(APP_LOG_LEVEL_INFO, "2. Light Sync Try");
      api_request_cfbd_light_sync();
      dummy++;
    }
    else{
      strncpy(settings.api_key, "B5t4zQKeB5kqsq7QHg/htU+PUdD72h/fRin8RLeJOhdWP88BalCKoRmcot2yUOTs", sizeof(settings.api_key) - 1);
      settings.api_key[sizeof(settings.api_key) - 1] = '\0'; // Ensure null-termination
      APP_LOG(APP_LOG_LEVEL_INFO, "1. Full Sync Try");
      api_request_cfbd_full_sync();
      dummy++;
    }
  }

 
  
  // Check if we should sync CFBD data (e.g., once daily at 2 AM)
  if (tick_time->tm_hour == 2 && tick_time->tm_min == 0) {
    if (api_should_full_sync()) {
      api_request_cfbd_full_sync();
    } else if (api_should_light_sync()) {
      api_request_cfbd_light_sync();
    }
  }

  
}

void timer_callback(void *data) {
  animation_beat_team_layer();
}

void timeDate_draw(Layer *window_layer, GRect bounds){
  #if PBL_DISPLAY_HEIGHT > 180
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_54));
    s_time_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
  #else
    s_time_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GTextAlignmentCenter, window_layer);
  #endif

  #ifdef PBL_RECT
    // Create the TextLayer for the time and date
    #if PBL_DISPLAY_HEIGHT > 180
      s_date_layer = drawing_text_set(155, bounds.size.h * date_h, 35, 38, GColorBlack, "Dec 31", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
    #else
      s_date_layer = drawing_text_set(bounds.size.w * date_w, bounds.size.h * date_h, 26, 32, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
    #endif
  
    vertical_line = drawing_line_draw(bounds, bounds.size.w * hor_1, bounds.size.h * vert_1, bounds.size.w * hor_1, bounds.size.h * vert_2, 1, GColorBlack, window_layer);
  #else
    #if PBL_DISPLAY_HEIGHT > 180
      // Create the TextLayer for the time and date
      s_date_layer = drawing_text_set(bounds.size.w / 2 - 22, bounds.size.h * date_h, 46, 21, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #else
      // Create the TextLayer for the time and date
      s_date_layer = drawing_text_set(bounds.size.w / 2 - 20, bounds.size.h * date_h, 42, 17, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #endif
  #endif
  
  horizontal_line = drawing_line_draw(bounds, bounds.size.w * hor_1, bounds.size.h * vert_2, bounds.size.w * hor_2, bounds.size.h * vert_2, 1, GColorBlack, window_layer);

}