#pragma once

#include "quantum.h"

// --- Layer indices (must match keymap.c) ---
enum layers {
    _BASE = 0,   // QWERTY + home row mods
    _NUMFN,      // Numbers (left) + F-keys (right)  — hold Space
    _NAV,        // Navigation + media               — hold MO·NAV (left inner) or Ent/NAV (right thumb)
    _SYM,        // Symbols for coding               — hold MO·SYM (right inner)
    _META,       // Keyboard settings                — hold META (left outer)
};

// --- Per-layer HSV colors — Catppuccin Mocha palette (boosted saturation) ---
// .v is ignored at runtime; rgb_matrix_get_val() drives brightness instead.
// h/s derived from hex: Lavender=#b4befe Yellow=#f9e2af Sky=#89dceb Green=#a6e3a1
//                       Red=#f38ba8      Peach=#fab387
#define LAYER_COLOR_BASE    {164, 124, 0}  // Lavender
#define LAYER_COLOR_NUMFN   { 28, 126, 0}  // Yellow
#define LAYER_COLOR_NAV     {134, 157, 0}  // Sky
#define LAYER_COLOR_SYM     { 81, 124, 0}  // Green
#define LAYER_COLOR_META    {243, 160, 0}  // Red
#define LAYER_COLOR_ACCENT  { 16, 167, 0}  // Peach — LT/MO layer-activating keys on BASE

// --- RGB animation override ---
// Set to true from META layer to let the active RGB Matrix animation run freely.
// When false, per-layer static colors are applied via rgb_matrix_indicators_advanced_user.
bool rgb_animation_override = false;

// brightness_scale: 0-255 multiplier applied on top of rgb_matrix_get_val().
//   255 = full brightness, 51 = 20% (idle dim target), 0 = off (hard-black all LEDs).
// When scale is 0 the function blacks out every LED in range and returns, even in
// animation-override mode — this is the hard-off state driven by the idle timer.
static inline bool apply_layer_rgb(uint8_t led_min, uint8_t led_max, uint8_t brightness_scale) {
    if (brightness_scale == 0) {
        for (uint8_t i = led_min; i < led_max; i++) {
            rgb_matrix_set_color(i, 0, 0, 0);
        }
        return false;
    }

    if (rgb_animation_override) {
        return false;  // Let the active animation run
    }

    uint8_t layer = get_highest_layer(layer_state);
    HSV hsv;

    switch (layer) {
        case _NUMFN: hsv = (HSV)LAYER_COLOR_NUMFN; break;
        case _NAV:   hsv = (HSV)LAYER_COLOR_NAV;   break;
        case _SYM:   hsv = (HSV)LAYER_COLOR_SYM;   break;
        case _META:  hsv = (HSV)LAYER_COLOR_META;   break;
        case _BASE:  hsv = (HSV)LAYER_COLOR_BASE;   break;
        default:     return false;
    }

    // Combine user brightness (RM_VALU/RM_VALD) with the idle-dim multiplier.
    hsv.v = (uint8_t)(((uint16_t)rgb_matrix_get_val() * brightness_scale) >> 8);
    RGB rgb = hsv_to_rgb(hsv);

    // Set all LEDs in range to the layer color (covers underglow + key LEDs).
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }

    // Black out key LEDs that have no action on this layer (KC_NO or KC_TRNS).
    //
    // Maps LED index (physical wiring order from keyboard.json) → keymap position.
    // Rows use MATRIX_ROWS=12 convention: right-side hw rows 5-9 → keymap rows 6-10.
    // Row 255 = underglow LED, skip.
    //
    // MAINTENANCE: The .row/.col values are derived from keyboard.json hardware wiring
    // and never change. The key labels in comments are BASE layer — update them if you
    // remap BASE layer keys (they are documentation only, not used by the firmware).
    //
    // Column direction:
    //   LEFT  side — col6=outermost(far-left)  …  col1=innermost(near-center)
    //                col0 only on bottom row = Caps (inner-most, near split center)
    //   RIGHT side — col1=innermost(near-center) …  col6=outermost(far-right)
    //                col0 only on bottom row = Del (inner-most, near split center)
    //
    // LED order follows PCB wiring chain (bottom-up, not row order) — so LEDs within
    // the same row may be non-contiguous (e.g. LED 48 is bottom-row but wired between
    // thumb-row LEDs because it sits physically between the two clusters).
    static const keypos_t led_to_keypos[74] = {
        // ── Left underglow (6 LEDs, no keys) ──────────────────────────────────────
        [ 0] = {.row = 255, .col = 0},
        [ 1] = {.row = 255, .col = 0},
        [ 2] = {.row = 255, .col = 0},
        [ 3] = {.row = 255, .col = 0},
        [ 4] = {.row = 255, .col = 0},
        [ 5] = {.row = 255, .col = 0},

        // ── Left thumb row (row 4) + bottom-corner key (row 3 col5) ──────────────
        // Physical order: MO3(inner) … MO_META(outer), with BSpc corner wired 2nd
        [ 6] = {.row =  4, .col = 0},   // L thumb inner  → MO(_NAV)
        [ 7] = {.row =  4, .col = 5},   // L bottom corner→ BSpc          ← between main & thumb physically
        [ 8] = {.row =  4, .col = 1},   // L thumb        → MEH/BSpc
        [ 9] = {.row =  4, .col = 2},   // L thumb        → Spc/MO(_NUMFN)
        [10] = {.row =  4, .col = 4},   // L thumb outer  → MO(_META)
        [12] = {.row =  4, .col = 3},   // L thumb        → Hypr

        // ── Left bottom row (row 3) ───────────────────────────────────────────────
        // col6=far-left(`) … col1=B, col0=Caps(inner, near split center)
        [11] = {.row =  3, .col = 0},   // L bottom inner → Caps
        [13] = {.row =  3, .col = 1},   // L bottom       → B
        [14] = {.row =  3, .col = 2},   // L bottom       → V
        [15] = {.row =  3, .col = 3},   // L bottom       → C
        [16] = {.row =  3, .col = 4},   // L bottom       → X
        [17] = {.row =  3, .col = 5},   // L bottom       → Z
        [18] = {.row =  3, .col = 6},   // L bottom outer → `

        // ── Left home row (row 2) ─────────────────────────────────────────────────
        // col6=Tab(outer) … col1=G(inner)
        [19] = {.row =  2, .col = 1},   // L home inner   → G
        [20] = {.row =  2, .col = 2},   // L home         → Ctl/F
        [21] = {.row =  2, .col = 3},   // L home         → Sft/D
        [22] = {.row =  2, .col = 4},   // L home         → Alt/S
        [23] = {.row =  2, .col = 5},   // L home         → GUI/A
        [24] = {.row =  2, .col = 6},   // L home outer   → Tab

        // ── Left 2nd row (row 1) ──────────────────────────────────────────────────
        // col6=Esc(outer) … col1=T(inner)
        [25] = {.row =  1, .col = 1},   // L 2nd inner    → T
        [26] = {.row =  1, .col = 2},   // L 2nd          → R
        [27] = {.row =  1, .col = 3},   // L 2nd          → E
        [28] = {.row =  1, .col = 4},   // L 2nd          → W
        [29] = {.row =  1, .col = 5},   // L 2nd          → Q
        [30] = {.row =  1, .col = 6},   // L 2nd outer    → Esc

        // ── Left top row (row 0) ──────────────────────────────────────────────────
        // col6=App(outer) … col1=5(inner)
        [31] = {.row =  0, .col = 1},   // L top inner    → 5
        [32] = {.row =  0, .col = 2},   // L top          → 4
        [33] = {.row =  0, .col = 3},   // L top          → 3
        [34] = {.row =  0, .col = 4},   // L top          → 2
        [35] = {.row =  0, .col = 5},   // L top          → 1
        [36] = {.row =  0, .col = 6},   // L top outer    → App

        // ── Right underglow (6 LEDs, no keys) ─────────────────────────────────────
        [37] = {.row = 255, .col = 0},
        [38] = {.row = 255, .col = 0},
        [39] = {.row = 255, .col = 0},
        [40] = {.row = 255, .col = 0},
        [41] = {.row = 255, .col = 0},
        [42] = {.row = 255, .col = 0},

        // ── Right thumb row (row 10) + bottom-corner key (row 9 col5) ─────────────
        // LED 48 (Del) is bottom-row but wired here in the PCB chain
        [43] = {.row = 10, .col = 0},   // R thumb inner  → MO(_SYM)
        [44] = {.row = 10, .col = 5},   // R bottom corner→ SS5 (M0_MACRO) ← between main & thumb physically
        [45] = {.row = 10, .col = 1},   // R thumb        → MEH/Del
        [46] = {.row = 10, .col = 2},   // R thumb        → Ent/MO(_NAV)
        [47] = {.row = 10, .col = 4},   // R thumb outer  → App
        [48] = {.row =  9, .col = 0},   // R bottom inner → Del             ← wired mid-cluster in PCB chain
        [49] = {.row = 10, .col = 3},   // R thumb        → Hypr

        // ── Right bottom row (row 9) ──────────────────────────────────────────────
        // col0=Del(inner, near split center) … col6=RSft(outer)
        [50] = {.row =  9, .col = 1},   // R bottom       → N
        [51] = {.row =  9, .col = 2},   // R bottom       → M
        [52] = {.row =  9, .col = 3},   // R bottom       → ,
        [53] = {.row =  9, .col = 4},   // R bottom       → .
        [54] = {.row =  9, .col = 5},   // R bottom       → /
        [55] = {.row =  9, .col = 6},   // R bottom outer → RSft

        // ── Right home row (row 8) ────────────────────────────────────────────────
        // col1=H(inner) … col6='/MO3(outer)
        [56] = {.row =  8, .col = 1},   // R home inner   → H
        [57] = {.row =  8, .col = 2},   // R home         → Ctl/J
        [58] = {.row =  8, .col = 3},   // R home         → Sft/K
        [59] = {.row =  8, .col = 4},   // R home         → Alt/L
        [60] = {.row =  8, .col = 5},   // R home         → GUI/;
        [61] = {.row =  8, .col = 6},   // R home outer   → '

        // ── Right 2nd row (row 7) ─────────────────────────────────────────────────
        // col1=Y(inner) … col6=BSpc(outer)
        [62] = {.row =  7, .col = 1},   // R 2nd inner    → Y
        [63] = {.row =  7, .col = 2},   // R 2nd          → U
        [64] = {.row =  7, .col = 3},   // R 2nd          → I
        [65] = {.row =  7, .col = 4},   // R 2nd          → O
        [66] = {.row =  7, .col = 5},   // R 2nd          → P
        [67] = {.row =  7, .col = 6},   // R 2nd outer    → BSpc

        // ── Right top row (row 6) ─────────────────────────────────────────────────
        // col1=6(inner) … col6=RAlt(outer)
        [68] = {.row =  6, .col = 1},   // R top inner    → 6
        [69] = {.row =  6, .col = 2},   // R top          → 7
        [70] = {.row =  6, .col = 3},   // R top          → 8
        [71] = {.row =  6, .col = 4},   // R top          → 9
        [72] = {.row =  6, .col = 5},   // R top          → 0
        [73] = {.row =  6, .col = 6},   // R top outer    → RAlt
    };

    for (uint8_t led = led_min; led < led_max; led++) {
        keypos_t pos = led_to_keypos[led];
        if (pos.row == 255) continue;  // underglow, no key
        uint16_t kc = keymap_key_to_keycode(layer, pos);
        if (kc == KC_TRNS || kc == KC_NO) {
            rgb_matrix_set_color(led, 0, 0, 0);
        }
    }

    // On BASE: paint LT() and MO() layer-activating keys in Peach accent.
    // This makes the thumb cluster layer keys visually distinct from regular keys.
    if (layer == _BASE) {
        HSV accent_hsv  = (HSV)LAYER_COLOR_ACCENT;
        accent_hsv.v    = hsv.v;
        RGB accent_rgb  = hsv_to_rgb(accent_hsv);
        for (uint8_t led = led_min; led < led_max; led++) {
            keypos_t pos = led_to_keypos[led];
            if (pos.row == 255) continue;
            uint16_t kc = keymap_key_to_keycode(_BASE, pos);
            if (IS_QK_LAYER_TAP(kc) || IS_QK_MOMENTARY(kc)) {
                rgb_matrix_set_color(led, accent_rgb.r, accent_rgb.g, accent_rgb.b);
            }
        }
    }

    return false;
}
