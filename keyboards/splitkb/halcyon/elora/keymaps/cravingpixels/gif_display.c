// gif_display.c — full-screen GIF mode (dsp4)
//
// Loads gif1 and plays it full-screen via qp_animate. gif_running is tracked
// separately from (gif_anim != INVALID_DEFERRED_TOKEN) because qp_animate
// returns 0 for the first deferred slot, which equals INVALID_DEFERRED_TOKEN.

#ifdef HLC_TFT_DISPLAY

#include QMK_KEYBOARD_H
#include "users/halcyon_modules/splitkb/hlc_tft_display/hlc_tft_display.h"
#include "gif1.qgf.h"

static painter_image_handle_t gif_handle  = NULL;
static deferred_token         gif_anim    = INVALID_DEFERRED_TOKEN;
static bool                   gif_running = false;

void gif_display_start(void) {
    if (gif_running) {
        qp_stop_animation(gif_anim);
        gif_running = false;
    }
    if (gif_handle) {
        qp_close_image(gif_handle);
        gif_handle = NULL;
    }
    gif_handle = qp_load_image_mem(gfx_gif1);
    if (gif_handle) {
        gif_anim    = qp_animate(lcd_surface, 0, 0, gif_handle);
        gif_running = true;
    }
}

void gif_display_stop(void) {
    if (gif_running) {
        qp_stop_animation(gif_anim);
        gif_running = false;
    }
    if (gif_handle) {
        qp_close_image(gif_handle);
        gif_handle = NULL;
    }
}

#endif  // HLC_TFT_DISPLAY
