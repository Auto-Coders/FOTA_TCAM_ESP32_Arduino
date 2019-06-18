


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



bool VerifyDowloadedFile_CRC32(fs::FS &fs, const char * path)
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

bool Check_update()
{

  return true;
}
