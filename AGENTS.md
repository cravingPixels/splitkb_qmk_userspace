# Halcyon Elora V2 — Agent Instructions & Technical Reference

---

## Implementation Status

| Feature | State | Notes |
|---|---|---|
| BASE layer | done | QWERTY + home-row mods |
| All layers (BASE/NUMFN/NAV/SYM/META) | done | 5 layers, reorganized from original 8 |
| Per-layer RGB colors | done | Catppuccin Mocha; LT/MO accent keys painted Peach on BASE |
| Per-key LED dimming | done | blacks out KC_TRNS/KC_NO keys per layer |
| RGB brightness controls | done | RM_VALD/RM_VALU via rgb_matrix_get_val() |
| Animation override | done | AniTgl synced to slave via split RPC; 4 presets (Breathe/Chevron/Heatmap/Splash) |
| Animation speed control | done | RM_SPDU/RM_SPDD keys + encoder in META; default 64/255 |
| LCD stock display | done | mode 1 — HLC default layer/WPM/modifier widget |
| LCD stats display | done | mode 2 — layer name, lock indicators, TT, LED val, WPM/GIF/marquee |
| LCD Conway's Game of Life | done | mode 3 |
| LCD full-screen GIF | done | mode 4 — shiny mew sleeping GIF |
| LCD pet/Tamagotchi mode | planned | sprites not started |
| LCD ambient/time-of-day mode | planned | needs macOS time-sync daemon (Raw HID) |
| Time sync daemon (macOS → Raw HID) | planned | needed for ambient mode |
| Tapping term tuning | pending | TAPPING_TERM=175ms, needs real-world testing |

## Key Reference Files (read these when debugging — don't guess)

- **LED layout source**: `~/.local/share/qmk_firmware/keyboards/splitkb/halcyon/elora/rev2/keyboard.json`
  → `rgb_matrix.layout[]` — each entry has `matrix: [hw_row, col]` or no matrix (underglow)
- **MATRIX_ROWS=12 override**: `users/halcyon_modules/splitkb/config.h`
  → contains the `LAYOUT_elora_hlc` macro and the encoder row insertions
- **Reference keymap** (ground truth for LAYOUT arg order): `keyboards/splitkb/halcyon/elora/keymaps/vial_hlc/keymap.json`
- **Original Vial export** (source of key position decisions): `keyboards/splitkb/halcyon/elora/keymaps/cravingpixels/elora.vil`

## Project Overview

QMK firmware for a **Halcyon Elora V2** split keyboard by splitkb.com.
RP2040 MCU, per-key RGB + underglow, 135×240 ST7789 TFT LCD (left half), rotary encoder (right half).

## Your Files

Everything under `keyboards/splitkb/halcyon/elora/keymaps/cravingpixels/` is the keymap:

```
keymap.c        — layers, custom keycodes, split sync, RGB callbacks
config.h        — tapping term, RGB settings, HID config, split transaction IDs
rules.mk        — enabled features
rgb_layers.h    — layer enum, Catppuccin HSV colors, per-key LED dimming, animation override
stats_ui.c      — LCD stats display (mode 2): layer name/color, locks, TT, LED val, WPM
conway.c        — LCD Conway's Game of Life (mode 3)
gif_display.c   — LCD full-screen GIF (mode 4)
*.qgf.* *.qff.* — converted image/font assets (build artifacts, not committed)
```

### Layers

| # | Name | Activation | Purpose |
|---|------|-----------|---------|
| 0 | BASE | — | QWERTY + home-row mods (GUI/Alt/Sft/Ctl) |
| 1 | NUMFN | hold Space | Numbers left, F-keys right, Undo/Cut/Copy/Paste at Z/X/C/V |
| 2 | NAV | hold MO·NAV (left inner thumb) or Ent/NAV (right thumb) | Mods + media left, arrows + Home/PgUp/PgDn/End right |
| 3 | SYM | hold SYM (right inner thumb) | Shifted symbols left mirroring NUMFN, brackets right |
| 4 | META | hold META (left outer thumb) | Display modes, tapping term, RGB/animation controls |

### Encoder map (Enc3 = right outer, main dial)

| Layer | Enc3 action |
|-------|------------|
| BASE | Volume ↕ |
| NUMFN | LED brightness ↕ |
| NAV | Page up/down |
| SYM | Volume ↕ |
| META | Animation speed ↕ |

### META layer RGB controls (right side)

```
Row 1:       | Brth | Chvr | Heat | Spls |      |
Home row: Ani| Hue+ | Val+ | Spd+ |      |      |
Bottom:       | Hue- | Val- | Spd- |      |      |
```
Columns aligned: each animation key sits above its fine-tune controls.
`AniTgl` (H): off = static Catppuccin layer colors; on = animation runs freely on both halves.

`users/halcyon_modules/` is splitkb's shared module code — do not touch it.

## Hardware Reference

- **MCU**: RP2040
- **Display**: ST7789 SPI, 135×240 px — **left half only**, compiled with `-e HLC_TFT_DISPLAY=1`
- **Encoder**: rotary encoder — **right half only**, compiled with `-e HLC_ENCODER_REV2=1`
- **RGB**: WS2812, per-key + underglow, 74 LEDs total (37 per side: 6 underglow + 31 key LEDs)
- **Encoder button**: R[row11][col0] in keymap

## LAYOUT Macro

- **Name**: `LAYOUT_elora_hlc` — NOT `LAYOUT` or `LAYOUT_elora`
- **Arguments**: 72 per layer

```
Row 0  (6+6):  L[row0][col6..1]                       R[row6][col1..6]
Row 1  (6+6):  L[row1][col6..1]                       R[row7][col1..6]
Row 2  (6+6):  L[row2][col6..1]                       R[row8][col1..6]
Row 3  (8+8):  L[row3][col6..1], L[row4][col5], L[row3][col0]
               R[row9][col0], R[row10][col5], R[row9][col1..6]
Thumb  (5+5):  L[row4][col4..0]                       R[row10][col0..4]
Enc    (5+5):  L[row5][col0..4]                       R[row11][col0..4]
```

- **Left side**: col6=outermost(far-left), col1=innermost(near-center)
- **Right side**: col1=innermost(near-center), col6=outermost(far-right)
- **Bottom row**: 8 keys per side — the 7th key comes from the thumb row array (col5),
  it is the angled inner corner key between main area and thumb cluster.

## Build System

- **Keymap path**: `keyboards/splitkb/halcyon/elora/keymaps/cravingpixels/` — edit here directly
- **Build wrapper**: `./build.sh compile|compile-left|compile-right|flash-left|flash-right`
- **Always compile via `mise exec -- qmk compile`** or `./build.sh`. Running bare `qmk`
  outside mise hits a system binary that only knows `config/clone/console/env/setup`.
- **Userspace registration**: `qmk config user.overlay_dir` must point to this directory.
  Run `mise run setup` once to configure it.

```bash
mise run compile          # both halves
mise run compile-left     # left only (TFT)
mise run compile-right    # right only (encoder)
mise run flash-left       # compile + flash left
mise run flash-right      # compile + flash right
mise run convert          # PNG/GIF → QGF
```

## Flashing

- Both halves flashed separately — each has its own RP2040
- **Bootloader**: double-tap reset → `RPI-RP2` USB drive appears → drag-drop `.uf2`
- **Emergency bootloader**: hold top-left key while plugging in USB
- **Recovery**: RP2040 is very hard to brick — re-enter bootloader and reflash
- **Stock firmware / rollback**: https://splitkb.com/fw

## Code Conventions

- C, QMK style (4-space indent, snake_case)
- Draw to `lcd_surface`, never directly to `lcd` — the module flushes automatically
- RGB layer config and LED map live in `rgb_layers.h`
- Display rendering lives in `display.c`
- PNG source assets live in `../custom-keyboard-config/graphics/lcd/assets/`

## What NOT to do

- Do not modify `users/halcyon_modules/` — that is splitkb's shared code
- Do not hardcode pin assignments — they come from the board definition
- Do not add Vial-specific code — this project targets QMK
- Do not commit `.qgf.*` files — they are generated build artifacts

---

## Technical Deep-Dives

### Display Hardware

- **Panel**: 1.14" TFT LCD, 135×240 px, RGB565 (16-bit color)
- **Driver IC**: ST7789V
- **Interface**: SPI via Halcyon module connector
- **Orientation**: portrait by default — rotate in firmware if needed

#### Quantum Painter

QMK's rendering framework for the display.

- **Device** (`painter_device_t`): represents the physical ST7789. Provided by the
  HLC module as `extern painter_device_t lcd`.
- **Surface** (`painter_device_t`): off-screen render target. Provided as
  `extern painter_device_t lcd_surface`. Draw here, never to `lcd` directly.
  The module calls `qp_surface_draw(lcd_surface, lcd, 0, 0, true)` + `qp_flush(lcd)`
  automatically — dirty-region tracking means unchanged frames cost near-zero SPI traffic.

```c
#include "users/halcyon_modules/splitkb/hlc_tft_display/hlc_tft_display.h"

bool display_module_housekeeping_task_user(bool second_display) {
    if (!second_display) {
        display_render(lcd_surface);
    }
    return true;
}
```

`second_display=false` = master half. Only master drives rendering.
`rules.mk` must have `USER_NAME := halcyon_modules`.

#### Images and Animations

```bash
qmk painter-convert-graphics -i input.png -o keymaps/cravingpixels/ -f rgb565
qmk painter-convert-graphics -i animation.gif -o keymaps/cravingpixels/ -f rgb565
qmk painter-convert-font -i font.ttf -o keymaps/cravingpixels/ -s 12 -f mono4
```

```c
#include "my_image.qgf.h"
qp_drawimage(lcd_surface, x, y, my_image);

deferred_token tok = qp_animate(lcd_surface, x, y, my_animation);
qp_stop_animation(tok);  // when switching modes

painter_font_handle_t font = qp_load_font_mem(my_font);
qp_drawtext(lcd_surface, x, y, font, "Hello");
```

#### Display Mode Architecture

```c
typedef void (*display_mode_fn)(painter_device_t surface);

static display_mode_fn modes[] = {
    display_mode_stock,
    display_mode_pet,
    display_mode_stats,
    display_mode_ambient,
};
static uint8_t current_mode = 0;
// cycle: current_mode = (current_mode + 1) % 4;
```

---

### RGB Matrix

74 LEDs total: 37 per side (6 underglow + 31 key LEDs).
WS2812-compatible, driven by QMK RGB Matrix (not RGB Light — different subsystem).

#### LED Index Layout

LEDs are ordered by PCB wiring chain (bottom-up), not row order.

```
Left:  0-5   = underglow
       6-36  = key LEDs (thumb row first, then bottom→top)
Right: 37-42 = underglow
       43-73 = key LEDs (thumb row first, then bottom→top)
```

LED 48 (right bottom inner, `KC_DEL` on BASE) is wired mid-thumb-cluster in the PCB chain
because it sits physically between the two clusters.

#### Per-Layer Colors

`apply_layer_rgb` in `rgb_layers.h`:
1. Reads active layer, looks up its HSV color (Catppuccin Mocha palette)
2. Sets all LEDs in range to that color (respecting `rgb_matrix_get_val()` for brightness)
3. Blacks out LEDs at KC_TRNS/KC_NO positions using `led_to_keypos[]` static table

#### LED → Keymap Position Table

`led_to_keypos[74]` in `rgb_layers.h` maps each LED index to a `keypos_t` in
MATRIX_ROWS=12 convention. Row 255 = underglow (skip).

Generated from `keyboard.json rgb_matrix.layout` with the row offset already applied.
**Do not re-derive at runtime.** The `.row`/`.col` values are hardware-wired and never
change. If you remap BASE layer keys, update only the comments in the table.

#### Animation Override

```c
bool rgb_animation_override = false;
// Set to true from _SYSTEM layer to let animations run freely
// Set to false to restore per-layer static colors
```

---

### Matrix Row Numbering (critical — two systems coexist)

`keyboard.json` (MATRIX_ROWS=10, source of `g_led_config`):
```
rows 0-4 = left physical
rows 5-9 = right physical
```

`users/halcyon_modules/splitkb/config.h` overrides to MATRIX_ROWS=12,
inserting virtual encoder rows:
```
rows 0-4  = left physical
row  5    = left encoder  (virtual, no LEDs)
rows 6-10 = right physical
row  11   = right encoder (virtual, no LEDs)
```

`keymaps[]` uses MATRIX_ROWS=12. `g_led_config` uses MATRIX_ROWS=10.
For right-side LEDs: g_led_config row = keymap row − 1.

**`keypos_t` field order**: QMK defines it as `{col, row}` — col is FIRST. Always use
named initializers: `(keypos_t){.row = r, .col = c}`. Positional init `{row, col}`
silently swaps them, causing right-side rows ≥ 7 to exceed MATRIX_COLS and return
KC_TRNS for everything, blacking out all right-side key LEDs.

---

### Ambient Mode — Time-of-Day

RP2040 has no RTC. Options:
1. **Host-driven** (preferred): macOS daemon sends time via Raw HID every few minutes
2. **Uptime-based**: user sets start time via keycombo, firmware tracks elapsed time

```c
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (data[0] == CMD_SET_TIME) {
        current_hour   = data[1];
        current_minute = data[2];
    }
}
```

Raw HID configured in `config.h`:
```c
#define RAW_USAGE_PAGE 0xFF60
#define RAW_USAGE_ID   0x61
```

---

### Pet/Tamagotchi Mode

- Idle: stands, blinks, looks around
- Typing (WPM > threshold): walks, jumps, dances
- Fast typing (> ~80 WPM): excited animation
- Long idle (> 30s): sits down, sleeps
- Sprite size: 48×48 or 64×64 — leaves room for status text on 135×240
- Format: pixel art GIF → QGF animation (4-8 frames per state)

---

### Stats Mode — Widgets

Use `qp_line`, `qp_rect`, `qp_drawtext` for real-time rendering (not pre-baked images):

- WPM rolling graph (last 60s)
- Active layer name
- Modifier state (Shift/Ctrl/Alt/GUI)
- Caps lock indicator
- Uptime since power-on

---

### Column Direction

Both sides count columns outward from the center split:

```
LEFT  side: col0=innermost (near center)  …  col6=outermost (far left)
RIGHT side: col0=innermost (near center)  …  col6=outermost (far right)
```

col0 only exists on the bottom row (left=Caps, right=Del) and thumb row.
Top/2nd/home rows use only cols 1-6.

### `user.overlay_dir` — Required Once Per Machine

QMK must know where the userspace repo is. Without this, it ignores the keymap entirely:

```bash
cd ~/personal/repos/splitkb_qmk_userspace
qmk config user.overlay_dir="$(realpath .)"
```

`mise run setup` does this automatically. If compilation fails with "keymap not found",
this is the first thing to check (`qmk config user.overlay_dir`).

### `rgb_layers.h` — Include From One Place Only

The file uses `static inline` and defines `rgb_animation_override` as a non-`extern`
global. Including it from more than one `.c` file will cause multiple-definition linker
errors. It is included only from `keymap.c`.

### Animation Override — Split Sync

Each half runs its own `rgb_matrix_indicators_advanced_user` (split_count=[37,37]).
`rgb_animation_override` must be synced from master to slave or the slave always
shows static layer colors regardless of AniTgl state.

Sync is implemented via `SPLIT_TRANSACTION_IDS_USER USER_SYNC_RGB_ANIM` (config.h),
registered in `keyboard_post_init_user`, and pushed in `housekeeping_task_user`
whenever the value changes. Do not remove this or AniTgl will only work on the left half.

## References

- [splitkb Elora product page](https://splitkb.com/products/halcyon-elora)
- [splitkb TFT LCD module](https://splitkb.com/products/halcyon-tft-lcd-display-module)
- [splitkb display module docs](https://docs.splitkb.com/product-guides/halcyon-series/modules/display)
- [splitkb firmware compilation guide](https://docs.splitkb.com/product-guides/halcyon-series/advanced/compiling-firmware)
- [splitkb QMK userspace](https://github.com/splitkb/qmk_userspace)
- [Halcyon Elora schematics](https://docs.splitkb.com/product-guides/halcyon-series/schematics/halcyon-elora)
- [QMK Quantum Painter docs](https://docs.qmk.fm/quantum_painter)
- [QMK RGB Matrix docs](https://docs.qmk.fm/features/rgb_matrix)
- [QMK Raw HID docs](https://docs.qmk.fm/features/rawhid)
- [QMK Elora V2 upstream PR #24790](https://github.com/qmk/qmk_firmware/pull/24790)
