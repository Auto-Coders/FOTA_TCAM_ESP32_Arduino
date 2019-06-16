
#define REQUEST_SEED 0x01
#define SEND_KEY 0x02

#define DEFAULT_SESSION 0x01 
#define EXTENDED_SESSION 0x03 
#define PROGRAMMING_SESSION 0x02

long addr=0x10000;
long len=0x10000;

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

void UDSTransfertDataCB(String SW_name)
{
  memset(tx_frame.data.u8, 0, 8);
  // get sw from SD card
  byte download_block_Sequence = 1;
  tx_frame.data.u8[0] = 0x36;
  tx_frame.data.u8[1] = download_block_Sequence;
  ESP32Can.CANWriteFrame(&tx_frame);
  // wait upto 500ms for response flow control
  rxframe = can_rx_frame();
  if (rxframe[1] == 0x76)
  {
    //success
  }
  else
  {
    //fail
  }
}

void UDSRequestTransfertExitCB()
{
  memset(tx_frame.data.u8, 0, 8);
  tx_frame.data.u8[0] = 0x37;
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

void Reflash_ECU_Runable()
{
  UDSDiagnosisSessionControlCB(EXTENDED_SESSION);
  UDSCommunicationControlCB();
  UDSControlDTCSettings();
  UDSSecurityAccessCB(REQUEST_SEED);
  UDSSecurityAccessCB(SEND_KEY);
  UDSDiagnosisSessionControlCB(PROGRAMMING_SESSION);
  UDSRequestDownloadCB(addr, len);
  UDSTransfertDataCB("SW_Red.bin");
  UDSRequestTransfertExitCB();
  UDSRoutinControlCB(CHECK_MEMORY);
  UDSRoutinControlCB(WRITE_SECURITY_KEY);
  UDSECUResetCB();
}
