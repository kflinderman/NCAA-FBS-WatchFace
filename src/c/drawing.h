#pragma once
#include <pebble.h>

Layer* drawing_line_draw(GRect bounds, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, GColor color, Layer *window_layer);
Layer* drawing_multiline_layer_create(GRect bounds, Layer *parent);
void drawing_multiline_add_segment(Layer *layer, GPoint p1, GPoint p2, uint16_t width, GColor color);
void drawing_multiline_clear(Layer *layer);
void drawing_multiline_layer_destroy(Layer *layer);
void drawing_multiline_set_all_colors(Layer *layer, GColor color);
BitmapLayer* drawing_bitmap_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GBitmap *bitmap, Layer *window);
TextLayer* drawing_text_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GColor text_color, const char *initial_text, GFont font_handle, GTextAlignment alignment, Layer *window);
void drawing_round_rect_update_proc(Layer *layer, GContext *ctx);