#include <pebble.h>
#include "drawarc.h"

enum {
  SCREEN_WIDTH = 144,
  SCREEN_HEIGHT = 168,
  CIRCLE_THICKNESS = 5
};

typedef struct {
  float percent;
  int radius;
} Arc;

static Window *window;
static Layer *hours_layer;
static Layer *minutes_layer;
static Layer *seconds_layer;

static void arc_update_proc(Layer *layer, GContext *ctx) {
  Arc *arc = (Arc*) layer_get_data(layer);

  GPoint origin = GPoint(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_arc(ctx, origin, arc->radius, CIRCLE_THICKNESS, -90, 360 * arc->percent - 90);
}

static Layer *create_arc_layer(Layer *window_layer, GRect bounds, Layer *arc_layer, Arc *arc) {
  arc_layer = layer_create_with_data(bounds, sizeof(Arc));
  layer_set_update_proc(arc_layer, arc_update_proc);
  layer_add_child(window_layer, arc_layer);

  Arc *arc_ctx = (Arc*) layer_get_data(arc_layer);
  arc_ctx->radius = arc->radius;
  arc_ctx->percent = arc->percent;

  return arc_layer;
}

static void update_arc(Layer *arc_layer, float p) {
  Arc *arc_ctx = (Arc*) layer_get_data(arc_layer);
  float old_p = arc_ctx->percent;
  arc_ctx->percent = p;

  if (p != old_p) layer_mark_dirty(arc_layer);
}

static void update_time() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  update_arc(hours_layer, (float) t->tm_hour / 12);
  update_arc(minutes_layer, (float) t->tm_min / 60);
  update_arc(seconds_layer, (float) t->tm_sec / 60);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  hours_layer = create_arc_layer(window_layer, bounds, hours_layer, &(Arc) {
    .percent = 0,
    .radius = 20
  });
  minutes_layer = create_arc_layer(window_layer, bounds, minutes_layer, &(Arc) {
    .percent = 0,
    .radius = 40
  });
  seconds_layer = create_arc_layer(window_layer, bounds, seconds_layer, &(Arc) {
    .percent = 0,
    .radius = 60
  });

  update_time();
}

static void window_unload(Window *window) {
  layer_destroy(hours_layer);
  layer_destroy(minutes_layer);
  layer_destroy(seconds_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
