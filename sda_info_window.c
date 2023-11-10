#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "SDA_OS/SDA_OS.h"
#include "sda-image.h"

extern uint8_t fb_used;
extern uint8_t info_window_enabled;
extern svsVM svm;

gr2context c_info;
gr2Element info_elements[40];
gr2Screen info_screens[10];
uint8_t info_window_init;

void info_window_reset() {
  info_window_init = 0;
}

/* Roadmap: reimplement this
  printf("SVS vm %s, (%lu): \n", s->vmName, (uint32_t) s);
  printf("    fname:           %s\n", s->fName);
  printf("Functions:\n");
  printf("    funcTableLen:    %u of %u\n", s->funcTableLen, FUNCTION_TABLE_L);
  printf("    syscallTableLen: %u of %u\n", s->syscallTableLen, SYSCALL_TABLE_L);
  printf("Variables & Arrays:\n");
  printf("    varTableLen:     %u of %u\n", s->varTableLen, VAR_TABLE_L);
  printf("    varArrayLen:     %u of %u\n", s->varArrayLen, SVS_ARRAY_LEN);
  printf("Strings:\n");
  printf("    stringFieldLen:  %u of %u\n", s->stringFieldLen, s->stringFieldMax);
  printf("    stringConstMax:  %u\n", s->stringConstMax);
  printf("    gcSafePoint:     %u\n", s->gcSafePoint);
  printf("Caching:\n");
  printf("    vmCacheUsed:     %u\n", s->vmCacheUsed);
  printf("    cacheStart:      %u\n", s->cacheStart);
  printf("    tokenMax:        %u\n", s->tokenMax);
*/

uint8_t info_window_loop(uint8_t touch, uint32_t mouse_x, uint32_t mouse_y) {
  static uint16_t scr;
  static uint8_t touchPrev;
  gr2EventType touchType = EV_NONE;
  uint8_t touchValid;
  static uint16_t string_bar;
  static uint16_t app_name;
  static uint16_t reset_button;
  
  if (info_window_init == 0) {
    gr2_init_context(&c_info, info_elements, 40, info_screens, 10);
    gr2_reset_context(&c_info);
    scr = gr2_add_screen(&c_info);
    gr2_set_relative_init(1, &c_info);

    gr2_add_text(1, 0, 5, 1, "Sim Info", scr, &c_info);

    app_name = gr2_add_text(1, 1, 10, 1, "", scr, &c_info);

    gr2_add_text(1, 2, 5, 1, "String mem:", scr, &c_info);
    string_bar = gr2_add_progbar_v(5, 2, 5, 1, svm.stringFieldMax, svm.stringFieldLen, scr, &c_info);

    reset_button = gr2_add_button(1, 4, 5, 1, "Reload app", scr, &c_info);
    info_window_init = 1;
  }

  // touch handling
  if ((touch == 1) && (touchPrev == 0)) {
    touchType = EV_PRESSED;
    touchValid = 1;
  }

  if ((touch == 0) && (touchPrev == 1)) {
    touchType = EV_RELEASED;
    touchValid = 1;
  }
  if ((touch == 1) && (touchPrev == 1)) {
    touchType = EV_HOLD;
    touchValid = 1;
  }
  if ((touch == 0) && (touchPrev == 0)) {
    touchType = EV_NONE;
    touchValid = 0;
  }

  touchPrev = touch;

  if(touchValid) {
    gr2_touch_input(0, 0, INFO_WIDTH, INFO_HEIGHT, mouse_x, mouse_y, touchType, scr, &c_info);
  }
  
  // redraw
  fb_used = INFO_FB;
  LCD_drawArea a;
  uint16_t x_size = LCD_get_x_size();
  uint16_t y_size = LCD_get_y_size();
  lcdOrientationType orientation = LCD_get_orientation();
  LCD_set_x_size(INFO_WIDTH);
  LCD_set_y_size(INFO_HEIGHT);
  LCD_set_orientation(OR_NORMAL);

  if (svmGetValid()) {
    gr2_set_value(string_bar, svm.stringFieldLen, &c_info);
    gr2_set_str(app_name, svmGetName(), &c_info);
  }

  if(gr2_get_event(reset_button, &c_info) == EV_RELEASED) {
    uint8_t namebuff[258];
    sda_strcp(svmGetName(), namebuff, sizeof(namebuff) - 1);
    svmCloseRunning();
    svmLaunch(namebuff, 0);
  }
  gr2_clear_event(reset_button, &c_info);

  LCD_getDrawArea(&a);
  LCD_setDrawArea(0, 0, INFO_WIDTH, INFO_HEIGHT);
  gr2_draw_screen(0, 0, INFO_WIDTH, INFO_HEIGHT, scr, 0, &c_info);

  gr2_draw_end(&c_info);
  LCD_setDrawAreaS(&a);

  LCD_set_x_size(x_size);
  LCD_set_y_size(y_size);
  LCD_set_orientation(orientation);
  fb_used = MAIN_FB;

}