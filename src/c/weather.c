#include "weather.h"
#include "globals.h"
#include "drawing.h"

static const char* const WEATHER_ICONS[] = {
  "\xEF\x80\x8D", // 0: Clear (wi-day-sunny)
  "\xEF\x80\x82", // 1: Cloudy (wi-day-cloudy)
  "\xEF\x80\x83", // 2: Fog (wi-day-fog)
  "\xEF\x80\x8B", // 3: Drizzle (wi-day-sprinkle)
  "\xEF\x82\xB2", // 4: Fz. Drizzle (wi-day-sleet)
  "\xEF\x80\x88", // 5: Rain (wi-day-rain)
  "\xEF\x80\x86", // 6: Fz. Rain (wi-day-rain-mix)
  "\xEF\x80\x8A", // 7: Snow (wi-day-snow)
  "\xEF\x80\x8A", // 8: Snow Grains (wi-day-snow)
  "\xEF\x80\x89", // 9: Showers (wi-day-showers)
  "\xEF\x81\xA5", // 10: Snow Shwrs (wi-day-snow-wind)
  "\xEF\x80\x90", // 11: T-Storm (wi-day-thunderstorm)
  "\xEF\x80\x85", // 12: T-Storm (wi-day-lightning)
  "\xEF\x81\xBB"  // 13: Unknown (wi-na)
};

void weather_temp_update(){
  if (settings.weatherBool) {
    int tempTemperature = 0;
    char unit = 'F';
    
    if (settings.weatherUnits){
      tempTemperature = temperatureValue;
      unit = 'C';
    }
    else{
      tempTemperature = ((temperatureValue * 9) / 5) + 32;
      unit = 'F';
    }
    
    static char s_temp_buffer[8];
    snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%d%c", tempTemperature, unit);
    text_layer_set_text(s_weather_layer, s_temp_buffer);

    layer_set_hidden(text_layer_get_layer(s_weather_layer), false);
    layer_set_hidden(text_layer_get_layer(s_conditions_layer), false);
  }
  else {
    layer_set_hidden(text_layer_get_layer(s_weather_layer), true);
    layer_set_hidden(text_layer_get_layer(s_conditions_layer), true);
  }
}

void weather_conditions_update(){
  if (settings.weatherBool) {
    static char s_temp_buffer[8];
    snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%s", WEATHER_ICONS[conditionValue]);
    text_layer_set_text(s_conditions_layer, s_temp_buffer);
  }
}

void weather_update(){
  weather_temp_update();
  weather_conditions_update();
}

void weather_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Weather - Dict size: %d", dict_size(iterator));
  
  bool weather_changed = false;
  
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  
  if (conditions_tuple){
    conditionValue = conditions_tuple->value->int16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Conditions: %d", conditionValue);
    weather_changed = true;
  }

  if (temp_tuple) {
    temperatureValue = temp_tuple->value->int16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %d", temperatureValue);
    weather_changed = true;
  }
  
  if (weather_changed){
    weather_update();
  }
}

void weather_draw(Layer *window_layer, GRect bounds){
  #if PBL_DISPLAY_HEIGHT > 180
    s_wIcon = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_ICONS_18));
    s_weather_layer = drawing_text_set(bounds.size.w / 2 + 65, (bounds.size.h * time_h) - 20, 35, 38, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    s_conditions_layer = drawing_text_set(bounds.size.w / 2 + 69, (bounds.size.h * time_h) - 40, 30, 30, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, WEATHER_ICONS[13], s_wIcon, GTextAlignmentCenter, window_layer);
  #else
    s_wIcon = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_ICONS_12));
    s_weather_layer = drawing_text_set(bounds.size.w / 2 + 46, (bounds.size.h * time_h) - 20, 26, 32, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_conditions_layer = drawing_text_set(bounds.size.w / 2 + 51, (bounds.size.h * time_h) - 34, 20, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, WEATHER_ICONS[13], s_wIcon, GTextAlignmentCenter, window_layer);
  #endif
}