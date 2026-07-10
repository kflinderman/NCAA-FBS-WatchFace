#include "weather.h"
#include "globals.h"
#include "drawing.h"


void weather_draw(Layer *window_layer, GRect bounds){
    #if PBL_DISPLAY_HEIGHT > 180
      s_weather_layer = text_set(bounds.size.w - 35, (bounds.size.h * time_h) - 20, 35, 38, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #else
      s_weather_layer = text_set(bounds.size.w - 26, (bounds.size.h * time_h) - 20, 26, 32, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #endif
}