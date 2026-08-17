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

//Returns true if it is quiet time
bool timekeeping_is_quiet_time() {
  return current_time_integer >= settings.quietTimeStart && current_time_integer <= settings.quietTimeEnd;
}

// Updates the time TextLayer
void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Time handler
  // TEXT_LAYER_TIME now also serves as the countdown-timer and score
  // text (merged - same position, never shown simultaneously). This
  // runs every tick regardless of which mode is active, so it must only
  // overwrite the slot with the clock string when time mode is actually
  // the one being displayed - otherwise it stomps on countdown/score
  // text a moment after timekeeping_countdown()/api_score_display()
  // wrote it (this exact scenario happens at every app launch, since
  // main_window_load() calls update_time() right after
  // globals_prv_update_display() already set the correct mode's text).
  bool countdown_active = settings.countdownBool &&
    (!settings.scoreDisplayBool || !after_time) && settings.countdownDisplay != 1;
  bool score_active = settings.scoreDisplayBool &&
    (!settings.countdownBool || after_time) && settings.scoreLocation != 1;

  if (!countdown_active && !score_active) {
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    text_layer_set_text(s_text_layers[TEXT_LAYER_TIME], s_buffer);
  }

  // Month/day handler
  static char s_date_buffer[10];
  #ifdef PBL_ROUND
  strftime(s_date_buffer, sizeof(s_date_buffer), "%b %e", tick_time);
  #else
  strftime(s_date_buffer, sizeof(s_date_buffer), "%b\n%e", tick_time);
  #endif
  text_layer_set_text(s_text_layers[TEXT_LAYER_DATE], s_date_buffer);

  // Convert current time to HHMM format
  // tick_time->tm_hour is 0-23
  current_time_integer = (tick_time->tm_hour * 100) + tick_time->tm_min;
}

// Handles time ticks (every minute)
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  //animation_beat_team_layer();
  
  time_t now = time(NULL);
  if(tick_time->tm_min % settings.watchUpdate == 0){
    update_time();

    #if defined(PBL_HEALTH)
    health_handler();
    #endif

    if (settings.weatherBool && (!settings.weatherQuiet || !timekeeping_is_quiet_time())){
      // Get weather update every 30 minutes
      if (tick_time->tm_min % 30 == 0) {
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Weather Send");
        #endif
        outbox_queue_send(build_request_weather);
      }
    }


    // Check if we should sync CFBD data (e.g., once daily at 2 AM)
    if (api_should_full_sync() || !settings.cfbd.api_data_valid) {
    //if ((tick_time->tm_hour == 2 && tick_time->tm_min == 0 && api_should_full_sync()) || !settings.cfbd.api_data_valid) {
      api_request_cfbd_full_sync();
    }

    if (settings.countdownBool){
      after_time = timekeeping_countdown();
      if (((!after_time && settings.scoreDisplayBool) || !settings.scoreDisplayBool) && settings.countdownDisplay != 1){
        // Countdown active: HOME/AWAY (merged with Day/Hour sub-labels)
        // should be visible.
        layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), false);
        layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), false);
      }
    }

    // I might need to look into if both countdown and scores are chosen + they're on different screens
    if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time))){
      time_t target_time = (time_t)TEAMS[settings.FavoriteTeam].gametime;
      int32_t seconds_diff = (int32_t)(target_time - now);
      int32_t minutes_diff = seconds_diff / 60;
      if (minutes_diff <= 0) gametime = true;
      else gametime = false;
      //I need to find out if the game is completed.

      if (api_should_light_sync() && gametime && !TEAMS[settings.FavoriteTeam].completed) {
        api_request_cfbd_light_sync();
      }

      api_score_display();
      if (settings.scoreLocation != 1){
        // Score active: HOME/AWAY visible (already holding home_str/
        // away_str content from api_score_display() above).
        layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), false);
        layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), false);
      }
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
  // 1. Replaced temp buffers 1 & 2 with lightweight pointers
  const char *label_top;
  const char *label_bot;
  
  // 2. Shrink buffer 3 to the exact size needed for "XX:YY\0"
  static char s_countdown_buffer[6];

  time_t now = time(NULL);
  struct tm *now_tm = localtime(&now);

  time_t target_time = 0;
  bool afterwards = false;

  // Custom Time
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
  // API Time
  else if (settings.countdownTime == 2) {
    target_time = (time_t)TEAMS[settings.FavoriteTeam].gametime;
  }
  // Saturday Noon Eastern Time (Default)
  else {
    target_time = get_saturday_noon_eastern_utc(now_tm);
  }

  // Difference calculation in seconds
  int32_t seconds_diff = (int32_t)(target_time - now);
  int32_t minutes_diff = seconds_diff / 60;

  uint16_t firstplace = 0;
  uint16_t secondplace = 0;

  if (minutes_diff <= 0) {
    // Point directly to string constants in ROM
      #ifdef PBL_ROUND
      label_top = "H\no\nu\nr";
      label_bot = "M\ni\nn\ns";
      #else
      label_top = "Hour";
      label_bot = "Mins";
      #endif
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
      // Point directly to string constants
      #ifdef PBL_ROUND
      label_top = "D\na\ny\ns";
      label_bot = "H\no\nu\nr";
      #else
      label_top = "Days";
      label_bot = "Hour";
      #endif
      firstplace = days;
      secondplace = hours;
    } else {
      // Point directly to string constants
      #ifdef PBL_ROUND
      label_top = "H\no\nu\nr";
      label_bot = "M\ni\nn\ns";
      #else
      label_top = "Hour";
      label_bot = "Mins";
      #endif
      firstplace = hours;
      secondplace = minutes;
    }
  }

  // Assemble the "XX:YY" string manually
  format_2digits(&s_countdown_buffer[0], firstplace);
  s_countdown_buffer[2] = ':';
  format_2digits(&s_countdown_buffer[3], secondplace);
  s_countdown_buffer[5] = '\0';

  // Apply to text layers - HOME/AWAY now also serve as the countdown's
  // Day/Hour sub-labels, and TIME also serves as the countdown value,
  // since they occupy the same positions and are never shown
  // simultaneously (visibility for HOME/AWAY is handled by the callers
  // of this function; TIME never needs hiding).
  text_layer_set_text(s_text_layers[TEXT_LAYER_HOME], label_top);
  text_layer_set_text(s_text_layers[TEXT_LAYER_AWAY], label_bot);
  text_layer_set_text(s_text_layers[TEXT_LAYER_TIME], s_countdown_buffer);

  return afterwards;
}

void timeDate_draw(Layer *window_layer, GRect bounds){
  
  #if PBL_DISPLAY_HEIGHT > 180
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_54));
  s_text_layers[TEXT_LAYER_TIME] = drawing_text_set(bounds.size.w / 2 - time_w, (bounds.size.h * time_h) / 1000, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_COUNTDOWN] = drawing_text_set(bounds.size.w / 2 - time_w, (bounds.size.h * time_h) / 1000, time_x, time_y, GColorBlack, "00:88", s_font, GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_SCORE] = drawing_text_set(bounds.size.w / 2 - time_w, (bounds.size.h * time_h) / 1000, time_x, time_y, GColorBlack, "88|00", s_font, GTextAlignmentCenter, window_layer);
  
  #ifdef PBL_ROUND
  s_text_layers[TEXT_LAYER_HOME] = drawing_text_set(bounds.size.w / 2 - (time_w-25), ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "HOME", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  s_text_layers[TEXT_LAYER_AWAY] = drawing_text_set(bounds.size.w / 2 + 3, ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "AWAY", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
  //s_text_layers[TEXT_LAYER_DAY] = drawing_text_set(bounds.size.w / 2 - (time_w-25), ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "Days", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  //s_text_layers[TEXT_LAYER_HOUR] = drawing_text_set(bounds.size.w / 2 + 3, ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "Hour", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
  #else
  s_text_layers[TEXT_LAYER_HOME] = drawing_text_set(bounds.size.w / 2 - (time_w-25), bounds.size.h - 18, 40, 16, GColorBlack, "HOME", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  s_text_layers[TEXT_LAYER_AWAY] = drawing_text_set(bounds.size.w / 2 + 3, bounds.size.h - 18, 40, 16, GColorBlack, "AWAY", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_DAY] = drawing_text_set(bounds.size.w / 2 - (time_w-25), bounds.size.h - 18, 40, 16, GColorBlack, "Days", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_HOUR] = drawing_text_set(bounds.size.w / 2 + 3, bounds.size.h - 18, 40, 16, GColorBlack, "Hour", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  #endif
  #else
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_CUSTOM_42));
  s_text_layers[TEXT_LAYER_TIME] = drawing_text_set(bounds.size.w / 2 - time_w, ((bounds.size.h * time_h) / 1000) - 3, time_x, time_y, GColorBlack, "00:00", s_font, GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_COUNTDOWN] = drawing_text_set(bounds.size.w / 2 - time_w, ((bounds.size.h * time_h) / 1000) - 3, time_x, time_y, GColorBlack, "00:88", s_font, GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_SCORE] = drawing_text_set(bounds.size.w / 2 - time_w, ((bounds.size.h * time_h) / 1000) - 3, time_x, time_y, GColorBlack, "88|00", s_font, GTextAlignmentCenter, window_layer);
  
  #ifdef PBL_ROUND
  s_text_layers[TEXT_LAYER_HOME] = drawing_text_set(bounds.size.w / 2 - (time_w), ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "HOME", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  s_text_layers[TEXT_LAYER_AWAY] = drawing_text_set(bounds.size.w / 2 + 20, ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "AWAY", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
  //s_text_layers[TEXT_LAYER_DAY] = drawing_text_set(bounds.size.w / 2 - (time_w), ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "Days", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  //s_text_layers[TEXT_LAYER_HOUR] = drawing_text_set(bounds.size.w / 2 + 20, ((bounds.size.h * time_h) / 1000) - (time_y - 20), 40, 18, GColorBlack, "Hour", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
  #else
  s_text_layers[TEXT_LAYER_HOME] = drawing_text_set(bounds.size.w / 2 - (time_w-10), bounds.size.h - 16, 40, 16, GColorBlack, "HOME", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  s_text_layers[TEXT_LAYER_AWAY] = drawing_text_set(bounds.size.w / 2, bounds.size.h - 16, 40, 16, GColorBlack, "AWAY", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_DAY] = drawing_text_set(bounds.size.w / 2 - (time_w-10), bounds.size.h - 16, 40, 16, GColorBlack, "Days", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  //s_text_layers[TEXT_LAYER_HOUR] = drawing_text_set(bounds.size.w / 2, bounds.size.h - 16, 40, 16, GColorBlack, "Hour", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  #endif
  #endif

  // Default state: HOME/AWAY hidden until countdown or score mode
  // explicitly shows them (TEXT_LAYER_TIME needs no such default - it
  // always holds valid content, set moments later by update_time()).
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), true);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), true);

  #ifdef PBL_RECT
  // Create the TextLayer for the time and date
  #if PBL_DISPLAY_HEIGHT > 180
  s_text_layers[TEXT_LAYER_DATE] = drawing_text_set(155, (bounds.size.h * date_h) / 1000, 35, 38, GColorBlack, "Dec 31", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
  #else
  s_text_layers[TEXT_LAYER_DATE] = drawing_text_set((bounds.size.w * date_w) / 1000, (bounds.size.h * date_h) / 1000, 26, 32, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  #endif

  s_layers[LAYER_VERT] = drawing_line_draw(bounds, (bounds.size.w * hor_1) / 1000, (bounds.size.h * vert_1) / 1000, (bounds.size.w * hor_1) / 1000, (bounds.size.h * vert_2) / 1000, 1, GColorBlack, window_layer);
  
  #else
  #if PBL_DISPLAY_HEIGHT > 180
  // Create the TextLayer for the time and date
  s_text_layers[TEXT_LAYER_DATE] = drawing_text_set(bounds.size.w / 2 - 22, (bounds.size.h * date_h) / 1000, 46, 21, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
  #else
  // Create the TextLayer for the time and date
  s_text_layers[TEXT_LAYER_DATE] = drawing_text_set(bounds.size.w / 2 - 20, (bounds.size.h * date_h) / 1000, 42, 17, GColorBlack, "", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  #endif
  #endif

  s_layers[LAYER_HOR] = drawing_line_draw(bounds, (bounds.size.w * hor_1) / 1000, (bounds.size.h * vert_2) / 1000, (bounds.size.w * hor_2) / 1000, (bounds.size.h * vert_2) / 1000, 1, GColorBlack, window_layer);
}