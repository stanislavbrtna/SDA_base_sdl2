#include "SDA_OS/SDA_OS.h"
#include <stdlib.h>

extern svsVM svm;

void svs_hardErrHandler() {
  strTablePrint(&svm);
  printf("hard error occured: terminating!\n");
  #ifndef WEBTARGET
  exit(1);
  #endif
}

// to emulate touch calibration API
touchCalibDataStruct touchCalibData;
uint8_t calibrationFlag;

void sda_calibrate () {
  printf("fake calibration\n");
  return;
}

void svp_setLcdCalibrationFlag(uint8_t val) {
  calibrationFlag = val;
}

uint8_t svp_getLcdCalibrationFlag() {
  return calibrationFlag;
}

void svp_set_calibration_data(touchCalibDataStruct input) {
  return;
}

// backlight
void svp_set_backlight(uint8_t val){
  printf("Setting backlight: %u\n", val);
  return;
}

uint8_t sda_is_battery_measured() {
  return 0;
}

float sda_get_battery_voltage() {
  return 0;
}


// RTC
void rtc_write_password(uint8_t *pwd) {
  // void
}

uint8_t rtc_read_password(uint8_t *pwd) {
  return 1;
}

uint32_t rtc_read_locked() {
  return 0;
}

void rtc_write_locked(uint32_t val) {
  // void
}


// Serial comms
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

void sda_usb_serial_enable() {
  printf("sda_dbg_serial_enable\n");
  sdaDbgSerialEnabled = 1;
}

void sda_usb_serial_init_bd(uint32_t val) {
  printf("%s: %u\n", __FUNCTION__, val);
}

void sda_serial_init_bd(uint32_t val) {
  printf("%s: %u\n", __FUNCTION__, val);
}

void sda_usb_serial_disable() {
  printf("sda_dbg_serial_disable\n");
  sdaDbgSerialEnabled = 0;
}

uint8_t sda_usb_serial_is_enabled() {
  return sdaDbgSerialEnabled;
}

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

uint8_t sda_serial_str[512];
uint8_t sda_sim_serial_trigger;

uint8_t sda_serial_recieve_init() {
  for (int i = 0; i < sizeof(sda_serial_str); i++)
    sda_serial_str[i] = 0;

  return 1;
}

uint8_t sda_serial_get_rdy() {
  uint8_t c;
  uint32_t i = 0;
  
  if (sda_sim_serial_trigger != 0)
    return sda_sim_serial_trigger;
  else
    return 0;
}

uint16_t sda_serial_get_str(uint8_t *str) {
  printf("Serial recieve: ");
  fgets(sda_serial_str, sizeof(sda_serial_str) , stdin);
  sda_strcp(sda_serial_str, str, sizeof(sda_serial_str));

  sda_sim_serial_trigger = 0;
  int i = 0;
  while(sda_serial_str[i] != 0) {
    i++;
  }

  return i;
}


// USB serial
uint8_t sda_usb_serial_recieve(uint8_t *str, uint32_t len, uint32_t timeout) {
  uint8_t c;
  uint32_t i = 0;
  printf("USB serial recieve: ");

  if (fgets(str, len , stdin) != 0)
    return 1;
  else
    return 0;
}

void sda_usb_serial_transmit(uint8_t *str, uint32_t len) {
  printf("Serial transmit:\n");
  for(int i = 0; i < len; i++){
    printf("%u: %x\n", i, str[i]);
  }
  printf("Serial transmit over\n");
}

uint8_t sda_usb_serial_recieve_init() {
  for (int i = 0; i < sizeof(sda_serial_str); i++)
    sda_serial_str[i] = 0;

  return 1;
}

uint8_t sda_usb_serial_get_rdy() {
  uint8_t c;
  uint32_t i = 0;
  printf("Serial recieve: ");

  if (fgets(sda_serial_str, sizeof(sda_serial_str) , stdin) != 0)
    return 2;
  else
    return 0;
}

uint16_t sda_usb_serial_get_str(uint8_t *str) {
  sda_strcp(sda_serial_str, str, sizeof(sda_serial_str));

  int i = 0;
  while(sda_serial_str[i] != 0) {
    i++;
  }

  return i;
}

static uint8_t enable_for_dbg;

void sda_usb_enable_for_dbg(uint8_t val) {
  enable_for_dbg = val;
}

uint8_t sda_usb_get_enable_for_dbg() {
  return enable_for_dbg;
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

// HW pins
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

uint8_t sda_card_inserted() {
  return 1;
}