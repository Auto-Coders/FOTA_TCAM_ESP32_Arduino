#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

const char* ssid = "HANDE_WIFI";
const char* password =  "3305725800";

void setup()
{

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

}

void writeFile(fs::FS &fs, const char * path, char * message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void readFile(fs::FS &fs, const char * path)
{
  char tempbuff[3];

  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    //Serial.write(file.read());
    sprintf(tempbuff, "%02X", file.read());
    Serial.print(tempbuff);
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char * path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println("File deleted");
  }
  else
  {
    Serial.println("Delete failed");
  }
}

// create buffer for read
uint8_t buff[64] = { 0 };

void loop()
{
  //Get start timestamp
  uint32_t start = millis();
  uint32_t end;
  HTTPClient http;
  
  if ((WiFi.status() == WL_CONNECTED)) 
  { //Check the current connection status

    //    http.begin("https://raw.githubusercontent.com/VPrajwal/Arduino_Libraries/master/rtsw_green_led.bin");
    http.begin("https://raw.githubusercontent.com/Auto-Coders/FOTA_TCAM_ESP32_Arduino/master/esp32can.bin");
    int httpCode = http.GET();                                        //Make the request

    if (httpCode > 0)
    { //Check for the returning code

      // get lenght of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      Serial.print("Size:");
      Serial.println(len);

      char tempbuff[8];

      // get tcp stream
      WiFiClient * stream = http.getStreamPtr();

      deleteFile(SD, "/hello.bin");
      File file = SD.open("/hello.bin", FILE_WRITE);

      // read all data from server
      while (http.connected() && (len > 0 || len == -1))
      {
        // get available data size
        size_t size = stream->available();

        if (size)
        {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

          //          for (int i = 0; i < sizeof(buff); i++)
          //          {
          //            if (i % 16 == 0)
          //            {
          //              Serial.println();
          //            }
          //            sprintf(tempbuff, "%02X", buff[i]);
          //            Serial.print(tempbuff);
          //          }

          //Append to file
          //file.write((const uint8_t*) buff,sizeof(buff));
          file.write((const uint8_t*) buff, c);
          memset(buff, 0, sizeof(buff));

          if (len > 0)
          {
            len -= c;
          }
        }
        //Disable
        //        delay(1);
      }
      file.close();
    }
    else
    {
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }

  uint32_t filelen = http.getSize();
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", filelen, end);
  //Serial.println("------------------------------------------------BIN read--------------------------------------------");
  //readFile(SD, "/hello.bin");
  Serial.println("--------------------------------------------------END-----------------------------------------------");

  while (1);
  delay(10000);

}
