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
  
  if (dummy == 1){
    APP_LOG(APP_LOG_LEVEL_INFO, "2. Light Sync Try");
    api_request_cfbd_light_sync();
    dummy++;
  }
  else if (dummy == 0){
    strncpy(settings.api_key, "B5t4zQKeB5kqsq7QHg/htU+PUdD72h/fRin8RLeJOhdWP88BalCKoRmcot2yUOTs", sizeof(settings.api_key) - 1);
    settings.api_key[sizeof(settings.api_key) - 1] = '\0'; // Ensure null-termination
    APP_LOG(APP_LOG_LEVEL_INFO, "1. Full Sync Try");
    api_request_cfbd_full_sync();
    dummy++;
  }

  if (settings.countdownBool){
    after_time = timekeeping_countdown();
    if (((!after_time && settings.scoreDisplayBool) || !settings.scoreDisplayBool) && settings.countdownDisplay != 1){
      animation_hide_text(false, true, true);
    }
  }
  
  if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time))){
    api_score_display();
    if (settings.scoreLocation != 1){
      animation_hide_text(true, false, true);
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

bool timekeeping_countdown() {
  static char s_temp_buffer1[8], s_temp_buffer2[8], s_temp_buffer3[8];
  char s_temp_time1[8], s_temp_time2[8];
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  struct tm countdown_time = *tick_time;
  uint8_t firstplace, secondplace;
  time_t timestamp;

  uint8_t current_wday = tick_time->tm_wday;
  uint8_t days_to_time;
  bool after_time = false;
  
  
  // This still needs to be fixed, currently it all points to Sat noon
  if (settings.countdownTime == 1){
    days_to_time = 6 - current_wday;
    countdown_time.tm_mday += days_to_time;
    countdown_time.tm_hour = 12;
    countdown_time.tm_min = 0;
    countdown_time.tm_sec = 0;
    countdown_time.tm_isdst = -1;
  }
  //Custom time
  else if (settings.countdownTime == 2){
    if (settings.countdownCustomDate < 10){}
    if (settings.countdownCustomTime < 10){}
    
    //timestamp = (time_t)settings.countdownCustom;
    //countdown_time  = *localtime(&timestamp);
    
    
    days_to_time = 6 - current_wday;
    countdown_time.tm_mday += days_to_time;
    countdown_time.tm_hour = 12;
    countdown_time.tm_min = 0;
    countdown_time.tm_sec = 0;
    countdown_time.tm_isdst = -1;
  }
  //API Time
  else {
    timestamp = (time_t)API_DATA[settings.FavoriteTeam].gametime;
    countdown_time  = *localtime(&timestamp);
    
    days_to_time = 6 - current_wday;
    countdown_time.tm_mday += days_to_time;
    countdown_time.tm_hour = 12;
    countdown_time.tm_min = 0;
    countdown_time.tm_sec = 0;
    countdown_time.tm_isdst = -1;
  }
  
  time_t the_time = mktime(&countdown_time);
  double seconds_diff = difftime(the_time, temp);
  int32_t minutes_diff = (int)(seconds_diff / 60.0);
  uint32_t total_mins = abs(minutes_diff);

  // 1 day = 1440 minutes (24 * 60)
  uint16_t days = total_mins / 1440;          
  
  // Remaining minutes after full days are removed
  uint16_t mins_after_days = total_mins % 1440; 
  
  // Extract remaining hours and minutes from the remainder
  uint8_t hours = mins_after_days / 60;
  uint8_t minutes = mins_after_days % 60;

  if (minutes_diff < 0){
    snprintf(s_temp_time1, sizeof(s_temp_time1), "Hours");
    snprintf(s_temp_time2, sizeof(s_temp_time2), "Mins");
    firstplace = 0;
    secondplace = 0;
    after_time = true;
  }
  else if(days > 0){
    if (days > 99){
      days = 99;
    }
    snprintf(s_temp_time1, sizeof(s_temp_time1), "Days");
    snprintf(s_temp_time2, sizeof(s_temp_time2), "Hour");
    firstplace = days;
    secondplace = hours;
  }
  else{
    snprintf(s_temp_time1, sizeof(s_temp_time1), "Hour");
    snprintf(s_temp_time2, sizeof(s_temp_time2), "Mins");
    firstplace = hours;
    secondplace = minutes;
  }
  snprintf(s_temp_buffer1, sizeof(s_temp_buffer1), "%s", s_temp_time1);
  text_layer_set_text(s_day_layer, s_temp_buffer1);
  snprintf(s_temp_buffer2, sizeof(s_temp_buffer2), "%s", s_temp_time2);
  text_layer_set_text(s_hour_layer, s_temp_buffer2);
  snprintf(s_temp_buffer3, sizeof(s_temp_buffer3), "%02d:%02d", firstplace, secondplace);
  text_layer_set_text(s_countdown_layer, s_temp_buffer3);
  
  return after_time;
}

void timeDate_draw(Layer *window_layer, GRect bounds){
  #if PBL_DISPLAY_HEIGHT > 180
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_54));
    s_time_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
    s_countdown_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:88", s_font, GTextAlignmentCenter, window_layer);
    s_score_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "88|00", s_font, GTextAlignmentCenter, window_layer);
    s_home_layer = drawing_text_set(bounds.size.w / 2 - (time_w-25), bounds.size.h - 18, 40, 16, GColorBlack, "HOME", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_away_layer = drawing_text_set(bounds.size.w / 2 + 3, bounds.size.h - 18, 40, 16, GColorBlack, "AWAY", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_day_layer = drawing_text_set(bounds.size.w / 2 - (time_w-25), bounds.size.h - 18, 40, 16, GColorBlack, "Days", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_hour_layer = drawing_text_set(bounds.size.w / 2 + 3, bounds.size.h - 18, 40, 16, GColorBlack, "Hour", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  #else
    s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_42));
    s_time_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
    s_countdown_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "00:88", s_font, GTextAlignmentCenter, window_layer);
    s_score_layer = drawing_text_set(bounds.size.w / 2 - time_w, bounds.size.h * time_h, time_x, time_y, GColorBlack, "88|00", s_font, GTextAlignmentCenter, window_layer);
    s_home_layer = drawing_text_set(bounds.size.w / 2 - (time_w-10), bounds.size.h - 18, 40, 16, GColorBlack, "HOME", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_away_layer = drawing_text_set(bounds.size.w / 2, bounds.size.h - 18, 40, 16, GColorBlack, "AWAY", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_day_layer = drawing_text_set(bounds.size.w / 2 - (time_w-10), bounds.size.h - 18, 40, 16, GColorBlack, "Days", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_hour_layer = drawing_text_set(bounds.size.w / 2, bounds.size.h - 18, 40, 16, GColorBlack, "Hour", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  #endif

  animation_hide_text(true, true, false);
  
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
