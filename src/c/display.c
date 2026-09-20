#include <pebble.h>
#include "display.h"
#include "globals.h"
#include "drawing.h"


#ifndef PBL_PLATFORM_APLITE
//Function for the embarrassed bag layer
void display_setupBag(GColor bagColor) {
  if (s_gbitmap_layers[GBITMAP_LAYER_BAG]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_BAG]);
    s_gbitmap_layers[GBITMAP_LAYER_BAG] = NULL;
  }
  if (settings.bagBool) {
    #if defined(PBL_COLOR)
    s_gbitmap_layers[GBITMAP_LAYER_BAG] = gbitmap_create_with_resource(RESOURCE_ID_BAG);
    #else
    //figure out whether to use the black or white bag based on the team colors
    uint32_t res_id = gcolor_equal(bagColor, GColorWhite) ? RESOURCE_ID_BAG : RESOURCE_ID_BAGB;
    s_gbitmap_layers[GBITMAP_LAYER_BAG] = gbitmap_create_with_resource(res_id);
    #endif

    //Determine where to put the bag, on animations or the main window
    if (settings.DisplayTeam > 1) {
      bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BAGB], s_gbitmap_layers[GBITMAP_LAYER_BAG]);
      bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BAG], NULL); 
    } else {
      bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BAG], s_gbitmap_layers[GBITMAP_LAYER_BAG]);
      bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BAGB], NULL); 
    }
  } else {
    // Clear the bitmap from BOTH layers before destroying it
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BAGB], NULL);
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BAG], NULL);

    // Safely destroy and nullify the pointer
    if(s_gbitmap_layers[GBITMAP_LAYER_BAG]) {
      gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_BAG]);
      s_gbitmap_layers[GBITMAP_LAYER_BAG] = NULL; 
    }
  }
}
#endif

//Function for the beat text box that appears on opposing teams
void display_beat_textbox(Layer *window_layer, GRect bounds){
  #ifdef PBL_RECT
  beat_spot = 0;
  #else
  beat_spot = bounds.size.w / 2 - 22;
  #endif

  beat_primary = settings.DisplayTeam;
  s_layers[LAYER_BEAT_RECT] = layer_create_with_data(GRect(beat_spot, -40 + beat_primary, 45, 40), sizeof(RoundRectData));
  RoundRectData *rect_beat_data = (RoundRectData *)layer_get_data(s_layers[LAYER_BEAT_RECT]);
  rect_beat_data->fill_color = GColorWhite;
  layer_set_update_proc(s_layers[LAYER_BEAT_RECT], drawing_round_rect_update_proc);
  layer_add_child(window_layer, s_layers[LAYER_BEAT_RECT]);

  s_text_layers[TEXT_LAYER_BEAT] = drawing_text_set(0, 10, 44, 30, GColorBlack, "BEAT", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter, s_layers[LAYER_BEAT_RECT]);

}

//Function for the main time box layer at the bottom
void display_main_time_layer(Layer *window_layer, GRect bounds){
  s_layers[LAYER_RECT] = layer_create_with_data(GRect(0, (bounds.size.h * RECT_H) / 1000, bounds.size.w, 100), sizeof(RoundRectData));

  RoundRectData *rect_data = (RoundRectData *)layer_get_data(s_layers[LAYER_RECT]);
  rect_data->fill_color = GColorWhite;
  layer_set_update_proc(s_layers[LAYER_RECT], drawing_round_rect_update_proc);
  layer_add_child(window_layer, s_layers[LAYER_RECT]);
}

#ifndef PBL_PLATFORM_APLITE
//Function for the logo and moving background for opposing teams
void display_beatteam(Layer *window_layer, GRect bounds){
  s_layers[LAYER_BEAT_TEAM] = layer_create_with_data(GRect(-bounds.size.w - 10, 0, bounds.size.w + 10, bounds.size.h / 2 + 100), sizeof(RoundRectData));
  RoundRectData *beat_data = (RoundRectData *)layer_get_data(s_layers[LAYER_BEAT_TEAM]);

  //Determine if the main display is favorite team or opposing
  if (settings.DisplayTeam > 1) {
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.FavoriteTeam].color};
  } else {
    beat_data->fill_color = (GColor){.argb = TEAMS[settings.BeatTeam].color};
  }

  // Create beat_team_layer with per-layer color data
  layer_set_update_proc(s_layers[LAYER_BEAT_TEAM], drawing_round_rect_update_proc);
  layer_add_child(window_layer, s_layers[LAYER_BEAT_TEAM]);
  s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM] = drawing_bitmap_set((bounds.size.w - BITMAP_SIZE) / 2, bounds.size.h * 0.025, BITMAP_SIZE, BITMAP_SIZE, s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM], s_layers[LAYER_BEAT_TEAM]);
  s_bitmap_layers[BITMAP_LAYER_BAGB] = drawing_bitmap_set(0, 0, BITMAP_SIZE, BITMAP_SIZE, s_gbitmap_layers[GBITMAP_LAYER_BAG], bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM]));
}
#endif