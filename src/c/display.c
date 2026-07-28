#include <pebble.h>
#include "display.h"
#include "globals.h"
#include "drawing.h"

void display_setupBag(){
   if(settings.bagBool) {
      // Prevent a memory leak by only creating the bitmap if it doesn't already exist
      if(!s_bag_bitmap) {
          s_bag_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BAG);
      }
  
      // Set the bitmap on the active layer, and explicitly clear it from the inactive layer
      if (settings.DisplayTeam > 1) {
          bitmap_layer_set_bitmap(s_bag_layerb, s_bag_bitmap);
          bitmap_layer_set_bitmap(s_bag_layerf, NULL); 
      } else {
          bitmap_layer_set_bitmap(s_bag_layerf, s_bag_bitmap);
          bitmap_layer_set_bitmap(s_bag_layerb, NULL); 
      }
  } else {
      // Clear the bitmap from BOTH layers before destroying it
      bitmap_layer_set_bitmap(s_bag_layerb, NULL);
      bitmap_layer_set_bitmap(s_bag_layerf, NULL);
  
      // Safely destroy and nullify the pointer
      if(s_bag_bitmap) {
          gbitmap_destroy(s_bag_bitmap);
          s_bag_bitmap = NULL; 
      }
  }
}

void display_beat_textbox(Layer *window_layer, GRect bounds){
  #ifdef PBL_RECT
    beat_spot = 0;
  #else
    beat_spot = bounds.size.w / 2 - 22;
  #endif
  
  beat_primary = settings.DisplayTeam;
  rect_beat_layer = layer_create_with_data(GRect(beat_spot, -40 + beat_primary, 45, 40), sizeof(RoundRectData));
  RoundRectData *rect_beat_data = (RoundRectData *)layer_get_data(rect_beat_layer);
  rect_beat_data->fill_color = GColorWhite;
  layer_set_update_proc(rect_beat_layer, drawing_round_rect_update_proc);
  layer_add_child(window_layer, rect_beat_layer);

  s_beat_layer = drawing_text_set(0, 10, 44, 30, GColorBlack, "BEAT", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter, rect_beat_layer);
  
}

void display_main_time_layer(Layer *window_layer, GRect bounds){
  rect_layer = layer_create_with_data(GRect(0, bounds.size.h * rect_h, bounds.size.w, 100), sizeof(RoundRectData));

  RoundRectData *rect_data = (RoundRectData *)layer_get_data(rect_layer);
  rect_data->fill_color = GColorWhite;
  layer_set_update_proc(rect_layer, drawing_round_rect_update_proc);
  layer_add_child(window_layer, rect_layer);
}

void display_beatteam(Layer *window_layer, GRect bounds){
  // Here's where I increased the size of the moving box FYI. Started at bounds.size.h / 2 + 50
  beat_team_layer = layer_create_with_data(GRect(-bounds.size.w - 10, 0, bounds.size.w + 10, bounds.size.h / 2 + 100), sizeof(RoundRectData));
  RoundRectData *beat_data = (RoundRectData *)layer_get_data(beat_team_layer);

  if (settings.DisplayTeam > 1) {
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color};
  } else {
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.BeatTeam].color};
  }
  
  // Create beat_team_layer with per-layer color data
  layer_set_update_proc(beat_team_layer, drawing_round_rect_update_proc);
  layer_add_child(window_layer, beat_team_layer);
  s_beat_team_layer = drawing_bitmap_set((bounds.size.w - bitmap_size) / 2, bounds.size.h * 0.025, bitmap_size, bitmap_size, s_beat_team_bitmap, beat_team_layer);
  s_bag_layerb = drawing_bitmap_set(0, 0, bitmap_size, bitmap_size, s_bag_bitmap, bitmap_layer_get_layer(s_beat_team_layer));
  
}