// usb_recover.c — re-attach to the USB bus when the link died under a running firmware.
//
// The RP2040 cannot tell a dead data link from a sleeping host: both are "bus idle",
// both put the USB driver into USB_SUSPENDED, and QMK then sits in its suspend loop
// (tmk_core/protocol/chibios/chibios.c, protocol_pre_task) waiting for a resume that
// never comes when the host has already dropped the device. Until now the only way
// out was a replug.
//
// Inside that loop QMK already scans the matrix every ~17 ms for a wake key and sends
// a remote-wakeup request. A sleeping host answers that within a second. If a key is
// still held after USB_RECOVER_HOLD_MS with the driver still suspended, the host is
// not coming back on its own: drop off the bus, re-initialise the driver and
// re-attach, which makes the host enumerate the keyboard again as if replugged.
// Then run the normal wake-up path so LEDs and LCD come back.
//
// Zero cost while the keyboard is active: this code is only reached from the suspend loop.

#include QMK_KEYBOARD_H
#include "usb_recover.h"
#include "usb_main.h"
#include "suspend.h"

#define USB_RECOVER_HOLD_MS 3000

static bool     key_held   = false;
static uint32_t held_since = 0;

void usb_recover_tick(void) {
    if (!suspend_wakeup_condition()) {
        key_held = false;
        return;
    }
    if (!key_held) {
        key_held   = true;
        held_since = timer_read32();
        return;
    }
    if (timer_elapsed32(held_since) < USB_RECOVER_HOLD_MS) return;

    key_held = false;
    restart_usb_driver(&USB_DRIVER);
    suspend_wakeup_init();
}
