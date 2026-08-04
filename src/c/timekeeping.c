#include "timekeeping.h"
#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "animation.h"
#include "api.h"
#include "outbox_queue.h"

static void build_request_weather(DictionaryIterator *iter) {
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
}

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
}

// Handles time ticks (every minute)
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  #if defined(PBL_HEALTH)
    health_handler();
  #endif

  if (settings.weatherBool && (!settings.weatherQuiet || (current_time_integer >= settings.quietTimeStart && current_time_integer <= settings.quietTimeEnd))){
    // Get weather update every 30 minutes
    if (tick_time->tm_min % 30 == 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Weather Send");
      outbox_queue_send(build_request_weather);
    }
  }
  
  /*
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
  */
  
  time_t now = time(NULL);
  
  // Check if we should sync CFBD data (e.g., once daily at 2 AM)
  if ((tick_time->tm_hour == 2 && tick_time->tm_min == 0 && api_should_full_sync()) || !settings.cfbd.api_data_valid) {
      api_request_cfbd_full_sync();
  }

  if (settings.countdownBool){
    after_time = timekeeping_countdown();
    if (((!after_time && settings.scoreDisplayBool) || !settings.scoreDisplayBool) && settings.countdownDisplay != 1){
      animation_hide_text(false, true, true);
    }
  }
  
  // I might need to look into if both countdown and scores are chosen + they're on different screens
  if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time))){
    time_t target_time = (time_t)API_DATA[settings.FavoriteTeam].gametime;
    double seconds_diff = difftime(target_time, now);
    int32_t minutes_diff = (int32_t)(seconds_diff / 60.0);
    if (minutes_diff <= 0) gametime = true;
    else gametime = false;
    //I need to find out if the game is completed.

    if (api_should_light_sync() && gametime) {
      api_request_cfbd_light_sync();
    }

    api_score_display();
    if (settings.scoreLocation != 1){
      animation_hide_text(true, false, true);
    }
  }
}

void timer_callback(void *data) {
  animation_beat_team_layer();
}

// Pure C converter from UTC components to epoch seconds (bypasses missing timegm)
static time_t utc_to_epoch(int year, int mon, int mday, int hour, int min) {
    if (mon <= 2) {
        year -= 1;
        mon += 12;
    }
    long era = (year >= 0 ? year : year - 399) / 400;
    unsigned yoe = (unsigned)(year - era * 400);
    unsigned doy = (153 * (mon - 3) + 2) / 5 + mday - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    long days = era * 146097 + (long)doe - 719468;
    return (time_t)(days * 86400 + hour * 3600 + min * 60);
}

// Returns upcoming Saturday 12:00 PM Eastern Time in UTC epoch
static time_t get_saturday_noon_eastern_utc(struct tm *now_tm) {
    // 1. Calculate Saturday's date relative to current local time
    struct tm sat = *now_tm;
    sat.tm_mday += (6 - now_tm->tm_wday);
    mktime(&sat); // Normalizes month/year rollovers

    int year  = sat.tm_year + 1900;
    int month = sat.tm_mon + 1; // 1-12
    int day   = sat.tm_mday;

    // 2. US Daylight Saving Time check (EDT vs EST)
    bool is_edt = false;
    if (month > 3 && month < 11) {
        is_edt = true; 
    } else if (month == 3) {
        // March: EDT begins on 2nd Sunday
        struct tm m = { .tm_year = sat.tm_year, .tm_mon = 2, .tm_mday = 1 };
        mktime(&m);
        int second_sunday = 1 + ((7 - m.tm_wday) % 7) + 7;
        if (day >= second_sunday) is_edt = true;
    } else if (month == 11) {
        // November: EDT ends on 1st Sunday
        struct tm m = { .tm_year = sat.tm_year, .tm_mon = 10, .tm_mday = 1 };
        mktime(&m);
        int first_sunday = 1 + ((7 - m.tm_wday) % 7);
        if (day < first_sunday) is_edt = true;
    }

    // 12:00 PM Eastern = 16:00 UTC (EDT) or 17:00 UTC (EST)
    int utc_hour = is_edt ? 16 : 17;

    return utc_to_epoch(year, month, day, utc_hour, 0);
}

// Primary Countdown Function
bool timekeeping_countdown() {
	static char s_temp_buffer1[8], s_temp_buffer2[8], s_temp_buffer3[8];
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);

    time_t target_time = 0;
    bool afterwards = false;

    // 1. Custom Time
    if (settings.countdownTime == 1) {
        if (settings.countdownCustomDate < 10000000) { 
            target_time = get_saturday_noon_eastern_utc(now_tm);
        } else {
            struct tm target = {0};
            target.tm_year = (settings.countdownCustomDate / 10000) - 1900;
            target.tm_mon  = ((settings.countdownCustomDate / 100) % 100) - 1;
            target.tm_mday = settings.countdownCustomDate % 100;
            target.tm_hour = (settings.countdownCustomTime < 10) ? 12 : settings.countdownCustomTime / 100;
            target.tm_min  = (settings.countdownCustomTime < 10) ? 0  : settings.countdownCustomTime % 100;
            target.tm_sec  = 0;
            target.tm_isdst = -1;
            target_time = mktime(&target);
        }
    }
    // 2. API Time
    else if (settings.countdownTime == 2) {
        target_time = (time_t)API_DATA[settings.FavoriteTeam].gametime;
    }
    // 3. Saturday Noon Eastern Time (Default)
    else {
        target_time = get_saturday_noon_eastern_utc(now_tm);
    }

    // Difference calculation in seconds
    double seconds_diff = difftime(target_time, now);
    int32_t minutes_diff = (int32_t)(seconds_diff / 60.0);

    uint16_t firstplace = 0;
    uint16_t secondplace = 0;

    if (minutes_diff <= 0) {
        snprintf(s_temp_buffer1, sizeof(s_temp_buffer1), "Hours");
        snprintf(s_temp_buffer2, sizeof(s_temp_buffer2), "Mins");
        firstplace = 0;
        secondplace = 0;
        afterwards = true;
    } else {
        uint32_t total_mins = (uint32_t)minutes_diff;
        uint16_t days = total_mins / 1440;          
        uint16_t mins_after_days = total_mins % 1440; 
        uint8_t hours = mins_after_days / 60;
        uint8_t minutes = mins_after_days % 60;

        if (days > 0) {
            if (days > 99) days = 99;
            snprintf(s_temp_buffer1, sizeof(s_temp_buffer1), "Days");
            snprintf(s_temp_buffer2, sizeof(s_temp_buffer2), "Hour");
            firstplace = days;
            secondplace = hours;
        } else {
            snprintf(s_temp_buffer1, sizeof(s_temp_buffer1), "Hour");
            snprintf(s_temp_buffer2, sizeof(s_temp_buffer2), "Mins");
            firstplace = hours;
            secondplace = minutes;
        }
    }

    text_layer_set_text(s_day_layer, s_temp_buffer1);
    text_layer_set_text(s_hour_layer, s_temp_buffer2);
    snprintf(s_temp_buffer3, sizeof(s_temp_buffer3), "%02d:%02d", firstplace, secondplace);
    text_layer_set_text(s_countdown_layer, s_temp_buffer3);

    return afterwards;
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