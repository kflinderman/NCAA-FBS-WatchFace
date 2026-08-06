#include "animation.h"
#include "globals.h"
#include "structure.h"
#include "timekeeping.h"

static bool returning = false;

// Animation complete handler
static void animation_beat_team_stopped(Animation *animation, bool finished, void *context) {
  //static bool returning = false;
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

void animation_hide_text(bool count, bool score, bool time){
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_DAY]), count);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOUR]), count);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_COUNTDOWN]), count);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), score);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), score);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_SCORE]), score);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_TIME]), time);
}

void animation_beat_team_layer(void) {
  //static bool returning = false;

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
  uint8_t target_mode = returning ? 1 : 2;

  bool show_countdown = settings.countdownBool && (!settings.scoreDisplayBool || !after_time);
  bool show_score     = settings.scoreDisplayBool && (!settings.countdownBool || after_time);

  if (show_countdown) {
    if (settings.countdownDisplay != target_mode) {
      animation_hide_text(false, true, true);
    } else {
      animation_hide_text(true, true, false);
    }
  }

  if (show_score) {
    if (settings.scoreLocation != target_mode) {
      animation_hide_text(true, false, true);
    } else {
      animation_hide_text(true, true, false);
    }
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

static void animation_layermove(GRect tmp_bounds, int diff, Layer *tmp_layer, float origin, uint16_t bump, float bmp_ratio) {
  GRect move_frame = layer_get_frame(tmp_layer);
  move_frame.origin.y = ((tmp_bounds.size.h * origin) + bump) - diff * bmp_ratio;
  layer_set_frame(tmp_layer, move_frame);
}

void animation_prv_unobstructed_change(AnimationProgress progress, void *context) {
  Layer *root = window_get_root_layer(s_main_window);
  GRect obsBounds = layer_get_unobstructed_bounds(root);
  GRect unBounds = layer_get_bounds(root);

  // Reposition to fit in the available space
  int bound_diff = unBounds.size.h - obsBounds.size.h;

  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_TIME]), time_h, 0, 1);
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_text_layers[TEXT_LAYER_DATE]), date_h, 0, 1);
  animation_layermove(unBounds, bound_diff, s_layers[LAYER_RECT], rect_h, 0, 1);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_LOGO]), 0.025, 0, 0.5);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM]), 0.025, 0, 0.5);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BT]), vert_2, 3, 1);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BATT]), vert_2, 3, 1);

#ifdef PBL_RECT
  LinePoints *points = (LinePoints *)layer_get_data(s_layers[LAYER_VERT]);
  points->y1 = unBounds.size.h * vert_1 - bound_diff;
  points->y2 = unBounds.size.h * vert_2 - bound_diff;
  layer_mark_dirty(s_layers[LAYER_VERT]);
#endif

  LinePoints *points2 = (LinePoints *)layer_get_data(s_layers[LAYER_HOR]);
  points2->y1 = unBounds.size.h * vert_2 - bound_diff;
  points2->y2 = unBounds.size.h * vert_2 - bound_diff;
  layer_mark_dirty(s_layers[LAYER_HOR]);
}

void animation_subscribe_unobstructed_area(void) {
  UnobstructedAreaHandlers handlers = {
    .will_change = NULL,
    .change = animation_prv_unobstructed_change,
    .did_change = NULL
  };
  unobstructed_area_service_subscribe(handlers, NULL);
}
