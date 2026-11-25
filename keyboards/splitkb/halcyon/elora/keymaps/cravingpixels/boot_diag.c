// boot_diag.c — why did this half boot, and how did the previous session end?
//
// The keyboard sometimes drops off USB with the LCD dark and needs a replug. The
// macOS log cannot tell a power loss on the keyboard side from a firmware hang, so
// the firmware records what it can see:
//   - the cause of this reset, from the RP2040 CHIP_RESET register
//       POR  power-on or brown-out       RUN  reset pin
//       PSM  power-manager / debug restart   SYS  none of the above (software reset)
//   - the previous session's uptime in minutes, written once a minute
//   - whether the previous session was sitting in USB suspend when it ended
//
// Readout on the stats page (KC_SHOW_METRICS cycle, and once for 3 s after boot):
//   "POR  --"     first boot after an EEPROM reset, nothing recorded yet
//   "POR 0m"      previous session lasted under a minute: the chip is reboot-looping
//   "POR 12m"     previous session ran 12 min and ended without a USB suspend:
//                 power vanished (cable, socket, host port)
//   "POR 12mz"    previous session ended while in USB suspend: the firmware was alive
//                 but the USB link had gone, or the host was asleep
//
// Storage is the 32-bit user EEPROM word (nothing else in this keymap uses it).

#ifdef HLC_TFT_DISPLAY

#include QMK_KEYBOARD_H
#include <stdio.h>
#include "boot_diag.h"
#include "hardware/structs/vreg_and_chip_reset.h"

#define DIAG_MAGIC      0xB0000000UL  // top byte: marks a word written by this code
#define DIAG_MAGIC_MASK 0xFF000000UL
#define DIAG_SUSPENDED  (1UL << 16)
#define DIAG_MINUTES    0xFFFFUL
#define DIAG_PERIOD_MS  60000UL

static const char *reason        = "???";
static bool        prev_valid    = false;
static uint16_t    prev_minutes  = 0;
static bool        prev_suspend  = false;
static bool        suspended     = false;
static bool        write_pending = false;

static const char *reset_reason(void) {
    uint32_t r = vreg_and_chip_reset_hw->chip_reset;
    if (r & VREG_AND_CHIP_RESET_CHIP_RESET_HAD_POR_BITS) return "POR";
    if (r & VREG_AND_CHIP_RESET_CHIP_RESET_HAD_RUN_BITS) return "RUN";
    if (r & VREG_AND_CHIP_RESET_CHIP_RESET_HAD_PSM_RESTART_BITS) return "PSM";
    return "SYS";
}

static void write_session(void) {
    uint32_t minutes = timer_read32() / DIAG_PERIOD_MS;
    if (minutes > DIAG_MINUTES) minutes = DIAG_MINUTES;
    uint32_t word = DIAG_MAGIC | minutes | (suspended ? DIAG_SUSPENDED : 0);
    eeconfig_update_user(word);  // writes only when the value changed
}

void boot_diag_init(void) {
    reason = reset_reason();

    uint32_t word = eeconfig_read_user();
    prev_valid    = (word & DIAG_MAGIC_MASK) == DIAG_MAGIC;
    prev_minutes  = word & DIAG_MINUTES;
    prev_suspend  = word & DIAG_SUSPENDED;

    write_session();
}

void boot_diag_task(void) {
    static uint32_t last_write = 0;
    if (!write_pending && timer_elapsed32(last_write) < DIAG_PERIOD_MS) return;
    write_pending = false;
    last_write    = timer_read32();
    write_session();
}

// Runs inside QMK's suspend loop every ~17 ms while the USB driver reports
// suspended. The flash write happens once, on entry.
void suspend_power_down_user(void) {
    if (suspended) return;
    suspended = true;
    write_session();
}

// Flag only: the wakeup hook may run from the USB event path, the write happens
// in boot_diag_task.
void boot_diag_wakeup(void) {
    suspended     = false;
    write_pending = true;
}

void boot_diag_label(char *buf, size_t len) {
    if (!prev_valid) {
        snprintf(buf, len, "%s  --", reason);
        return;
    }
    const char *flag = prev_suspend ? "z" : "";  // 8 chars max fit the 135 px row
    if (prev_minutes < 100) {
        snprintf(buf, len, "%s %um%s", reason, (unsigned)prev_minutes, flag);
    } else {
        unsigned hours = prev_minutes / 60;
        if (hours > 99) hours = 99;
        snprintf(buf, len, "%s %uh%s", reason, hours, flag);
    }
}

#endif  // HLC_TFT_DISPLAY
