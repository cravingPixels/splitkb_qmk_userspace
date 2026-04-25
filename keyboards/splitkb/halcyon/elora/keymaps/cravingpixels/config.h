#pragma once

// --- Version ---
#define KEYMAP_VERSION "v3"

// --- Tapping ---
// Value from Vial settings key "4": 175
#define TAPPING_TERM 175
#define TAPPING_TERM_PER_KEY

// --- RGB Matrix ---
#define RGB_MATRIX_KEYPRESSES          // needed for reactive animations (solid_reactive, splash)
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS // needed for typing_heatmap

// Keep board default max brightness (128) — raising it increases LED heat on the RP2040
// Idle timeout is handled in keymap.c (dim to 20% at 5 min, off at 15 min).
// RGB_MATRIX_TIMEOUT is intentionally not set here — setting it would cut power to the
// RGB matrix before our smooth-dim logic can run.

// Enable specific animations (keep only what you use to save firmware space)
#define ENABLE_RGB_MATRIX_SOLID_COLOR
#define ENABLE_RGB_MATRIX_BREATHING
#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
#define ENABLE_RGB_MATRIX_TYPING_HEATMAP
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
#define ENABLE_RGB_MATRIX_SPLASH

// --- Raw HID (for time sync from host) ---
#define RAW_USAGE_PAGE 0xFF60
#define RAW_USAGE_ID   0x61
