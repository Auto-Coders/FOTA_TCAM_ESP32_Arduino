#include <WiFi.h>
#include <HTTPClient.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <CRC32.h>

const char* ssid = "HANDE_WIFI";
const char* password =  "3305725800";

const char* filenamePath = "/NewFirmware.bin";
const char* DownloadLink = "https://raw.githubusercontent.com/Auto-Coders/FOTA_TCAM_ESP32_Arduino/master/esp32can.bin";

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
  // Calculate a checksum one byte at a time.
  CRC32 crc;

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
    uint8_t readbyte = file.read();
    crc.update(readbyte);
  }
  file.close();
  // Once we have added all of the data, generate the final CRC32 checksum.
  uint32_t checksum = crc.finalize();
  Serial.printf("CRC is %X", checksum);
}

bool VerifyFile_CRC32(fs::FS &fs, const char * path)
{
  //Last 4 bytes of the bin file is the crc
  // Calculate a checksum one byte at a time.
  CRC32 crc;

  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);

  uint32_t flen = file.size();

  //Ignoring last 4 bytes as it is CRC
  flen = flen - 4;

  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.print("Read from file: \n");
  while (flen)
  {
    uint8_t readbyte = file.read();
    crc.update(readbyte);
    flen--;
  }

  uint8_t ds[4] = {0};
  uint32_t CRCinFile = 0;
  Serial.printf("File size is %d\n", file.size());
  if (file.seek((file.size() - 4)))
  {
    Serial.printf("Seek success\n");
  }
  else
  {
    Serial.printf("Seek fail\n");
  }

  Serial.printf("Current position is %d\n", file.position());

  for (int i = 0; i < 4; i++)
  {
    ds[i] = file.read();
    CRCinFile = (CRCinFile | (ds[i] << (8 * i))); //8*i
  }

  Serial.printf("CRC: %0.2X%0.2X%0.2X%0.2X\n", ds[3], ds[2], ds[1], ds[0]);

  file.close();
  // Once we have added all of the data, generate the final CRC32 checksum.
  uint32_t CRCCalculated = crc.finalize();

  if (CRCCalculated == CRCinFile)
  {
    Serial.printf("CRC is %X calculated is %X, File OK\n", CRCinFile, CRCCalculated);
    return true;
  }
  else
  {
    Serial.printf("CRC is %X calculated is %X, File corrupted\n", CRCinFile, CRCCalculated);
    return false;
  }
}

//Max bytes is 8
bool TransferFile(fs::FS &fs, const char * path)
{
  //Last 4 bytes of the bin file is the crc
  //Calculate a checksum one byte at a time.
  CRC32 crc;
  uint32_t ByteCounter = 0, i = 0;
  uint8_t ReadBuffer[8] = {0};

  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);

  uint32_t flen = file.size();

  //Ignoring last 4 bytes as it is CRC
  flen = flen - 4;

  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.print("Read from file: \n");
  while(flen)
  {
    ReadBuffer[i++] = file.read();

    i = i%8;
    
    ByteCounter++;
    if(ByteCounter % 8 == 0)
    {
      //Send via CAN
      //Remove, added for debugging only
//      Serial.printf("%2X %2X %2X %2X %2X %2X %2X %2X\n", ReadBuffer[0], ReadBuffer[1], ReadBuffer[2], ReadBuffer[3],
//                                                        ReadBuffer[4], ReadBuffer[5], ReadBuffer[6], ReadBuffer[7]);
    }

    flen--;

    //Leftover bytes
    if((flen == 0)&&(ByteCounter % 8 != 0))
    {
      //Send via CAN
      //Remove, added for debugging only
//      Serial.printf("%2X %2X %2X %2X %2X %2X %2X %2X\n", ReadBuffer[0], ReadBuffer[1], ReadBuffer[2], ReadBuffer[3],
//                                                        ReadBuffer[4], ReadBuffer[5], ReadBuffer[6], ReadBuffer[7]);
    }
  }

  return true;
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
uint8_t buff[512] = { 0 };

//Call this only when Wifi is connected
bool DownloadFile(void)
{
  uint32_t filelen;
  HTTPClient http;
  bool retval = false;
  
  http.begin(DownloadLink);
  int httpCode = http.GET();                                        //Make the request

  if (httpCode > 0)
  { //Check for the returning code

    // get lenght of document (is -1 when Server sends no Content-Length header)
    int len = http.getSize();
    filelen = len;

    // get tcp stream
    WiFiClient * stream = http.getStreamPtr();

    deleteFile(SD, filenamePath);
    File file = SD.open(filenamePath, FILE_WRITE);

    // read all data from server
    while (http.connected() && (len > 0 || len == -1))
    {
      // get available data size
      size_t size = stream->available();

      if (size)
      {
        // read up to 128 byte
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

        //Append to file
        file.write((const uint8_t*) buff, c);
        memset(buff, 0, sizeof(buff));

        if (len > 0)
        {
          len -= c;
        }
      }
    }
    file.close();

    retval = true;
  }
  else
  {
    Serial.println("Error on HTTP request");
  }

  http.end(); //Free the resources

  return retval;
}

void loop()
{
  //Get start timestamp
  uint32_t start;
  uint32_t end;
  
  uint32_t LoopItr = 0;

  if ((WiFi.status() == WL_CONNECTED))
  { //Check the current connection status
    if(DownloadFile())
    {
      Serial.println("Download successful");
    }
    else
    {
      Serial.println("Error Download failed");
    }
  }

//  end = millis() - start;
//  Serial.printf("%u bytes written for %u ms in %u iterations\n", filelen, end, LoopItr);
  //Serial.println("------------------------------------------------BIN read--------------------------------------------");
  //readFile(SD, "/hello.bin");
//  start = millis();
  if (VerifyFile_CRC32(SD, filenamePath))
  {
    Serial.println("File OK");
  }
  else
  {
    Serial.println("File Not OK");
  }
  TransferFile(SD, filenamePath);
//  end = millis() - start;
//  Serial.printf("Time required to verify CRC is %u\n", end);
  Serial.println("--------------------------------------------------END-----------------------------------------------");

  while (1);
}
