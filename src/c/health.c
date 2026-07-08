#include "health.h"
#include "globals.h"

#if defined(PBL_HEALTH)
void health_handler() {
  if (settings.hrBool) {
    static char s_hr_buffer[8];
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

  if (!settings.hrBool || noHR) {
    layer_set_hidden(text_layer_get_layer(s_hr_layer), true);
    layer_set_hidden(hr_icon, true);
  }

  HealthValue stepvalue = 0;
  if(settings.stepsBool || settings.stepsGoalBool){
    //stepvalue = health_service_sum_today(HealthMetricStepCount);
    stepvalue = 4000;
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
#endif
