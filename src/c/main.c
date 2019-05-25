#include <pebble.h>

#define EMPTY_SLOT -1
#define TOTAL_IMAGE_SLOTS 4
#define NUMBER_OF_IMAGES 10
  
static Window *my_window;
static BitmapLayer *background_layer;

const uint32_t IMAGE_BOLD_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_ZERO_BOLD_BLACK, RESOURCE_ID_ONE_BOLD_BLACK, RESOURCE_ID_TWO_BOLD_BLACK,
  RESOURCE_ID_THREE_BOLD_BLACK, RESOURCE_ID_FOUR_BOLD_BLACK, RESOURCE_ID_FIVE_BOLD_BLACK,
  RESOURCE_ID_SIX_BOLD_BLACK, RESOURCE_ID_SEVEN_BOLD_BLACK, RESOURCE_ID_EIGHT_BOLD_BLACK,
  RESOURCE_ID_NINE_BOLD_BLACK
};

const uint32_t IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_ZERO_BLACK, RESOURCE_ID_ONE_BLACK, RESOURCE_ID_TWO_BLACK,
  RESOURCE_ID_THREE_BLACK, RESOURCE_ID_FOUR_BLACK, RESOURCE_ID_FIVE_BLACK,
  RESOURCE_ID_SIX_BLACK, RESOURCE_ID_SEVEN_BLACK, RESOURCE_ID_EIGHT_BLACK,
  RESOURCE_ID_NINE_BLACK
};

static GBitmap *images[TOTAL_IMAGE_SLOTS];
static BitmapLayer *image_layers[TOTAL_IMAGE_SLOTS];
static int image_slot_state[TOTAL_IMAGE_SLOTS] = {EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT};

void load_bitmap(uint8_t slot_number, uint8_t digit_value, bool isBold){
   if (image_slot_state[slot_number] != EMPTY_SLOT) {
    return;
  }else{
    image_slot_state[slot_number] = digit_value;
    if(isBold){
      images[slot_number] = gbitmap_create_with_resource(IMAGE_BOLD_RESOURCE_IDS[digit_value]);
    }else{
      images[slot_number] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[digit_value]);
    }
    BitmapLayer *bitmap_layer = NULL;
    if(slot_number == 0){
      bitmap_layer = bitmap_layer_create(GRect (4, 4, 72, 73));
    }else if(slot_number == 1){
      bitmap_layer = bitmap_layer_create(GRect (74, 4, 72, 73));
    }else if(slot_number == 2){
      bitmap_layer = bitmap_layer_create(GRect (4, 86, 72, 73));
    }else if(slot_number == 3){
      bitmap_layer = bitmap_layer_create(GRect (74, 86, 72, 73)); 
    }
    image_layers[slot_number] = bitmap_layer;
    bitmap_layer_set_bitmap(bitmap_layer, images[slot_number]);
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(bitmap_layer));
  }
  
}

void unload_bitmap(uint8_t slot_number){
  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    layer_remove_from_parent(bitmap_layer_get_layer(image_layers[slot_number]));
    bitmap_layer_destroy(image_layers[slot_number]);
    gbitmap_destroy(images[slot_number]);
    image_slot_state[slot_number] = EMPTY_SLOT;
  }
}

void display_number(char current, uint8_t slot_number, bool isBold){
  uint8_t current_digit = current - '0';
  unload_bitmap(slot_number);
  load_bitmap(slot_number, current_digit, isBold);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  char hour[] = "00";
  char mins[] = "00";
  strftime(hour, 3, "%H", tick_time);
  strftime(mins, 3, "%M", tick_time);
  display_number(hour[0], 0, true);
  display_number(hour[1], 1, true);
  display_number(mins[0], 2, false);
  display_number(mins[1], 3, false);
}

void init_screen(){
   //Get a time structure so that the face doesn't start blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
   
  //Manually call the tick handler when the window is loaded
  tick_handler(t, MINUTE_UNIT);
}

void window_load(Window *window){
  background_layer = bitmap_layer_create(GRect (0,0,144,168));  
  bitmap_layer_set_background_color(background_layer, GColorBlack);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(background_layer));
  
  init_screen();
}

void window_unload(Window *window){  
  bitmap_layer_destroy(background_layer);
  for(uint8_t i = 0; i < TOTAL_IMAGE_SLOTS; i++){
    unload_bitmap(i);  
  }
}

void app_in_focus_callback(bool in_focus) {
  if(in_focus) {
    init_screen();
  } 
}


void handle_init(void) {
  my_window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(my_window, handlers);
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
  app_focus_service_subscribe(app_in_focus_callback);
  window_stack_push(my_window, true);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  app_focus_service_unsubscribe();
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
