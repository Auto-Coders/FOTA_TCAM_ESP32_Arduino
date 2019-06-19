#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <CRC32.h>
CAN_frame_t tx_frame;
CAN_frame_t rx_frame;
CAN_device_t CAN_cfg;               // CAN Config
byte* rxframe;

const char* ssid = "Prajwal21991";
const char* password =  "Gamblefish123";

const char* filenamePath = "/NewFirmware.bin";
const char* DownloadLink = "https://raw.githubusercontent.com/Auto-Coders/FOTA_TCAM_ESP32_Arduino/master/esp32can.bin";
//const char* DownloadLink = "https://github.com/Auto-Coders/FOTA_TCAM_ESP32_Arduino/raw/master/README.md";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
    delay(4000);
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
  
    Serial.println("Connected to the WiFi network");
  
    if (!SD.begin())
    {
      Serial.println("Card Mount Failed");
      return;
    }
    uint8_t cardType = SD.cardType();
  
    if (cardType == CARD_NONE)
    {
      Serial.println("No SD card attached");
      return;
    }
  
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
      Serial.println("MMC");
    } else if (cardType == CARD_SD)
    {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC)
    {
      Serial.println("SDHC");
    }
    else
    {
      Serial.println("UNKNOWN");
    }
  
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
  CAN_MCAL_init();
  Diagnostic_FlashTool_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  bool   result = false;
    if (Check_update())
    {
  
      result = DownloadFile();
      result = result & VerifyFile_CRC32(SD, filenamePath);
    }

  if (result == true)
  {
    Reflash_ECU_Runable();
  }
  delay(100000);
}
