#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>
byte CANRX_State = 0;
byte CANTX_State = 0;

struct Can_Frame
{
  long MsgID;
  byte DLC;
  byte data[8];
};
Can_Frame CANtoCanIF_RX_Frame;
Can_Frame CanIFtoCAN_CAN_TX_Frame;

void setup() {

}

void loop()
{



}
