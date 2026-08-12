#include "health.h"
#include "globals.h"
#include "drawing.h"
#include "timekeeping.h"


#if defined(PBL_HEALTH)
void health_heartRateHandler(void) {
  // Early Return: If HR is disabled in settings, hide elements and exit immediately
  if (!settings.hrBool) {
    layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HR]), true);
    layer_set_hidden(hr_icon, true);
    return;
  }

  // Poll Heart Rate Sensor
  HealthValue hrvalue;
  if (!settings.healthQuiet || !timekeeping_is_quiet_time()) {
    hrvalue = health_service_peek_current_value(HealthMetricHeartRateBPM);
  }
  else{
    hrvalue = 70;
  }

  if (hrvalue > 0) {
    static char s_hr_buffer[4];
    snprintf(s_hr_buffer, sizeof(s_hr_buffer), "%d", (int)hrvalue);
    text_layer_set_text(s_text_layers[TEXT_LAYER_HR], s_hr_buffer);
    noHR = false;
  } else {
    noHR = true;
  }

  // Set Final Visibility
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HR]), noHR);
  layer_set_hidden(hr_icon, noHR);
}

void health_stepHandler(){
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_STEP]), !settings.stepsBool);
  layer_set_hidden(step_ladder, !settings.stepsGoalBool);
  layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_FOOTBALL]), !settings.stepsGoalBool);
  // This one is this way because we only want to hide it without the setting in case it's not been met yet
  if (!settings.stepsGoalBool) {
    layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_TD]), true);
  }

  // Early Exit during Quiet Time or if features are toggled off
  if ((settings.healthQuiet && timekeeping_is_quiet_time()) || (!settings.stepsBool && !settings.stepsGoalBool)) {
    return;
  }

  // Fetch Step Metrics
  HealthValue stepvalue = health_service_sum_today(HealthMetricStepCount);
  //HealthValue stepvalue = 4000;

  // Update Step Text
  if (settings.stepsBool) {
    static char s_step_buffer[8];
    snprintf(s_step_buffer, sizeof(s_step_buffer), "%d", (int)stepvalue);
    text_layer_set_text(s_text_layers[TEXT_LAYER_STEP], s_step_buffer);
  }

  // Update Goal Progress & Football Position
  if (settings.stepsGoalBool) {
    // Clamp steps to the goal (so the football never overshoots) and
    // track goal-reached directly, rather than computing a 0.0-1.0 ratio
    // first - the cross-multiplication below (steps_clamped * span) /
    // goal gets the same interpolated position without ever forming a
    // fractional progress value.
    int32_t steps_clamped = (settings.stepsGoal > 0) ?
      ((int32_t)stepvalue < settings.stepsGoal ? (int32_t)stepvalue : settings.stepsGoal) : 0;
    bool goal_reached = (settings.stepsGoal > 0) && ((int32_t)stepvalue >= settings.stepsGoal);

    layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_TD]), !goal_reached);

    // Calculate Y Position
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_bounds(window_layer);

    #if PBL_DISPLAY_HEIGHT > 180
    uint16_t top_y = stepy - 14;
    #else
    uint16_t top_y = stepy - 10;
    #endif
    uint16_t bottom_y = ((bounds.size.h * time_h) / 1000) - 25;

    // Reposition Football
    GRect frame = layer_get_frame(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_FOOTBALL]));
    uint16_t y_offset = (settings.stepsGoal > 0) ?
      (uint16_t)(((int32_t)(bottom_y - top_y) * steps_clamped) / settings.stepsGoal) : 0;
    frame.origin.y = bottom_y - y_offset;
    layer_set_frame(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_FOOTBALL]), frame);
  }
}

void health_handler() {
  health_heartRateHandler();
  health_stepHandler();
}

void health_draw(Layer *window_layer, GRect bounds){
  #if PBL_DISPLAY_HEIGHT > 180
    s_text_layers[TEXT_LAYER_HR] = drawing_text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight, window_layer);
  #else
    s_text_layers[TEXT_LAYER_HR] = drawing_text_set(bounds.size.w - 30, 0, 25, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "100", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentRight, window_layer);
  #endif
  
  hr_icon = drawing_multiline_layer_create(bounds, window_layer);
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 23 + (6*hr_w), 30), GPoint(bounds.size.w - 20 + (5*hr_w), 30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 20 + (5*hr_w), 30), GPoint(bounds.size.w - 17 + (4*hr_w), 35), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 17 + (4*hr_w), 35), GPoint(bounds.size.w - 11 + (2*hr_w), 25), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 11 + (2*hr_w), 25), GPoint(bounds.size.w - 8 + (hr_w),    30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
  drawing_multiline_add_segment(hr_icon, GPoint(bounds.size.w - 8 + (hr_w), 30),    GPoint(bounds.size.w - 5,             30), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});

  #if PBL_DISPLAY_HEIGHT > 180
  s_text_layers[TEXT_LAYER_STEP] = drawing_text_set(bounds.size.w / 2 - stepx2, ((bounds.size.h * time_h) / 1000) - 20, 50, 20, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentLeft, window_layer);
  #else
  s_text_layers[TEXT_LAYER_STEP] = drawing_text_set(bounds.size.w / 2 - stepx2, ((bounds.size.h * time_h) / 1000) - 20, 50, 16, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "00000", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft, window_layer);
  #endif
  uint16_t gaps = (((bounds.size.h * time_h) / 1000) - stepy - 25) / 3;
  uint16_t gaps2 = gaps / 5;
  step_ladder = drawing_multiline_layer_create(bounds, window_layer);
  //drawing_multiline_add_segment(step_ladder, GPoint((bounds.size.w / 2 - stepx2) + (stepx1 / 2), ((bounds.size.h * time_h) / 1000) - 27), GPoint((bounds.size.w / 2 - stepx2) + (stepx1 / 2), stepy), hr_thick, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});

  for (uint16_t x = 0; x < 4; x++) {
    drawing_multiline_add_segment(step_ladder, GPoint(bounds.size.w / 2 - stepx2, stepy + (gaps * x)), GPoint((bounds.size.w / 2 - stepx2) + stepx1, stepy + (gaps * x)), 2, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});

    if (x < 3){
      for (uint16_t y = 1; y < 6; y++) {
        drawing_multiline_add_segment(step_ladder, GPoint((bounds.size.w / 2 - stepx2) + 3, stepy + (gaps * x) + (gaps2 * y)), GPoint((bounds.size.w / 2 - stepx2) + stepx1 - 3, stepy + (gaps * x) + (gaps2 * y)), 1, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color});
      }
    }
  }

  s_gbitmap_layers[GBITMAP_LAYER_FOOTBALL] = gbitmap_create_with_resource(RESOURCE_ID_football);
  #if PBL_DISPLAY_HEIGHT > 180
    s_bitmap_layers[BITMAP_LAYER_FOOTBALL] = drawing_bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 4, stepy + gaps * 4 - 6, 12, 12, s_gbitmap_layers[GBITMAP_LAYER_FOOTBALL], window_layer);
    #ifdef PBL_RECT
      s_text_layers[TEXT_LAYER_TD] = drawing_text_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 10, stepy - (14+21), 25, 21, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "TD!", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #else
      s_text_layers[TEXT_LAYER_TD] = drawing_text_set((bounds.size.w / 2 - stepx2) - 15, (bounds.size.w / 2) - 55, 10, 80, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "T D !", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter, window_layer);
    #endif
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_TD]), true);
  #else
    s_bitmap_layers[BITMAP_LAYER_FOOTBALL] = drawing_bitmap_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 4, stepy + gaps * 4 - 4, 8, 8, s_gbitmap_layers[GBITMAP_LAYER_FOOTBALL], window_layer);
    #ifdef PBL_RECT
      s_text_layers[TEXT_LAYER_TD] = drawing_text_set((bounds.size.w / 2 - stepx2) + (stepx1 / 2) - 10, stepy - (10+17), 25, 17, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "TD!", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #else
      s_text_layers[TEXT_LAYER_TD] = drawing_text_set((bounds.size.w / 2 - stepx2) - 15, (bounds.size.w / 2) - 35, 10, 50, (GColor){.argb = TEAMS[settings.FavoriteTeam].icon_color}, "T D !", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentCenter, window_layer);
    #endif
    layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_TD]), true);
  #endif
}
#endif