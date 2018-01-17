#include <pebble.h>

static Window *main_window;
static TextLayer *text_layer_time;
static TextLayer *text_layer_date;
static GRect bounds;
static int color_index = 0;

static void layer_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char time_buffer[8];
  static char time_buffer_fixed[7];
  static char date_buffer[16];
  GRect bounds = layer_get_bounds(layer);
  
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(date_buffer, sizeof(date_buffer), "%a, %d %b", tick_time);
  
  if (false) {
    color_index++;
    GColor color = GColorRed;
    
    switch (color_index) {
      case 0:
        color = GColorRed;
        break;
      case 1:
        color = GColorOrange;
        break;
      case 2:
        color = GColorYellow;
        break;
      case 3:
        color = GColorGreen;
        break;
      case 4:
        color = GColorCyan;
        break;
      case 6:
        color = GColorPurple;
        break;
      default:
        color_index = 0;
        break;
    }
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, GRect(bounds.origin.x + 2, bounds.origin.y + 2, bounds.size.w - 4, bounds.size.h - 4), 10, GCornersAll);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(bounds.origin.x + 4, bounds.origin.y + 4, bounds.size.w - 8, bounds.size.h - 8), 10, GCornersAll);
  } 
  else {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  }
  
  if (time_buffer[0] == '0') {    
    strncpy(time_buffer_fixed, time_buffer + 1, 7);
    text_layer_set_text(text_layer_time, time_buffer_fixed);
  }
  else {
    text_layer_set_text(text_layer_time, time_buffer);
  }
  
  text_layer_set_text(text_layer_date, date_buffer);
  
  GSize text_time_size = text_layer_get_content_size(text_layer_time);
  GRect text_time_rect = GRect(bounds.size.w / 2 - text_time_size.w / 2, (bounds.size.h - 16) / 2 - text_time_size.h / 2, text_time_size.w + 64, text_time_size.h);
  layer_set_frame(text_layer_get_layer(text_layer_time), text_time_rect);
  text_time_rect.size.w -= 64;
  
  GSize text_date_size = text_layer_get_content_size(text_layer_date);
  GRect text_date_rect = GRect(text_time_rect.origin.x + text_time_rect.size.w / 2 - text_date_size.w / 2, text_time_rect.origin.y - text_date_size.h + 4, text_date_size.w + 64, text_date_size.h + 8);
  layer_set_frame(text_layer_get_layer(text_layer_date), text_date_rect);
  text_date_rect.size.w -= 64;

  int battery_percentage = battery_state_service_peek().charge_percent;
  GPoint battery_position = GPoint(bounds.size.w / 2 - 12, text_time_rect.origin.y + text_time_rect.size.h + 14);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(battery_position.x, battery_position.y, 24, 12), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(battery_position.x + 24, battery_position.y + 2, 2, 8), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(battery_position.x + 2, battery_position.y + 2, 20, 8), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, battery_percentage >= 25 ? GColorGreen : GColorRed);
  graphics_fill_rect(ctx, GRect(battery_position.x + 2, battery_position.y + 2, 20.0f * (battery_percentage / 100.0f), 8), 0, GCornerNone);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  
  text_layer_time = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(text_layer_time, GColorClear);
  text_layer_set_text_color(text_layer_time, GColorBlack);
  text_layer_set_font(text_layer_time, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_enable_screen_text_flow_and_paging(text_layer_time, -16);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_time));
  
  text_layer_date = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(text_layer_date, GColorClear);
  text_layer_set_text_color(text_layer_date, GColorBlack);
  text_layer_set_font(text_layer_date, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_enable_screen_text_flow_and_paging(text_layer_date, -16);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_date));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(text_layer_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(main_window));
}

void init(void) {
  main_window = window_create();
  layer_set_update_proc(window_get_root_layer(main_window), layer_update_proc);

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(main_window, true);
  layer_mark_dirty(window_get_root_layer(main_window));
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void deinit(void) {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}