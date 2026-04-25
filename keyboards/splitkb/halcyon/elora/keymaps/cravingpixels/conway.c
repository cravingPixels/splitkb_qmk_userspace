// conway.c — Conway's Game of Life for dsp3 mode
//
// 67×120 cell grid, 2px per cell → 134×240 px, toroidal wrapping.
// Keypresses inject an R-pentomino + scatter to keep life flowing.
// Stagnation detector auto-injects when evolution stalls.
//
// Render strategy: on the first tick draw everything (full clear + live cells);
// subsequent ticks delta-render only cells that changed state.

#ifdef HLC_TFT_DISPLAY

#include QMK_KEYBOARD_H
#include "users/halcyon_modules/splitkb/hlc_tft_display/hlc_tft_display.h"
#include <string.h>

// ── Grid dimensions (2 px/cell on 135×240 LCD) ──────────────────────────────
#define CW    67   // grid columns (CELL pixels wide each → 134 px covered)
#define CH   120   // grid rows    (CELL pixels tall each → 240 px covered)
#define CELL   2   // pixels per cell edge
#define CBYTES ((CW * CH + 7) / 8)

// ── HSV palette — Catppuccin Mocha (desaturated for display) ─────────────────
#define CW_H_LIVE  82
#define CW_S_LIVE  47
#define CW_V_LIVE 215
#define CW_H_DEAD 120
#define CW_S_DEAD  20
#define CW_V_DEAD  12

// ── State ─────────────────────────────────────────────────────────────────────
static uint8_t  grid[2][CBYTES];
static uint8_t  cur_gen     = 0;
static bool     initialized = false;
static bool     full_redraw = true;
static uint8_t  stale       = 0;
static uint16_t rng_state   = 0;

// ── PRNG ──────────────────────────────────────────────────────────────────────
static uint16_t cw_rand(void) {
    rng_state ^= rng_state << 7;
    rng_state ^= rng_state >> 9;
    rng_state ^= rng_state << 8;
    return rng_state;
}

// ── Bit-level cell access ─────────────────────────────────────────────────────
static inline bool cell_get(uint8_t g, uint8_t x, uint8_t y) {
    uint16_t idx = (uint16_t)y * CW + x;
    return (grid[g][idx >> 3] >> (idx & 7)) & 1;
}

static inline void cell_put(uint8_t g, uint8_t x, uint8_t y, bool v) {
    uint16_t idx = (uint16_t)y * CW + x;
    if (v) grid[g][idx >> 3] |=  (1u << (idx & 7));
    else   grid[g][idx >> 3] &= ~(1u << (idx & 7));
}

static uint8_t neighbors(uint8_t g, uint8_t x, uint8_t y) {
    uint8_t xl = (x == 0)     ? CW - 1 : x - 1;
    uint8_t xr = (x == CW-1) ? 0      : x + 1;
    uint8_t yu = (y == 0)     ? CH - 1 : y - 1;
    uint8_t yd = (y == CH-1) ? 0      : y + 1;
    return cell_get(g, xl, yu) + cell_get(g, x, yu) + cell_get(g, xr, yu)
         + cell_get(g, xl, y)                        + cell_get(g, xr, y)
         + cell_get(g, xl, yd) + cell_get(g, x, yd) + cell_get(g, xr, yd);
}

// ── Pattern injection ─────────────────────────────────────────────────────────
// R-pentomino centred at (cx, cy):  .XX / XX. / .X.
static void inject_pattern(uint8_t cx, uint8_t cy) {
    static const int8_t pts[5][2] = {{1,-1},{2,-1},{0,0},{1,0},{1,1}};
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t nx = (uint8_t)((cx + pts[i][0] + CW) % CW);
        uint8_t ny = (uint8_t)((cy + pts[i][1] + CH) % CH);
        cell_put(cur_gen, nx, ny, true);
    }
    for (uint8_t i = 0; i < 20; i++) {
        cell_put(cur_gen, (uint8_t)(cw_rand() % CW), (uint8_t)(cw_rand() % CH), true);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────
void conway_init(void) {
    rng_state = (uint16_t)(timer_read32() ^ 0xA5A5u);
    if (rng_state == 0) rng_state = 1;
    memset(grid, 0, sizeof(grid));
    cur_gen     = 0;
    full_redraw = true;
    stale       = 0;
    // seed ~35% initial density
    for (uint8_t y = 0; y < CH; y++)
        for (uint8_t x = 0; x < CW; x++)
            cell_put(0, x, y, (cw_rand() & 0xFF) < 89);
    initialized = true;
}

void conway_keypress(void) {
    if (!initialized) return;
    inject_pattern((uint8_t)(cw_rand() % CW), (uint8_t)(cw_rand() % CH));
}

void conway_tick(void) {
    if (!initialized) return;

    if (full_redraw) {
        qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, CW_H_DEAD, CW_S_DEAD, CW_V_DEAD, true);
        for (uint8_t y = 0; y < CH; y++)
            for (uint8_t x = 0; x < CW; x++)
                if (cell_get(cur_gen, x, y))
                    qp_rect(lcd_surface, x*CELL, y*CELL, x*CELL+CELL-1, y*CELL+CELL-1, CW_H_LIVE, CW_S_LIVE, CW_V_LIVE, true);
        full_redraw = false;
        return;
    }

    uint8_t  nxt     = 1 - cur_gen;
    uint16_t changes = 0;

    for (uint8_t y = 0; y < CH; y++) {
        for (uint8_t x = 0; x < CW; x++) {
            uint8_t n   = neighbors(cur_gen, x, y);
            bool    was = cell_get(cur_gen, x, y);
            bool    now = was ? (n == 2 || n == 3) : (n == 3);
            cell_put(nxt, x, y, now);
            if (now != was) {
                qp_rect(lcd_surface, x*CELL, y*CELL, x*CELL+CELL-1, y*CELL+CELL-1,
                    now ? CW_H_LIVE : CW_H_DEAD,
                    now ? CW_S_LIVE : CW_S_DEAD,
                    now ? CW_V_LIVE : CW_V_DEAD, true);
                changes++;
            }
        }
    }

    if (changes == 0) {
        stale++;
        if (stale >= 5) {
            inject_pattern((uint8_t)(cw_rand() % CW), (uint8_t)(cw_rand() % CH));
            stale = 0;
        }
    } else {
        stale = 0;
    }

    cur_gen = nxt;
}

void conway_cleanup(void) {
    memset(grid, 0, sizeof(grid));
    initialized = false;
    full_redraw  = true;
    stale        = 0;
}

#endif  // HLC_TFT_DISPLAY
