#pragma once
#include <pebble.h>

Layer* line_draw(GRect bounds, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t width, GColor color, Layer *window_layer);

BitmapLayer* bitmap_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         GBitmap *bitmap, Layer *window);

TextLayer* text_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GColor text_color,
                     const char *initial_text, GFont font_handle,
                     GTextAlignment alignment, Layer *window);

// Exposed so main.c can assign it via layer_set_update_proc()
void round_rect_update_proc(Layer *layer, GContext *ctx);
