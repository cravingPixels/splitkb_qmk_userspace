# Halcyon Elora V2 — QMK Keymap

Custom QMK firmware for the [splitkb Halcyon Elora V2](https://splitkb.com/products/halcyon-elora) split keyboard.

- RP2040 MCU, per-key RGB + underglow
- 135×240 ST7789 TFT LCD — left half
- Rotary encoder — right half
- Keymap: `cravingpixels` under `keyboards/splitkb/halcyon/elora/keymaps/cravingpixels/`

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
```

Compiled `.uf2` files land in `~/.local/share/qmk_firmware/`.
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

## Flashing notes

- Both halves must be flashed separately — each has its own RP2040
- **Bootloader**: double-tap reset button → `RPI-RP2` USB drive appears
- **Emergency bootloader**: hold top-left key while plugging in USB
- **Recovery**: RP2040 is very hard to brick — re-enter bootloader and reflash
- **Stock firmware / rollback**: https://splitkb.com/fw
