

const int rx_queue_size = 10;       // Receive Queue size

void CAN_MCAL_init()
{

  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
}

byte* can_rx_frame()
{
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 500 * portTICK_PERIOD_MS) == pdTRUE)
  {
    return rx_frame.data.u8;
  }
  return NULL;
}
