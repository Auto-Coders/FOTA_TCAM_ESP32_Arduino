
enum can_return_val
{
  CAN_OK = 0,
  CAN_NOK = 1
};

enum can_Rx_notification
{
  CANMCAL_NOTIFICATION = 0,
  CANIF_RX_NOTIFICATION = 1,
};

enum can_Tx_notification
{
  CANMCAL_NOTIFICATION = 0,
  CANMCAL_TX_NOTIFICATION = 1,
};
const int rx_queue_size = 10;       // Receive Queue size


void CAN_MCAL_init()
{
  CAN_device_t CAN_cfg;               // CAN Config
  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
}


byte CAN_MCAL_Recieve_Frame()
{
  CAN_frame_t rx_frame;
  // Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
  {
    CANtoCanIF_RX_Frame.MsgID = rx_frame.MsgID;
    CANtoCanIF_RX_Frame.DLC = rx_frame.FIR.B.DLC;
    memset(CANtoCanIF_RX_Frame.data, 0, 8);
    for (int i = 0; i < rx_frame.FIR.B.DLC; i++)
    {
      CANtoCanIF_RX_Frame.data[i] = rx_frame.data.u8[i];
    }
    return CAN_OK;
  }
  return CAN_NOK;
}

void CAN_MCAL_Transmit_Frame()
{
  CAN_frame_t tx_frame;
  if (CanIFtoCAN_CAN_TX_Frame.MsgID <= 0x7FF)
  {
    tx_frame.FIR.B.FF = CAN_frame_std;
  }
  else
  {
    tx_frame.FIR.B.FF = CAN_frame_ext;
  }
  tx_frame.MsgID = CanIFtoCAN_CAN_TX_Frame.MsgID;
  tx_frame.FIR.B.DLC =  CanIFtoCAN_CAN_TX_Frame.DLC;
  tx_frame.data.u8[0] = CanIFtoCAN_CAN_TX_Frame.data[0];
  tx_frame.data.u8[1] = CanIFtoCAN_CAN_TX_Frame.data[1];
  tx_frame.data.u8[2] = CanIFtoCAN_CAN_TX_Frame.data[2];
  tx_frame.data.u8[3] = CanIFtoCAN_CAN_TX_Frame.data[3];
  tx_frame.data.u8[4] = CanIFtoCAN_CAN_TX_Frame.data[4];
  tx_frame.data.u8[5] = CanIFtoCAN_CAN_TX_Frame.data[5];
  tx_frame.data.u8[6] = CanIFtoCAN_CAN_TX_Frame.data[6];
  tx_frame.data.u8[7] = CanIFtoCAN_CAN_TX_Frame.data[7];
  ESP32Can.CANWriteFrame(&tx_frame);
}


void CAN_MCAL_Runable()
{
  if (CAN_MCAL_Recieve_Frame() == CAN_OK)
  {
    CANRX_State = CANIF_RX_NOTIFICATION;
  }
  else
  {
    CANRX_State = CANMCAL_RX_NOTIFICATION;
  }
  if (CANTX_State == CANMCAL_TX_NOTIFICATION)
  {
    CAN_MCAL_Transmit_Frame(CanIFtoCAN_CAN_TX_Frame);
  }
}
