#include "animation.h"
#include "globals.h"
#include "structure.h"
#include "timekeeping.h"


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

void animation_hide_text(bool count, bool score, bool time){
  layer_set_hidden(text_layer_get_layer(s_day_layer), count);
  layer_set_hidden(text_layer_get_layer(s_hr_layer), count);
  layer_set_hidden(text_layer_get_layer(s_countdown_layer), count);
  layer_set_hidden(text_layer_get_layer(s_home_layer), score);
  layer_set_hidden(text_layer_get_layer(s_away_layer), score);
  layer_set_hidden(text_layer_get_layer(s_score_layer), score);
  layer_set_hidden(text_layer_get_layer(s_time_layer), time);
}

void animation_beat_team_layer() {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GRect beat_from, beat_to;
  GRect rect_from, rect_to;
  static bool returning = false;

  if (!returning) {
    beat_from = GRect(-bounds.size.w, 0, bounds.size.w, bounds.size.h / 2 + 50);
    beat_to   = GRect(0, 0, bounds.size.w, bounds.size.h / 2 + 50);

    rect_from = GRect(beat_spot, -40 + beat_primary, 44, 40);
    rect_to   = GRect(beat_spot, -10 - beat_primary, 44, 40);
    
    if (settings.countdownBool){
      if ((!after_time && settings.scoreDisplayBool) || !settings.scoreDisplayBool){
        if (settings.countdownDisplay != 2){
          animation_hide_text(false, true, true);
        }
        else{
          animation_hide_text(true, true, false);
        }
      }
    }

    if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time)){
        if (settings.scoreLocation != 2){
          animation_hide_text(true, false, true);
        }
      else{
          animation_hide_text(true, true, false);
      }
    }
    
  } else {
    beat_from = GRect(0, 0, bounds.size.w, bounds.size.h / 2 + 50);
    beat_to   = GRect(-bounds.size.w, 0, bounds.size.w, bounds.size.h / 2 + 50);

    rect_from = GRect(beat_spot, -10 - beat_primary, 44, 40);
    rect_to   = GRect(beat_spot, -40 + beat_primary, 44, 40);
    
    if (settings.countdownBool){
      if ((!after_time && settings.scoreDisplayBool) || !settings.scoreDisplayBool){
        if (settings.countdownDisplay != 1){
          animation_hide_text(false, true, true);
        }
        else{
          animation_hide_text(true, true, false);
        }
      }
    }

    if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time)){
        if (settings.scoreLocation != 1){
          animation_hide_text(true, false, true);
        }
      else{
          animation_hide_text(true, true, false);
      }
    }
  }

  // Animate beat_team_layer
  PropertyAnimation *anim_beat = property_animation_create_layer_frame(beat_team_layer, &beat_from, &beat_to);
  animation_set_duration((Animation*)anim_beat, 1000);
  animation_set_handlers((Animation*)anim_beat, (AnimationHandlers){
    .stopped = animation_beat_team_stopped
  }, beat_team_layer);
  animation_schedule((Animation*)anim_beat);

  // Animate rect_beat_layer
  PropertyAnimation *anim_rect = property_animation_create_layer_frame(rect_beat_layer, &rect_from, &rect_to);
  animation_set_duration((Animation*)anim_rect, 1000);
  animation_schedule((Animation*)anim_rect);

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

  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_time_layer), time_h, 0, 1);
  animation_layermove(unBounds, bound_diff, text_layer_get_layer(s_date_layer), date_h, 0, 1);
  animation_layermove(unBounds, bound_diff, rect_layer, rect_h, 0, 1);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_logo_layer), 0.025, 0, 0.5);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_beat_team_layer), 0.025, 0, 0.5);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_bt_layer), vert_2, 3, 1);
  animation_layermove(unBounds, bound_diff, bitmap_layer_get_layer(s_batt_layer), vert_2, 3, 1);

#ifdef PBL_RECT
  LinePoints *points = (LinePoints *)layer_get_data(vertical_line);
  points->y1 = unBounds.size.h * vert_1 - bound_diff;
  points->y2 = unBounds.size.h * vert_2 - bound_diff;
  layer_mark_dirty(vertical_line);
#endif

  LinePoints *points2 = (LinePoints *)layer_get_data(horizontal_line);
  points2->y1 = unBounds.size.h * vert_2 - bound_diff;
  points2->y2 = unBounds.size.h * vert_2 - bound_diff;
  layer_mark_dirty(horizontal_line);
}

void animation_subscribe_unobstructed_area(void) {
  UnobstructedAreaHandlers handlers = {
    .will_change = NULL,
    .change = animation_prv_unobstructed_change,
    .did_change = NULL
  };
  unobstructed_area_service_subscribe(handlers, NULL);
}
