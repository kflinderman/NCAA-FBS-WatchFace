#include "animation.h"
#include "globals.h"
#include "timekeeping.h"


#ifndef PBL_PLATFORM_APLITE
static void animation_beat_team_stopped(Animation *animation, bool finished, void *context) {
  static bool returning = false;
  Layer *layer = (Layer *)context;

  if (!returning) {
    returning = true;
    app_timer_register(1000, (AppTimerCallback)animation_beat_team_layer, layer);
  } else {
    returning = false;
    s_animation = false;
  }
}
#endif

void animation_beat_team_layer(void) {
  static bool returning = false;

  #ifndef PBL_PLATFORM_APLITE
  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));

  GRect beat_on_screen  = GRect(0, 0, bounds.size.w, bounds.size.h / 2 + 50);
  GRect beat_off_screen = GRect(-bounds.size.w, 0, bounds.size.w, bounds.size.h / 2 + 50);

  GRect rect_pos_start = GRect(beat_spot, -40 + beat_primary, 44, 40);
  GRect rect_pos_end   = GRect(beat_spot, -10 - beat_primary, 44, 40);

  GRect beat_from = returning ? beat_on_screen  : beat_off_screen;
  GRect beat_to   = returning ? beat_off_screen : beat_on_screen;

  GRect rect_from = returning ? rect_pos_end   : rect_pos_start;
  GRect rect_to   = returning ? rect_pos_start : rect_pos_end;
  #endif

  uint8_t target_mode = returning ? 1 : 2;

  bool show_countdown = settings.countdownBool && (!settings.scoreDisplayBool || !after_time);
  bool show_score     = settings.scoreDisplayBool && (!settings.countdownBool || after_time);

  bool timeTrue = true;

  if (show_countdown) {
    bool sub_labels_hidden = (settings.countdownDisplay == target_mode);
    globals_what2show(s_day_text, s_hour_text, s_countdown_text, false, true);
    if (!sub_labels_hidden) timeTrue = false;
  }

  if (show_score) {
    bool sub_labels_hidden = (settings.scoreLocation == target_mode);
    globals_what2show(s_home_text, s_away_text, s_score_text, false, false);
    if (!sub_labels_hidden) timeTrue = false;
  }

  if (timeTrue) {
    globals_what2show("", "", s_time_text, true, true);
  }

  #ifndef PBL_PLATFORM_APLITE
  PropertyAnimation *anim_beat = property_animation_create_layer_frame(s_layers[LAYER_BEAT_TEAM], &beat_from, &beat_to);
  animation_set_duration((Animation*)anim_beat, 1000);
  animation_set_handlers((Animation*)anim_beat, (AnimationHandlers){
    .stopped = animation_beat_team_stopped
  }, s_layers[LAYER_BEAT_TEAM]);
  animation_schedule((Animation*)anim_beat);

  PropertyAnimation *anim_rect = property_animation_create_layer_frame(s_layers[LAYER_BEAT_RECT], &rect_from, &rect_to);
  animation_set_duration((Animation*)anim_rect, 1000);
  animation_schedule((Animation*)anim_rect);
  #else
  if (!returning) {
    app_timer_register(3000, (AppTimerCallback)animation_beat_team_layer, NULL);
  } else {
    s_animation = false;
  }
  #endif

  returning = !returning;
}

static void animation_layermove(GRect tmp_bounds, int diff, Layer *tmp_layer, uint16_t origin_permil, int bump, uint16_t bmp_ratio_permil) {
  GRect move_frame = layer_get_frame(tmp_layer);
  move_frame.origin.y = (((tmp_bounds.size.h * origin_permil) / 1000) + bump) - (diff * bmp_ratio_permil) / 1000;
  layer_set_frame(tmp_layer, move_frame);
}

static void animation_linemove(GRect tmp_bounds, int diff, Layer *tmp_layer, uint16_t Y1RATIO, uint16_t Y2RATIO) {
  LinePoints *pointsMove = (LinePoints *)layer_get_data(tmp_layer);
  pointsMove->y1 = (tmp_bounds.size.h * Y1RATIO) / 1000 - diff;
  pointsMove->y2 = (tmp_bounds.size.h * Y2RATIO) / 1000 - diff;
  layer_mark_dirty(tmp_layer);
}

void animation_prv_unobstructed_change(AnimationProgress progress, void *context) {
  Layer *root = window_get_root_layer(s_main_window);
  GRect obsBounds = layer_get_unobstructed_bounds(root);
  GRect unBounds = layer_get_bounds(root);

  // Reposition to fit in the available space
  int bound_diff = unBounds.size.h - obsBounds.size.h;

  #if PBL_DISPLAY_HEIGHT > 180
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_TIME]), RECT_H, -2, 1000);
  
  #ifdef PBL_ROUND
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), TIME_H, 8, 1000);
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), TIME_H, 8, 1000);
  #else
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), 1000, -16, 1000);
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), 1000, -16, 1000);
  animation_linemove(unBounds, bound_diff, s_layers[LAYER_VERT], VERT_1, VERT_2);
  #endif
  
  #else
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_TIME]), RECT_H, -5, 1000);
  
  #ifdef PBL_ROUND
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), TIME_H, TIME_Y - 20, 1000);
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), TIME_H, TIME_Y - 20, 1000);
  #else
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), 1000, -14, 1000);
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), 1000, -14, 1000);
  animation_linemove(unBounds, bound_diff, s_layers[LAYER_VERT], VERT_1, VERT_2);
  #endif
  #endif
  
  #ifndef PBL_PLATFORM_APLITE
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM]), 25, 0, 500);
  #endif
  
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_DATE]), DATE_H, 0, 1000);
  animation_layermove(unBounds, bound_diff, s_layers[LAYER_RECT], RECT_H, 0, 1000);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]), 25, 0, 500);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BT]), VERT_2, 3, 1000);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BATT]), VERT_2, 3, 1000);
  animation_linemove(unBounds, bound_diff, s_layers[LAYER_HOR], VERT_2, VERT_2);
  animation_linemove(unBounds, bound_diff, s_layers[LAYER_SCORE_I], VERT_3, VERT_4);
}

  

void animation_subscribe_unobstructed_area(void) {
  UnobstructedAreaHandlers handlers = {
    .will_change = NULL,
    .change = animation_prv_unobstructed_change,
    .did_change = NULL
  };
  unobstructed_area_service_subscribe(handlers, NULL);
}