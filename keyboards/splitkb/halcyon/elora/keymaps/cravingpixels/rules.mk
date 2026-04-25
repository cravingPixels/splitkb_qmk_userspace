USER_NAME := halcyon_modules
DEFERRED_EXEC_ENABLE = yes

QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += st7789_spi
RGB_MATRIX_ENABLE = yes
ENCODER_MAP_ENABLE = yes
RAW_ENABLE = yes
WPM_ENABLE = yes
OS_DETECTION_ENABLE = yes

ifdef HLC_TFT_DISPLAY
SRC += gif_wpm.qgf.c
SRC += FiraCodeNerdFontMono-Regular-24.qff.c
SRC += stats_ui.c
SRC += conway.c
endif

