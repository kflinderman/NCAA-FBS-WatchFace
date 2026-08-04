#include "health.h"
#include "globals.h"
#include "drawing.h"
#include "timekeeping.h"


#if defined(PBL_HEALTH)
void health_heartRateHandler() {
  if (settings.hrBool) {
    if (!settings.healthQuiet || (settings.healthQuiet && !timekeeping_is_quiet_time())) {
      static char s_hr_buffer[12];
      HealthValue hrvalue = health_service_peek_current_value(HealthMetricHeartRateBPM);
      if (hrvalue > 0) {
        snprintf(s_hr_buffer, sizeof(s_hr_buffer), "%d", (int)hrvalue);
        text_layer_set_text(s_hr_layer, s_hr_buffer);

        layer_set_hidden(text_layer_get_layer(s_hr_layer), false);
        layer_set_hidden(hr_icon, false);
        noHR = false;
      } else {
        noHR = true;
      }
    }
    else {
      layer_set_hidden(text_layer_get_layer(s_hr_layer), false);
      layer_set_hidden(hr_icon, false);
    }
  }

  if (!settings.hrBool || noHR) {
    layer_set_hidden(text_layer_get_layer(s_hr_layer), true);
    layer_set_hidden(hr_icon, true);
  }
}

void health_stepHandler(){
  if (!settings.healthQuiet || (settings.healthQuiet && !timekeeping_is_quiet_time())){
    HealthValue stepvalue = 0;
    if(settings.stepsBool || settings.stepsGoalBool){
      stepvalue = health_service_sum_today(HealthMetricStepCount);
      //stepvalue = 4000;
    }

    if (settings.stepsBool) {
      static char s_step_buffer[8];
      snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", (int)stepvalue);
      text_layer_set_text(s_step_layer, s_step_buffer);
      layer_set_hidden(text_layer_get_layer(s_step_layer), false);
    }
    else {
      layer_set_hidden(text_layer_get_layer(s_step_layer), true);
    }

    if (settings.stepsGoalBool) {
      Layer *window_layer = window_get_root_layer(s_main_window);
      GRect bounds = layer_get_bounds(window_layer);
      float stepDiff = (float)stepvalue / settings.stepsGoal;
      #if PBL_DISPLAY_HEIGHT > 180
      uint16_t top_y = stepy - 14;
      #else
      uint16_t top_y = stepy - 10;
      #endif
      uint16_t bottom_y = (bounds.size.h * time_h) - 25;
      if (stepDiff >= 1){
        stepDiff = 1;
        layer_set_hidden(text_layer_get_layer(s_td_layer), false);
      }
      else{
        layer_set_hidden(text_layer_get_layer(s_td_layer), true);
      }

      GRect frame = layer_get_frame(bitmap_layer_get_layer(s_football_layer));
      frame.origin.y = bottom_y - (bottom_y - top_y) * stepDiff;

      layer_set_frame(bitmap_layer_get_layer(s_football_layer), frame);

      layer_set_hidden(step_ladder, false);
      layer_set_hidden(bitmap_layer_get_layer(s_football_layer), false);
    }
    else {
      layer_set_hidden(step_ladder, true);
      layer_set_hidden(bitmap_layer_get_layer(s_football_layer), true);
      layer_set_hidden(text_layer_get_layer(s_td_layer), true);
    }
  }
  else{
    if (settings.stepsBool) layer_set_hidden(text_layer_get_layer(s_step_layer), false);
    else layer_set_hidden(text_layer_get_layer(s_step_layer), true);
    
    if (settings.stepsGoalBool) {
      layer_set_hidden(step_ladder, false);
      layer_set_hidden(bitmap_layer_get_layer(s_football_layer), false);
    }
    else {
      layer_set_hidden(step_ladder, true);
      layer_set_hidden(bitmap_layer_get_layer(s_football_layer), true);
      layer_set_hidden(text_layer_get_layer(s_td_layer), true);
    }
  }
}

void health_handler() {
  health_heartRateHandler();
  health_stepHandler();
}

void health_draw(Layer *window_layer, GRect bounds){
  #if PBL_DISPLAY_HEIGHT > 180
    s_hr_layer = drawing_text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
  #else
    s_hr_layer = drawing_text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  #endif
  
  hr_icon = drawing_multiline_layer_create(bounds, window_layer);
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 23 + (6*hr_w), 30), GPoint(bounds.size.w - 20 + (5*hr_w), 30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 20 + (5*hr_w), 30), GPoint(bounds.size.w - 17 + (4*hr_w), 35), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 17 + (4*hr_w), 35), GPoint(bounds.size.w - 11 + (2*hr_w), 25), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 11 + (2*hr_w), 25), GPoint(bounds.size.w - 8 + (hr_w),    30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 8 + (hr_w), 30),    GPoint(bounds.size.w - 5,             30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});

  #if PBL_DISPLAY_HEIGHT > 180
  s_step_layer = drawing_text_set(bounds.size.w / 2 - stepx2, (bounds.size.h * time_h) - 20, 50, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentLeft, window_layer);
  #else
  s_step_layer = drawing_text_set(bounds.size.w / 2 - stepx2, (bounds.size.h * time_h) - 20, 50, 16, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
  #endif
  uint16_t gaps = ((bounds.size.h * time_h) - stepy - 25) / 3;
  uint16_t gaps2 = gaps / 5;
  step_ladder = drawing_multiline_layer_create(bounds, window_layer);
  //drawing_multiline_add_segment(step_ladder, GPoint((bounds.size.w / 2 - stepx2) + (stepx1 / 2), (bounds.size.h * time_h) - 27), GPoint((bounds.size.w / 2 - stepx2) + (stepx1 / 2), stepy), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});

  for (uint16_t x = 0; x < 4; x++) {
    drawing_multiline_add_segment(step_ladder, GPoint(bounds.size.w / 2 - stepx2, stepy + (gaps * x)), GPoint((bounds.size.w / 2 - stepx2) + stepx1, stepy + (gaps * x)), 2, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});

    if (x < 3){
      for (uint16_t y = 1; y < 6; y++) {
        drawing_multiline_add_segment(step_ladder, GPoint((bounds.size.w / 2 - stepx2) + 3, stepy + (gaps * x) + (gaps2 * y)), GPoint((bounds.size.w / 2 - stepx2) + stepx1 - 3, stepy + (gaps * x) + (gaps2 * y)), 1, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
      }
    }
  }

  s_football_bitmap = gbitmap_create_with_resource(RESOURCE_ID_football);
  #if PBL_DISPLAY_HEIGHT > 180
    s_football_layer = drawing_bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 4, stepy + gaps * 4 - 6, 12, 12, s_football_bitmap, window_layer);
    #ifdef PBL_RECT
      s_td_layer = drawing_text_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 10, stepy - (14+21), 25, 21, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "TD!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #else
      s_td_layer = drawing_text_set((bounds.size.w / 2 - stepx2) - 15, (bounds.size.w / 2) - 55, 10, 80, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "T D !", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #endif
      layer_set_hidden(text_layer_get_layer(s_td_layer), true);
  #else
    s_football_layer = drawing_bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 4, stepy + gaps * 4 - 4, 8, 8, s_football_bitmap, window_layer);
    #ifdef PBL_RECT
      s_td_layer = drawing_text_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 10, stepy - (10+17), 25, 17, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "TD!", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #else
      s_td_layer = drawing_text_set((bounds.size.w / 2 - stepx2) - 15, (bounds.size.w / 2) - 35, 10, 50, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "T D !", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #endif
    layer_set_hidden(text_layer_get_layer(s_td_layer), true);
  #endif
}
#endif
