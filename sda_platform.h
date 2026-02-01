#ifndef SDA_PLATFORM_H
#define SDA_PLATFORM_H

#define SDA_BASE_PIN_IN 0
#define SDA_BASE_PIN_OUT 1
#define SDA_BASE_PIN_ALT 2

#define SDA_BASE_PIN_NOPULL 0
#define SDA_BASE_PIN_PULLDOWN 1
#define SDA_BASE_PIN_PULLUP 1

#include <stdio.h>
#include <stdint.h>
#include "sda_fs_pc.h"
#include "sda_media_utils.h"

#define INFO_WIDTH 600
#define INFO_HEIGHT 400

#define INFO_FB 1
#define MAIN_FB 0

#define SIM_X 67
#define SIM_Y 58

uint8_t info_window_loop(uint8_t touch, uint32_t mouse_x, uint32_t mouse_y);
void info_window_reset();

// Featureset

#define SDA_FEATURE_NOTIF_VIBRO
#define SDA_FEATURE_AMB_LIGHT_SENS
#define SDA_FEATURE_IEXP_V2
#define SDA_FEATURE_PCM_SOUND
#define SDA_FEATURE_EEXP_V1
#define SDA_FEATURE_FREQ_SCALING

#define MIN_PCM_VOLUME_VALUE 0
#define MAX_PCM_VOLUME_VALUE 128

#endif
