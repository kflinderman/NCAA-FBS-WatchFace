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
      layer_set_hidden(hr1, false);
      layer_set_hidden(hr2, false);
      layer_set_hidden(hr3, false);
      layer_set_hidden(hr4, false);
      layer_set_hidden(hr5, false);
      noHR = false;
    } else {
      noHR = true;
    }
  }

  if (!settings.hrBool || noHR) {
    layer_set_hidden(text_layer_get_layer(s_hr_layer), true);
    layer_set_hidden(hr1, true);
    layer_set_hidden(hr2, true);
    layer_set_hidden(hr3, true);
    layer_set_hidden(hr4, true);
    layer_set_hidden(hr5, true);
  }

  if (settings.stepsBool) {
    static char s_step_buffer[8];
    HealthValue stepvalue = health_service_sum_today(HealthMetricStepCount);
    snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", (int)stepvalue);
    text_layer_set_text(s_step_layer, s_step_buffer);
    layer_set_hidden(text_layer_get_layer(s_step_layer), false);

    if (settings.stepsGoalBool) {
      float stepDiff = (int)stepvalue / settings.stepsGoal;
      (void)stepDiff; // currently unused, kept from original logic

      layer_set_hidden(step1, false);
      layer_set_hidden(step2, false);
      layer_set_hidden(step3, false);
      layer_set_hidden(step4, false);
      layer_set_hidden(step5, false);
      layer_set_hidden(step6, false);
      layer_set_hidden(bitmap_layer_get_layer(s_football_layer), false);
    } else {
      layer_set_hidden(step1, true);
      layer_set_hidden(step2, true);
      layer_set_hidden(step3, true);
      layer_set_hidden(step4, true);
      layer_set_hidden(step5, true);
      layer_set_hidden(step6, true);
      layer_set_hidden(bitmap_layer_get_layer(s_football_layer), true);
    }
  }

  if (!settings.stepsBool) {
    layer_set_hidden(text_layer_get_layer(s_step_layer), true);
    layer_set_hidden(step1, true);
    layer_set_hidden(step2, true);
    layer_set_hidden(step3, true);
    layer_set_hidden(step4, true);
    layer_set_hidden(step5, true);
    layer_set_hidden(step6, true);
    layer_set_hidden(bitmap_layer_get_layer(s_football_layer), true);
  }
}
#endif
