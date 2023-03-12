#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2023-03-12 22:05:21

#include "Arduino.h"
#include "Arduino.h"
#include "Wire.h"
#include <STM32RTC.h>
#include <SdFat.h>
#include <SPI.h>
#include <LoRa.h>
#include <malloc.h>
#include <TimeLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GSMmodem.h"
extern HardwareSerial debug;
extern HardwareSerial modemSerial;
extern TwoWire i2c;
extern STM32RTC& rtc;
extern Adafruit_SSD1306 display;
#define SD_EN PB6
#define RF95_NSS PC12
#define MODEM_EN PD4
#define LED_BUILTIN PA5
extern SdFat sd;
extern SPIClass SPI_2;
#define SD_CONFIG SdSpiConfig(PD5, SHARED_SPI, SD_SCK_MHZ(16), &SPI_2)
#include <StreamDebugger.h>

void printFileContent(char *filename) ;
void setSensorPower(int power) ;
void formatDateTime(char *timeStr, int size, tmElements_t &tm) ;
void printMem(int l) ;
void setup() ;
void loop() ;

#include "modemStation.ino"


#endif
