#pragma once

// --- Version ---
#define KEYMAP_VERSION "v5"

// --- Tapping ---
// Value from Vial settings key "4": 175
#define TAPPING_TERM 175
#define TAPPING_TERM_PER_KEY
// Home-row mods: a same-hand roll (e.g. "sd" with LALT_T(S)) always settles as a tap,
// an opposite-hand nested press settles as a hold before the tapping term. Thumb,
// encoder and inner Bspc/Del keys are exempt in chordal_hold_layout (keymap.c) so
// one-handed Ctrl+Backspace style chords keep working.
#define CHORDAL_HOLD
#define PERMISSIVE_HOLD

// --- RGB Matrix ---
#define RGB_MATRIX_KEYPRESSES          // needed for reactive animations (solid_reactive, splash)
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS // needed for typing_heatmap

// Default animation speed — 0-255, QMK default is 128. Lower = slower.
// Only applies on first boot or after EEPROM clear; runtime changes via Spd±/encoder persist.
#define RGB_MATRIX_DEFAULT_SPD 64

// Max brightness: 100 (splitkb reference is 128). The USB descriptor promises the host
// 500 mA; 74 LEDs plus the TFT backlight exceeded that at 154 and macOS cut power to
// the port whenever another device was plugged in. Raise only in small steps and only
// if the shutoffs stay gone.
// Val step: 2 units = 2 % of 100 per encoder click, 50 clicks over the full range.
// Idle timeout is handled in keymap.c (dim to 20% at 5 min, off at 15 min).
// RGB_MATRIX_TIMEOUT is intentionally not set here — setting it would cut power to the
// RGB matrix before our smooth-dim logic can run.
#undef  RGB_MATRIX_MAXIMUM_BRIGHTNESS
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 100
#define RGB_MATRIX_VAL_STEP            2

// Enable specific animations (keep only what you use to save firmware space)
#define ENABLE_RGB_MATRIX_SOLID_COLOR
#define ENABLE_RGB_MATRIX_BREATHING
#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
#define ENABLE_RGB_MATRIX_TYPING_HEATMAP
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTINEXUS
#define ENABLE_RGB_MATRIX_SPLASH

// --- Split sync: user-defined transaction IDs ---
// USER_SYNC_RGB_ANIM syncs rgb_animation_override from master to slave.
#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_RGB_ANIM

// --- Raw HID (for time sync from host) ---
#define RAW_USAGE_PAGE 0xFF60
#define RAW_USAGE_ID   0x61
