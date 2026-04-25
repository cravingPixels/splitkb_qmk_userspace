// Halcyon Elora rev2 — custom keymap (cravingpixels v1)
// Translated from elora.vil (Vial export, vial_protocol 6)
//
// LT keys (TAPPING_TERM 175 ms):
//   tap ' / hold MO(MODNAV)  — right home row outermost
//   tap Spc / hold MO(NUMFN) — left thumb
//   tap Ent / hold MO(RGBNAV)— right thumb
//
// SS5_KEY: OS-aware screenshot shortcut — modifier report flushed before key fires.
//   macOS  → Cmd+Shift+5 (screenshot menu). Ctrl↔GUI swapped, so QMK sends KC_LCTL.
//   Windows→ Win+Shift+S (Snipping Tool). Detected via OS_DETECTION at enumeration.
// Left half compiled with HLC_TFT_DISPLAY=1, right half with HLC_ENCODER_REV2=1.

#include QMK_KEYBOARD_H
#include "rgb_layers.h"
#include "os_detection.h"

// ---------------------------------------------------------------------------
// Custom keycodes
// ---------------------------------------------------------------------------
enum custom_keycodes {
    SS5_KEY = SAFE_RANGE,       // Cmd+Shift+5 (macOS screenshot menu)
    KC_DISP_1,                  // LCD display mode 1 — stock HLC widget
    KC_DISP_2,                  // LCD display mode 2 — stats UI (press again to cycle sub-mode)
    KC_DISP_3,                  // LCD display mode 3 — Conway's Game of Life
    KC_DISP_4,                  // LCD display mode 4 — full-screen GIF
    KC_TT_UP,                   // Tapping term +5 ms (max 300)
    KC_TT_DN,                   // Tapping term −5 ms (min 100)
    KC_RGB_ANIM_TOGGLE,         // Toggle RGB animation override
};

// ---------------------------------------------------------------------------
// Keymap
// ---------------------------------------------------------------------------
// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/*
 * Layer 0 — BASE (QWERTY + home row mods)
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |  App   |   1  |   2  |   3  |   4  |   5  |                              |   6  |   7  |   8  |   9  |   0  |  RAlt  |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |  Esc   |   Q  |   W  |   E  |   R  |   T  |                              |   Y  |   U  |   I  |   O  |   P  |  BSpc  |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |  Tab   | GUI/A| Alt/S| Sft/D| Ctl/F|   G  |                              |   H  | Ctl/J| Sft/K| Alt/L| GUI/;|' /MO3  |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |   `    |   Z  |   X  |   C  |   V  |   B  | BSpc | Caps |  | MO5  | SS5  |   N  |   M  |  ,   |  .   |  /   | RSft   |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        | SYS  | Hypr | Spc  |MEH/BS| MO3  |  | MO4  |MEH/DL| Ent  | Hypr | App  |
 *                        |      |      | MO1  |      |      |  |      |      | MO2  |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              | Mute |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_BASE] = LAYOUT_elora_hlc(
      KC_APPLICATION, KC_1,          KC_2,          KC_3,          KC_4,          KC_5,                                        KC_6,          KC_7,              KC_8,          KC_9,              KC_0,              KC_RALT,
      KC_ESCAPE,      KC_Q,          KC_W,          KC_E,          KC_R,          KC_T,                                        KC_Y,          KC_U,              KC_I,          KC_O,              KC_P,              KC_BSPC,
      KC_TAB,         LGUI_T(KC_A),  LALT_T(KC_S),  LSFT_T(KC_D),  LCTL_T(KC_F),  KC_G,                                        KC_H,          RCTL_T(KC_J),      RSFT_T(KC_K),  LALT_T(KC_L),      RGUI_T(KC_SCLN),   LT(_MODNAV, KC_QUOTE),
      KC_GRAVE,       KC_Z,          KC_X,          KC_C,          KC_V,          KC_B,    KC_BSPC,        KC_CAPS,  MO(_FN), SS5_KEY,          KC_N,           KC_M,              KC_COMMA,      KC_DOT,            KC_SLASH,          KC_RSFT,
      MO(_SYSTEM),    OSM(MOD_HYPR), LT(_NUMFN, KC_SPACE), MEH_T(KC_BSPC), MO(_MODNAV),    MO(_SYM),      MEH_T(KC_DELETE),      LT(_RGBNAV, KC_ENTER), OSM(MOD_HYPR), KC_APPLICATION,
      KC_NO,          KC_NO,         KC_NO,         KC_NO,         KC_NO,                   KC_MUTE,       KC_NO,             KC_NO,         KC_NO,             KC_NO
    ),

/*
 * Layer 1 — NUMFN: Numbers (left) + F-keys (right)
 * Accessed via: tap Spc / hold MO1 (left thumb)
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   =    |   1  |   2  |   3  |   4  |   5  |                              | F11  |  F1  |  F2  |  F3  |  F4  |   F5   |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   -    | GUI/6| Alt/7| Sft/8| Ctl/9|   0  |                              | F12  |Ctl/F6|Sft/F7|Alt/F8|GUI/F9|  F10   |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |  ;   |  [   |  ]   |  ,   |  .   |      |      |  |      |      |      |      |      |      |  \   |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      | MEH  |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * | Mute |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_NUMFN] = LAYOUT_elora_hlc(
      KC_NO,    KC_NO,         KC_NO,         KC_NO,         KC_NO,         KC_NO,                               KC_NO,         KC_NO,             KC_NO,         KC_NO,             KC_NO,             KC_NO,
      KC_EQUAL, KC_1,          KC_2,          KC_3,          KC_4,          KC_5,                                KC_F11,        KC_F1,             KC_F2,         KC_F3,             KC_F4,             KC_F5,
      KC_MINUS, LGUI_T(KC_6),  LALT_T(KC_7),  LSFT_T(KC_8),  LCTL_T(KC_9),  KC_0,                                KC_F12,        RCTL_T(KC_F6),     RSFT_T(KC_F7), LALT_T(KC_F8),     RGUI(KC_F9),       KC_F10,
      KC_NO,    KC_SCLN,       KC_LBRC,       KC_RBRC,       KC_COMMA,      KC_DOT,  KC_NO,  KC_NO,  KC_NO,  KC_NO, KC_NO,       KC_NO,             KC_NO,         KC_NO,             KC_BSLS,           KC_NO,
      KC_NO,    KC_NO,         KC_NO,         KC_NO,         KC_NO,                   KC_NO,  OSM(MOD_MEH),      KC_NO,         KC_NO,             KC_NO,
      KC_MUTE,  KC_NO,         KC_NO,         KC_NO,         KC_NO,                   KC_NO,  KC_NO,             KC_NO,         KC_NO,             KC_NO
    ),

/*
 * Layer 2 — RGBNAV: RGB control (left) + Navigation (right)
 * Accessed via: tap Ent / hold MO2 (right thumb)
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * | RMTog  |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      | RM V-| RM V+| Next |      |                              | Home | PgDn | PgUp | End  |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * | RMTog  | GUI  | Alt  | Sft  | Ctl  | Play |                              | Left | Down |  Up  | Rght |  <   |   >    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * | RM Nx  | RM Pv| RM H-| RM H+| Prev |      |      |      |  | MO5  |      |      |      | BSpc | Del  |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * | Mute |      |       |      |      |                                              | Mute |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_RGBNAV] = LAYOUT_elora_hlc(
      RM_TOGG,  KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,                                KC_NO,    KC_NO,    KC_NO,    KC_NO,          KC_NO,          KC_NO,
      KC_NO,    KC_NO,    RM_VALD,  RM_VALU,  KC_MNXT,  KC_NO,                                KC_HOME,  KC_PGDN,  KC_PGUP,  KC_END,         KC_NO,          KC_NO,
      RM_TOGG,  KC_LGUI,  KC_LALT,  KC_LSFT,  KC_LCTL,  KC_MPLY,                              KC_LEFT,  KC_DOWN,  KC_UP,    KC_RIGHT,       LSFT(KC_COMMA), LSFT(KC_DOT),
      RM_NEXT,  RM_PREV,  RM_HUED,  RM_HUEU,  KC_MPRV,  KC_NO, KC_NO, KC_NO, MO(_FN), KC_NO, KC_NO,    KC_NO,    KC_BSPC,  KC_DELETE,      KC_NO,          KC_NO,
      KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,                    KC_NO, KC_NO,          KC_NO,    KC_NO,    KC_NO,
      KC_MUTE,  KC_NO,    KC_NO,    KC_NO,    KC_NO,                    KC_MUTE, KC_NO,        KC_NO,    KC_NO,    KC_NO
    ),

/*
 * Layer 3 — MODNAV: Modifiers (left) + Navigation (right)
 * Accessed via: MO3 right thumb, or tap ' / hold MO3 (right home row outermost)
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |                              | PgUp | Home |  Up  | End  | VolUp| Del    |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        | GUI  | Alt  | Ctl  |      |      |                              | PgDn | Left | Down | Rght | VolDn| Ins    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      | ScLk |  | MO5  |      | Pause| Prev | Play | Next | Mute | PrtSc  |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_MODNAV] = LAYOUT_elora_hlc(
      _______, _______, _______, _______, _______, _______,                                        _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______,                                        KC_PGUP, KC_HOME, KC_UP,   KC_END,  KC_VOLU, KC_DELETE,
      _______, KC_LGUI, KC_LALT, KC_LCTL, _______, _______,                                        KC_PGDN, KC_LEFT, KC_DOWN, KC_RIGHT,KC_VOLD, KC_INSERT,
      _______, _______, _______, _______, _______, _______, _______, KC_SCRL, MO(_FN), _______, KC_PAUSE, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_PSCR,
      _______, _______, _______, _______, _______,                   _______, _______,           _______, _______, _______,
      _______, _______, _______, _______, _______,                   _______, _______,           _______, _______, _______
    ),

/*
 * Layer 4 — SYM: Symbols
 * Accessed via: MO4 left thumb outermost on right hand (MO4)
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   `    |   1  |   2  |   3  |   4  |   5  |                              |   6  |   7  |   8  |   9  |   0  |   =    |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   ~    |   !  |   @  |   #  |   $  |   %  |                              |   ^  |   &  |   *  |   (  |   )  |   +    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |   |    |   \  |   :  |   ;  |   -  |   [  |      |      |  |      |      |   ]  |   _  |   ,  |   .  |   /  |   ?    |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |  {   |      |      |      |  |      |      |      |  }   |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_SYM] = LAYOUT_elora_hlc(
      _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
      KC_GRAVE,KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                                        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_EQUAL,
      KC_TILD, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                                     KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PLUS,
      KC_PIPE, KC_BSLS, KC_COLN, KC_SCLN, KC_MINUS,KC_LBRC, _______, _______, _______, _______, KC_RBRC, KC_UNDS, KC_COMMA,KC_DOT,  KC_SLASH,KC_QUES,
      _______, KC_LCBR, _______, _______, _______,                     _______, _______,         _______, KC_RCBR, _______,
      _______, _______, _______, _______, _______,                     _______, _______,         _______, _______, _______
    ),

/*
 * Layer 5 — FN: Function keys
 * Accessed via: MO5 (inner bottom key, both halves)
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |  F9  | F10  | F11  | F12  |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |  F5  |  F6  |  F7  |  F8  |      |                              |      | RSft | RCtl | LAlt | RGUI |        |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |  F1  |  F2  |  F3  |  F4  |      |      |      |  |      |      |      |      |      |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_FN] = LAYOUT_elora_hlc(
      _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
      _______, KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______,                                     _______, _______, _______, _______, _______, _______,
      _______, KC_F5,   KC_F6,   KC_F7,   KC_F8,   _______,                                     _______, KC_RSFT, KC_RCTL, KC_LALT, KC_RGUI, _______,
      _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______,                    _______, _______,          _______, _______, _______,
      _______, _______, _______, _______, _______,                    _______, _______,          _______, _______, _______
    ),

/*
 * Layer 6 — SYSTEM: Layer switches (left) + Display modes / TT / RGB ctrl (right)
 * Accessed via: MO6 left thumb outermost (SYS) — right hand free for right-side keys
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * | EEClr  |      |      |      |      |      |                              |Dsp 1 |Dsp 2 |Dsp 3 |Dsp 4 |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |DF(0) |      |      |                              | TT-  | TT+  |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |DF(1) |      |      |                              |RMTog | RM S+| RM H+| RM V+| RM Nx|AnimTgl |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |DF(2) |      |      |      |      |  |      |      |      | RM S-| RM H-| RM V-| RM Pv|        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_SYSTEM] = LAYOUT_elora_hlc(
      QK_CLEAR_EEPROM, _______, _______, _______,     _______, _______,   KC_DISP_1, KC_DISP_2, KC_DISP_3, KC_DISP_4, _______, _______,
      _______,         _______, _______, DF(_BASE),   _______, _______,   KC_TT_DN,  KC_TT_UP,  _______, _______, _______, _______,
      _______,         _______, _______, DF(_NUMFN),  _______, _______,   RM_TOGG,   RM_SATU,   RM_HUEU, RM_VALU, RM_NEXT, KC_RGB_ANIM_TOGGLE,
      _______,         _______, _______, DF(_RGBNAV), _______, _______, _______, _______, _______, _______, _______, RM_SATD, RM_HUED, RM_VALD, RM_PREV, _______,
      _______,         _______, _______, _______,     _______,                   _______, _______,          _______, _______, _______,
      KC_NO,           KC_NO,   KC_NO,   KC_NO,       KC_NO,                     KC_NO,   KC_NO,            KC_NO,   KC_NO,   KC_NO
    ),

/*
 * Layer 7 — EXTRA: Empty placeholder
 */
    [_EXTRA] = LAYOUT_elora_hlc(
      _______, _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______,
      _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______
    ),
};
// clang-format on

// ---------------------------------------------------------------------------
// Encoder map
// 4 encoders: Enc0=left-outer, Enc1=left-inner, Enc2=right-inner, Enc3=right-outer
// ---------------------------------------------------------------------------
#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]   = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)  },
    [_NUMFN]  = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(KC_LEFT, KC_RIGHT)  },
    [_RGBNAV] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(RM_VALD, RM_VALU)   },
    [_MODNAV] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(KC_PGUP, KC_PGDN)   },
    [_SYM]    = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(KC_PGUP, KC_PGDN)   },
    [_FN]     = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(KC_PGUP, KC_PGDN)   },
    [_SYSTEM] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_PGUP, KC_PGDN), ENCODER_CCW_CW(KC_PGUP, KC_PGDN)   },
    [_EXTRA]  = { ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)   },
};
#endif

// ---------------------------------------------------------------------------
// Idle brightness — shared by RGB LEDs and the LCD curtain effect.
//
// Timeline:
//   0:00 – 5:00      full brightness  (scale = 255)
//   5:00 – 5:20      smooth transition full → 20 %  (scale 255 → 51)
//   5:20 – 15:00     hold dim  (scale = 51)
//   15:00 – 15:20    smooth transition 20 % → off   (scale 51 → 0)
//   ≥ 15:20          off  (scale = 0)
//
// Each 20 s window uses a smooth-step curve f(t)=3t²–2t³ so the transition
// starts and ends gently.  All arithmetic is integer-only (no float).
// ---------------------------------------------------------------------------
#define IDLE_DIM_START_MS   (5UL  * 60UL * 1000UL)           // 5:00
#define IDLE_DIM_END_MS     (IDLE_DIM_START_MS + 20000UL)    // 5:20
#define IDLE_OFF_START_MS   (15UL * 60UL * 1000UL)           // 15:00
#define IDLE_OFF_END_MS     (IDLE_OFF_START_MS + 20000UL)    // 15:20
#define IDLE_DIM_MIN        51                                // 20 % of 255

// Smooth-step helper: maps t ∈ [0,255] → ease ∈ [0,255] with zero derivative
// at both ends.  Result is clamped; all temporaries are uint16.
static inline uint8_t smoothstep8(uint8_t t) {
    uint16_t t2   = ((uint16_t)t * t) >> 8;
    uint16_t t3   = (t2 * t) >> 8;
    uint16_t ease = 3u * t2 > 2u * t3 ? 3u * t2 - 2u * t3 : 0u;
    return ease > 255u ? 255u : (uint8_t)ease;
}

static uint8_t compute_idle_brightness_scale(void) {
    uint32_t elapsed = last_input_activity_elapsed();

    if (elapsed < IDLE_DIM_START_MS)  return 255;
    if (elapsed >= IDLE_OFF_END_MS)   return 0;

    if (elapsed < IDLE_DIM_END_MS) {
        // First window: 255 → IDLE_DIM_MIN over 20 s
        uint8_t t    = (uint8_t)((elapsed - IDLE_DIM_START_MS) * 255UL / 20000UL);
        uint8_t ease = smoothstep8(t);
        return (uint8_t)(255u - ((uint16_t)ease * (255u - IDLE_DIM_MIN) >> 8));
    }

    if (elapsed < IDLE_OFF_START_MS)  return IDLE_DIM_MIN;

    // Second window: IDLE_DIM_MIN → 0 over 20 s
    uint8_t t    = (uint8_t)((elapsed - IDLE_OFF_START_MS) * 255UL / 20000UL);
    uint8_t ease = smoothstep8(t);
    return (uint8_t)(IDLE_DIM_MIN - ((uint16_t)IDLE_DIM_MIN * ease >> 8));
}

// ---------------------------------------------------------------------------
// RGB Matrix — per-layer static colors
// ---------------------------------------------------------------------------
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    return apply_layer_rgb(led_min, led_max, compute_idle_brightness_scale());
}

// ---------------------------------------------------------------------------
// Tapping term — adjustable at runtime via KC_TT_UP / KC_TT_DN (±5 ms).
// Clamped to [100, 300].  Not persisted — resets to TAPPING_TERM on power cycle.
// ---------------------------------------------------------------------------
uint16_t g_tapping_term = TAPPING_TERM;

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    return g_tapping_term;
}

// ---------------------------------------------------------------------------
// SS5 deferred callback — runs from the main loop 50 ms after SS5_KEY press,
// by which time the modifier-only HID report has already been flushed to the host.
//
// cb_arg points to a static ss_action_t that encodes which key+mods to fire,
// chosen at press-time based on the detected host OS.
// ---------------------------------------------------------------------------
typedef struct { uint8_t mods; uint16_t keycode; } ss_action_t;
static ss_action_t ss_action;

static uint32_t ss5_deferred(uint32_t trigger_time, void *cb_arg) {
    ss_action_t *a = (ss_action_t *)cb_arg;
    register_code(a->keycode);
    wait_ms(20);
    unregister_code(a->keycode);
    unregister_mods(a->mods);
    return 0;
}

// ---------------------------------------------------------------------------
// Display mode switching
//
// display_module_housekeeping_task_user is called every main-loop tick by the
// HLC module. Return false = we own the surface (HLC won't redraw); true = let
// HLC draw its stock layer/lock widget.
//
// Mode transitions are edge-triggered via prev_mode. Each display adds its own
// enter/exit/tick logic below as more modes are wired in.
// ---------------------------------------------------------------------------
#ifdef HLC_TFT_DISPLAY

#    include "gif_display.h"
#    include "stats_ui.h"
#    include "conway.h"

static uint8_t current_display_mode = 1;  // 1 = stock HLC

bool display_module_housekeeping_task_user(bool second_display) {
    if (second_display) return true;

    static uint8_t prev_mode = 0xFF;

    if (prev_mode != current_display_mode) {
        // ── Exit old mode ──────────────────────────────────────────────────────
        if (prev_mode == 4) {
            gif_display_stop();
            qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
            qp_surface_draw(lcd_surface, lcd, 0, 0, 0);
            qp_flush(lcd);
            display_invalidate_cache();
        }
        if (prev_mode == 2) {
            stats_ui_cleanup();
            qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
            qp_surface_draw(lcd_surface, lcd, 0, 0, 0);
            qp_flush(lcd);
            display_invalidate_cache();
        }
        if (prev_mode == 3) {
            conway_cleanup();
            qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
            qp_surface_draw(lcd_surface, lcd, 0, 0, 0);
            qp_flush(lcd);
            display_invalidate_cache();
        }
        // ── Enter new mode ─────────────────────────────────────────────────────
        if (current_display_mode == 4) {
            qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
            qp_surface_draw(lcd_surface, lcd, 0, 0, 0);
            qp_flush(lcd);
            gif_display_start();
        }
        if (current_display_mode == 2) {
            stats_ui_invalidate();
        }
        if (current_display_mode == 3) {
            conway_init();
        }
        prev_mode = current_display_mode;
    }

    if (current_display_mode == 4) {
        return false;  // gif_display is autonomous (driven by qp_animate)
    }

    if (current_display_mode == 2) {
        static uint32_t last_stats = 0;
        if (timer_elapsed32(last_stats) >= 100) {
            stats_ui_draw();
            last_stats = timer_read32();
        }
        return false;
    }

    if (current_display_mode == 3) {
        static uint32_t last_life = 0;
        if (timer_elapsed32(last_life) >= 100) {
            conway_tick();
            last_life = timer_read32();
        }
        return false;
    }

    return true;  // mode 1: let HLC draw stock layer/lock widget
}

#endif  // HLC_TFT_DISPLAY

// ---------------------------------------------------------------------------
// Custom keycode handling
// ---------------------------------------------------------------------------
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef HLC_TFT_DISPLAY
    if (current_display_mode == 3 && record->event.pressed) {
        conway_keypress();
    }
#endif

    switch (keycode) {

        case SS5_KEY:
            // Screenshot shortcut — needs modifier report flushed before the key fires.
            // We register mods here and return; a deferred callback taps the key 50 ms
            // later from the main loop, after USB has sent the modifier-only report.
            //
            // OS-aware:
            //   macOS  — Cmd+Shift+5 (screenshot menu). Ctrl↔GUI is swapped in
            //             macOS Modifier Keys for the Elora, so QMK sends KC_LCTL
            //             for macOS to see it as Command.
            //   Windows — Win+Shift+S (Snipping Tool overlay).
            //   Other   — falls back to macOS behavior.
            if (record->event.pressed) {
                if (detected_host_os() == OS_WINDOWS) {
                    ss_action.mods    = MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT);
                    ss_action.keycode = KC_S;
                } else {
                    ss_action.mods    = MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT);
                    ss_action.keycode = KC_5;
                }
                register_mods(ss_action.mods);
                defer_exec(50, ss5_deferred, &ss_action);
            }
            return false;

        case KC_DISP_1:
        case KC_DISP_2:
        case KC_DISP_3:
        case KC_DISP_4:
#ifdef HLC_TFT_DISPLAY
            if (record->event.pressed) {
                if (keycode == KC_DISP_1) {
                    current_display_mode = 1;
                } else if (keycode == KC_DISP_2) {
                    if (current_display_mode == 2) {
                        stats_ui_cycle_bottom();
                    } else {
                        current_display_mode = 2;
                    }
                } else if (keycode == KC_DISP_3) {
                    current_display_mode = 3;
                } else {
                    current_display_mode = 4;
                }
            }
#endif
            return false;

        case KC_TT_UP:
            if (record->event.pressed && g_tapping_term < 300) g_tapping_term += 5;
            return false;

        case KC_TT_DN:
            if (record->event.pressed && g_tapping_term > 100) g_tapping_term -= 5;
            return false;

        case KC_RGB_ANIM_TOGGLE:
            if (record->event.pressed) {
                rgb_animation_override = !rgb_animation_override;
            }
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Raw HID — time sync from macOS host (for ambient LCD mode)
// ---------------------------------------------------------------------------
uint8_t host_hour   = 12;
uint8_t host_minute = 0;

#define CMD_SET_TIME 0x01

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length < 3) return;
    if (data[0] == CMD_SET_TIME) {
        host_hour   = data[1];
        host_minute = data[2];
    }
}

