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

/*
#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
*/

#include "GSMmodem.h"

HardwareSerial debug(PA3,PA2);
HardwareSerial modemSerial(PC11,PC10);

TwoWire i2c = TwoWire(PA12,PB13);
STM32RTC& rtc = STM32RTC::getInstance();
Adafruit_SSD1306 display(128, 64, &i2c, -1);
#define SD_EN PB6
#define RF95_NSS PC12
#define MODEM_EN PD4
#define LED_BUILTIN PA5

SdFat sd;
SPIClass SPI_2(PC3, PC2, PB10);
#define SD_CONFIG SdSpiConfig(PD5, SHARED_SPI, SD_SCK_MHZ(16), &SPI_2)

#if 1
#include <StreamDebugger.h>
StreamDebugger debugger(modemSerial, debug);
GSMmodem        modem(debugger,debug);
# else
GSMmodem        modem(modemSerial,debug);
#endif

//TinyGsmClient client(modem);
//HttpClient    http(client, "cinter.wiedmer-electro.ch", 80);
char apn[]="gprs.swisscom.ch";

int counter = 0;

int period=29;
int sendOffset=20;

void printFileContent(char *filename) {
    File myFile = sd.open(filename);
    if (myFile) {
        debug.println("File content:");
        while (myFile.available()) {
            debug.write(myFile.read());
        }
    } else {
        debug.println("Error opening file");
    }
    myFile.close();
}

void setSensorPower(int power) {
    i2c.beginTransmission(0x20);  // Transmit to device number 44 (0x2C)
    if (power) {
        i2c.write(1 << 7 | 1 << 5 | 1 << 3 | 1 << 1);
    } else {
        i2c.write(0x00);
    }
    i2c.endTransmission();
}
void formatDateTime(char *timeStr, int size, tmElements_t &tm) {
    snprintf(timeStr, size, "%04d-%02d-%02d %02d:%02d:%02d", tm.Year + 1970,
            tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
}


size_t prev_uordblks = 0;
void printMem(int l) {
    struct mallinfo mi = mallinfo();
    debug.print("@");
    debug.print(l);
    debug.print(": Total allocated space (uordblks): ");
    debug.print(mi.uordblks);
    debug.print("> ");
    debug.println((int) (mi.uordblks - prev_uordblks));
    prev_uordblks = mi.uordblks;
}
int loopCnt = 0;


void setup() {
  debug.begin(115200);
  modemSerial.begin(115200);
  i2c.begin();
  rtc.begin();
  pinMode(SD_EN, OUTPUT);
  digitalWrite(SD_EN, HIGH);
  pinMode(RF95_NSS, OUTPUT);
  digitalWrite(RF95_NSS, HIGH);

  pinMode(MODEM_EN, OUTPUT);
  digitalWrite(MODEM_EN, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);



  //setPins(int ss = LORA_DEFAULT_SS_PIN, int reset = LORA_DEFAULT_RESET_PIN, int dio0 = LORA_DEFAULT_DIO0_PIN);
  LoRa.setPins(RF95_NSS,PD3,PD0);
  LoRa.setSPI(SPI_2);
  LoRa.setSPIFrequency(16E6);

  if (!LoRa.begin(868E6)) {
      Serial.println("Starting LoRa failed!");
      while (1);
    }
  LoRa.setSyncWord(0x12);
  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(20, 1);
  setSensorPower(1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("No OLED display found");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 32);
  display.println("¡¡Hola mundo!!");
  display.display();
  Serial.println("display init");


  //modem.restart();
}
int wakeupTime=0;
void loop() {
  debug.println("Start loop");
  /*
  bool sdOk = false;
  if (!sd.begin(SD_CONFIG)) {
      debug.println("Card failed, or not present");
  } else {
      sdOk = true;
      debug.println("SD card OK");
      printFileContent("config.json");
  }
  sd.end();
*/
  char timeStr[32];
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);

  debug.printf("%02d/%02d/%02d ", rtc.getDay(), rtc.getMonth(), rtc.getYear());
  debug.printf("%02d:%02d:%02d.%03d\n", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());

  LoRa.disableInvertIQ();
  LoRa.receive(0);
  volatile int cmdCont=1;
  debug.println("Command:");
  while(cmdCont){

      int packetSize;
      if(debug.available()){
            switch (debug.read()) {
            case 'c':
                cmdCont = 0;
                break;
            case 'g':
                modem.httpGet("/data/test/");
                break;
            case 'p':
                modem.httpPost("/data/put/","{asdf}");
                break;
            case 's':
                LoRa.beginPacket();
                LoRa.print("hello ");
                LoRa.print(counter);
                LoRa.endPacket();
                LoRa.disableInvertIQ();
                LoRa.receive(0);
                counter++;
                break;

            case 'm':
                debug.print(F("Connecting to "));
                debug.print(apn);
                if (!modem.gprsConnect(apn)) {
                    debug.println(" fail");
                }
                else{
                    debug.println(" ok");
                }

                break;
            case 'l':


                break;
            case 'r':
                modem.restart();
                break;
            case 't':
                volatile int cont=1;
              while(cont){
                  if(debug.available()){
                      modemSerial.write(debug.read());
                  }
                  if(modemSerial.available()){
                      debug.write(modemSerial.read());
                  }
              }
                break;
            }
            printMem(__LINE__);
            debug.println("Command:");
        }
      packetSize = LoRa.parsePacket();
      if (packetSize) {
          digitalWrite(LED_BUILTIN, HIGH);
        // received a packet
        debug.print("Received packet '");
        // read packet
        char recivedText[256];
        int recivedTextind=0;
        while (LoRa.available()) {
            recivedText[recivedTextind]=(char)LoRa.read();
            recivedTextind++;
        }


        LoRa.disableInvertIQ();
        LoRa.receive(0);
        recivedText[recivedTextind]=0;
        debug.print(recivedText);
        debug.print("' with RSSI ");
        debug.println(LoRa.packetRssi());
        String recivedString="[{\"RSSI\":"+(String)+LoRa.packetRssi()+",\"data\":\""+recivedText+"\"}]";

        display.fillRect(0, 24, 128, 16, 0);
        display.setCursor(0, 24);
        display.println(recivedString);
        display.display();

        modem.httpPost("/data/put/",recivedString);
        digitalWrite(LED_BUILTIN, LOW);

        // print RSSI of packet

      }

      tmElements_t now;
        now.Day=rtc.getDay();
        now.Month= rtc.getMonth();
        now.Year= rtc.getYear();
        now.Hour= rtc.getHours();
        now.Minute= rtc.getMinutes();
        now.Second= rtc.getSeconds();

        int nowint=makeTime(now);
        int sleepTime=wakeupTime-nowint;

        formatDateTime(timeStr, 32, now);
        display.fillRect(0, 0, 128, 16, 0);
        display.setCursor(0, 0);
        display.println(timeStr);
        display.print("wakeup in ");
        display.println(sleepTime);
        display.display();

        if (nowint>wakeupTime){
            LoRa.beginPacket();
            LoRa.print("BaseStation");
            LoRa.print(counter);
            LoRa.endPacket();
            LoRa.disableInvertIQ();
            LoRa.receive(0);

            wakeupTime=(int)(((nowint-sendOffset)/period)+1)*period+sendOffset;

        }

  }




}
