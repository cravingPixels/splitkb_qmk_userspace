# Halcyon Elora V2 — QMK Keymap

Custom QMK firmware for the [splitkb Halcyon Elora V2](https://splitkb.com/products/halcyon-elora) split keyboard.

**Keymap:** `cravingpixels` under `keyboards/splitkb/halcyon/elora/keymaps/cravingpixels/`

---

## Hardware

**MCU / Flash**
- RP2040 MCU
- 16 MB flash (AT25SF128A, 128 Mbit)

**Display — left half**
- ST7789 TFT LCD, 135×240 px, 1.14", 262k colors
- LVGL-compatible via QMK Quantum Painter
- Display modes (switchable at runtime from the SYSTEM layer):
  - `dsp1` — stock HLC widget: current layer number + Caps/Num/Scroll lock
  - `dsp4` — animated GIF carousel
  - Conway's Game of Life (background animation, always on)

**RGB / LEDs**
- Per-key RGB: SK6812MINI-E (one per switch, per half)
- Underglow: 6× WS2812B per half
- Per-layer static colors (Catppuccin Mocha palette)

**Encoder — right half**
- ALPS EC12 rotary encoder with push-button action
- Rotate: volume / page scroll (layer-dependent)
- Press: mapped to `KC_MUTE` on BASE layer (configurable per layer)

---

## First-time setup

```bash
mise run setup
```

This installs the ARM compiler, QMK CLI, sets up the QMK firmware source, and registers
this directory as your QMK userspace.

---

## Daily use

```bash
mise run compile          # both halves
mise run compile-left     # left only  (TFT display)
mise run compile-right    # right only (encoder)
mise run flash-left       # compile + flash left  — double-tap reset first
mise run flash-right      # compile + flash right — double-tap reset first
mise run convert          # PNG/GIF → QGF assets
mise run flash-stats      # show firmware flash usage per side
```

Compiled `.uf2` files land in `~/.local/share/qmk_firmware/.build/`.
Preferred flashing method: drag-drop the `.uf2` onto the `RPI-RP2` drive.

---

## Layers

| # | Name    | Access              | Color (Catppuccin Mocha) |
|---|---------|---------------------|--------------------------|
| 0 | BASE    | always              | Lavender                 |
| 1 | NUMFN   | hold Space          | Yellow                   |
| 2 | RGBNAV  | hold Enter          | Sky                      |
| 3 | MODNAV  | hold ' or MO(3)     | Mauve                    |
| 4 | SYM     | hold MO(4)          | Green                    |
| 5 | FN      | hold MO(5)          | Peach                    |
| 6 | SYSTEM  | hold MO(6)          | Red                      |

---

## Branch strategy

This repo is a fork of [splitkb/qmk_userspace](https://github.com/splitkb/qmk_userspace).

- **`halcyon`** is the only working branch — based on `upstream/halcyon` (splitkb's halcyon-modules branch) with all keymap work on top.
- When splitkb publishes updates: `git fetch upstream && git rebase upstream/halcyon`, then `git push origin halcyon --force-with-lease`.
- There is no `main` branch. Do not create one.

---

## Flashing notes

- Both halves must be flashed separately — each has its own RP2040
- **Bootloader**: double-tap reset button → `RPI-RP2` USB drive appears
- **Emergency bootloader**: hold top-left key while plugging in USB
- **Recovery**: RP2040 is very hard to brick — re-enter bootloader and reflash
- **Stock firmware / rollback**: https://splitkb.com/fw
