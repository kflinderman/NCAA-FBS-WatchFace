#include "weather.h"
#include "globals.h"
#include "drawing.h"

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
    layer_set_hidden(bitmap_layer_get_layer(s_conditions_layer), false);
  }
  else {
    layer_set_hidden(text_layer_get_layer(s_weather_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(s_conditions_layer), true);
  }
}

void weather_conditions_update(){
  if (settings.weatherBool) {
    
    const weatherIcons conditionIcons[] = {
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
      { .black = RESOURCE_ID_BT, .white = RESOURCE_ID_BT  },
    };
    
    if (s_logo_bitmap) {
      gbitmap_destroy(s_conditions_bitmap);
    }
    
    uint8_t comparison;
    if (settings.DisplayTeam > 1) {
      comparison = TEAMS[settings.BeatTeam].icon_color;
    }
    else{
      comparison = TEAMS[settings.FavoriteTeam].icon_color;
    }
    
    if (comparison == GColorBlackARGB8){
      s_conditions_bitmap = gbitmap_create_with_resource(conditionIcons[conditionValue].black);
    }
    else{
      s_conditions_bitmap = gbitmap_create_with_resource(conditionIcons[conditionValue].white);
    }
    
    
    if (s_conditions_layer) {
      bitmap_layer_set_bitmap(s_conditions_layer, s_conditions_bitmap);
    }
  }
}

void weather_update(){
  weather_temp_update();
  weather_conditions_update();
}

void weather_draw(Layer *window_layer, GRect bounds){
  //Need to fix conditions location
  //Need to fix locations on round faces > see health
  #if PBL_DISPLAY_HEIGHT > 180
    s_weather_layer = drawing_text_set(bounds.size.w - 35, (bounds.size.h * time_h) - 20, 35, 38, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    s_conditions_layer = drawing_bitmap_set(bounds.size.w - 20, (bounds.size.h * time_h) - 40, 15, 15, s_conditions_bitmap, window_layer);
  #else
    s_weather_layer = drawing_text_set(bounds.size.w - 26, (bounds.size.h * time_h) - 20, 26, 32, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100F", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    s_conditions_layer = drawing_bitmap_set(bounds.size.w - 15, (bounds.size.h * time_h) - 35, 10, 10, s_conditions_bitmap, window_layer);
  #endif
}