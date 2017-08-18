#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_first_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_second_time_layer;

static GFont s_time_font;

#define CCT (+8)

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_first_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(28, 22), bounds.size.w, 50));
  
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(78, 72), bounds.size.w, 50));
  
  s_second_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(128, 122), bounds.size.w, 50));
  
  // Create GFont
  // s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

  // Apply to TextLayer
  //text_layer_set_font(s_first_time_layer, s_time_font);

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_first_time_layer, GColorClear);
  text_layer_set_text_color(s_first_time_layer, GColorBlack);
  text_layer_set_text(s_first_time_layer, "00:00");
  text_layer_set_font(s_first_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_first_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_first_time_layer));
  
  // date 
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "00/00");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  
  // second time
  text_layer_set_background_color(s_second_time_layer, GColorClear);
  text_layer_set_text_color(s_second_time_layer, GColorBlack);
  text_layer_set_text(s_second_time_layer, "00:00");
  text_layer_set_font(s_second_time_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_second_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_second_time_layer));
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_localtime_buffer[8];
  strftime(s_localtime_buffer, sizeof(s_localtime_buffer), "%H:%M" , tick_time);

  // Display this time on the fist TextLayer
  text_layer_set_text(s_first_time_layer, s_localtime_buffer);
  
  // Display date
  static char s_date_buffer[8];
  snprintf(s_date_buffer, sizeof(s_date_buffer), "%02d/%02d", tick_time->tm_mon+1, tick_time->tm_mday);
  text_layer_set_text(s_date_layer, s_date_buffer);
  
  // Display China time
  struct tm *gmt = gmtime(&temp);
  static char s_cct_buffer[12];
  
  snprintf(s_cct_buffer, sizeof(s_cct_buffer), "- %02d:%02d -", (gmt->tm_hour+CCT)%24, gmt->tm_min);
  text_layer_set_text(s_second_time_layer, s_cct_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_first_time_layer);
  text_layer_destroy(s_second_time_layer);
  text_layer_destroy(s_date_layer);
  
  // Unload GFont
  //fonts_unload_custom_font(s_time_font);
}
 
static void init() {
// Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}