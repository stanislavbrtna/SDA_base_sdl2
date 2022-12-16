#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "SDA_OS/SDA_OS.h"
#include "sda-image.h"

#ifdef WEBTARGET
#include <emscripten.h>
#endif

// sdl window dimensions
const int SCREEN_WIDTH = 455;
const int SCREEN_HEIGHT = 733;

#define SIM_X 67
#define SIM_Y 58

//==========global svp vars=====
svpStatusStruct svpSGlobal;
volatile sdaLockState touch_lock;
volatile sdaLockState irq_lock;
volatile uint32_t counter;
volatile uint16_t sdaAppCounter;
volatile uint16_t svsLoadCounter;

volatile sdaLockState tick_lock;
//==============================

eventType pwr_bttn;

static int quit;
static uint8_t preload;
static uint8_t *preload_fname;

SDL_Renderer* gRenderer;
SDL_Texture * gTexture;

SDL_Texture * bgTexture;

pwrModeType oldPowerMode;

uint8_t draw_flag;

// software framebuffer
uint16_t sw_fb[320][480];

uint16_t btn_pos_x[6];
uint16_t btn_pos_y[6];


// used for debug-loading apps from commandline
extern svsVM svm;

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
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0xAA);
  } else {
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 0xAA);
  }

  SDL_RenderFillRect(gRenderer, &rect);

}

// redraws simulated buttons
void DrawSwButtons(gr2EventType *btn, gr2EventType btn_off){
  SDL_Rect rect;

  for(int i = 0; i < 6; i++) {
    DrawButton(btn_pos_x[i], btn_pos_y[i], (int) btn[i]);
  }

  DrawButton2(393, 616, (int) btn_off);

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

  dstrect.x = SIM_X;
  dstrect.y = SIM_Y;
  dstrect.w = 320;
  dstrect.h = 480;

  SDL_RenderCopy(gRenderer, gTexture, NULL, &dstrect);
}

void fb_render_bg() {
  int a,i;
  SDL_Rect dstrect;
  uint8_t r, g, b;
  Uint32* pixels = 0;
  int pitch = 0;
  int format;
  char *data;
  uint8_t pixel[3];

  SDL_LockTexture(bgTexture, 0, (void**)&pixels, &pitch);

  data = header_data;

  for (a = 0; a < 455; a++) {
    for (i = 0; i < 733; i++) {

      HEADER_PIXEL(data, pixel);

      r = pixel[0];
      g = pixel[1];
      b = pixel[2];

      pixels[a * 733 + i] = r << 24 | g << 16 | b << 8 | SDL_ALPHA_OPAQUE;
    }
  }

  SDL_UnlockTexture(bgTexture);

  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = 455;
  dstrect.h = 733;

  SDL_RenderCopy(gRenderer, bgTexture, NULL, &dstrect);
}


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
  static uint8_t setupLck;
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

    if (e.type == SDL_FINGERDOWN) {
      touchNow = 1;
      svpSGlobal.touchX = LCD_rotr_x((uint16_t)e.tfinger.x - SIM_X, (uint16_t)e.tfinger.y - SIM_Y);
      svpSGlobal.touchY = LCD_rotr_y((uint16_t)e.tfinger.x - SIM_X, (uint16_t)e.tfinger.y - SIM_Y);
      printf("fingertouch! %u\n", svpSGlobal.touchX);
    } else if (e.type == SDL_FINGERUP) {
      touchNow = 0;
    } else if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
      touchNow = 1;
      svpSGlobal.touchX = LCD_rotr_x((uint16_t)e.button.x - SIM_X, (uint16_t)e.button.y - SIM_Y);
      svpSGlobal.touchY = LCD_rotr_y((uint16_t)e.button.x - SIM_X, (uint16_t)e.button.y - SIM_Y);
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
    svpSGlobal.uptimeMs = SDL_GetTicks();
    if (sdaAppCounter > 10) {
      sdaAppCounter -= 10;
    } else {
      sdaAppCounter = 0;
    }
  }

  // hw buttons handlingbuttons
  button_flag = 0;

  for (int i = 0; i < 6; i++) {
    if((e.button.x > btn_pos_x[i]) && (e.button.x < (btn_pos_x[i] + 32))
    && (e.button.y > btn_pos_y[i]) && (e.button.y < (btn_pos_y[i] + 32))) {
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
  if((e.button.x > 380) && (e.button.x < 380 + 32)
     && (e.button.y > 590) && (e.button.y < 600 + 45)) {
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

  if (setupLck < 10) {
    setupLck++;
  } else {
    setupLck = 11;
    // sets up battery voltage after the SDA_OS main loop init.
    svpSGlobal.battPercentage = 75;
    svpSGlobal.battString[0] = 'N';
		svpSGlobal.battString[1] = 'a';
		svpSGlobal.battString[2] = 'N';
		svpSGlobal.battString[3] = 0;
		svpSGlobal.battString[4] = 0;
		svpSGlobal.battString[5] = 0;
  }

  SDL_RenderClear(gRenderer);
  fb_render_bg();
  fb_copy_to_renderer();
  draw_flag = 0;
  //DrawSwButtons((gr2EventType *)svpSGlobal.keyEv, pwr_bttn);
  SDL_RenderPresent(gRenderer);
}


int main(int argc, char *argv[]) {
  //events
  SDL_Window* window = NULL;
  eventType touchEvPrev = RELEASED;
  //time
  quit = 0;
  oldPowerMode = SDA_PWR_MODE_NORMAL;

  btn_pos_x[0] = 89 - 16;
  btn_pos_y[0] = 650- 16;

  btn_pos_x[1] = 155- 16;
  btn_pos_y[1] = 650- 16;

  btn_pos_x[2] = 227- 16;
  btn_pos_y[2] = 622- 16;

  btn_pos_x[3] = 227- 16;
  btn_pos_y[3] = 676- 16;

  btn_pos_x[4] = 298- 16;
  btn_pos_y[4] = 650- 16;

  btn_pos_x[5] = 362- 16;
  btn_pos_y[5] = 650- 16;

  SDL_Init( SDL_INIT_VIDEO );
  SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &gRenderer) ;

  fb_clear();

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
  SDL_RenderClear(gRenderer);

  gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 320, 480);

  bgTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 455, 733);

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

    // get rid of the APPS prefix, if present
    if (preload_fname[0] == 'A' && preload_fname[1] == 'P' && preload_fname[2] == 'P' && preload_fname[3] == 'S') {
      preload_fname += 5;
    }
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
