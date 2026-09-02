#ifndef PBL_PLATFORM_APLITE

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
  //Determine whether to show temperature information
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_WEATHER]), !settings.weatherBool);

  //If not, exit
  if (!settings.weatherBool) {
    return;
  }

  uint8_t tempTemperature = temperatureValue;
  char unit = 'F';

  //Determine temperature units
  if (settings.weatherUnits){
    unit = 'C';
  }
  else{
    //Convert to F
    tempTemperature = ((temperatureValue * 9) / 5) + 32;
    unit = 'F';
  }

  //Add buffer to text layer
  static char s_temp_buffer[5];
  snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%d%c", tempTemperature, unit);
  text_layer_set_text(s_text_layers[TEXT_LAYER_WEATHER], s_temp_buffer);

}

void weather_conditions_update() {
  //Determine whether to show conditions information
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_CONDITIONS]), !settings.weatherBool);
  if (settings.weatherBool) {
    static char s_temp_buffer[8];
    uint8_t max_idx = (uint8_t)(sizeof(WEATHER_ICONS) / sizeof(WEATHER_ICONS[0])) - 1;
    
    //Determine condition position in array
    uint8_t idx = (conditionValue >= 0 && conditionValue <= max_idx) ? (uint8_t)conditionValue : max_idx;
    snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%s", WEATHER_ICONS[idx]);
    text_layer_set_text(s_text_layers[TEXT_LAYER_CONDITIONS], s_temp_buffer);
  }
}

//Main function to call weather screen updates
void weather_update(){
  weather_temp_update();
  weather_conditions_update();
}

//Get weather information from phone
void weather_callback(DictionaryIterator *iterator, void *context){
  bool weather_changed = false;

  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  if (conditions_tuple){
    conditionValue = conditions_tuple->value->int16;
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Conditions: %d", conditionValue);
    #endif
    weather_changed = true;
  }

  if (temp_tuple) {
    temperatureValue = temp_tuple->value->int16;
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %d", temperatureValue);
    #endif
    weather_changed = true;
  }

  //If we got an update trigger screen update
  if (weather_changed){
    weather_update();
  }
}

//First function to draw all necessary items.
void weather_draw(Layer *window_layer, GRect bounds){
  
  //Determine text/icon color based on Favorite team
  GColor icon_color;
  #if defined(PBL_COLOR)
  icon_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color};
  #else
  if(gcolor_equal((GColor){.argb = TEAMS[settings.FavoriteTeam].color}, GColorWhite)){
    icon_color = GColorBlack;
  }
  else{
    icon_color = GColorWhite;
  }
  #endif
  
  //Draw layers based on large or small screen
  #if PBL_DISPLAY_HEIGHT > 180
  s_wIcon = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_ICONS_18));
  s_text_layers[TEXT_LAYER_WEATHER] = drawing_text_set(bounds.size.w / 2 + 65, ((bounds.size.h * TIME_H) / 1000) - 20, 35, 38, icon_color, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
  s_text_layers[TEXT_LAYER_CONDITIONS] = drawing_text_set(bounds.size.w / 2 + 69, ((bounds.size.h * TIME_H) / 1000) - 40, 35, 35, icon_color, WEATHER_ICONS[13], s_wIcon, GTextAlignmentCenter, window_layer);
  #else
  s_wIcon = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHER_ICONS_12));
  s_text_layers[TEXT_LAYER_WEATHER] = drawing_text_set(bounds.size.w / 2 + 46, ((bounds.size.h * TIME_H) / 1000) - 20, 26, 32, icon_color, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
  s_text_layers[TEXT_LAYER_CONDITIONS] = drawing_text_set(bounds.size.w / 2 + 51, ((bounds.size.h * TIME_H) / 1000) - 34, 23, 23, icon_color, WEATHER_ICONS[13], s_wIcon, GTextAlignmentCenter, window_layer);
  #endif
  
  //hide layers
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_WEATHER]), true);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_CONDITIONS]), true);
}


void weather_build_request(DictionaryIterator *iter) {
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
}
#endif