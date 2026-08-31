#include "animation.h"
#include "globals.h"
#include "structure.h"
#include "timekeeping.h"


#ifndef PBL_PLATFORM_APLITE
// Animation complete handler
static void animation_beat_team_stopped(Animation *animation, bool finished, void *context) {
  static bool returning = false;
  Layer *layer = (Layer *)context;

  if (!returning) {
    returning = true;
    // Wait 1 second, then animate back
    app_timer_register(1000, (AppTimerCallback)animation_beat_team_layer, layer);
  } else {
    returning = false; // Reset for next cycle
    s_animation = false;
  }
}

// animation_hide_text() removed: TIME/COUNTDOWN/SCORE and HOME/AWAY/DAY/
// HOUR were merged into shared layer slots (TEXT_LAYER_TIME and
// TEXT_LAYER_HOME/AWAY respectively) since each group occupied the same
// position and was never shown simultaneously. With one layer per
// position instead of three, there's nothing left to hide/show - the
// layer just always displays whichever mode's text was last written to
// it (see update_time()'s guard in timekeeping.c, and
// timekeeping_countdown()/api_score_display(), which only run when
// their own mode is the active one).

void animation_beat_team_layer(void) {
  static bool returning = false;

  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));

  // Define Keyframe Bounds (Calculated once)
  GRect beat_on_screen  = GRect(0, 0, bounds.size.w, bounds.size.h / 2 + 50);
  GRect beat_off_screen = GRect(-bounds.size.w, 0, bounds.size.w, bounds.size.h / 2 + 50);

  GRect rect_pos_start = GRect(beat_spot, -40 + beat_primary, 44, 40);
  GRect rect_pos_end   = GRect(beat_spot, -10 - beat_primary, 44, 40);

  // Assign Animation Direction based on 'returning' state
  GRect beat_from = returning ? beat_on_screen  : beat_off_screen;
  GRect beat_to   = returning ? beat_off_screen : beat_on_screen;

  GRect rect_from = returning ? rect_pos_end   : rect_pos_start;
  GRect rect_to   = returning ? rect_pos_start : rect_pos_end;

  // Unified Text Visibility Logic
  // TEXT_LAYER_TIME never needs hiding - one of update_time()/
  // timekeeping_countdown()/api_score_display() always keeps it holding
  // valid content for whichever mode is active. TEXT_LAYER_HOME/AWAY
  // (merged with the old DAY/HOUR sub-labels) still needs a real hidden
  // state though, for the case where neither countdown nor score is
  // active and only the plain clock should show.
  uint8_t target_mode = returning ? 1 : 2;

  bool show_countdown = settings.countdownBool && (!settings.scoreDisplayBool || !after_time);
  bool show_score     = settings.scoreDisplayBool && (!settings.countdownBool || after_time);

  
  bool timeTrue = true;
  
  if (show_countdown) {
    bool sub_labels_hidden = (settings.countdownDisplay == target_mode);
    //if(!sub_labels_hidden){
      globals_what2show(s_day_text, s_hour_text, s_countdown_text, !sub_labels_hidden, sub_labels_hidden);
      if (!sub_labels_hidden) timeTrue = false;
      /*
      text_layer_set_text(s_text_layers[TEXT_LAYER_HOME], s_day_text);
      text_layer_set_text(s_text_layers[TEXT_LAYER_AWAY], s_hour_text);
      text_layer_set_text(s_text_layers[TEXT_LAYER_TIME], s_countdown_text);

      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), false);
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), false);
      layer_set_hidden(s_layers[LAYER_SCORE_I], true);
      timeTrue = false;
      */
    //}
  }

  if (show_score) {
    bool sub_labels_hidden = (settings.scoreLocation == target_mode);
    //if(!sub_labels_hidden){
      globals_what2show(s_home_text, s_away_text, s_score_text, !sub_labels_hidden, !sub_labels_hidden);
       if (!sub_labels_hidden) timeTrue = false;
      /*
      text_layer_set_text(s_text_layers[TEXT_LAYER_HOME], s_home_text);
      text_layer_set_text(s_text_layers[TEXT_LAYER_AWAY], s_away_text);
      text_layer_set_text(s_text_layers[TEXT_LAYER_TIME], s_score_text);

      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), false);
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), false);
      layer_set_hidden(s_layers[LAYER_SCORE_I], false);
      timeTrue = false;
      */
    //}
  }
  
  if(timeTrue){
    globals_what2show("", "", s_time_text, true, true);
    /*
    text_layer_set_text(s_text_layers[TEXT_LAYER_TIME], s_time_text);

    layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), true);
    layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), true);
    layer_set_hidden(s_layers[LAYER_SCORE_I], true);
    */
  }

  // Schedule Layer Animations
  PropertyAnimation *anim_beat = property_animation_create_layer_frame(s_layers[LAYER_BEAT_TEAM], &beat_from, &beat_to);
  animation_set_duration((Animation*)anim_beat, 1000);
  animation_set_handlers((Animation*)anim_beat, (AnimationHandlers){
    .stopped = animation_beat_team_stopped
  }, s_layers[LAYER_BEAT_TEAM]);
  animation_schedule((Animation*)anim_beat);

  PropertyAnimation *anim_rect = property_animation_create_layer_frame(s_layers[LAYER_BEAT_RECT], &rect_from, &rect_to);
  animation_set_duration((Animation*)anim_rect, 1000);
  animation_schedule((Animation*)anim_rect);

  // Toggle State
  returning = !returning;
}
#endif

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