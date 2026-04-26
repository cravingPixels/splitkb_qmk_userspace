// stats_ui.c — tech/cyber stats display for dsp2 mode
// Draws into lcd_surface using QP primitives; HLC flushes to hardware each tick.
// All Y positions assume FiraCodeNerdFontMono-Regular-24 line height ≈ 27 px.

#ifdef HLC_TFT_DISPLAY

#include QMK_KEYBOARD_H
#include "users/halcyon_modules/splitkb/hlc_tft_display/hlc_tft_display.h"
#include "FiraCodeNerdFontMono-Regular-24.qff.h"
#include "sleeping_snorlax_120x96.qgf.h"

extern uint16_t g_tapping_term;
extern bool     rgb_animation_override;
extern int8_t   layer_hue_offset;

// ── Layout ────────────────────────────────────────────────────────────────────
//   y=0   ██ accent bar
//   y=5   L0 - BASE          one-line layer label (colour-matched)
//   y=35  ─── separator
//   y=39  [■] [■] [■]        lock indicators (CAP / NUM / SCR)
//   y=66  ─── separator
//   y=70  metric toast        blank when idle; flashes for 3 s on value change
//   y=100 metric bar          (or colour swatch for HUE toast)
//   y=112 ─── separator
//   y=116 bottom strip — cycles per dsp2 press:
//           0: WPM value
//           1: blank
//           2: shiny mew sleeping GIF (centred vertically in strip)
//           3: "You look ahead, we've got your back." marquee
//           4: random Italian wit marquee

#define UI_M         4
#define UI_BAR_H     8
#define UI_LOCK_SZ  24
#define UI_LOCK_GAP  9

#define UI_Y_ACCENT      0
#define UI_Y_LYR         5
#define UI_Y_SEP1       35
#define UI_Y_LOCKS      39
#define UI_Y_SEP2       66
#define UI_Y_METRIC_LBL 70
#define UI_Y_METRIC_BAR 100
#define UI_Y_SEP3       112
#define UI_Y_WPM_LBL    116

// 128 = 100 % brightness; display can show values above 100 % if val > 128.
#define UI_LED_MAX      128
#define LOCK_NOTIFY_MS 3000
#define TOAST_MS       3000

// ── Metric toast types ────────────────────────────────────────────────────────
typedef enum {
    TOAST_NONE = 0,
    TOAST_TT,    // tapping term (ms)
    TOAST_LED,   // LED brightness (% where 128 = 100 %)
    TOAST_SPD,   // animation speed (0-255)
    TOAST_HUE,   // anim hue (animation mode) or palette hue offset (static mode)
} toast_which_t;

// ── dsp2 bottom sub-modes ─────────────────────────────────────────────────────
#define DSP2_WPM      0
#define DSP2_BLANK    1
#define DSP2_GIF      2
#define DSP2_SENTENCE 3
#define DSP2_WIT      4
#define DSP2_COUNT    5

// ── GIF positioning ────────────────────────────────────────────────────────────
// Source: sleeping_snorlax.gif (1280×1024, 5:4) resized to 120×96 (exact same ratio).
// To regenerate: resize source GIF, then run from keymap dir:
//   qmk painter-convert-graphics -f pal16 -i sleeping_snorlax_120x96.gif -o .
#define GIF_WPM_W   120
#define GIF_WPM_H    96
#define GIF_WPM_X   ((LCD_WIDTH  - GIF_WPM_W) / 2)
// Centre the GIF vertically inside the bottom strip.
#define GIF_WPM_Y   (UI_Y_WPM_LBL + ((LCD_HEIGHT - UI_Y_WPM_LBL) - GIF_WPM_H) / 2)

// ── Marquee parameters ────────────────────────────────────────────────────────
#define MARQUEE_SPEED   3
#define MARQUEE_CW_EST 15

// ── Italian wit phrases ───────────────────────────────────────────────────────
static const char * const wit_phrases[] = {
    "Ho dovuto lavare il caffe'. Era macchiato.",
    "Ogni mattina mi alzo e dico 20 volte ciao. Dicono che sia salutare.",
    "Cosa sono il cubo e il cilindro per uno studente ignorante? I solidi ignoti.",
    "Hanno rubato un tir pieno di lampadine. La polizia brancola nel buio.",
    "Il deserto del Sahara e' in Africa. Su questo non ci piove.",
};
#define WIT_COUNT ((uint8_t)(sizeof(wit_phrases) / sizeof(wit_phrases[0])))

// ── Layer metadata ────────────────────────────────────────────────────────────
typedef struct { const char *name; uint8_t h, s, v; } lyr_t;
// Colors mirror rgb_layers.h Catppuccin Mocha palette (v boosted to 255 for display).
static const lyr_t lyr[] = {
    { "BASE",  164, 124, 255 },  // Lavender
    { "NUMFN",  28, 126, 255 },  // Yellow
    { "NAV",   134, 157, 255 },  // Sky
    { "SYM",    81, 124, 255 },  // Green
    { "META",  243, 160, 255 },  // Red
};
#define LYR_COUNT ((uint8_t)(sizeof(lyr) / sizeof(lyr[0])))

// ── State ─────────────────────────────────────────────────────────────────────
static painter_font_handle_t  ui_font         = NULL;
static uint8_t                dsp2_submode    = 0;

// GIF animation (sub-mode 2)
static painter_image_handle_t gif_wpm_handle  = NULL;
static deferred_token         gif_wpm_anim    = INVALID_DEFERRED_TOKEN;
static bool                   gif_wpm_running = false;

// Marquee (sub-modes 3 and 4)
static uint16_t marquee_px  = 0;
static int16_t  marquee_tw  = -1;
static uint8_t  marquee_cw  = MARQUEE_CW_EST;
static uint8_t  wit_idx     = 0;

// Lock notification
static bool     lock_notify_active = false;
static uint32_t lock_notify_start  = 0;

// Metric toast state
static toast_which_t toast_which = TOAST_NONE;
static uint32_t      toast_start = 0;
static toast_which_t toast_drawn = TOAST_NONE;  // what is currently on-screen

// Dirty-tracking previous values
static struct {
    uint8_t  layer;
    uint8_t  leds_raw;
    uint16_t tt;
    uint8_t  led_val;
    uint16_t wpm;
    uint8_t  speed;
    uint8_t  anim_hue;
    int8_t   hue_offset;
    bool     is_anim;
    bool     initialized;
} prev = { 0xFF, 0xFF, 0xFFFF, 0xFF, 0xFFFF, 0xFF, 0xFF, 0, false, false };

// ── GIF helpers ───────────────────────────────────────────────────────────────
static void gif_wpm_start(void) {
    if (gif_wpm_running) {
        qp_stop_animation(gif_wpm_anim);
        gif_wpm_running = false;
    }
    if (gif_wpm_handle) {
        qp_close_image(gif_wpm_handle);
        gif_wpm_handle = NULL;
    }
    gif_wpm_handle = qp_load_image_mem(gfx_sleeping_snorlax_120x96);
    if (gif_wpm_handle) {
        gif_wpm_anim    = qp_animate(lcd_surface, GIF_WPM_X, GIF_WPM_Y, gif_wpm_handle);
        gif_wpm_running = true;
    }
}

static void gif_wpm_stop(void) {
    if (gif_wpm_running) {
        qp_stop_animation(gif_wpm_anim);
        gif_wpm_running = false;
    }
    if (gif_wpm_handle) {
        qp_close_image(gif_wpm_handle);
        gif_wpm_handle = NULL;
    }
}

// ── Drawing primitives ────────────────────────────────────────────────────────
static void ui_hline(uint16_t y, uint8_t h, uint8_t s, uint8_t v) {
    qp_rect(lcd_surface, 0, y, LCD_WIDTH - 1, y, h, s, v, true);
}

static void ui_clear(uint16_t y0, uint16_t y1) {
    qp_rect(lcd_surface, 0, y0, LCD_WIDTH - 1, y1, 0, 0, 0, true);
}

static void ui_bar(uint16_t y, uint8_t pct, uint8_t h, uint8_t s, uint8_t v) {
    uint16_t w      = LCD_WIDTH - UI_M * 2;
    uint16_t filled = (uint16_t)pct * w / 100;
    if (filled > 0)
        qp_rect(lcd_surface, UI_M, y, UI_M + filled - 1, y + UI_BAR_H - 1, h, s, v, true);
    if (filled < w)
        qp_rect(lcd_surface, UI_M + filled, y, UI_M + w - 1, y + UI_BAR_H - 1, 0, 0, 18, true);
}

// ── Section renderers ─────────────────────────────────────────────────────────
static void draw_layer(uint8_t layer) {
    uint8_t       idx = layer < LYR_COUNT ? layer : LYR_COUNT - 1;
    const lyr_t  *m   = &lyr[idx];

    ui_clear(UI_Y_ACCENT, UI_Y_SEP1 - 1);
    qp_rect(lcd_surface, 0, UI_Y_ACCENT, LCD_WIDTH - 1, UI_Y_ACCENT + 1, m->h, m->s, m->v, true);

    char buf[16];
    snprintf(buf, sizeof(buf), "L%u %s", (unsigned)layer, m->name);
    qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_LYR, ui_font, buf, m->h, m->s, m->v, 0, 0, 0);

    ui_hline(UI_Y_SEP1, m->h, m->s, m->v / 5);
}

static void draw_lock_notification(uint8_t old_leds, uint8_t new_leds) {
    led_t old_l = { .raw = old_leds };
    led_t new_l = { .raw = new_leds };

    const char *name  = NULL;
    bool        is_on = false;
    uint8_t     h = 0, s = 0, v = 160;

    if (old_l.caps_lock != new_l.caps_lock) {
        name = "CAPS"; is_on = new_l.caps_lock;   h = 17;  s = 191; v = 245;
    } else if (old_l.num_lock != new_l.num_lock) {
        name = "NUM";  is_on = new_l.num_lock;    h = 142; s = 191; v = 245;
    } else if (old_l.scroll_lock != new_l.scroll_lock) {
        name = "SCR";  is_on = new_l.scroll_lock; h = 202; s = 191; v = 245;
    } else {
        return;
    }

    ui_clear(UI_Y_ACCENT, UI_Y_SEP1 - 1);
    qp_rect(lcd_surface, 0, UI_Y_ACCENT, LCD_WIDTH - 1, UI_Y_ACCENT + 1, h, s, v, true);

    char nbuf[16];
    snprintf(nbuf, sizeof(nbuf), "%s %s", name, is_on ? "ON" : "OFF");
    qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_LYR, ui_font, nbuf, h, s, v, 0, 0, 0);

    ui_hline(UI_Y_SEP1, h, s, v / 5);
}

static void draw_locks(uint8_t leds_raw) {
    led_t leds = { .raw = leds_raw };

    uint16_t total = 3 * UI_LOCK_SZ + 2 * UI_LOCK_GAP;
    uint16_t x0    = (LCD_WIDTH - total) / 2;
    uint16_t x1    = x0 + UI_LOCK_SZ + UI_LOCK_GAP;
    uint16_t x2    = x1 + UI_LOCK_SZ + UI_LOCK_GAP;
    uint16_t y1    = UI_Y_LOCKS + UI_LOCK_SZ - 1;

    leds.caps_lock
        ? qp_rect(lcd_surface, x0, UI_Y_LOCKS, x0 + UI_LOCK_SZ - 1, y1, HSV_CAPS_ON,    true)
        : qp_rect(lcd_surface, x0, UI_Y_LOCKS, x0 + UI_LOCK_SZ - 1, y1, HSV_CAPS_OFF,   true);
    leds.num_lock
        ? qp_rect(lcd_surface, x1, UI_Y_LOCKS, x1 + UI_LOCK_SZ - 1, y1, HSV_NUM_ON,     true)
        : qp_rect(lcd_surface, x1, UI_Y_LOCKS, x1 + UI_LOCK_SZ - 1, y1, HSV_NUM_OFF,    true);
    leds.scroll_lock
        ? qp_rect(lcd_surface, x2, UI_Y_LOCKS, x2 + UI_LOCK_SZ - 1, y1, HSV_SCROLL_ON,  true)
        : qp_rect(lcd_surface, x2, UI_Y_LOCKS, x2 + UI_LOCK_SZ - 1, y1, HSV_SCROLL_OFF, true);

    ui_hline(UI_Y_SEP2, 0, 0, 35);
}

// ── Metric toast ──────────────────────────────────────────────────────────────
// Clears the metric area, draws label + bar/swatch, redraws the separator.
static void draw_toast_content(toast_which_t which, uint8_t layer) {
    ui_clear(UI_Y_METRIC_LBL, UI_Y_SEP3 - 1);

    char buf[16];
    switch (which) {
        case TOAST_TT: {
            uint16_t tt = g_tapping_term;
            snprintf(buf, sizeof(buf), "TT  %3u", (unsigned)tt);
            qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_METRIC_LBL, ui_font, buf, 28, 126, 230, 0, 0, 0);
            uint8_t pct = (tt <= 100) ? 0 : (tt >= 300) ? 100 : (uint8_t)((tt - 100) * 100UL / 200);
            ui_bar(UI_Y_METRIC_BAR, pct, 28, 126, 200);
            break;
        }
        case TOAST_LED: {
            uint16_t pct = (uint16_t)rgb_matrix_get_val() * 100 / UI_LED_MAX;
            if (pct > 199) pct = 199;
            snprintf(buf, sizeof(buf), "LED %3u%%", (unsigned)pct);
            qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_METRIC_LBL, ui_font, buf, 120, 57, 220, 0, 0, 0);
            uint8_t bar_pct = (pct >= 100) ? 100 : (uint8_t)pct;
            ui_bar(UI_Y_METRIC_BAR, bar_pct, 120, 57, 200);
            break;
        }
        case TOAST_SPD: {
            uint8_t spd = rgb_matrix_config.speed;
            snprintf(buf, sizeof(buf), "SPD %3u", (unsigned)spd);
            qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_METRIC_LBL, ui_font, buf, 134, 157, 230, 0, 0, 0);
            ui_bar(UI_Y_METRIC_BAR, (uint8_t)((uint16_t)spd * 100 / 255), 134, 157, 200);
            break;
        }
        case TOAST_HUE: {
            uint8_t h, s;
            if (rgb_animation_override) {
                h = rgb_matrix_config.hsv.h;
                s = rgb_matrix_config.hsv.s > 80 ? rgb_matrix_config.hsv.s : 200;
                snprintf(buf, sizeof(buf), "HUE %3u", (unsigned)h);
            } else {
                // Static mode: show signed offset and current effective layer hue.
                uint8_t idx = layer < LYR_COUNT ? layer : LYR_COUNT - 1;
                h = (uint8_t)((int16_t)lyr[idx].h + layer_hue_offset);
                s = 200;
                int8_t off = layer_hue_offset;
                if (off >= 0) {
                    snprintf(buf, sizeof(buf), "OFS +%2u", (unsigned)off);
                } else {
                    snprintf(buf, sizeof(buf), "OFS -%2u", (unsigned)(-off));
                }
            }
            // Label painted in the effective hue so the colour change is instantly visible.
            qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_METRIC_LBL, ui_font, buf, h, 200, 230, 0, 0, 0);
            // Swatch: full-width rect in the effective colour.
            uint16_t w = LCD_WIDTH - UI_M * 2;
            qp_rect(lcd_surface, UI_M, UI_Y_METRIC_BAR, UI_M + w - 1, UI_Y_METRIC_BAR + UI_BAR_H - 1, h, s, 200, true);
            break;
        }
        default:
            break;
    }
    ui_hline(UI_Y_SEP3, 0, 0, 35);
}

static void draw_wpm(uint16_t wpm) {
    ui_clear(UI_Y_WPM_LBL, LCD_HEIGHT - 1);
    char buf[16];
    snprintf(buf, sizeof(buf), "WPM %3u", (unsigned)wpm);
    qp_drawtext_recolor(lcd_surface, UI_M, UI_Y_WPM_LBL, ui_font, buf, 154, 75, 235, 0, 0, 0);
}

// ── Marquee renderer ──────────────────────────────────────────────────────────
static void draw_marquee_tick(const char *text, uint8_t fg_h, uint8_t fg_s, uint8_t fg_v) {
    if (marquee_tw < 0) {
        marquee_tw = (int16_t)qp_textwidth(ui_font, text);
        uint8_t len = (uint8_t)strlen(text);
        marquee_cw  = (len > 0 && marquee_tw > 0)
                      ? (uint8_t)((uint16_t)marquee_tw / len)
                      : MARQUEE_CW_EST;
        if (marquee_cw == 0) marquee_cw = MARQUEE_CW_EST;
    }

    ui_clear(UI_Y_WPM_LBL, LCD_HEIGHT - 1);

    int16_t draw_x = (int16_t)LCD_WIDTH - (int16_t)marquee_px;

    if (draw_x >= 0) {
        qp_drawtext_recolor(lcd_surface, (uint16_t)draw_x, UI_Y_WPM_LBL, ui_font, text, fg_h, fg_s, fg_v, 0, 0, 0);
    } else {
        uint16_t off  = (uint16_t)(-draw_x);
        uint16_t skip = off / marquee_cw;
        const char *p = text;
        for (uint16_t i = 0; i < skip && *p; i++) p++;

        if (*p) {
            int16_t sub_x = (int16_t)(skip * (uint16_t)marquee_cw) - (int16_t)off;
            if (sub_x < 0) sub_x = 0;
            qp_drawtext_recolor(lcd_surface, (uint16_t)sub_x, UI_Y_WPM_LBL, ui_font, p, fg_h, fg_s, fg_v, 0, 0, 0);
        }
    }

    marquee_px += MARQUEE_SPEED;

    if (draw_x + marquee_tw < 0) {
        marquee_px = 0;
        marquee_tw = -1;
        if (dsp2_submode == DSP2_WIT) {
            wit_idx = (wit_idx + 1 + (uint8_t)((timer_read32() >> 2) & 3)) % WIT_COUNT;
        }
    }
}

// ── Public API ────────────────────────────────────────────────────────────────
void stats_ui_show_next_metric(void) {
    // Cycle: NONE → TT → LED → SPD → HUE → NONE → …
    switch (toast_which) {
        case TOAST_NONE: toast_which = TOAST_TT;  break;
        case TOAST_TT:   toast_which = TOAST_LED; break;
        case TOAST_LED:  toast_which = TOAST_SPD; break;
        case TOAST_SPD:  toast_which = TOAST_HUE; break;
        default:         toast_which = TOAST_NONE; break;
    }
    toast_start = timer_read32();
}

void stats_ui_cycle_bottom(void) {
    uint8_t old = dsp2_submode;
    dsp2_submode = (dsp2_submode + 1) % DSP2_COUNT;

    if (old == DSP2_GIF) gif_wpm_stop();

    ui_clear(UI_Y_WPM_LBL, LCD_HEIGHT - 1);

    switch (dsp2_submode) {
        case DSP2_WPM:
            prev.wpm = 0xFFFF;
            break;
        case DSP2_GIF:
            gif_wpm_start();
            break;
        case DSP2_SENTENCE:
            marquee_px = 0;
            marquee_tw = -1;
            break;
        case DSP2_WIT:
            marquee_px = 0;
            marquee_tw = -1;
            wit_idx    = (uint8_t)((timer_read32() >> 3) % WIT_COUNT);
            break;
        default:
            break;
    }
}

void stats_ui_invalidate(void) {
    gif_wpm_stop();
    dsp2_submode       = 0;
    marquee_px         = 0;
    marquee_tw         = -1;
    prev.layer         = 0xFF;
    prev.leds_raw      = 0xFF;
    prev.tt            = 0xFFFF;
    prev.led_val       = 0xFF;
    prev.wpm           = 0xFFFF;
    prev.speed         = 0xFF;
    prev.anim_hue      = 0xFF;
    prev.hue_offset    = 0;
    prev.is_anim       = false;
    prev.initialized   = false;
    toast_which        = TOAST_NONE;
    toast_drawn        = TOAST_NONE;
    lock_notify_active = false;
}

void stats_ui_invalidate_gif(void) {
    stats_ui_invalidate();
    dsp2_submode = DSP2_GIF;
    gif_wpm_start();
}

void stats_ui_cleanup(void) {
    gif_wpm_stop();
    if (ui_font) {
        qp_close_font(ui_font);
        ui_font = NULL;
    }
    dsp2_submode       = 0;
    marquee_px         = 0;
    marquee_tw         = -1;
    toast_which        = TOAST_NONE;
    toast_drawn        = TOAST_NONE;
    prev.initialized   = false;
    lock_notify_active = false;
}

void stats_ui_draw(void) {
    if (!ui_font) {
        ui_font = qp_load_font_mem(font_FiraCodeNerdFontMono_Regular_24);
        if (!ui_font) return;
    }

    uint8_t  layer    = get_highest_layer(layer_state | default_layer_state);
    uint8_t  leds_raw = host_keyboard_led_state().raw;
    uint16_t wpm      = get_current_wpm();
    bool     is_anim  = rgb_animation_override;

    // ── First draw: clear screen, draw all sections, seed prev ───────────────
    if (!prev.initialized) {
        qp_rect(lcd_surface, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, 0, 0, 0, true);
        prev.initialized = true;
        draw_layer(layer);    prev.layer    = layer;
        draw_locks(leds_raw); prev.leds_raw = leds_raw;
        ui_hline(UI_Y_SEP3, 0, 0, 35);  // metric area separator (blank initially)
        toast_drawn = TOAST_NONE;
        // Seed prev without triggering any toast.
        prev.tt         = g_tapping_term;
        prev.led_val    = rgb_matrix_get_val();
        prev.speed      = rgb_matrix_config.speed;
        prev.anim_hue   = rgb_matrix_config.hsv.h;
        prev.hue_offset = layer_hue_offset;
        prev.is_anim    = is_anim;
        if (dsp2_submode == DSP2_WPM) { draw_wpm(wpm); prev.wpm = wpm; }
        return;
    }

    // ── Layer section ─────────────────────────────────────────────────────────
    bool notify_active = lock_notify_active && (timer_elapsed32(lock_notify_start) < LOCK_NOTIFY_MS);

    if (notify_active && layer != prev.layer) {
        lock_notify_active = false;
        notify_active      = false;
    }
    if (lock_notify_active && !notify_active) {
        lock_notify_active = false;
        draw_layer(layer);
        prev.layer = layer;
    } else if (!notify_active && layer != prev.layer) {
        draw_layer(layer);
        prev.layer = layer;
    }

    // ── Lock indicators ───────────────────────────────────────────────────────
    if (leds_raw != prev.leds_raw) {
        draw_lock_notification(prev.leds_raw, leds_raw);
        lock_notify_active = true;
        lock_notify_start  = timer_read32();
        draw_locks(leds_raw);
        prev.leds_raw = leds_raw;
    }

    // ── Metric toast ──────────────────────────────────────────────────────────
    bool toast_dirty = false;

    // Expiry check
    if (toast_which != TOAST_NONE && timer_elapsed32(toast_start) >= TOAST_MS) {
        toast_which = TOAST_NONE;
        toast_dirty = true;
    }

    // Animation-mode transition: reset hue tracking to avoid false triggers.
    if (is_anim != prev.is_anim) {
        prev.anim_hue   = rgb_matrix_config.hsv.h;
        prev.hue_offset = layer_hue_offset;
        prev.is_anim    = is_anim;
        if (toast_which == TOAST_HUE) { toast_which = TOAST_NONE; toast_dirty = true; }
    }

    // Auto-show on value change (most recent change wins when several happen at once).
    uint16_t tt      = g_tapping_term;
    uint8_t  led_val = rgb_matrix_get_val();
    uint8_t  speed   = rgb_matrix_config.speed;

    if (tt != prev.tt) {
        prev.tt = tt;
        toast_which = TOAST_TT; toast_start = timer_read32(); toast_dirty = true;
    }
    if (led_val != prev.led_val) {
        prev.led_val = led_val;
        toast_which = TOAST_LED; toast_start = timer_read32(); toast_dirty = true;
    }
    if (speed != prev.speed) {
        prev.speed = speed;
        toast_which = TOAST_SPD; toast_start = timer_read32(); toast_dirty = true;
    }
    if (is_anim) {
        uint8_t cur_hue = rgb_matrix_config.hsv.h;
        if (cur_hue != prev.anim_hue) {
            prev.anim_hue = cur_hue;
            toast_which = TOAST_HUE; toast_start = timer_read32(); toast_dirty = true;
        }
    } else {
        if (layer_hue_offset != prev.hue_offset) {
            prev.hue_offset = layer_hue_offset;
            toast_which = TOAST_HUE; toast_start = timer_read32(); toast_dirty = true;
        }
    }

    // Draw or clear metric area when content changed.
    if (toast_dirty || toast_which != toast_drawn) {
        if (toast_which == TOAST_NONE) {
            ui_clear(UI_Y_METRIC_LBL, UI_Y_SEP3 - 1);
            ui_hline(UI_Y_SEP3, 0, 0, 35);
        } else {
            draw_toast_content(toast_which, layer);
        }
        toast_drawn = toast_which;
    }

    // ── Bottom strip ──────────────────────────────────────────────────────────
    switch (dsp2_submode) {
        case DSP2_WPM:
            if (wpm != prev.wpm) { draw_wpm(wpm); prev.wpm = wpm; }
            break;
        case DSP2_SENTENCE:
            draw_marquee_tick("You look ahead, we've got your back.", 154, 75, 235);
            break;
        case DSP2_WIT:
            draw_marquee_tick(wit_phrases[wit_idx], 189, 55, 230);
            break;
        // DSP2_BLANK: nothing drawn
        // DSP2_GIF: qp_animate is autonomous, don't touch the strip
        default:
            break;
    }
}

#endif  // HLC_TFT_DISPLAY
