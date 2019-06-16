byte pdu[272];
byte* Send_pdu;
byte* Receive_pdu;
int pdu_len = 0;

void Diagnostic_FlashTool_init()
{
  Send_pdu = pdu;
  Receive_pdu = pdu;
}

void UDSECUResetCB()
{
  pdu_len = 2;
  Send_pdu[0] = 0x11;
  Send_pdu[1] = 0x01;
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void UDSDiagnosisSessionControlCB(byte Session_Request)
{
  pdu_len = 2;
  Send_pdu[0] = 0x10;
  Send_pdu[1] = Session_Request;
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void UDSCommunicationControlCB()
{
  pdu_len = 3;
  Send_pdu[0] = 0x28;
  Send_pdu[1] = 0x01; // subfunction
  Send_pdu[2] = 0x01; // comm type
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void UDSControlDTCSettings()
{
  pdu_len = 2;
  Send_pdu[0] = 0x85;
  Send_pdu[1] = 0x01;
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}


void UDSSecurityAccessCB(byte Request_Fn, byte* seed_key)
{
  if (Request_Fn == REQUEST_SEED)
  {
    pdu_len = 2;
    Send_pdu[0] = 0x27;
    Send_pdu[1] = 0x01;
    Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
  }
  else if (Request_Fn == SEND_KEY)
  {
    pdu_len = 6;
    Send_pdu[0] = 0x27;
    Send_pdu[1] = 0x02;
    Send_pdu[2] = seed_key[0];
    Send_pdu[3] = seed_key[2];
    Send_pdu[4] = seed_key[3];
    Send_pdu[5] = seed_key[4];
    Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
  }

}

void UDSRequestDownloadCB(long addr, long len)
{
  byte *ptr
  pdu_len = 2;
  Send_pdu[0] = 0x34;
  Send_pdu[1] = 0x00;
  Send_pdu[2] = 0x44;
  //address
  ptr = &addr;
  Send_pdu[2] = ptr[0];
  Send_pdu[2] = ptr[1];
  Send_pdu[2] = ptr[2];
  Send_pdu[2] = ptr[3];
  //length
  ptr = &len;
  Send_pdu[2] = ptr[0];
  Send_pdu[2] = ptr[1];
  Send_pdu[2] = ptr[2];
  Send_pdu[2] = ptr[3];
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void UDSTransfertDataCB(int len, String SW_name)
{
  // get sw from SD card
  byte download_block_Sequence = 1;
  pdu_len = 2;
  Send_pdu[0] = 0x36;
  Send_pdu[1] = download_block_Sequence;
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
  //wait for response
}

void UDSRequestTransfertExitCB()
{
  pdu_len = 1;
  Send_pdu[0] = 0x37;
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void UDSTesterPresent()
{
  pdu_len = 2;
  Send_pdu[0] = 0x3E;
  Send_pdu[1] = 0x00;
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}



void UDSReadDataByIDCB(byte* subfn_id)
{
  pdu_len = 2;
  Send_pdu[0] = 0x22;
  Send_pdu[1] = subfn_id[0];
  Send_pdu[2] = subfn_id[1];
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void UDSWriteDataByIDCB(byte len, byte* data)
{
  pdu_len = 2;
  Send_pdu[0] = 0x22;
  Send_pdu[1] = subfn_id[0];
  Send_pdu[2] = subfn_id[1];
  for (i = 0; i < len; i++)
  {
    Send_pdu[3 + i] = data[i];
  }
  Receive_pdu = ISOTP_CanSend_Rec(pdu_len, Send_pdu);
}

void DCM_RUNABLE()
{

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
  UDSTransfertDataCB();
  UDSRequestTransfertExitCB();
  UDSRoutinControlCB(CHECK_MEMORY);
  UDSRoutinControlCB(WRITE_SECURITY_KEY);
  UDSECUResetCB();
}
