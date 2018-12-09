#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "SDA_OS/SDA_OS.h"

#ifdef WEBTARGET
#include <emscripten.h>
#endif

// sdl window dimensions
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 552;

//==========global svp vars=====
svpStatusStruct svpSGlobal;
volatile uint8_t touch_lock;
volatile uint8_t redraw_lock;
volatile uint8_t irq_lock;
volatile uint32_t counter;
volatile uint16_t svsCounter;
volatile uint16_t svsLoadCounter;

volatile uint8_t tickLock;
//==============================

eventType pwr_bttn;

static int quit;
static uint8_t preload;
static uint8_t *preload_fname;

SDL_Renderer* gRenderer;
SDL_Texture * gTexture;

pwrModeType oldPowerMode;

uint8_t draw_flag;

// software framebuffer
uint16_t sw_fb[320][480];

uint8_t calibrationFlag;

void svp_set_calibration_data(touchCalibDataStruct input) {
  return;
}

// used for debug-loading apps from commandline
extern svsVM svm;

void svs_hardErrHandler(){
  strTablePrint(&svm);
  getchar();
}

// to emulate touch calibration API
touchCalibDataStruct touchCalibData;

void svp_setLcdCalibrationFlag(uint8_t val) {
  calibrationFlag = val;
}

uint8_t svp_getLcdCalibrationFlag() {
  return calibrationFlag;
}

uint8_t sdaSerialEnabled;
uint8_t sdaDbgSerialEnabled;

void sda_serial_enable() {
  printf("sda_serial_enable\n");
  sdaSerialEnabled = 1;
}

void sda_serial_disable() {
  printf("sda_serial_disable\n");
  sdaSerialEnabled = 0;
}

uint8_t sda_serial_is_enabled() {
  return sdaSerialEnabled;
}

void sda_dbg_serial_enable() {
  printf("sda_dbg_serial_enable\n");
  sdaDbgSerialEnabled = 1;
}

void sda_dbg_serial_disable() {
  printf("sda_dbg_serial_disable\n");
  sdaDbgSerialEnabled = 0;
}

uint8_t sda_dbg_serial_is_enabled() {
  return sdaDbgSerialEnabled;
}

void sda_calibrate () {
  printf("fake calibration\n");
  return;
}

void svp_set_backlight(uint8_t val){
  printf("Setting backlight: %u\n", val);
  return;
}

// serial
uint8_t sda_serial_recieve(uint8_t *str, uint32_t len, uint32_t timeout) {
  uint8_t c;
  uint32_t i = 0;
  printf("Serial recieve: ");

  if (fgets(str, len , stdin) != 0)
    return 1;
  else
    return 0;
}

void sda_serial_transmit(uint8_t *str, uint32_t len) {
  printf("Serial transmit:\n");
  for(int i = 0; i < len; i++){
    printf("%u: %x\n", i, str[i]);
  }
  printf("Serial transmit over\n");
}

// lcd shutdown
void svp_set_lcd_state(lcdStateType state){
  if (state == LCD_OFF) {
    printf("setting lcd state to OFF\n");
  } else {
    printf("setting lcd state to ON\n");
  }

  svpSGlobal.lcdState = state;
}

// set led pattern
void led_set_pattern(ledPatternType pat){
  return;
}

// set system clock low (used when lcd is shut down)
void system_clock_set_low(void){
  return;
}

// sets normal cpu frequency
void system_clock_set_normal(void){
  return;
}

void sda_internal_pin_def(uint8_t pinNum, uint8_t pinType, uint8_t pull) {
  printf("sda_internal_pin_def: pin:%u, type: %u, pull%u\n", pinNum, pinType, pull);
}

void sda_internal_pin_set(uint8_t pinNum, uint8_t val) {
  printf("sda_internal_pin_set: pin:%u, val: %u\n", pinNum, val);
}

uint8_t sda_internal_pin_get(uint8_t pinNum) {
  printf("sda_internal_pin_get: pin:%u, returning zero.", pinNum);
  return 0;
}


void sda_external_pin_def(uint8_t pinNum, uint8_t pinType, uint8_t pull) {
  printf("sda_external_pin_def: pin:%u, type: %u, pull%u\n", pinNum, pinType, pull);
}

void sda_external_pin_set(uint8_t pinNum, uint8_t val) {
  printf("sda_external_pin_set: pin:%u, val: %u\n", pinNum, val);
}

uint8_t sda_external_pin_get(uint8_t pinNum) {
  printf("sda_external_pin_get: pin:%u, returning zero.", pinNum);
  return 0;
}

float sda_external_ADC_get() {
  printf("sda_external_ADC_get: returning 8.88");
  return 8.88;
}

// acoustic alarm
void svp_beep() {
  printf("beep\n\a");
  return;
}

// acoustic alarm customistaion
void svp_beep_set_t(uint16_t time){
  return;
}

void svp_beep_set_pf(uint16_t val){
  return;
}

void svp_beep_set_def(){
  return;
}


// rng api
uint32_t svp_random(){
  return (uint32_t) rand();
}

// draws simulated hw buttons
void DrawButton(int x, int y, gr2EventType act){
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = 32;
  rect.h = 32;

  if (act != EV_NONE) {
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0xFF);
  } else {
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 0xFF);
  }

  SDL_RenderFillRect(gRenderer, &rect);

}

// simulated powerbutton
void DrawButton2(int x, int y, gr2EventType act){
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = 8;
  rect.h = 32;

  if (act != EV_NONE){
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0xFF);
  } else {
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 0xFF);
  }

  SDL_RenderFillRect(gRenderer, &rect);

}

// redraws simulated buttons
void DrawSwButtons(gr2EventType *btn, gr2EventType btn_off){
  SDL_Rect rect;

  rect.x = 0;
  rect.y = 480;
  rect.w = 319;
  rect.h = 71;

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

  SDL_RenderFillRect(gRenderer, &rect);

  DrawButton(32, 496, (int) btn[0]);

  DrawButton(96, 496, (int) btn[1]);

  DrawButton(144, 485, (int) btn[2]);

  DrawButton(144, 485 + 35, (int) btn[3]);

  DrawButton(192, 496, (int) btn[4]);

  DrawButton(256, 496, (int) btn[5]);

  DrawButton2(300, 496, (int) btn_off);

}

// draws point in swfb
void ExtDrawPoint(int x, int y, uint16_t color){
  #ifdef SIM_SHOW_REDRAW
  static uint8_t red_flag;

  red_flag++;

  if (red_flag > 3){
    red_flag = 0;
  }
  if (red_flag == 1) {
    sw_fb[x][y] = 0xFE00;
    draw_flag = 1;
  } else {
    sw_fb[x][y] = color;
    draw_flag = 1;
  }

  #else
  sw_fb[x][y] = color;
  draw_flag = 1;
  #endif
}

// clears software framebuffer
void fb_clear(){
  int a,b;
  for (a = 0; a < 320; a++) {
    for (b = 0; b < 480; b++) {
      sw_fb[a][b] = 0xFFFF;
    }
  }
}

// renders swfb

void fb_copy_to_renderer() {
  int a,i;
  SDL_Rect dstrect;
  uint8_t r, g, b;
  Uint32* pixels = 0;
  int pitch = 0;
  int format;

  SDL_LockTexture(gTexture, 0, (void**)&pixels, &pitch);

  for (a = 0; a < 320; a++) {
    for (i = 0; i < 480; i++) {
      r = (uint8_t)(((float)((sw_fb[a][i]>>11)&0x1F)/32)*256);
      g = (uint8_t)(((float)(((sw_fb[a][i]&0x07E0)>>5)&0x3F)/64)*256);
      b = (uint8_t)(((float)(sw_fb[a][i]&0x1F)/32)*256);

      if (svpSGlobal.lcdState == LCD_OFF) {
        r = 18;
        g = 18;
        b = 18;
      }

      pixels[i * 320 + a] = r << 24 | g << 16 | b << 8 | SDL_ALPHA_OPAQUE;
    }
  }

  SDL_UnlockTexture(gTexture);

  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = 320;
  dstrect.h = 480;

  SDL_RenderCopy(gRenderer, gTexture, NULL, &dstrect);
}


uint16_t btn_pos_x[6];
uint16_t btn_pos_y[6];

void sda_sim_loop() {
  static time_t ti;
  static struct tm * timeinfo;
  //time init

  svpSGlobal.touchValid = 0;
  //touch misc
  static uint8_t touchPrev;
  static uint8_t touchNow;
  static uint8_t oldsec;
  static uint8_t dateSetup;
  static uint8_t LdLck;
  static int old_ticks;
  static uint8_t button_flag;
  static uint8_t button_flag_prev;
  SDL_Event e;

  static uint32_t timer_help;

  pwr_bttn = EV_PRESSED;

  ti = time(0);
  timeinfo = localtime(&ti);

  sda_irq_update_timestruct(
    timeinfo->tm_year + 1900,
    timeinfo->tm_mon + 1,
    timeinfo->tm_mday,
    timeinfo->tm_wday,
    timeinfo->tm_hour,
    timeinfo->tm_min,
    timeinfo->tm_sec
  );

  while(SDL_PollEvent(&e) != 0) {
    if(e.type == SDL_QUIT) {
      quit = 1;
    }

    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
      touchNow = 1;
      svpSGlobal.touchX = LCD_rotr_x((uint16_t)e.button.x, (uint16_t)e.button.y);
      svpSGlobal.touchY = LCD_rotr_y((uint16_t)e.button.x, (uint16_t)e.button.y);
    } else {
      touchNow = 0;
    }
  }

  if ((touchNow == 1) && (touchPrev == 0)) {
    svpSGlobal.touchType = EV_PRESSED;
    svpSGlobal.touchValid = 1;
  }

  if ((touchNow == 0) && (touchPrev == 1)) {
    svpSGlobal.touchType = EV_RELEASED;
    svpSGlobal.touchValid = 1;
  }
  if ((touchNow == 1) && (touchPrev == 1)) {
    svpSGlobal.touchType = EV_HOLD;
    svpSGlobal.touchValid = 1;
  }
  if ((touchNow == 0) && (touchPrev == 0)) {
    svpSGlobal.touchType = EV_NONE;
    svpSGlobal.touchValid = 0;
  }
  touchPrev=touchNow;

  // svsTimer emulation
  if (timer_help < SDL_GetTicks()) {
    timer_help = SDL_GetTicks();
    if (svsCounter > 10) {
      svsCounter -= 10;
    } else {
      svsCounter = 0;
    }
  }

  // hw buttons handlingbuttons

  button_flag = 0;

  for (int i = 0; i < 6; i++) {
    if((svpSGlobal.touchX > btn_pos_x[i]) && (svpSGlobal.touchX < (btn_pos_x[i] + 32))
    && (svpSGlobal.touchY > btn_pos_y[i]) && (svpSGlobal.touchY < (btn_pos_y[i] + 32))) {
      button_flag = 1;

      if (svpSGlobal.keyEv[i] == EV_NONE) {
        svpSGlobal.keyEv[i] = svpSGlobal.touchType;
      }
    } else {
      svpSGlobal.keyEv[i] = EV_NONE;
    }
  }

  button_flag_prev = button_flag;

  //power button
  if((svpSGlobal.touchX > 300) && (svpSGlobal.touchX < 309)
     && (svpSGlobal.touchY > 496) && (svpSGlobal.touchY < 528)) {

    if (svpSGlobal.touchType == EV_PRESSED) {
      draw_flag = 1;
      if(svpSGlobal.lcdState == LCD_OFF) {
        svp_set_lcd_state(LCD_ON);
        pwr_bttn = EV_PRESSED;
      } else {
        svp_set_lcd_state(LCD_OFF);
        pwr_bttn = EV_NONE;
      }
    }
  }

  if(svpSGlobal.lcdState == LCD_OFF){
    svpSGlobal.touchValid = 0;
    svpSGlobal.touchType = EV_NONE;
  }

  // this is noramlly in an irq, but in simulator we don't care
  svp_irq();

  // remove the touch event if it is "out of screen"
  if (svpSGlobal.touchY > 480) {
    svpSGlobal.touchValid = 0;
    svpSGlobal.touchType = EV_NONE;
  }

  // check for power state change and display it
  if (oldPowerMode != svpSGlobal.powerMode) {
    if (svpSGlobal.powerMode == SDA_PWR_MODE_SLEEP) {
      printf("SDA entered SLEEP state.\n");
    } else {
      printf("SDA waked from sleep and is now in NORMAL state.\n");
    }
  }
  oldPowerMode = svpSGlobal.powerMode;

  // main loop
  sda_main_loop();

  // loading app from cmdline
  if ((preload == 1) && (LdLck < 10)){
    LdLck++;
  } else {
    if (LdLck == 10) {
      printf("DevLoading: %s\n", preload_fname);
      sdaSvmLaunch(preload_fname, 0);
      LdLck = 20;
    }
  }

  SDL_RenderClear(gRenderer);
  fb_copy_to_renderer();
  draw_flag = 0;
  DrawSwButtons((gr2EventType *)svpSGlobal.keyEv, pwr_bttn);
  SDL_RenderPresent(gRenderer);
}


int main(int argc, char *argv[]) {
  //events

  SDL_Window* window = NULL;
  eventType touchEvPrev = RELEASED;
  //time
  quit = 0;
  oldPowerMode = SDA_PWR_MODE_NORMAL;

  btn_pos_x[0] = 32;
  btn_pos_y[0] = 496;

  btn_pos_x[1] = 96;
  btn_pos_y[1] = 496;

  btn_pos_x[2] = 144;
  btn_pos_y[2] = 485;

  btn_pos_x[3] = 144;
  btn_pos_y[3] = 485 + 35;

  btn_pos_x[4] = 192;
  btn_pos_y[4] = 496;

  btn_pos_x[5] = 256;
  btn_pos_y[5] = 496;

  SDL_Init( SDL_INIT_VIDEO );
  SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &gRenderer) ;

  fb_clear();

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
  SDL_RenderClear(gRenderer);

  gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 320, 480);

  srand(time(NULL));

  svpSGlobal.lcdState = LCD_ON;

  //lcd init
  LCD_init(319, 479, OR_NORMAL);
  draw_flag = 1;


  #ifdef WEBTARGET
    preload = 0;
  #else
  // loading app from cmdline
  if (argc == 2){
    preload = 1;
    preload_fname = argv[1];
  } else {
    preload = 0;
  }
  #endif

  #ifdef WEBTARGET
  printf("SDA_OS for the internetz!\n");
  emscripten_set_main_loop(sda_sim_loop, 30, 1);
  #else
  uint32_t sdl_time;
  while (!quit) { // quit is handled only from SDL_QUIT
    sdl_time = SDL_GetTicks();
    sda_sim_loop();

    if (SDL_GetTicks() - sdl_time < 33) {
      SDL_Delay(33 - (SDL_GetTicks() - sdl_time));
    }
  }
  #endif

  // Destroy window
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(window); // Quit SDL subsystems
  window = NULL;
  gRenderer = NULL;
  SDL_Quit();
  return 0;
}
