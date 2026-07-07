#include "drawing.h"
#include "structure.h"

static void line_update_proc(Layer *layer, GContext *ctx) {
  LinePoints *points = (LinePoints *)layer_get_data(layer);
  graphics_context_set_stroke_width(ctx, points->width);
  graphics_context_set_stroke_color(ctx, points->color);
  graphics_draw_line(ctx, GPoint(points->x1, points->y1), GPoint(points->x2, points->y2));
}

Layer* line_draw(GRect bounds, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t width, GColor color, Layer *window_layer) {
  Layer *line_layer = layer_create_with_data(bounds, sizeof(LinePoints));
  LinePoints *points = (LinePoints *)layer_get_data(line_layer);
  points->x1 = x1;
  points->y1 = y1;
  points->x2 = x2;
  points->y2 = y2;
  points->width = width;
  points->color = color;
  layer_set_update_proc(line_layer, line_update_proc);
  layer_add_child(window_layer, line_layer);
  return line_layer;
}

BitmapLayer* bitmap_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GBitmap *bitmap, Layer *window) {
  BitmapLayer *s_bitmap_layer = bitmap_layer_create(GRect(x, y, w, h));
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bitmap_layer, bitmap);
  layer_add_child(window, bitmap_layer_get_layer(s_bitmap_layer));

  return s_bitmap_layer;
}

// Kept for parity with the original file — not currently wired to any layer's
// update_proc, but preserved in case you want a bordered TextLayer later.
static void text_layer_border_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, bounds);
}

TextLayer* text_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GColor text_color,
                     const char *initial_text, GFont font_handle,
                     GTextAlignment alignment, Layer *window) {
  TextLayer *text_layer = text_layer_create(GRect(x, y, w, h));

  text_layer_set_text_color(text_layer, text_color);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_font(text_layer, font_handle);
  text_layer_set_text_alignment(text_layer, alignment);
  text_layer_set_text(text_layer, initial_text);
  layer_add_child(window, text_layer_get_layer(text_layer));

  return text_layer;
}

void round_rect_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  uint16_t stroke_radius = 8;
  // If layer has RoundRectData, use its fill_color; otherwise default to white
  RoundRectData *data = (RoundRectData *)layer_get_data(layer);
  GColor fill = (data != NULL) ? data->fill_color : GColorWhite;
  graphics_context_set_fill_color(ctx, fill);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, stroke_radius, GCornersAll);
  graphics_draw_round_rect(ctx, bounds, stroke_radius);
}
