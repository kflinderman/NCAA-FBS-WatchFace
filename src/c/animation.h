#pragma once
#include <pebble.h>

void animation_beat_team_layer(void);
#ifndef PBL_PLATFORM_APLITE
void animation_subscribe_unobstructed_area(void);
void animation_prv_unobstructed_change(AnimationProgress progress, void *context);
#endif
