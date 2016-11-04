#include <pebble.h>
#include "main.h"
#define null NULL
/*
Ideas:

Draw date and battery onto background image
draw pebble logo and "time round" onto background image

On a major change like day or battery change:
  unload background, reload it and draw date and battery onto background





*/
bool emulator = false;

static Window *main_window = null;
static Layer *root_layer = null;
static GRect root_layer_bounds;
static GPoint center;


GBitmap *background_bitmap,
        *center_dot_bitmap,
        *percentage_bitmap,
        *months_bitmap,
        *digits_bitmap;

char batt_text[10] = "";
char date_text[10] = "";

// ----------------------------------------------------------------------------------------------------------------------------
//  Library Functions
// ------------------------------------------------------
// get_gbitmapformat_text() lovingly stolen from:
//   https://github.com/rebootsramblings/GBitmap-Colour-Palette-Manipulator/blob/master/src/gbitmap_color_palette_manipulator.c
char* get_gbitmapformat_text(GBitmapFormat format) {
	switch (format) {
		case GBitmapFormat1Bit:        return "GBitmapFormat1Bit";
    #ifdef PBL_COLOR
		case GBitmapFormat8Bit:        return "GBitmapFormat8Bit";
		case GBitmapFormat1BitPalette: return "GBitmapFormat1BitPalette";
		case GBitmapFormat2BitPalette: return "GBitmapFormat2BitPalette";
		case GBitmapFormat4BitPalette: return "GBitmapFormat4BitPalette";
    #endif
		default:                       return "UNKNOWN FORMAT";
	}
}

uint8_t get_number_ofcolors(GBitmapFormat format) {
  switch (format) {
    case GBitmapFormat1Bit:        return  2;
    #ifdef PBL_COLOR
    case GBitmapFormat8Bit:        return 64;
    case GBitmapFormat1BitPalette: return  2;
    case GBitmapFormat2BitPalette: return  4;
    case GBitmapFormat4BitPalette: return 16;
    #endif
    default:                       return  0;
  }
}

// ----------------------------------------------------------------------------------------------------------------------------
//  Handlers
// ------------------------------------------------------
// Second or Minute tick handler
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(root_layer);
}



BatteryChargeState watch_battery;

void battery_handler(BatteryChargeState charge_state) {
  if (  // In case battery state didn't change
    watch_battery.charge_percent == charge_state.charge_percent &&
    watch_battery.is_charging    == charge_state.is_charging    &&
    watch_battery.is_plugged     == charge_state.is_plugged
  ) return;
  
  // Save battery state
  watch_battery.charge_percent = charge_state.charge_percent;
  watch_battery.is_charging    = charge_state.is_charging;
  watch_battery.is_plugged     = charge_state.is_plugged;
  
  // Commenting out dirty since battery will update on next minute change, that's fast enough.
  //layer_mark_dirty(root_layer);  // Update battery percentage on display
  
  //printf("Watch Battery %s: %03d%%", watch_battery.is_charging?"Charging":watch_battery.is_plugged?"Powered":"Discharging", watch_battery.charge_percent);
}

// ----------------------------------------------------------------------------------------------------------------------------
//  Save and Load Settings
// ------------------------------------------------------
ClaySettings settings;


// Read settings from persistent storage
static void load_settings() {
  // Load the default settings
  settings.BackgroundColor = (GColor){.argb = 0b11000110};//GColorDukeBlue;
  settings.ForegroundColor = GColorWhite;
  settings.SecondTick = false;

  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Handle the response from AppMessage
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *option_seconds;
  
// Background Color
//   if (bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor))
//     settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);

  // Foreground Color
//   if (fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor))
//     settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);

  // Second Tick
  if ((option_seconds = dict_find(iter, MESSAGE_KEY_SecondTick))) {
    settings.SecondTick = option_seconds->value->int32 == 1;
    
    // Apply new timer settings
    tick_timer_service_unsubscribe();
    if (settings.SecondTick)
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    else
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }

  // Save the new settings to persistent storage
  save_settings();
  
  // Update the display with the new settings
  layer_mark_dirty(root_layer);
}


// ----------------------------------------------------------------------------------------------------------------------------
//  Drawing
// ------------------------------------------------------
// ================================================================ //
//   How to support transparencies and the alpha channel
// ================================================================ //
uint8_t shadowtable[] = {192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,  /* ------------------ */ \
                         192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,  /*      0% alpha      */ \
                         192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,  /*        Clear       */ \
                         192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,192,  /* ------------------ */ \

                         192,192,192,193,192,192,192,193,192,192,192,193,196,196,196,197,  /* ------------------ */ \
                         192,192,192,193,192,192,192,193,192,192,192,193,196,196,196,197,  /*     33% alpha      */ \
                         192,192,192,193,192,192,192,193,192,192,192,193,196,196,196,197,  /*    Transparent     */ \
                         208,208,208,209,208,208,208,209,208,208,208,209,212,212,212,213,  /* ------------------ */ \

                         192,192,193,194,192,192,193,194,196,196,197,198,200,200,201,202,  /* ------------------ */ \
                         192,192,193,194,192,192,193,194,196,196,197,198,200,200,201,202,  /*     66% alpha      */ \
                         208,208,209,210,208,208,209,210,212,212,213,214,216,216,217,218,  /*    Translucent     */ \
                         224,224,225,226,224,224,225,226,228,228,229,230,232,232,233,234,  /* ------------------ */ \

                         192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,  /* ------------------ */ \
                         208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,  /*    100% alpha      */ \
                         224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,  /*      Opaque        */ \
                         240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255}; /* ------------------ */

uint8_t combine_colors(uint8_t bg_color, uint8_t fg_color) {
  return (shadowtable[((~fg_color)&0b11000000) + (bg_color&63)]&63) + shadowtable[fg_color];
}

// ================================================================ //

void draw_image(GContext *ctx, GBitmap *image, int16_t start_x, int16_t start_y) {
  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);  // Get framebuffer
  if(framebuffer) {                                           // If successfully captured the framebuffer
    uint8_t        *screen = gbitmap_get_data(framebuffer);   // Get pointer to framebuffer data
    
    uint8_t          *data = (uint8_t*)gbitmap_get_data(image);
    int16_t          width = gbitmap_get_bounds(image).size.w;
    int16_t         height = gbitmap_get_bounds(image).size.h;
    uint16_t bytes_per_row = gbitmap_get_bytes_per_row(image);
    
    #if defined(PBL_BW)
    
    // Bounds Checking -- feel free to remove this section if you know you won't go out of bounds
    int16_t           top = (start_y < 0) ? 0 - start_y : 0;
    int16_t          left = (start_x < 0) ? 0 - start_x : 0;
    int16_t        bottom = (height + start_y) > root_layer_bounds.size.h ? root_layer_bounds.size.h - start_y : height;
    int16_t         right = (width  + start_x) > root_layer_bounds.size.w ? root_layer_bounds.size.w - start_x : width;
    // End Bounds Checking


    for(int16_t y=top; y<bottom; ++y) {
      for(int16_t x=left; x<right; ++x) {

        uint16_t addr = ((y + start_y) * 20) + ((x + start_x) >> 3);             // the screen memory address of the 8bit byte where the pixel is
        uint8_t  xbit = (x + start_x) & 7;                                       // which bit is the pixel inside of 8bit byte
        screen[addr] &= ~(1<<xbit);                                              // assume pixel to be black
        screen[addr] |= ((data[(y*bytes_per_row) + (x>>3)] >> (x&7))&1) << xbit; // draw white pixel if image has a white pixel
      }
    }

    #elif defined(PBL_COLOR)
    GBitmapFormat   format = gbitmap_get_format(image);
    uint8_t       *palette = (uint8_t*)gbitmap_get_palette(image);
    
    // Section 1: Bounds Checking and Iteration
    if(start_y<0) {height += start_y; start_y = 0;}  // if top    is BEYOND screen bounds, then adjust
    if(start_y + height > root_layer_bounds.size.h) height = root_layer_bounds.size.h - start_y;    // if bottom is BEYOND screen bounds, then adjust
    for(int y = start_y; y < start_y+height; y++) {        // Iterate over all y of visible part of rect

      GBitmapDataRowInfo info = gbitmap_get_data_row_info(framebuffer, y);  // Get visible width
      if(info.min_x < start_x) info.min_x = start_x;            // If left  is WITHIN screen bounds, then adjust
      if(info.max_x > start_x + width)                          // If right is WITHIN screen bounds
        info.max_x = start_x + width;                           //   then adjust right side (else just use max)
      for(int x = info.min_x; x <= info.max_x; x++) {                       // Iterate over all x of visible part of rect
        switch(format) {
          case GBitmapFormat1Bit:        info.data[x] =                                     ((data[(y-start_y)*bytes_per_row + ((x-start_x)>>3)] >> ((7-((x-start_x)&7))   )) &  1) ? GColorWhiteARGB8 : GColorBlackARGB8; break;
          case GBitmapFormat8Bit:        info.data[x] = combine_colors(info.data[x],          data[(y-start_y)*bytes_per_row +  (x-start_x)    ]);                                    break;
          case GBitmapFormat1BitPalette: info.data[x] = combine_colors(info.data[x], palette[(data[(y-start_y)*bytes_per_row + ((x-start_x)>>3)] >> ((7-((x-start_x)&7))   )) &  1]); break;
          case GBitmapFormat2BitPalette: info.data[x] = combine_colors(info.data[x], palette[(data[(y-start_y)*bytes_per_row + ((x-start_x)>>2)] >> ((3-((x-start_x)&3))<<1)) &  3]); break;
          case GBitmapFormat4BitPalette: info.data[x] = combine_colors(info.data[x], palette[(data[(y-start_y)*bytes_per_row + ((x-start_x)>>1)] >> ((1-((x-start_x)&1))<<2)) & 15]); break;
          default: break;
        }
      }
    }
    #endif

    graphics_release_frame_buffer(ctx, framebuffer);  // Release the Kraken!  ... err, framebuffer.
  }
} 



void draw_sprite(GContext *ctx, GBitmap *image, int16_t start_x, int16_t start_y, GRect rect) {
  GBitmap *framebuffer = graphics_capture_frame_buffer(ctx);  // Get framebuffer
  if(framebuffer) {                                           // If successfully captured the framebuffer
    uint8_t        *screen = gbitmap_get_data(framebuffer);   // Get pointer to framebuffer data
    
    uint8_t          *data = (uint8_t*)gbitmap_get_data(image);
    GSize       image_size = gbitmap_get_bounds(image).size;
    uint16_t bytes_per_row = gbitmap_get_bytes_per_row(image);
    
    
    // Bounds Checking -- feel free to remove this section if you know you won't go out of bounds
    if (rect.origin.y < 0) {rect.size.h += rect.origin.y; rect.origin.y = 0;}                      // if top    is BEYOND image bounds, then adjust
    if (rect.origin.y + rect.size.h > image_size.h) rect.size.h = image_size.h - rect.origin.y;    // if bottom is BEYOND image bounds, then adjust
    if (rect.origin.x < 0) {rect.size.w += rect.origin.x; rect.origin.x = 0;}                      // if left   is BEYOND image bounds, then adjust
    if (rect.origin.x + rect.size.w > image_size.w) rect.size.w = image_size.w - rect.origin.x;    // if right  is BEYOND image bounds, then adjust
    // End Bounds Checking

    #if defined(PBL_BW)
    
//     // Bounds Checking -- feel free to remove this section if you know you won't go out of bounds
//     int16_t           top = (start_y < 0) ? 0 - start_y : 0;
//     int16_t          left = (start_x < 0) ? 0 - start_x : 0;
//     int16_t        bottom = (height + start_y) > root_layer_bounds.size.h ? root_layer_bounds.size.h - start_y : height;
//     int16_t         right = (image_size.w  + start_x) > root_layer_bounds.size.w ? root_layer_bounds.size.w - start_x : image_size.w;
//     // End Bounds Checking


//     for(int16_t y=top; y<bottom; ++y) {
//       for(int16_t x=left; x<right; ++x) {

//         uint16_t addr = ((y + start_y) * 20) + ((x + start_x) >> 3);             // the screen memory address of the 8bit byte where the pixel is
//         uint8_t  xbit = (x + start_x) & 7;                                       // which bit is the pixel inside of 8bit byte
//         screen[addr] &= ~(1<<xbit);                                              // assume pixel to be black
//         screen[addr] |= ((data[(y*bytes_per_row) + (x>>3)] >> (x&7))&1) << xbit; // draw white pixel if image has a white pixel
//       }
//     }

    #elif defined(PBL_COLOR)
    GBitmapFormat   format = gbitmap_get_format(image);
    uint8_t       *palette = (uint8_t*)gbitmap_get_palette(image);
    
    // Section 1: Bounds Checking and Iteration
    if(start_y<0) {rect.size.h += start_y; start_y = 0;}  // if top    is BEYOND screen bounds, then adjust
    if(start_y + rect.size.h > root_layer_bounds.size.h) rect.size.h = root_layer_bounds.size.h - start_y;    // if bottom is BEYOND screen bounds, then adjust
    for(int y = start_y; y < start_y+rect.size.h; y++) {        // Iterate over all y of visible part of rect

      GBitmapDataRowInfo info = gbitmap_get_data_row_info(framebuffer, y);  // Get visible width
      if(info.min_x < start_x) info.min_x = start_x;            // If left  is WITHIN screen bounds, then adjust
      if(info.max_x > start_x + rect.size.w)                          // If right is WITHIN screen bounds
        info.max_x = start_x + rect.size.w;                           //   then adjust right side (else just use max)
      for(int x = info.min_x; x <= info.max_x; x++) {                       // Iterate over all x of visible part of rect
        switch(format) {
          case GBitmapFormat1Bit:        info.data[x] =                                     ((data[(y+rect.origin.y-start_y)*bytes_per_row + ((x+rect.origin.x-start_x)>>3)] >> ((7-((x+rect.origin.x-start_x)&7))   )) &  1) ? GColorWhiteARGB8 : GColorBlackARGB8; break;
          case GBitmapFormat8Bit:        info.data[x] = combine_colors(info.data[x],          data[(y+rect.origin.y-start_y)*bytes_per_row +  (x+rect.origin.x-start_x)    ]);                                    break;
          case GBitmapFormat1BitPalette: info.data[x] = combine_colors(info.data[x], palette[(data[(y+rect.origin.y-start_y)*bytes_per_row + ((x+rect.origin.x-start_x)>>3)] >> ((7-((x+rect.origin.x-start_x)&7))   )) &  1]); break;
          case GBitmapFormat2BitPalette: info.data[x] = combine_colors(info.data[x], palette[(data[(y+rect.origin.y-start_y)*bytes_per_row + ((x+rect.origin.x-start_x)>>2)] >> ((3-((x+rect.origin.x-start_x)&3))<<1)) &  3]); break;
          case GBitmapFormat4BitPalette: info.data[x] = combine_colors(info.data[x], palette[(data[(y+rect.origin.y-start_y)*bytes_per_row + ((x+rect.origin.x-start_x)>>1)] >> ((1-((x+rect.origin.x-start_x)&1))<<2)) & 15]); break;
          default: break;
        }
      }
    }
    #endif

    graphics_release_frame_buffer(ctx, framebuffer);  // Release the Kraken!  ... err, framebuffer.
  }
} 




static void draw_hand(GContext *ctx, int32_t angle, int32_t length, int32_t back_length, GPoint center, uint8_t thickness, GColor color) {
  GPoint point;
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, thickness);

  point = (GPoint){
    .x = (int16_t)(sin_lookup(angle) * length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle) * length / TRIG_MAX_RATIO) + center.y,
  };
  graphics_draw_line(ctx, center, point);

  point = (GPoint){
    .x = (int16_t)(sin_lookup(angle + (TRIG_MAX_ANGLE / 2)) * back_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle + (TRIG_MAX_ANGLE / 2)) * back_length / TRIG_MAX_RATIO) + center.y,
  };
  graphics_draw_line(ctx, center, point);
}






static void root_layer_update(Layer *layer, GContext *ctx) {
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  graphics_draw_bitmap_in_rect(ctx, background_bitmap, root_layer_bounds);
  
  //draw_image(ctx, background_bitmap, 20, 20);
  //draw_image(ctx, background_bitmap, 0, 0);
  

  time_t now = time(NULL);
  struct tm *t = localtime(&now); // Current Watch Time
  
  // ==============
  //  Draw Text
  // ==============
  int mday = ((t->tm_mday) % 32); // day of the month (%32, just in case)
  int month = (t->tm_mon)%12; // Current month of hte year (%12, just in case)
  int batt = ((watch_battery.charge_percent) / 10) % 11;  // watch battery (%11, just in case)
  
  draw_sprite(ctx, percentage_bitmap, 28, 85, GRect(0, 11*batt, 26, 11));  // percentage
  draw_sprite(ctx, months_bitmap, 119, 85, GRect(0, 11*month, 21, 11)); // months
  draw_sprite(ctx, digits_bitmap, 143, 84, GRect(0, 12*mday, 13, 12));      // digits


  // ==============
  //  Center Hands
  // ==============
  // Dark circle behind hands
  #define CIRCLE_SIZE 9
  graphics_context_set_fill_color(ctx, (GColor){.argb = 0b11000001});
  graphics_fill_radial(ctx, GRect(center.x - CIRCLE_SIZE, center.y - CIRCLE_SIZE, CIRCLE_SIZE*2, CIRCLE_SIZE*2), GOvalScaleModeFitCircle, CIRCLE_SIZE, 0, TRIG_MAX_ANGLE);

  // Minute Hour and Second Hands
  draw_hand(ctx, (TRIG_MAX_ANGLE * t->tm_min  / 60) + (TRIG_MAX_ANGLE * t->tm_sec / (60 * 60)),  65, 17, center, 4, GColorWhite);  // minute hand
  draw_hand(ctx, (TRIG_MAX_ANGLE * t->tm_hour / 12) + (TRIG_MAX_ANGLE * t->tm_min / (12 * 60)),  50, 8, center, 6, GColorRed);  // hour hand
  if (settings.SecondTick) {
    int second_angle = (TRIG_MAX_ANGLE * t->tm_sec  / 60);
    draw_hand(ctx, second_angle, 81, 17, center, 2, (GColor){.argb = 0b11111000});  // second hand

    // second hand circle
    int16_t second_hand_circle_length = 73;
    GPoint second_hand_circle = {
      .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_circle_length / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_circle_length / TRIG_MAX_RATIO) + center.y,
    };
    graphics_context_set_fill_color(ctx, (GColor){.argb = 0b11111000});
    graphics_fill_radial(ctx, GRect(second_hand_circle.x - 4, second_hand_circle.y - 4, 9, 9), GOvalScaleModeFitCircle, 4, 0, TRIG_MAX_ANGLE);
  }
  
  
  // Circle in the middle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(ctx, GRect(center.x - 7, center.y - 7, 14, 14), GOvalScaleModeFitCircle, 4, 0, TRIG_MAX_ANGLE);
  //graphics_context_set_fill_color(ctx, GColorRed);
  //graphics_fill_radial(ctx, GRect(center.x / 2 - 4, center.y - 4, 8, 8), GOvalScaleModeFitCircle, 4, 0, TRIG_MAX_ANGLE);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, center_dot_bitmap, GRect(85, 85, 10, 10));
  

}



// ----------------------------------------------------------------------------------------------------------------------------
//  Main Init/Deinit Functions
// ------------------------------------------------------
static void printf_palette(GBitmap *bitmap) {
  GColor *palette = gbitmap_get_palette(bitmap);
  GBitmapFormat format = gbitmap_get_format(bitmap);
  printf("format %d: %s", (int)format, get_gbitmapformat_text(format));
  
  
  if (format == GBitmapFormat1BitPalette ||
      format == GBitmapFormat2BitPalette ||
      format == GBitmapFormat4BitPalette)
    for (int i = 0, c = palette[i].argb; (i < get_number_ofcolors(format)) && (c>0); i++, c = palette[i].argb)
    printf("Color %d = 0b%d%d %d%d %d%d %d%d", i,
           (c>>7)&1,
           (c>>6)&1,
           (c>>5)&1,
           (c>>4)&1,
           (c>>3)&1,
           (c>>2)&1,
           (c>>1)&1,
           (c>>0)&1
          );
}

static void main_window_load(Window *window) {
  // Save global settings
  root_layer = window_get_root_layer(window);
  root_layer_bounds = layer_get_bounds(root_layer);
  center = grect_center_point(&root_layer_bounds);
  layer_set_update_proc(root_layer, root_layer_update);
  GColor *palette;
  
  // Load images
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FULLBG);
//   background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
//   background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BG);
  center_dot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CENTER_DOT);
  months_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MONTHS);
  percentage_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PERCENTAGE);
  digits_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIGITS);


  //   palette[0] = (GColor){.argb=0b11000001};
  //   palette[1] = (GColor){.argb=0b11001100};
  //   palette[2] = (GColor){.argb=0b11001111};
  //   palette[3] = (GColor){.argb=0b11111100};

  /*
  (RESOURCE_ID_BG)
  format: 4
  Color 0 = 0b11000010
  Color 1 = 0b11010111
  Color 2 = 0b11000011
  Color 3 = 0b11101011
  Color 4 = 0b11111111
  */

  
  /*
  (RESOURCE_ID_BACKGROUND)
  format 4: GBitmapFormat4BitPalette
  Color 0 = 0b11 00 00 00
  Color 1 = 0b11 01 01 01
  Color 2 = 0b11 10 10 10
  Color 3 = 0b11 11 00 00
  Color 4 = 0b11 01 00 00
  Color 5 = 0b11 10 00 00
  Color 6 = 0b11 11 11 11
  */
  
  /*
  RESOURCE_ID_FULLBG
  [DEBUG] main.c:421: format 4: GBitmapFormat4BitPalette
Color  0 = 0b11 00 00 10
Color  1 = 0b11 01 01 10
Color  2 = 0b11 10 10 11
Color  3 = 0b11 01 00 10
Color  4 = 0b11 11 00 00
Color  5 = 0b11 10 00 00
Color  6 = 0b11 11 11 11
Color  7 = 0b11 10 00 01
Color  8 = 0b11 01 00 01
Color  9 = 0b11 10 10 10
Color 10 = 0b11 00 01 11
Color 11 = 0b11 00 00 00
Color 12 = 0b11 00 00 11
Color 13 = 0b11 00 00 01
Color 14 = 0b11 01 01 11
Color 15 = 0b11 01 10 11
  */
  
  palette = gbitmap_get_palette(background_bitmap);
//   palette[0] = (GColor){.argb=0b00110000};
//   palette[1] = (GColor){.argb=0b01111111};
//   palette[2] = (GColor){.argb=0b10111111};
//   palette[3] = (GColor){.argb=0b11111111};
  
  
  
  printf_palette(background_bitmap);
// [DEBUG] main.c:393: format 3: GBitmapFormat2BitPalette
// [DEBUG] main.c:409: Color 0 = 0b11 00 00 00
// [DEBUG] main.c:409: Color 1 = 0b11 01 01 01
// [DEBUG] main.c:409: Color 2 = 0b11 10 10 10
// [DEBUG] main.c:409: Color 3 = 0b11 11 11 11
  palette = gbitmap_get_palette(percentage_bitmap);
  palette[0] = (GColor){.argb=0b00111111};
  palette[1] = (GColor){.argb=0b01111111};
  palette[2] = (GColor){.argb=0b10111111};
  palette[3] = (GColor){.argb=0b11111111};
  
  palette = gbitmap_get_palette(months_bitmap);
  palette[0] = (GColor){.argb=0b00111111};
  palette[1] = (GColor){.argb=0b01111111};
  palette[2] = (GColor){.argb=0b10111111};
  palette[3] = (GColor){.argb=0b11111111};

  palette = gbitmap_get_palette(digits_bitmap);
  palette[0] = (GColor){.argb=0b00111111};
  palette[1] = (GColor){.argb=0b01111111};
  palette[2] = (GColor){.argb=0b10111111};
  palette[3] = (GColor){.argb=0b11111111};
  
  //-----------------------------------------------------------------
  // Subscribe to Battery and Bluetooth and TickTimer and Health
  //-----------------------------------------------------------------
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  
  if (settings.SecondTick)
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  else
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //if (settings.use_health -- or something)
  // Subscribe to health events if we can
}



static void main_window_unload(Window *window) {
  // Destroy images
  
}

static void init(void) {
  load_settings();
  
  // Listen for AppMessages
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 128);

  // Set up main window
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  // And push the main window to the window stack
  window_stack_push(main_window, true);
  
  //-----------------------------------------------------------------
  // Check Emulator
  //-----------------------------------------------------------------
//   emulator = watch_info_get_model()==WATCH_INFO_MODEL_UNKNOWN;
//   if(emulator) {
//     printf("Emulator Detected: Turning Backlight On");
//     light_enable(true);
//   }
}



static void deinit(void) {
  if (main_window) window_destroy(main_window);
}



int main(void) {
  init();
  app_event_loop();
  deinit();
}
