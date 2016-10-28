#ifdef dontrunme
#include <pebble.h>

static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer, *small_dial_layer;

static TextLayer *s_day_label, *s_num_label;

static GPath *s_minute_arrow, *s_hour_arrow;
static const GPathInfo MINUTE_HAND_POINTS = {
  3, (GPoint []) {
    { -8, 20 },
    { 8, 20 },
    { 0, -80 }
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  3, (GPoint []){
    {-6, 20},
    {6, 20},
    {0, -60}
  }
};

static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, settings.BackgroundColor);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

  // init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);
//   s_simple_bg_layer = layer_create(bounds);
//   layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
//   layer_add_child(window_layer, s_simple_bg_layer);
  
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBG);
//   background_layer = bitmap_layer_create(bounds);
//   bitmap_layer_set_bitmap(background_layer, background_bitmap);
//   layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  
  center_dot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CENTER_DOT);
steps_center_dot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STEPS_CENTER_DOT);
//   s_date_layer = layer_create(bounds);
//   layer_set_update_proc(s_date_layer, date_update_proc);
//   layer_add_child(window_layer, s_date_layer);

//   GRect batt_rect = GRect(29, 78+6-4, 56-27+100, 98-82+100);
//   s_day_label = text_layer_create(batt_rect);
//   text_layer_set_text(s_day_label, "");
//   text_layer_set_background_color(s_day_label, GColorClear);
//   text_layer_set_text_color(s_day_label, GColorWhite);
// //   text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
//   text_layer_set_font(s_day_label, font);
//   layer_add_child(window_layer, text_layer_get_layer(s_day_label));

//   s_num_label = text_layer_create(GRect(118, 78+6-4, 158-117+100, 98-82+100));
//   text_layer_set_text(s_num_label, "");
//   text_layer_set_background_color(s_num_label, GColorClear);
//   text_layer_set_text_color(s_num_label, GColorWhite);
// //   text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
//   text_layer_set_font(s_num_label, font);
//   layer_add_child(window_layer, text_layer_get_layer(s_num_label));

//   s_hands_layer = layer_create(bounds);
//   layer_set_update_proc(s_hands_layer, hands_update_proc);
//   layer_add_child(window_layer, s_hands_layer);



  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);








static void hands_update_proc(Layer *layer, GContext *ctx) {
  
  
  
  
  /*
  Background layer
  White min hand
  Red hour Hand
  Yellow sec Hand
  White Dot
  Red Dot
  
  
  
  
  
  
  */
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  
  
  graphics_context_set_fill_color(ctx, (GColor){.argb = 0b11000001});
  graphics_fill_radial(ctx, GRect(bounds.size.w / 2 - 9, bounds.size.h / 2 - 9, 18, 18), GOvalScaleModeFitCircle, 9, 0, TRIG_MAX_ANGLE);

  
  
  
  
  const int16_t second_hand_length = 81;
  int16_t second_hand_circle_length = 73;
  int16_t second_hand_opposite_length = 17;
  int16_t minute_hand_length = 65;
  int16_t minute_hand_opposite_length = 17;
  int16_t hour_hand_length = 50;
  int16_t hour_hand_opposite_length = 8;
  int16_t steps_hand_length = 18;
  int16_t steps_hand_opposite_length = 10;
  GPoint steps_center = {.x = 89, .y = 130};

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
//   t->tm_hour = 12;
//   t->tm_min = 50;
//   t->tm_sec = 38;
  
  
  
  
  
  
    // Steps
  // steps hand
  int32_t steps_angle = 0;
  if (settings.StepType == 0) {
    steps_angle = TRIG_MAX_ANGLE * s_step_count / s_step_goal;
  } else {
    steps_angle = TRIG_MAX_ANGLE * s_step_count / settings.StepType;
  }
  //int32_t steps_angle = TRIG_MAX_ANGLE * 1 / 12;
  //int32_t steps_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  
  
  //int32_t minute_angle = ((TRIG_MAX_ANGLE * t->tm_min) / 60) + ((TRIG_MAX_ANGLE * t->tm_sec) / (60 * 60));
  GPoint steps_hand = {
    .x = (int16_t)(sin_lookup(steps_angle) * (int32_t)steps_hand_length / TRIG_MAX_RATIO) + steps_center.x,
    .y = (int16_t)(-cos_lookup(steps_angle) * (int32_t)steps_hand_length / TRIG_MAX_RATIO) + steps_center.y,
  };
  GPoint steps_hand_opposite = {
    .x = (int16_t)(sin_lookup(steps_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)steps_hand_opposite_length / TRIG_MAX_RATIO) + steps_center.x,
    .y = (int16_t)(-cos_lookup(steps_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)steps_hand_opposite_length / TRIG_MAX_RATIO) + steps_center.y,
  };
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, steps_hand, steps_hand_opposite);
  
  // steps center dot
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  graphics_draw_bitmap_in_rect(ctx, steps_center_dot_bitmap, GRect(86, 127, 8, 8));
  
  
  // hour hand
  //int32_t hour_angle = (TRIG_MAX_ANGLE * t->tm_hour / 12);
  int32_t hour_angle = (TRIG_MAX_ANGLE * t->tm_hour / 12) + (TRIG_MAX_ANGLE * t->tm_min / (12 * 60));
  GPoint hour_hand = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hour_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  GPoint hour_hand_opposite = {
    .x = (int16_t)(sin_lookup(hour_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)hour_hand_opposite_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)hour_hand_opposite_length / TRIG_MAX_RATIO) + center.y,
  };
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_line(ctx, hour_hand, hour_hand_opposite);
  
  

  if (settings.SecondTick) {
    // second hand
    int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    GPoint second_hand = {
      .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
    };
    GPoint second_hand_opposite = {
      .x = (int16_t)(sin_lookup(second_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)second_hand_opposite_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(second_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)second_hand_opposite_length / TRIG_MAX_RATIO) + center.y,
    };

    graphics_context_set_stroke_color(ctx, (GColor){.argb = 0b11111000});
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, second_hand, second_hand_opposite);

    // second hand circle
    GPoint second_hand_circle = {
      .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_circle_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_circle_length / TRIG_MAX_RATIO) + center.y,
    };
    graphics_context_set_fill_color(ctx, (GColor){.argb = 0b11111000});
    graphics_fill_radial(ctx, GRect(second_hand_circle.x - 4, second_hand_circle.y - 4, 9, 9), GOvalScaleModeFitCircle, 4, 0, TRIG_MAX_ANGLE);
    //graphics_draw_circle(ctx, second_hand_circle, 3);
  }

  
  // minute hand
  //int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  int32_t minute_angle = ((TRIG_MAX_ANGLE * t->tm_min) / 60) + ((TRIG_MAX_ANGLE * t->tm_sec) / (60 * 60));
  GPoint minute_hand = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)minute_hand_length / TRIG_MAX_RATIO) + center.y,
  };
  GPoint minute_hand_opposite = {
    .x = (int16_t)(sin_lookup(minute_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)minute_hand_opposite_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(minute_angle + (TRIG_MAX_ANGLE / 2)) * (int32_t)minute_hand_opposite_length / TRIG_MAX_RATIO) + center.y,
  };
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, minute_hand, minute_hand_opposite);
  
  
  
  // Circle in the middle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(ctx, GRect(bounds.size.w / 2 - 7, bounds.size.h / 2 - 7, 14, 14), GOvalScaleModeFitCircle, 4, 0, TRIG_MAX_ANGLE);
  //graphics_context_set_fill_color(ctx, GColorRed);
  //graphics_fill_radial(ctx, GRect(bounds.size.w / 2 - 4, bounds.size.h / 2 - 4, 8, 8), GOvalScaleModeFitCircle, 4, 0, TRIG_MAX_ANGLE);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
   graphics_draw_bitmap_in_rect(ctx, center_dot_bitmap, GRect(85, 85, 10, 10));
  
    
  //graphics_fill_circle(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 9, 9), 0, GCornerNone);
  
//   // minute/hour hand
//   graphics_context_set_fill_color(ctx, GColorWhite);
//   graphics_context_set_stroke_color(ctx, GColorBlack);

//   gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
//   gpath_draw_filled(ctx, s_minute_arrow);
//   gpath_draw_outline(ctx, s_minute_arrow);

//   gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
//   gpath_draw_filled(ctx, s_hour_arrow);
//   gpath_draw_outline(ctx, s_hour_arrow);

//   // dot in the middle
//   graphics_context_set_fill_color(ctx, GColorBlack);
//   graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
  
  /*
  snprintf(batt_text, sizeof(batt_text), "%02d %%", watch_battery.charge_percent);  // What text to draw
  text_layer_set_text(s_day_label, batt_text);
  
  snprintf(date_text, sizeof(date_text), "OCT %02d", (int)(t->tm_mday));  // What text to draw
  text_layer_set_text(s_num_label, date_text);
  */
  

  
  
}



#endif

