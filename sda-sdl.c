#include <SDL2/SDL.h>
#include <stdlib.h>
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
static uint8_t *preload_param;

SDL_Renderer* gRenderer;
SDL_Renderer* gRenderer2;
SDL_Texture * gTexture;

SDL_Texture * gTexture2;

SDL_Texture * bgTexture;

pwrModeType oldPowerMode;

// sdl windows
SDL_Window* window = NULL;
SDL_Window* window2 = NULL;

uint8_t draw_flag;

// software framebuffer
uint16_t sw_fb[320][480];

uint16_t btn_pos_x[6];
uint16_t btn_pos_y[6];

uint16_t info_fb[INFO_WIDTH][INFO_HEIGHT];

uint8_t fb_used;
uint8_t info_window_enabled;

// used for debug-loading apps from commandline
extern svsVM svm;

// acoustic alarm
SDL_AudioDeviceID audio_device_id;
volatile float beep_frequency;
uint16_t beep_t;
uint64_t beep_cnt;


// rng api
uint32_t svp_random() {
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


void DrawInfoButton(int x, int y, gr2EventType act){
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = 32;
  rect.h = 32;

  if (act != EV_NONE) {
    SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0xFF);
  } else {
    SDL_SetRenderDrawColor(gRenderer, 60, 60, 155, 0xFF);
  }
  SDL_RenderFillRect(gRenderer, &rect);

  rect.x = x + 14;
  rect.y = y + 6;
  rect.w = 4;
  rect.h = 4;

  SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 0xFF);
  SDL_RenderFillRect(gRenderer, &rect);

  rect.x = x + 14;
  rect.y = y + 15;
  rect.w = 4;
  rect.h = 12;

  SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 0xFF);
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
  if(fb_used == MAIN_FB) {
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
    if (x > 320) x = 319;
    if (y > 480) y = 479;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    sw_fb[x][y] = color;
    draw_flag = 1;
    #endif
  } else {
    if (x > INFO_WIDTH) x = INFO_WIDTH;
    if (y > INFO_HEIGHT) y = INFO_HEIGHT;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    info_fb[x][y] = color;
  }
}

// clears software framebuffer
void fb_clear(uint16_t *r, uint16_t w, uint16_t h) {
  int a,b;
  for (a = 0; a < w; a++) {
    for (b = 0; b < h; b++) {
      r[a*b] = 0xFFFF;
    }
  }
}

// renders swfb

void fb_copy_to_renderer(uint16_t *ar, uint16_t w, uint16_t h, SDL_Renderer * renderer, SDL_Texture* texture, int x, int y) {
  int a,i;
  SDL_Rect dstrect;
  uint8_t r, g, b;
  Uint32* pixels = 0;
  int pitch = 0;
  int format;

  SDL_LockTexture(texture, 0, (void**)&pixels, &pitch);

  for (a = 0; a < w; a++) {
    for (i = 0; i < h; i++) {
      r = (uint8_t)(((float)((ar[i+(a*h)]>>11)&0x1F)/32)*256);
      g = (uint8_t)(((float)(((ar[i+(a*h)]&0x07E0)>>5)&0x3F)/64)*256);
      b = (uint8_t)(((float)(ar[i+(a*h)]&0x1F)/32)*256);

      if (svpSGlobal.lcdState == LCD_OFF && ar == sw_fb) {
        r = 18;
        g = 18;
        b = 18;
      }

      pixels[i * w + a] = r << 24 | g << 16 | b << 8 | SDL_ALPHA_OPAQUE;
    }
  }

  SDL_UnlockTexture(texture);

  dstrect.x = x;
  dstrect.y = y;
  dstrect.w = (int)w;
  dstrect.h = (int)h;

  SDL_RenderCopy(renderer, texture, NULL, &dstrect);
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
  static uint8_t touchInfo;
  static uint8_t oldsec;
  static uint8_t dateSetup;
  static uint8_t LdLck;
  static uint8_t setupLck;
  static int old_ticks;
  static uint8_t button_flag;
  static uint8_t button_flag_prev;
  static uint32_t sdl_time;
  SDL_Event e;

  static uint32_t timer_help;

  pwr_bttn = EV_PRESSED;

  sdl_time = SDL_GetTicks();

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

  uint32_t mouse_x = 0;
  uint32_t mouse_y = 0;

  SDL_GetMouseState(&mouse_x, &mouse_y);
  svpSGlobal.touchX = LCD_rotr_x((uint16_t)mouse_x - SIM_X, (uint16_t)mouse_y - SIM_Y);
  svpSGlobal.touchY = LCD_rotr_y((uint16_t)mouse_x - SIM_X, (uint16_t)mouse_y - SIM_Y);

  while(SDL_PollEvent(&e) != 0) {
    // main window
    if ((window != NULL) && (SDL_GetWindowFromID(e.window.windowID) == window)) {
      if (e.type == SDL_MOUSEBUTTONDOWN) {
        touchNow = 1;
      }
    
      if (e.type == SDL_MOUSEBUTTONUP) {
        touchNow = 0;
      }
      
      if((e.type == SDL_WINDOWEVENT) && (e.window.event == SDL_WINDOWEVENT_CLOSE)) {
        quit = 1;
      }
    }

    // info window
    if ((window2 != NULL) && (SDL_GetWindowFromID(e.window.windowID) == window2)) {
      if((e.type == SDL_WINDOWEVENT) && (e.window.event == SDL_WINDOWEVENT_CLOSE)) {
        SDL_DestroyWindow(window2);
        window2 = NULL;
        info_window_enabled = 0;
        info_window_reset();
      }
      if (e.type == SDL_MOUSEBUTTONDOWN) {
        touchInfo = 1;
      }
    
      if (e.type == SDL_MOUSEBUTTONUP) {
        touchInfo = 0;
      }
    }

    if(e.type == SDL_QUIT) {
      quit = 1;
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

  touchPrev = touchNow;

  // hw buttons handlingbuttons
  if (svpSGlobal.touchValid) {
    for (int i = 0; i < 6; i++) {
      //printf("m_x: %u, m_y: %u\np_x: %u, p_y: %u\n", mouse_x, mouse_y, btn_pos_x[i], btn_pos_y[i]);
      if((mouse_x > btn_pos_x[i]) && (mouse_x < (btn_pos_x[i] + 32))
         && (mouse_y > btn_pos_y[i]) && (mouse_y < (btn_pos_y[i] + 32))) {
        if (svpSGlobal.keyEv[i] == EV_NONE) {
          svpSGlobal.keyEv[i] = svpSGlobal.touchType;
        }
        } else {
        svpSGlobal.keyEv[i] = EV_NONE;
      }
    }

    //power button
    if((mouse_x > 380) && (mouse_x < 380 + 32)
      && (mouse_y > 590) && (mouse_y < 600 + 45)) {
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

    // info button
    if((mouse_x > 400) && (mouse_x < 450)
      && (mouse_y > 650) && (mouse_y < 733)) {
      if (svpSGlobal.touchType == EV_PRESSED) {
        if(info_window_enabled == 0) {
          SDL_CreateWindowAndRenderer(INFO_WIDTH, INFO_HEIGHT, 0, &window2, &gRenderer2);
          fb_clear((uint16_t * )info_fb, INFO_WIDTH, INFO_HEIGHT);
          SDL_SetRenderDrawColor(gRenderer2, 0, 0, 0, 0xFF);
          SDL_RenderClear(gRenderer2);
          gTexture2 = SDL_CreateTexture(gRenderer2, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, INFO_WIDTH, INFO_HEIGHT);
          info_window_enabled = 1;
        }
      }
    }
  }

  if(svpSGlobal.lcdState == LCD_OFF) {
    svpSGlobal.touchValid = 0;
    svpSGlobal.touchType = EV_NONE;
  }

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
      svmSetNocacheFlag();
      svmLaunch(preload_fname, 0);
      if(preload_param != 0) {
        svmSetArgumentStr(0, preload_param);
      }
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
  fb_copy_to_renderer((uint16_t *)sw_fb, 320, 480, gRenderer, gTexture, SIM_X, SIM_Y);
  draw_flag = 0;
  //DrawSwButtons((gr2EventType *)svpSGlobal.keyEv, pwr_bttn);
  if(info_window_enabled == 0) {
    DrawInfoButton(420, 700, 0);
  }
  SDL_RenderPresent(gRenderer);

  if(info_window_enabled) {
    info_window_loop(touchInfo, mouse_x, mouse_y);
    SDL_RenderClear(gRenderer2);
    fb_copy_to_renderer((uint16_t *)info_fb, INFO_WIDTH, INFO_HEIGHT, gRenderer2, gTexture2, 0, 0);
    SDL_RenderPresent(gRenderer2);
  }
  
  if (SDL_GetTicks() - sdl_time < 22) {
    SDL_Delay(22 - (SDL_GetTicks() - sdl_time));
  }

  if (svpSGlobal.uptimeMs > beep_cnt) {
    SDL_PauseAudioDevice(audio_device_id, 1);
  }
}


void svp_beep() {
  if (svpSGlobal.mute == 1) 
    return;

  printf("DBG: beep\n");
  beep_cnt = svpSGlobal.uptimeMs + beep_t;
  SDL_PauseAudioDevice(audio_device_id, 0);
  return;
}

// acoustic alarm customistaion
void svp_beep_set_t(uint16_t time) {
  beep_t = time;
}

void svp_beep_set_pf(uint16_t val) {
  beep_frequency = (float) val;
}

void svp_beep_set_def() {
  beep_frequency = 440.0;
  beep_t = 250;
  return;
}

// taken from https://github.com/Grieverheart/sdl_tone_generator
void audio_callback(void* userdata, uint8_t* stream, int len) {
    uint64_t* samples_played = (uint64_t*)userdata;
    float* fstream = (float*)(stream);
    static const float volume = 0.2;

    for(int sid = 0; sid < (len / 8); ++sid)
    {
        double time = (*samples_played + sid) / 44100.0;
        fstream[2 * sid + 0] = volume * sin(beep_frequency * 2.0 * 3.14159265 * time); /* L */
        fstream[2 * sid + 1] = volume * sin(beep_frequency * 2.0 * 3.14159265 * time); /* R */
    }

    *samples_played += (len / 8);
}


int main(int argc, char *argv[]) {
  //events
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

  fb_clear((uint16_t * )sw_fb, 320, 480);

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
  if (argc == 2 || argc == 3) {
    preload = 1;
    preload_fname = argv[1];
    
    if (argc == 3) {
      preload_param = argv[2];
    } else {
      preload_param = 0;
    }

    // get rid of the APPS prefix, if present
    if (preload_fname[0] == 'A' && preload_fname[1] == 'P' && preload_fname[2] == 'P' && preload_fname[3] == 'S') {
      preload_fname += 5;
    }

    if (preload_param != 0 && preload_param[0] == 'A' && preload_param[1] == 'P' && preload_param[2] == 'P' && preload_param[3] == 'S') {
      preload_param += 5;
    }

  } else {
    preload = 0;
  }
  #endif

  // audio stuff
  // taken from https://github.com/Grieverheart/sdl_tone_generator
  uint64_t samples_played = 0;

  if(SDL_Init(SDL_INIT_AUDIO) < 0)
  {
      fprintf(stderr, "Error initializing SDL. SDL_Error: %s\n", SDL_GetError());
      return -1;
  }
  svp_beep_set_def();

  SDL_AudioSpec audio_spec_want, audio_spec;
  SDL_memset(&audio_spec_want, 0, sizeof(audio_spec_want));

  audio_spec_want.freq     = 44100;
  audio_spec_want.format   = AUDIO_F32;
  audio_spec_want.channels = 2;
  audio_spec_want.samples  = 512;
  audio_spec_want.callback = audio_callback;
  audio_spec_want.userdata = (void*)&samples_played;

  audio_device_id = SDL_OpenAudioDevice(
      NULL, 0,
      &audio_spec_want, &audio_spec,
      SDL_AUDIO_ALLOW_FORMAT_CHANGE
  );

  if(!audio_device_id)
  {
      fprintf(stderr, "Error creating SDL audio device. SDL_Error: %s\n", SDL_GetError());
      SDL_Quit();
      return -1;
  }

  SDL_PauseAudioDevice(audio_device_id, 1);

  #ifdef WEBTARGET
  printf("SDA_OS for the internetz!\n");
  emscripten_set_main_loop(sda_sim_loop, 30, 1);
  #else
  
  while (!quit) { // quit is handled only from SDL_QUIT
    sda_sim_loop();
  }
  #endif

  // Destroy window
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(window); // Quit SDL subsystems
  SDL_CloseAudioDevice(audio_device_id);
  window = NULL;
  gRenderer = NULL;
  SDL_Quit();
  return 0;
}
