// Halcyon Elora rev2 — custom keymap (cravingpixels v6)
// Halcyon Elora rev2 — custom keymap (cravingpixels v6)
//
// Layers: BASE · NUMFN · NAV · SYM · META
//
// LT keys (TAPPING_TERM 175 ms):
//   tap Spc / hold MO(NUMFN) — left thumb
//   tap Ent / hold MO(NAV)   — right thumb
//
// SS5_KEY: OS-aware screenshot shortcut — modifier report flushed before key fires.
//   macOS  → Cmd+Shift+5 (screenshot menu). Ctrl↔GUI swapped, so QMK sends KC_LCTL.
//   Windows→ Win+Shift+S (Snipping Tool). Detected via OS_DETECTION at enumeration.
// Left half compiled with HLC_TFT_DISPLAY=1, right half with HLC_ENCODER_REV2=1.

#include QMK_KEYBOARD_H
#include "rgb_layers.h"
#include "os_detection.h"
#include "transactions.h"
#ifdef HLC_TFT_DISPLAY
#    include "users/halcyon_modules/splitkb/hlc_tft_display/hlc_tft_display.h"
#    include "qp_surface_internal.h"  // surface dirty flag, see lcd_reinit
#    include "gif_display.h"
#    include "stats_ui.h"
#    include "conway.h"
#    include "boot_diag.h"
#endif
#include "usb_recover.h"

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
    KC_RGB_ANIM_TOGGLE,         // Toggle RGB animation override (static layer colors ↔ animation)
    KC_ANIM_BREATHE,            // Animation: Breathing
    KC_ANIM_CHEVRON,            // Animation: Rainbow Moving Chevron
    KC_ANIM_HEATMAP,            // Animation: Typing Heatmap
    KC_ANIM_SPLASH,             // Animation: Splash (reactive)
    KC_SHOW_METRICS,            // Cycle metric toast: TT → LED → SPD → HUE → off
    KC_HUE_RESET,               // Reset layer palette hue offset to 0
    KC_VOL_UP,                  // macOS fine volume up (Sft+Opt+Vol, VOL_TAPS_PER_TICK taps per tick)
    KC_VOL_DN,                  // macOS fine volume down (Sft+Opt+Vol, VOL_TAPS_PER_TICK taps per tick)
    KC_KNOB,                    // Encoder button: tap = mute, hold + turn = seek (see knob_*)
    SLACK_REACT,                // Slack: react to last message — taps Up, then Cmd+Shift+backslash
    // SYM home row mods on shifted symbols. QMK's MT() drops the shift bit
    // from the tap keycode, so LGUI_T(KC_CIRC) would send "6" on tap instead
    // of "^". These custom keycodes preserve shift on tap (see hrm_table).
    HRM_CIRC,                   // hold = GUI    tap = ^   (Shift+6)
    HRM_AMPR,                   // hold = Alt    tap = &   (Shift+7)
    HRM_ASTR,                   // hold = Shift  tap = *   (Shift+8)
    HRM_QUES,                   // hold = Ctrl   tap = ?   (Shift+/)
    HRM_LABK,                   // hold = R-GUI  tap = <   (Shift+,)
};

// Aerospace window-resize shortcuts. macOS aerospace.toml binds
//   alt-shift-cmd-minus = 'resize smart -100'
//   alt-shift-cmd-equal = 'resize smart +100'
// With Ctrl↔GUI swapped in macOS Modifier Keys, QMK's LCTL is what macOS
// interprets as Cmd. Defined as macros (not custom keycodes) so the token
// stays short and doesn't widen the layer grid columns.
#define AS_DEC LALT(LSFT(LCTL(KC_MINUS)))
#define AS_INC LALT(LSFT(LCTL(KC_EQUAL)))

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
 * |  Tab   | GUI/A| Alt/S| Sft/D| Ctl/F|   G  |                              |   H  | Ctl/J| Sft/K| Alt/L| GUI/;|   '    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |   `    |   Z  |   X  |   C  |   V  |   B  | BSpc | Caps |  | Del  | SS5  |   N  |   M  |  ,   |  .   |  /   | RSft   |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        | META | Hypr | Spc  |MEH/BS| SYM  |  | SYM  |MEH/DL| Ent  | Hypr | APPS |
 *                        |      |      | NUMFN|      |      |  |      |      | NAV  |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              | Mute |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_BASE] = LAYOUT_elora_hlc(
      KC_APPLICATION,  KC_1,         KC_2,          KC_3,          KC_4,            KC_5,                                                                          KC_6,               KC_7,            KC_8,            KC_9,            KC_0,            KC_RALT,
      KC_ESCAPE,       KC_Q,         KC_W,          KC_E,          KC_R,            KC_T,                                                                          KC_Y,               KC_U,            KC_I,            KC_O,            KC_P,            KC_BSPC,
      KC_TAB,          LGUI_T(KC_A), LALT_T(KC_S),  LSFT_T(KC_D),  LCTL_T(KC_F),    KC_G,                                                                          KC_H,               RCTL_T(KC_J),    RSFT_T(KC_K),    LALT_T(KC_L),    RGUI_T(KC_SCLN), KC_QUOTE,
      KC_GRAVE,        KC_Z,         KC_X,          KC_C,          KC_V,            KC_B,                 KC_BSPC,        KC_CAPS,     KC_DEL,   SS5_KEY,          KC_N,               KC_M,            KC_COMMA,        KC_DOT,          KC_SLASH,        KC_RSFT,
                                                    MO(_META),     OSM(MOD_HYPR),   LT(_NUMFN, KC_SPACE), MEH_T(KC_BSPC), MO(_SYM),    MO(_SYM), MEH_T(KC_DELETE), LT(_NAV, KC_ENTER), OSM(MOD_HYPR),   MO(_APPS),

      KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,                                                                                                                                             KC_KNOB,      KC_NO, KC_NO, KC_NO, KC_NO
    ),

/*
 * Layer 1 — NUMFN: Numbers (left) + F-keys (right)   [hold Space]
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   =    |   1  |   2  |   3  |   4  |   5  |                              | F11  |  F1  |  F2  |  F3  |  F4  |   F5   |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   -    | GUI/6| Alt/7| Sft/8| Ctl/9|   0  |                              | F12  |Ctl/F6|Sft/F7|Alt/F8|GUI/F9|  F10   |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        | Undo | Cut  | Copy |Paste |  .   |      |      |  |      |      |      |      |      |      |  \   |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * | LED- |      |       |      |      |                                              | LED+ |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_NUMFN] = LAYOUT_elora_hlc(
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                                                                         KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_EQUAL,        KC_1,         KC_2,          KC_3,          KC_4,            KC_5,                                                                          KC_F11,             KC_F1,           KC_F2,           KC_F3,           KC_F4,           KC_F5,
      KC_MINUS,        LGUI_T(KC_6), LALT_T(KC_7),  LSFT_T(KC_8),  LCTL_T(KC_9),    KC_0,                                                                          KC_F12,             RCTL_T(KC_F6),   RSFT_T(KC_F7),   LALT_T(KC_F8),   RGUI_T(KC_F9),   KC_F10,
      KC_NO,           LCTL(KC_Z),   LCTL(KC_X),    LCTL(KC_C),    LCTL(KC_V),      KC_DOT,               KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_BSLS,         KC_NO,
                                                    KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,

      RM_VALD, KC_NO, KC_NO, KC_NO, KC_NO,                                                                                                                                             RM_VALU,      KC_NO, KC_NO, KC_NO, KC_NO
    ),

/*
 * Layer 2 — NAV: Modifiers + media (left) + Navigation (right)   [hold NAV or Ent/NAV]
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      |      | Next |      |                              | Home | PgDn | PgUp | End  |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        | GUI  | Alt  | Sft  | Ctl  | Play |                              | Left | Down |  Up  | Rght |  <   |   >    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |      | Prev |      |      |      |  |      |      |      |      | BSpc | Del  |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * | PgUp |      |       |      |      |                                              | PgDn |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_NAV] = LAYOUT_elora_hlc(
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                                                                         KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_MNXT,         KC_NO,                                                                         KC_HOME,            KC_PGDN,         KC_PGUP,         KC_END,          KC_NO,           KC_NO,
      KC_NO,           KC_LGUI,      KC_LALT,       KC_LSFT,       KC_LCTL,         KC_MPLY,                                                                       KC_LEFT,            KC_DOWN,         KC_UP,           KC_RIGHT,        LSFT(KC_COMMA),  LSFT(KC_DOT),
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_MPRV,         KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_BSPC,         KC_DEL,          KC_NO,           KC_NO,
                                                    KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,

      KC_PGUP, KC_NO, KC_NO, KC_NO, KC_NO,                                                                                                                                             KC_PGDN,      KC_NO, KC_NO, KC_NO, KC_NO
    ),

/*
 * Layer 3 — SYM: Symbols for coding   [hold SYM]
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   +    |   !  |   @  |   #  |   $  |   %  |                              |   =  |   (  |   )  |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |   _    |GUI ^ |Alt & |Sft × |Ctl ? |   /  |                              |      |Ctl/[ |Sft/] | Alt  |GUI < |   >    |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |   ~    |   \  |   :  |   ;  |   -  |   |  |      |      |  |      |      |      |   {  |   }  |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_SYM] = LAYOUT_elora_hlc(
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                                                                         KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_PLUS,         KC_EXLM,      KC_AT,         KC_HASH,       KC_DLR,          KC_PERC,                                                                       KC_EQUAL,           KC_LPRN,         KC_RPRN,         KC_NO,           KC_NO,           KC_NO,
      KC_UNDS,         HRM_CIRC,     HRM_AMPR,      HRM_ASTR,      HRM_QUES,        KC_SLASH,                                                                      KC_NO,              RCTL_T(KC_LBRC), RSFT_T(KC_RBRC), LALT_T(KC_NO),   HRM_LABK,        LSFT(KC_DOT),
      KC_TILD,         KC_BSLS,      KC_COLN,       KC_SCLN,       KC_MINUS,        KC_PIPE,              KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_LCBR,         KC_RCBR,         KC_NO,           KC_NO,           KC_NO,
                                                    KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,

      KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,                                                                                                                                             KC_NO,        KC_NO, KC_NO, KC_NO, KC_NO
    ),

/*
 * Layer 4 — META: Keyboard settings   [hold META]
 *
 * AniTgl: off = per-layer Catppuccin static colors; on = animation runs freely.
 * Encoder 3 (META): hue encoder — shifts animation hue (anim mode) or
 *   layer palette hue offset (static mode). Click resets palette hue to 0.
 * Hue± also work as grid keys. Val± / Spd± adjust brightness / animation speed.
 * Columns aligned: Brth/Hue+/Hue-  Chvr/Val+/Val-  Heat/Spd+/Spd-
 * Mtrc: cycle LCD metric toast (TT→LED→SPD→HUE→off).
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        | TT-  | TT+  |      | Mtrc |      |                              |      | Brth | Chvr | Heat | Spls |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * | EEClr  |Dsp 1 |Dsp 2 |Dsp 3 |Dsp 4 |RMTog|                              |AniTgl| Hue+ | Val+ | Spd+ |HueRst|        |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |      | Hue- | Val- | Spd- |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * | Hue- |      |       |      |      |                                              | HRst |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_META] = LAYOUT_elora_hlc(
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                                                                         KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_NO,           KC_TT_DN,     KC_TT_UP,      KC_NO,         KC_SHOW_METRICS, KC_NO,                                                                         KC_NO,              KC_ANIM_BREATHE, KC_ANIM_CHEVRON, KC_ANIM_HEATMAP, KC_ANIM_SPLASH,  KC_NO,
      QK_CLEAR_EEPROM, KC_DISP_1,    KC_DISP_2,     KC_DISP_3,     KC_DISP_4,       RM_TOGG,                                                                       KC_RGB_ANIM_TOGGLE, RM_HUEU,         RM_VALU,         RM_SPDU,         KC_HUE_RESET,    KC_NO,
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              RM_HUED,         RM_VALD,         RM_SPDD,         KC_NO,           KC_NO,
                                                    KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,

      RM_HUED, KC_NO, KC_NO, KC_NO, KC_NO,                                                                                                                                             KC_HUE_RESET, KC_NO, KC_NO, KC_NO, KC_NO
    ),

/*
 * Layer 5 — APPS: app-specific shortcuts   [hold right outer thumb]
 *
 * Zellij    : Alt+]            — next swap layout (Alt+[/PreviousSwapLayout
 *                                  not bound here — see config.kdl)
 * IntelliJ  : Cmd+[ / Cmd+]    — back / forward navigation (also works in many editors)
 * Slack     : Cmd+Shift+\      — react to last message
 * Aerospace : Opt+Sft+Cmd+- / Opt+Sft+Cmd+=  — shrink / grow focused window
 *
 * ,-------------------------------------------.                              ,-------------------------------------------.
 * |        |      |      |      |      |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      |      | AS - | AS + |      |                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|                              |------+------+------+------+------+--------|
 * |        |      | Z ]  | IJ [ | IJ ] | React|                              |      |      |      |      |      |        |
 * |--------+------+------+------+------+------+-------------.  ,-------------+------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 * ,-----------------------------------.                                              ,-----------------------------------.
 * |      |      |       |      |      |                                              |      |      |       |      |      |
 * `-----------------------------------'                                              `-----------------------------------'
 */
    [_APPS] = LAYOUT_elora_hlc(
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                                                                         KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_NO,           KC_NO,        KC_NO,         AS_DEC,        AS_INC,          KC_NO,                                                                         KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_NO,           KC_NO,        LALT(KC_RBRC), LCTL(KC_LBRC), LCTL(KC_RBRC),   SLACK_REACT,                                                                   KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
      KC_NO,           KC_NO,        KC_NO,         KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,           KC_NO,           KC_NO,           KC_NO,
                                                    KC_NO,         KC_NO,           KC_NO,                KC_NO,          KC_NO,       KC_NO,    KC_NO,            KC_NO,              KC_NO,           KC_NO,

      KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,                                                                                                                                             KC_NO,        KC_NO, KC_NO, KC_NO, KC_NO
    ),
};
// clang-format on

// ---------------------------------------------------------------------------
// Chordal Hold handedness (see CHORDAL_HOLD in config.h)
// 'L'/'R' = hand, '*' = exempt: may be held together with any key.
// ---------------------------------------------------------------------------
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT_elora_hlc(
    'L', 'L', 'L', 'L', 'L', 'L',                                     'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',                                     'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',                                     'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L', '*', 'L',                 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
                        '*', '*', '*', '*', '*',            '*', '*', '*', '*', '*',
    '*', '*', '*', '*', '*',                                          '*', '*', '*', '*', '*'
);

// ---------------------------------------------------------------------------
// Encoder map
// 4 encoders: Enc0=left-outer, Enc1=left-inner, Enc2=right-inner, Enc3=right-outer
// ---------------------------------------------------------------------------
#ifdef ENCODER_MAP_ENABLE
// Enc0=left-outer  Enc1=left-inner  Enc2=right-inner  Enc3=right-outer (main dial)
// Left thumb activates layers; right hand turns the dial.
// BASE=volume 1.6%/tick (see vol_request)  NUMFN(Space)=LED brightness (step ~5%)  NAV(MO·NAV)=page
// META(META)=hue: shifts animation hue (anim mode) or layer palette offset (static mode); click resets hue
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]  = { ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP) },
    [_NUMFN] = { ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(RM_VALD,    RM_VALU   ) },
    [_NAV]   = { ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_PGUP,    KC_PGDN   ) },
    [_SYM]   = { ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN,  KC_VOL_UP ) },
    [_META]  = { ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(RM_HUED,    RM_HUEU   ) },
    [_APPS]  = { ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN, KC_VOL_UP), ENCODER_CCW_CW(KC_VOL_DN,  KC_VOL_UP ) },
};
#endif

// ---------------------------------------------------------------------------
// Split sync — rgb_animation_override
//
// Each half runs its own rgb_matrix_indicators_advanced_user (split_count=[37,37]).
// The slave's copy of rgb_animation_override is always false unless we push it.
// USER_SYNC_RGB_ANIM is registered in config.h via SPLIT_TRANSACTION_IDS_USER.
// ---------------------------------------------------------------------------
typedef struct { bool anim_override; int8_t hue_offset; } rgb_sync_t;

static void rgb_anim_sync_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    const rgb_sync_t *d = (const rgb_sync_t *)in_data;
    rgb_animation_override = d->anim_override;
    layer_hue_offset       = d->hue_offset;
}

// Suspend loop hook: USB link watchdog on every half, boot diagnostics on the LCD half.
void suspend_power_down_user(void) {
#ifdef HLC_TFT_DISPLAY
    boot_diag_suspend();
#endif
    usb_recover_tick();
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(USER_SYNC_RGB_ANIM, rgb_anim_sync_handler);
#ifdef HLC_TFT_DISPLAY
    boot_diag_init();
#endif
}

void housekeeping_task_user(void) {
    if (!is_keyboard_master()) return;
#ifdef HLC_TFT_DISPLAY
    boot_diag_task();
#endif
    static bool  last_anim   = false;
    static int8_t last_offset = 0;
    if (last_anim == rgb_animation_override && last_offset == layer_hue_offset) return;
    last_anim   = rgb_animation_override;
    last_offset = layer_hue_offset;
    rgb_sync_t d = { .anim_override = rgb_animation_override, .hue_offset = layer_hue_offset };
    transaction_rpc_send(USER_SYNC_RGB_ANIM, sizeof(d), &d);
}

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
// macOS fine-step volume — Shift+Option+Volume = 1/64 of full scale (1.5625 %).
//
// Modifier reports and consumer (volume) reports travel on different USB
// endpoints, so sending them back to back lets the host process the volume
// report before, or between, the modifier reports: the tick then lands as a
// plain 6.25 % step, or as Option-only, which opens Sound Settings. The drain
// below holds Shift+Option for the whole burst, gives the host VOL_MOD_SETTLE_MS
// to apply each modifier change, and spaces the volume press/release by
// VOL_TAP_MS. Encoder ticks only bump a counter, so a fast spin never blocks
// the main loop or overflows the 4-slot encoder event queue.
//
// Only macOS understands Shift+Option as "fine step". Windows ignores the
// modifiers (always 2 % per key), and Linux desktops may grab Shift+Alt as a
// shortcut, so on any host that OS detection does not report as macOS/iOS the
// drain sends a bare volume key, one per click.
// ---------------------------------------------------------------------------
#define VOL_TAPS_PER_TICK   1    // macOS only: 1 = 1.6 % per click, 2 = 3.1 %, 4 = 6.25 %
#define VOL_MOD_SETTLE_MS   15
#define VOL_TAP_MS          10
#define VOL_PENDING_MAX     16   // caps the backlog from a long fast spin
#define VOL_MODS            (MOD_BIT(KC_LSFT) | MOD_BIT(KC_LALT))

enum vol_state { VOL_IDLE, VOL_MODS_SETTLING, VOL_KEY_DOWN, VOL_KEY_UP };

static enum vol_state vol_state     = VOL_IDLE;
static int8_t         vol_pending   = 0;    // > 0 taps up, < 0 taps down
static uint8_t        vol_mods_added = 0;   // only the mods we added, so held HRMs survive
static uint16_t       vol_key_down  = KC_NO;

static uint32_t vol_drain(uint32_t trigger_time, void *cb_arg) {
    switch (vol_state) {
        case VOL_MODS_SETTLING:
        case VOL_KEY_UP:
            if (vol_pending == 0) {
                unregister_mods(vol_mods_added);
                vol_mods_added = 0;
                vol_state      = VOL_IDLE;
                return 0;
            }
            vol_key_down = vol_pending > 0 ? KC_VOLU : KC_VOLD;
            vol_pending += vol_pending > 0 ? -1 : 1;
            register_code(vol_key_down);
            vol_state = VOL_KEY_DOWN;
            return VOL_TAP_MS;

        case VOL_KEY_DOWN:
            unregister_code(vol_key_down);
            vol_key_down = KC_NO;
            vol_state    = VOL_KEY_UP;
            return VOL_TAP_MS;

        case VOL_IDLE:
            break;
    }
    return 0;
}

// Encoder button. Tap = mute. While held, the volume keycodes coming from the
// encoder map turn into seek keys instead, so hold + turn scrubs a video and the
// knob is volume again on release. A hold that never turned sends nothing, so a
// long press cannot mute by accident.
#define KNOB_SEEK_CCW KC_LEFT
#define KNOB_SEEK_CW  KC_RIGHT

static bool     knob_held       = false;
static bool     knob_turned     = false;
static uint16_t knob_press_time = 0;

static void knob_press(bool pressed) {
    if (pressed) {
        knob_held       = true;
        knob_turned     = false;
        knob_press_time = timer_read();
        return;
    }
    knob_held = false;
    if (!knob_turned && timer_elapsed(knob_press_time) < g_tapping_term) tap_code(KC_MUTE);
}

// Returns true when the encoder click was consumed as a seek step.
static bool knob_seek(int8_t direction) {
    if (!knob_held) return false;
    knob_turned = true;
    tap_code(direction > 0 ? KNOB_SEEK_CW : KNOB_SEEK_CCW);
    return true;
}

static bool vol_host_has_fine_step(void) {
    os_variant_t os = detected_host_os();
    return os == OS_MACOS || os == OS_IOS;
}

// direction: +1 volume up, -1 volume down (one encoder click).
static void vol_request(int8_t direction) {
    bool    fine = vol_host_has_fine_step();
    int8_t  taps = fine ? VOL_TAPS_PER_TICK : 1;
    int16_t next = (int16_t)vol_pending + direction * taps;
    if (next >  VOL_PENDING_MAX) next =  VOL_PENDING_MAX;
    if (next < -VOL_PENDING_MAX) next = -VOL_PENDING_MAX;
    vol_pending = (int8_t)next;

    if (vol_state != VOL_IDLE) return;

    vol_mods_added = fine ? (VOL_MODS & ~get_mods()) : 0;
    register_mods(vol_mods_added);
    vol_state = VOL_MODS_SETTLING;
    defer_exec(VOL_MOD_SETTLE_MS, vol_drain, NULL);
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

static uint8_t current_display_mode = 2;  // 1 = stock HLC, 2 = stats (default)

// The ST7789 keeps its own state and gets its init sequence exactly once, at boot
// (module_post_init_kb in hlc_tft_display.c). A dip on the USB rail or a host sleep
// can reset the panel while the RP2040 keeps running: the panel then sits in sleep
// mode with the backlight on and ignores every pixel write until it is initialised
// again. Nothing in QMK or the module does that, so do it here whenever the display
// was dark anyway: on the first input after the module's backlight timeout, and
// after a USB resume. The surface framebuffer in RAM is still valid, so it is pushed
// back as a whole instead of resetting any UI state. qp_init blocks for ~150 ms.
static volatile bool lcd_reinit_pending = false;

static void lcd_reinit(void) {
    qp_init(lcd, LCD_ROTATION);
    qp_set_viewport_offsets(lcd, LCD_OFFSET_X, LCD_OFFSET_Y);
    // qp_surface_draw returns early on a clean surface even with entire_surface set,
    // and the surface is clean whenever nothing changed since the last flush. Mark
    // it dirty so the whole framebuffer reaches the freshly reset panel.
    ((surface_painter_device_t *)lcd_surface)->dirty.is_dirty = true;
    qp_surface_draw(lcd_surface, lcd, 0, 0, true);
    qp_flush(lcd);
}

// Called from the USB interrupt: only raise the flag, the SPI work runs in the
// housekeeping task.
void suspend_wakeup_init_user(void) {
    lcd_reinit_pending = true;
    boot_diag_wakeup();
}

static void lcd_reinit_task(void) {
    static bool was_idle = false;
    bool        idle     = last_input_activity_elapsed() > HLC_BACKLIGHT_TIMEOUT;
    if (was_idle && !idle) lcd_reinit_pending = true;
    was_idle = idle;
    if (!lcd_reinit_pending) return;
    lcd_reinit_pending = false;
    lcd_reinit();
}

bool display_module_housekeeping_task_user(bool second_display) {
    if (second_display) return true;
    lcd_reinit_task();

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
            stats_ui_invalidate_gif();
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
// SYM home row mod-taps with shifted-symbol tap actions
//
// QMK's built-in MT()/LSFT_T()/etc. mask the tap keycode to 8 bits, dropping
// the shift bit baked into shifted "convenience" keycodes (KC_CIRC == S(KC_6),
// KC_AMPR == S(KC_7), KC_ASTR == S(KC_8), KC_QUES == S(KC_SLASH), KC_LABK ==
// S(KC_COMMA)). LGUI_T(KC_CIRC) would therefore send "6" on tap instead of "^".
//
// This minimal hold-on-other-key-press implementation gives correct behavior
// for the SYM home row:
//   press                                         → arm timer, do NOT register mod yet
//   release before TAPPING_TERM, no interruption  → tap_code16(shifted symbol)
//   any other key pressed while still armed       → register mod (becomes a hold)
//   release after mod was registered              → unregister mod
//   release after TAPPING_TERM, no interruption   → swallow (held alone too long)
//
// Trade-off vs QMK's built-in mod-tap: no permissive-hold or retro-tap. Fast
// rolls (HRM_CIRC then HRM_AMPR while CIRC still in its window) commit CIRC
// as a hold, so the second key fires with that mod. Acceptable on SYM.
// ---------------------------------------------------------------------------
enum hrm_slot { HRM_S_CIRC, HRM_S_AMPR, HRM_S_ASTR, HRM_S_QUES, HRM_S_LABK, HRM_SLOT_COUNT };

static struct {
    uint16_t timer;
    bool     active;        // pressed, not yet released
    bool     held_active;   // mod has been registered
} hrm_state[HRM_SLOT_COUNT];

static const struct {
    uint8_t  mod_bit;
    uint16_t tap_kc;
} hrm_table[HRM_SLOT_COUNT] = {
    [HRM_S_CIRC] = { MOD_BIT(KC_LGUI), KC_CIRC          },
    [HRM_S_AMPR] = { MOD_BIT(KC_LALT), KC_AMPR          },
    [HRM_S_ASTR] = { MOD_BIT(KC_LSFT), KC_ASTR          },
    [HRM_S_QUES] = { MOD_BIT(KC_LCTL), KC_QUES          },
    [HRM_S_LABK] = { MOD_BIT(KC_RGUI), LSFT(KC_COMMA)   },
};

static int hrm_slot_for(uint16_t kc) {
    switch (kc) {
        case HRM_CIRC: return HRM_S_CIRC;
        case HRM_AMPR: return HRM_S_AMPR;
        case HRM_ASTR: return HRM_S_ASTR;
        case HRM_QUES: return HRM_S_QUES;
        case HRM_LABK: return HRM_S_LABK;
        default:       return -1;
    }
}

static void hrm_promote_pending(void) {
    for (int i = 0; i < HRM_SLOT_COUNT; i++) {
        if (hrm_state[i].active && !hrm_state[i].held_active) {
            register_mods(hrm_table[i].mod_bit);
            hrm_state[i].held_active = true;
        }
    }
}

static bool hrm_process(int slot, keyrecord_t *record) {
    if (record->event.pressed) {
        hrm_state[slot].timer       = timer_read();
        hrm_state[slot].active      = true;
        hrm_state[slot].held_active = false;
    } else {
        if (hrm_state[slot].held_active) {
            unregister_mods(hrm_table[slot].mod_bit);
        } else if (timer_elapsed(hrm_state[slot].timer) < g_tapping_term) {
            tap_code16(hrm_table[slot].tap_kc);
        }
        hrm_state[slot].active      = false;
        hrm_state[slot].held_active = false;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Custom keycode handling
// ---------------------------------------------------------------------------
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef HLC_TFT_DISPLAY
    if (current_display_mode == 3 && record->event.pressed) {
        conway_keypress();
    }
#endif

    // SYM home row mod-tap: any key press while an HRM slot is armed promotes
    // it to a held modifier (hold-on-other-key-press semantics).
    if (record->event.pressed) {
        int armed_slot = hrm_slot_for(keycode);
        if (armed_slot < 0) {
            // Pressed key is not an HRM — promote any HRMs in their deciding window.
            hrm_promote_pending();
        }
    }
    {
        int slot = hrm_slot_for(keycode);
        if (slot >= 0) return hrm_process(slot, record);
    }

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

        // In static mode (no animation) Hue± shift the layer palette offset instead.
        // In animation mode QMK handles them normally (shifts rgb_matrix_config.hsv.h).
        case RM_HUEU:
            if (record->event.pressed && !rgb_animation_override) {
                layer_hue_offset += 4;
                return false;
            }
            return true;
        case RM_HUED:
            if (record->event.pressed && !rgb_animation_override) {
                layer_hue_offset -= 4;
                return false;
            }
            return true;

        case KC_HUE_RESET:
            if (record->event.pressed) layer_hue_offset = 0;
            return false;

        // Brightness keys stay QMK-handled; this only makes the LED toast appear
        // when the value is already pinned at the cap or at zero.
        case RM_VALU:
        case RM_VALD:
#ifdef HLC_TFT_DISPLAY
            if (record->event.pressed && current_display_mode == 2) stats_ui_show_led();
#endif
            return true;

        case KC_VOL_UP:
            if (record->event.pressed && !knob_seek(+1)) vol_request(+1);
            return false;
        case KC_VOL_DN:
            if (record->event.pressed && !knob_seek(-1)) vol_request(-1);
            return false;
        case KC_KNOB:
            knob_press(record->event.pressed);
            return false;

        // Slack: react to last message. Slack default = Shift+Cmd+\.
        // With Ctrl↔GUI swapped in macOS Modifier Keys, QMK sends LCTL so macOS
        // interprets it as Cmd.
        case SLACK_REACT:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_LSFT("\\")));
            }
            return false;

        case KC_SHOW_METRICS:
#ifdef HLC_TFT_DISPLAY
            if (record->event.pressed && current_display_mode == 2) stats_ui_show_next_metric();
#endif
            return false;

        // Animation preset keys — enable override and switch to the chosen mode.
        // rgb_matrix_mode_noeeprom so rapid switching doesn't hammer flash.
        case KC_ANIM_BREATHE:
            if (record->event.pressed) {
                rgb_animation_override = true;
                // Breathing uses rgb_matrix_config.hsv for its color. Default
                // saturation is 0 after EEPROM reset → white. Pre-set Lavender
                // so it looks colored immediately; hue can be shifted with Hue±.
                rgb_matrix_sethsv_noeeprom(164, 200, rgb_matrix_get_val());
                rgb_matrix_mode_noeeprom(RGB_MATRIX_BREATHING);
            }
            return false;
        case KC_ANIM_CHEVRON:
            if (record->event.pressed) {
                rgb_animation_override = true;
                rgb_matrix_mode_noeeprom(RGB_MATRIX_RAINBOW_MOVING_CHEVRON);
            }
            return false;
        case KC_ANIM_HEATMAP:
            if (record->event.pressed) {
                rgb_animation_override = true;
                rgb_matrix_mode_noeeprom(RGB_MATRIX_TYPING_HEATMAP);
            }
            return false;
        case KC_ANIM_SPLASH:
            if (record->event.pressed) {
                rgb_animation_override = true;
                rgb_matrix_mode_noeeprom(RGB_MATRIX_SPLASH);
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

