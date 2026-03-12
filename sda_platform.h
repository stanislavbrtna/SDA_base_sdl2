#ifndef SDA_PLATFORM_H
#define SDA_PLATFORM_H

#include <stdio.h>
#include <stdint.h>
#include "sda_fs_pc.h"
#include "sda_media_utils.h"

uint8_t info_window_loop(uint8_t touch, uint32_t mouse_x, uint32_t mouse_y);
void info_window_reset();

// Featureset

#define SDA_FEATURE_NOTIF_VIBRO
#define SDA_FEATURE_AMB_LIGHT_SENS
#define SDA_FEATURE_IEXP_V2
#define SDA_FEATURE_PCM_SOUND
#define SDA_FEATURE_EEXP_V1
#define SDA_FEATURE_FREQ_SCALING

#endif
