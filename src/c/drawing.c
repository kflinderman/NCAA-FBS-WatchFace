#include "drawing.h"
#include "structure.h"

static void drawing_line_update_proc(Layer *layer, GContext *ctx) {
  LinePoints *points = (LinePoints *)layer_get_data(layer);
  graphics_context_set_stroke_width(ctx, points->width);
  graphics_context_set_stroke_color(ctx, points->color);
  graphics_draw_line(ctx, GPoint(points->x1, points->y1), GPoint(points->x2, points->y2));
}

Layer* drawing_line_draw(GRect bounds, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                  uint16_t width, GColor color, Layer *window_layer) {
  Layer *line_layer = layer_create_with_data(bounds, sizeof(LinePoints));
  LinePoints *points = (LinePoints *)layer_get_data(line_layer);
  points->x1 = x1;
  points->y1 = y1;
  points->x2 = x2;
  points->y2 = y2;
  points->width = width;
  points->color = color;
  layer_set_update_proc(line_layer, drawing_line_update_proc);
  layer_add_child(window_layer, line_layer);
  return line_layer;
}

typedef struct {
  GPoint p1;
  GPoint p2;
  uint16_t width;
  GColor color;
} LineSegment;

typedef struct {
  LineSegment *segments; // heap-allocated, grown with realloc()
  uint16_t count;
  uint16_t capacity;
} MultiLineData;

static void drawing_multiline_update_proc(Layer *layer, GContext *ctx) {
  MultiLineData *data = (MultiLineData *)layer_get_data(layer);
  for (uint16_t i = 0; i < data->count; i++) {
    LineSegment *seg = &data->segments[i];
    graphics_context_set_stroke_width(ctx, seg->width);
    graphics_context_set_stroke_color(ctx, seg->color);
    graphics_draw_line(ctx, seg->p1, seg->p2);
  }
}

Layer* drawing_multiline_layer_create(GRect bounds, Layer *parent) {
  Layer *layer = layer_create_with_data(bounds, sizeof(MultiLineData));
  MultiLineData *data = (MultiLineData *)layer_get_data(layer);
  data->segments = NULL;
  data->count = 0;
  data->capacity = 0;
  layer_set_update_proc(layer, drawing_multiline_update_proc);
  layer_add_child(parent, layer);
  return layer;
}

void drawing_multiline_add_segment(Layer *layer, GPoint p1, GPoint p2, uint16_t width, GColor color) {
  MultiLineData *data = (MultiLineData *)layer_get_data(layer);

  if (data->count >= data->capacity) {
    uint16_t new_capacity = (data->capacity == 0) ? 4 : data->capacity * 2;
    LineSegment *new_segments = realloc(data->segments, new_capacity * sizeof(LineSegment));
    if (!new_segments) {
      #if defined(DEBUG)
      APP_LOG(APP_LOG_LEVEL_ERROR, "multiline_add_segment: realloc failed, segment dropped");
      #endif
      return;
    }
    data->segments = new_segments;
    data->capacity = new_capacity;
  }

  data->segments[data->count++] = (LineSegment){
    .p1 = p1, .p2 = p2, .width = width, .color = color
  };

  layer_mark_dirty(layer);
}

void drawing_multiline_clear(Layer *layer) {
  MultiLineData *data = (MultiLineData *)layer_get_data(layer);
  data->count = 0;
  layer_mark_dirty(layer);
}

void drawing_multiline_layer_destroy(Layer *layer) {
  if (!layer) return;
  MultiLineData *data = (MultiLineData *)layer_get_data(layer);
  free(data->segments);
  layer_destroy(layer);
}

void drawing_multiline_set_all_colors(Layer *layer, GColor color) {
  MultiLineData *data = (MultiLineData *)layer_get_data(layer);
  for (uint16_t i = 0; i < data->count; i++) {
    data->segments[i].color = color;
  }
  layer_mark_dirty(layer);
}

BitmapLayer* drawing_bitmap_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GBitmap *bitmap, Layer *window) {
  BitmapLayer *s_bitmap_layer = bitmap_layer_create(GRect(x, y, w, h));
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bitmap_layer, bitmap);
  layer_add_child(window, bitmap_layer_get_layer(s_bitmap_layer));

  return s_bitmap_layer;
}

TextLayer* drawing_text_set(uint16_t x, uint16_t y, uint16_t w, uint16_t h, GColor text_color,
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

void drawing_round_rect_update_proc(Layer *layer, GContext *ctx) {
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
