#pragma once
#include <pebble.h>


Layer* line_draw(GRect bounds, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t width, GColor color, Layer *window_layer);

/*******************************************************************
 * Multi-line layer
 *
 * Use this instead of line_draw() when you're drawing a *group* of
 * lines that will always move/hide/show together and don't need to
 * be individually repositioned after creation (e.g. the heart-rate
 * zigzag icon, the step-goal ladder). One Layer, any number of
 * segments, instead of one Layer per segment.
 *
 * Typical usage:
 *   Layer *hr_icon = multiline_layer_create(GRect(x, y, w, h), window_layer);
 *   multiline_add_segment(hr_icon, GPoint(0,0), GPoint(3,0), 1, GColorBlack);
 *   multiline_add_segment(hr_icon, GPoint(3,0), GPoint(6,5), 1, GColorBlack);
 *   ...
 *   // later, in main_window_unload:
 *   multiline_layer_destroy(hr_icon);
 *******************************************************************/

// Creates an empty multi-line layer and adds it as a child of `parent`.
// Add segments to it with multiline_add_segment().
Layer* multiline_layer_create(GRect bounds, Layer *parent);

// Appends one line segment to the layer and marks it dirty. Segment
// coordinates are relative to the layer's own bounds, same as any
// other layer's update_proc.
void multiline_add_segment(Layer *layer, GPoint p1, GPoint p2, uint16_t width, GColor color);

// Removes every segment from the layer (e.g. to redraw from scratch)
// without destroying the layer itself.
void multiline_clear(Layer *layer);

// Frees the layer's heap-allocated segment array, then destroys the
// layer. Always use this instead of plain layer_destroy() for layers
// created with multiline_layer_create().
void multiline_layer_destroy(Layer *layer);

void multiline_set_all_colors(Layer *layer, GColor color);

BitmapLayer* bitmap_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         GBitmap *bitmap, Layer *window);

TextLayer* text_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GColor text_color,
                     const char *initial_text, GFont font_handle,
                     GTextAlignment alignment, Layer *window);

// Exposed so main.c can assign it via layer_set_update_proc()
void round_rect_update_proc(Layer *layer, GContext *ctx);
