
#define REQUEST_SEED 0x01
#define SEND_KEY 0x02

#define DEFAULT_SESSION 0x01
#define EXTENDED_SESSION 0x03
#define PROGRAMMING_SESSION 0x02

#define CHECK_MEMORY 0xFF00
#define ERASE_MEMORY 0xFF01
#define WRITE_SECURITY_KEY 0xFF02

long addr = 0x10000;
long len = 0x10000;

void Diagnostic_FlashTool_init()
{
  tx_frame.FIR.B.FF = CAN_frame_std;
  tx_frame.MsgID = 0x001;
  tx_frame.FIR.B.DLC = 8;
}

void UDSECUResetCB()
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x02;
  tx_frame.data.u8[1] = 0x11;
  tx_frame.data.u8[2] = 0x01;
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response
  rxframe = can_rx_frame();
  if (rxframe[1] == 0x51)
  {
    //success
  }
  else
  {
    //fail
  }
}

void UDSDiagnosisSessionControlCB(byte Session_Request)
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x02;
  tx_frame.data.u8[1] = 0x10;
  tx_frame.data.u8[2] = Session_Request;
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response
  rxframe = can_rx_frame();
  if (rxframe[1] == 0x50)
  {
    //success
  }
  else
  {
    //fail
  }
}

void UDSCommunicationControlCB()
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x03;
  tx_frame.data.u8[1] = 0x28;
  tx_frame.data.u8[2] = 0x01; // subfunction
  tx_frame.data.u8[3] = 0x01; // comm type
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response
  rxframe = can_rx_frame();
  if (rxframe[1] == 0x68)
  {
    //success
  }
  else
  {
    //fail
  }
}

void UDSControlDTCSettings()
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x02;
  tx_frame.data.u8[1] = 0x85;
  tx_frame.data.u8[2] = 0x01;
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response
  rxframe = can_rx_frame();
  if (rxframe[1] == 0xC5)
  {
    //success
  }
  else
  {
    //fail
  }
}


void UDSSecurityAccessCB(byte Request_Fn)
{
  memset(tx_frame.data.u8, 0, 8);
  byte seed_key[4];
  if (Request_Fn == REQUEST_SEED)
  {
    tx_frame.data.u8[0] = 0x02;
    tx_frame.data.u8[1] = 0x27;
    tx_frame.data.u8[2] = 0x01;
    ESP32Can.CANWriteFrame(&tx_frame);
    // wait upto 500ms for response
    rxframe = can_rx_frame();
  }
  else if (Request_Fn == SEND_KEY)
  {
    memset(tx_frame.data.u8, 0, 8);
    tx_frame.data.u8[0] = 0x06;
    tx_frame.data.u8[1] = 0x27;
    tx_frame.data.u8[2] = 0x02;
    tx_frame.data.u8[3] = seed_key[0];
    tx_frame.data.u8[4] = seed_key[2];
    tx_frame.data.u8[5] = seed_key[3];
    tx_frame.data.u8[6] = seed_key[4];
    ESP32Can.CANWriteFrame(&tx_frame);
    // wait upto 500ms for response
    rxframe = can_rx_frame();
  }
  if (rxframe[1] == 0x67)
  {
    //success
  }
  else
  {
    //fail
  }
}

void UDSRequestDownloadCB(long addr, long len)
{
  byte *ptr;
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x10;
  tx_frame.data.u8[1] = 0x0b;
  tx_frame.data.u8[2] = 0x34;
  tx_frame.data.u8[3] = 0x00;
  tx_frame.data.u8[4] = 0x44;
  //address
  ptr = (byte*)&addr;
  tx_frame.data.u8[5] = ptr[0];
  tx_frame.data.u8[6] = ptr[1];
  tx_frame.data.u8[7] = ptr[2];
  //can send
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait for flow control
  // wait upto 500ms for response
  rxframe = can_rx_frame();
  memset(tx_frame.data.u8, 0, 8);
  //canreceive  send remaining frame
  tx_frame.data.u8[0] = 0x21;
  tx_frame.data.u8[1] = ptr[3];
  //length
  ptr = (byte*)&len;
  tx_frame.data.u8[2] = ptr[0];
  tx_frame.data.u8[3] = ptr[1];
  tx_frame.data.u8[4] = ptr[2];
  tx_frame.data.u8[5] = ptr[3];
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response
  rxframe = can_rx_frame();


  if (rxframe[1] == 0x74)
  {
    //success
  }
  else
  {
    //fail
  }
}

boolean UDSTransfertDataCB(fs::FS &fs, const char * path)
{
  memset(tx_frame.data.u8, 0, 8);
  // get sw from SD card
  byte download_block_Sequence = 1;
  int i, j;
  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }
  uint32_t flen = file.size();
  byte sequence_counter = 0;
  //Ignoring last 4 bytes as it is CRC
  flen = flen - 4;

  while (flen)
  {
    memset(tx_frame.data.u8, 0, 8);
    if (flen > 6)
    {
      tx_frame.data.u8[0] = 0x11;
      tx_frame.data.u8[1] = 0x02;
      tx_frame.data.u8[2] = 0x36;
      tx_frame.data.u8[3] = download_block_Sequence;
      for (i = 4; (i < 8) || (flen > 0); i++)
      {
        tx_frame.data.u8[i] = file.read();
        flen--;
      }
      ESP32Can.CANWriteFrame(&tx_frame);
      rxframe = can_rx_frame();
      if (rxframe[0] == 0x30)
      {
        //success
      }
      else
      {
        return false;
      }
      memset(tx_frame.data.u8, 0, 8);

      for (j = 0; (j < 252) && (flen > 0); j = j + 7)
      {
        sequence_counter++;
        tx_frame.data.u8[0] = 0x20 + (0x0F & sequence_counter);
        for (i = 1; (i < 8) && (flen > 0); i++)
        {
          tx_frame.data.u8[i] = file.read();
          flen--;
        }
        ESP32Can.CANWriteFrame(&tx_frame);
      }
      rxframe = can_rx_frame();
      if (rxframe[1] == 0x76)
      {
        //success
      }
      else
      {
        return false;
      }
    }
    else
    {
      tx_frame.data.u8[0] = flen + 2;
      tx_frame.data.u8[1] = 0x36;
      tx_frame.data.u8[2] = download_block_Sequence;
      for (i = 3; (i < 8) || (flen > 0); i++)
      {
        tx_frame.data.u8[i] = file.read();
        flen--;
      }
      ESP32Can.CANWriteFrame(&tx_frame);
      rxframe = can_rx_frame();
      if (rxframe[1] == 0x76)
      {
        //success
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

void UDSRequestTransfertExitCB()
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x01;
  tx_frame.data.u8[1] = 0x37;
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response
  rxframe = can_rx_frame();
  if (rxframe[1] == 0x77)
  {
    //success
  }
  else
  {
    //fail
  }
}

void UDSRoutinControlCB(unsigned int Fn_ID)
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x37;
  ESP32Can.CANWriteFrame(&tx_frame);
}

void Reflash_ECU_Runable()
{
  UDSDiagnosisSessionControlCB(EXTENDED_SESSION);
  UDSCommunicationControlCB();
  UDSControlDTCSettings();
  UDSSecurityAccessCB(REQUEST_SEED);
  UDSSecurityAccessCB(SEND_KEY);
  UDSDiagnosisSessionControlCB(PROGRAMMING_SESSION);
  UDSRoutinControlCB(ERASE_MEMORY);
  UDSRequestDownloadCB(addr, len);
  UDSTransfertDataCB(SD, "SW_Red.bin");
  UDSRequestTransfertExitCB();
  UDSRoutinControlCB(CHECK_MEMORY);
  UDSRoutinControlCB(WRITE_SECURITY_KEY);
  UDSECUResetCB();
}
