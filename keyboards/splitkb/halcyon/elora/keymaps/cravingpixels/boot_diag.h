#pragma once

#ifdef HLC_TFT_DISPLAY

#include <stddef.h>

void boot_diag_init(void);                      // keyboard_post_init_user
void boot_diag_task(void);                      // housekeeping, master only
void boot_diag_wakeup(void);                    // suspend_wakeup_init_user
void boot_diag_label(char *buf, size_t len);    // e.g. "POR 11mz", see boot_diag.c

#endif  // HLC_TFT_DISPLAY
