#pragma once
#include <pebble.h>

//void animation_hide_text(bool score, bool time);

void animation_beat_team_layer(void);

// Subscribes this window's unobstructed-area handlers. Call once from
// main_window_load() after all layers exist.
void animation_subscribe_unobstructed_area(void);

// Applies the correct layout immediately (e.g. if Quick View is already
// active when the window loads).
void animation_prv_unobstructed_change(AnimationProgress progress, void *context);
