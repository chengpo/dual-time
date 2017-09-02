/*
Copyright 2017 Po Cheng

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <pebble.h>

#define CCT (+8)

#define TOTAL_TEXT_LAYER (4)
#define LOCAL_WDAY_TEXT_LAYER (text_layers[0])
#define LOCAL_TIME_TEXT_LAYER (text_layers[1])
#define LOCAL_DATE_TEXT_LAYER (text_layers[2])
#define FOREIGN_TIME_TEXT_LAYER (text_layers[3])

typedef struct {
    GRect  rect;
    char   *init_text;
    GFont  font;
    GTextAlignment text_alignment;
} TextLayerProp;

typedef struct {
    GColor forground_color;
    GColor background_color;
} TextColorProp;

static TextLayer *text_layers[TOTAL_TEXT_LAYER];
static Window *s_main_window;

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
  
    bool revert_color = true;
  
    // Init text color
    TextColorProp text_color_prop;
  
    text_color_prop.forground_color = revert_color ? GColorWhite : GColorBlack;
    text_color_prop.background_color = revert_color ? GColorBlack : GColorWhite;
  
    if (revert_color) {
        window_set_background_color(window, GColorBlack);
    } else {
        window_set_background_color(window, GColorWhite);
    }
    
    // Init text layer properties
    TextLayerProp text_layer_props[] = {
        // local week day
        {.rect = GRect(0, PBL_IF_ROUND_ELSE(18, 10), bounds.size.w, 50),
            .init_text = "Mon",
            .font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
            .text_alignment = GTextAlignmentCenter},
        // local time
        {.rect = GRect(0, PBL_IF_ROUND_ELSE(38, 35), bounds.size.w, 50),
            .init_text = "00:00",
            .font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS),
            .text_alignment = GTextAlignmentCenter},
        // local date
        {.rect = GRect(0, PBL_IF_ROUND_ELSE(88, 80), bounds.size.w, 50),
            .init_text = "00:00",
            .font = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS),
            .text_alignment = GTextAlignmentCenter},
        // foreign time
        {.rect = GRect(0, PBL_IF_ROUND_ELSE(138, 128), bounds.size.w, 50),
            .init_text = "- 00:00 -",
            .font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS),
            .text_alignment = GTextAlignmentCenter}
    };

    for (int i = 0; i < TOTAL_TEXT_LAYER; i++) {
        text_layers[i] =  text_layer_create(text_layer_props[i].rect);

        text_layer_set_background_color(text_layers[i], text_color_prop.background_color);
        text_layer_set_text_color(text_layers[i], text_color_prop.forground_color);
        text_layer_set_text(text_layers[i], text_layer_props[i].init_text);
        text_layer_set_font(text_layers[i], text_layer_props[i].font);
        text_layer_set_text_alignment(text_layers[i], text_layer_props[i].text_alignment);

        layer_add_child(window_layer, text_layer_get_layer(text_layers[i]));
    }
}

static void update_local_wday() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char *wday[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

    text_layer_set_text(LOCAL_WDAY_TEXT_LAYER, wday[tick_time->tm_wday]);
}

static void update_local_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char local_time[8];
    strftime(local_time, sizeof(local_time), "%H:%M" , tick_time);
    text_layer_set_text(LOCAL_TIME_TEXT_LAYER, local_time);
}

static void update_local_date() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char local_date[8];
    snprintf(local_date, sizeof(local_date), "%02d/%02d", tick_time->tm_mon+1, tick_time->tm_mday);
    text_layer_set_text(LOCAL_DATE_TEXT_LAYER, local_date);
}

static void update_foreign_time() {
    time_t temp = time(NULL);
    struct tm *gmt = gmtime(&temp);

    static char foreign_time[12];
    snprintf(foreign_time, sizeof(foreign_time), "- %02d:%02d -", (gmt->tm_hour+CCT)%24, gmt->tm_min);
    text_layer_set_text(FOREIGN_TIME_TEXT_LAYER, foreign_time);
}

static void update_time() {
    update_local_wday();
    update_local_time();
    update_local_date();
    update_foreign_time();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void main_window_unload(Window *window) {
    for (int i = 0; i < TOTAL_TEXT_LAYER; i++) {
        text_layer_destroy(text_layers[i]);
    }
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
    return 0;
}
