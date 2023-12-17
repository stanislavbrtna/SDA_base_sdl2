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

extern uint8_t sda_sim_serial_trigger;

void sda_sim_set_pin_dbg(uint8_t val);

uint8_t info_window_loop(uint8_t touch, uint32_t mouse_x, uint32_t mouse_y) {
  static uint16_t scr;
  static uint8_t touchPrev;
  gr2EventType touchType = EV_NONE;
  uint8_t touchValid;

  // gr2 screen elements
  static uint16_t string_bar;
  static uint16_t app_name;
  static uint16_t reset_button;
  static uint16_t collect_button;
  static uint16_t lock_txt;

  static uint16_t file_conf_txt;
  static uint16_t file_csv_txt;

  static uint16_t file_cwd_txt;
  static uint8_t  cwd_buff[156];

  static uint16_t elements_bar;

  static uint16_t trigger_btn;
  static uint16_t pindbg_btn;
  
  if (info_window_init == 0) {
    gr2_init_context(&c_info, info_elements, 40, info_screens, 10);
    gr2_reset_context(&c_info);

    gr2_set_border_color(gr2_get_border_color(&sda_sys_con), &c_info);
    gr2_set_text_color(gr2_get_text_color(&sda_sys_con), &c_info);
    gr2_set_background_color(gr2_get_background_color(&sda_sys_con), &c_info);
    gr2_set_fill_color(gr2_get_fill_color(&sda_sys_con), &c_info);
    gr2_set_active_color(gr2_get_active_color(&sda_sys_con), &c_info);

    scr = gr2_add_screen(&c_info);
    gr2_set_relative_init(1, &c_info);

    uint16_t line = 0;

    gr2_add_text(1, line, 5, 1, "Sim Info", scr, &c_info);

    line++;

    app_name = gr2_add_text(1, line, 15, 1, "", scr, &c_info);

    line++;

    gr2_add_text(1, line, 5, 1, "String mem:", scr, &c_info);
    string_bar = gr2_add_progbar_v(6, line, 5, 1, svm.stringFieldMax, svm.stringFieldLen, scr, &c_info);

    collect_button = gr2_add_button(12, 2, 5, 1, "GarbageCollect", scr, &c_info);

    line++;

    gr2_add_text(1, line, 6, 1, "GR2 elements:", scr, &c_info);
    elements_bar = gr2_add_progbar_v(6, line, 5, 1, sda_app_con.elementsMax, sda_app_con.elementsUsed, scr, &c_info);

    lock_txt = gr2_add_text(12, line, 6, 1, "N/A", scr, &c_info);

    line++;
    line++;

    reset_button = gr2_add_button(1, line, 5, 1, "Reload app", scr, &c_info);
    trigger_btn  = gr2_add_button(8, line, 4, 1, "Uart trigger", scr, &c_info);
    pindbg_btn   = gr2_add_checkbox(13, line, 5, 1, "I/O Debug", scr, &c_info);

    line +=2;

    gr2_add_text(1, line, 6, 1, "Openned files:", scr, &c_info);
    gr2_add_text(8, line, 6, 1, "Working directory:", scr, &c_info);
    line++;
    file_conf_txt = gr2_add_text(1, line, 10, 1, "none", scr, &c_info);
    file_cwd_txt  = gr2_add_text(8, line, 10, 1, "n/a", scr, &c_info);
    line++;
    file_csv_txt = gr2_add_text(1, line, 7, 1, "none", scr, &c_info); line++;

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
    gr2_set_value(elements_bar, sda_app_con.elementsUsed, &c_info);
    gr2_set_str(app_name, svmGetName(), &c_info);
    if(svp_crypto_get_lock()) {
      gr2_set_str(lock_txt, "Crypto unlocked", &c_info);
    } else {
      gr2_set_str(lock_txt, "Crypto locked", &c_info);
    }

    if (sda_get_conf_fname()) {
      gr2_set_str(file_conf_txt, sda_get_conf_fname(), &c_info);
    } else {
      gr2_set_str(file_conf_txt, "none", &c_info);
    }

    if (sda_get_csv_fname()) {
      gr2_set_str(file_csv_txt, sda_get_csv_fname(), &c_info);
    } else {
      gr2_set_str(file_csv_txt, "none", &c_info);
    }

    svp_getcwd(cwd_buff, sizeof(cwd_buff));
    // lets live dangerously...
    int i = 0;
    while(cwd_buff[i] != 0) {
      if(cwd_buff[i] == 'B' && cwd_buff[i + 1] == 'I' && cwd_buff[i + 2] == 'N') {
        i += 4;
        break;
      }
      i++;
    }
    gr2_set_str(file_cwd_txt, (uint8_t*)((uint64_t)cwd_buff + (uint64_t) i), &c_info);
  }

  if(gr2_get_event(reset_button, &c_info) == EV_RELEASED) {
    uint8_t namebuff[258];
    sda_strcp(svmGetName(), namebuff, sizeof(namebuff) - 1);
    svmCloseRunning();
    svmLaunch(namebuff, 0);
  }
  gr2_clear_event(reset_button, &c_info);


  if(gr2_get_event(collect_button, &c_info) == EV_RELEASED) {
    garbageCollect(-1, &svm);
    gr2_set_value(string_bar, svm.stringFieldLen, &c_info);
  }
  gr2_clear_event(collect_button, &c_info);

  if(gr2_get_event(trigger_btn, &c_info) == EV_RELEASED) {
    if (sda_sim_serial_trigger == 0) {
      sda_sim_serial_trigger = 1;
      gr2_set_select(trigger_btn, 1, &c_info);
    } else if (sda_sim_serial_trigger == 1) {
      sda_sim_serial_trigger = 2;
    } else {
      gr2_set_select(trigger_btn, 0, &c_info);
      sda_sim_serial_trigger = 0;
    }
    
  }
  gr2_clear_event(trigger_btn, &c_info);

  if(gr2_get_event(pindbg_btn, &c_info) == EV_RELEASED) {
    sda_sim_set_pin_dbg((uint8_t) gr2_get_value(pindbg_btn, &c_info));
  }
  gr2_clear_event(pindbg_btn, &c_info);

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