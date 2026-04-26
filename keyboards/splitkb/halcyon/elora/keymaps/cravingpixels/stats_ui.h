#pragma once

#ifdef HLC_TFT_DISPLAY

void stats_ui_draw(void);
void stats_ui_invalidate(void);
void stats_ui_invalidate_gif(void);  // invalidate and start in GIF sub-mode
void stats_ui_cleanup(void);
void stats_ui_cycle_bottom(void);       // cycle dsp2 bottom strip sub-mode
void stats_ui_show_next_metric(void);   // manually cycle the metric toast (TT→LED→SPD→HUE→none)

#endif  // HLC_TFT_DISPLAY
