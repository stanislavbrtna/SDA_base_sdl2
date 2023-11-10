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

#define INFO_WIDTH 600
#define INFO_HEIGHT 400

#define INFO_FB 1
#define MAIN_FB 0

#define SIM_X 67
#define SIM_Y 58

uint8_t info_window_loop(uint8_t touch, uint32_t mouse_x, uint32_t mouse_y);
void info_window_reset();
#endif
