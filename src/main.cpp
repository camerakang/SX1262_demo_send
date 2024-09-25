#include <Arduino.h>
#include "SX1262_device.h"

void setup()
{
  Serial.begin(115200);
  SX1262_init();
}
int count = 0;
//发送程序
void loop()
{
  String ack;
  String str = "Hello World! #" + String(count);
  if (communicate(str.c_str(), 12, ack))
  {
    Serial.print(F("Received ACK: "));
    Serial.println(ack);
    count++;
  }
  else
  {
    Serial.println(F("Communication failed"));
  }
  delay(1000); // 等待5秒后再尝试下一次通信
}

//接收程序
// void loop()
// {
//   String receivedData;
//   if (handleRadioCommunication("ACK!", receivedData))
//   {
//     // 在这里使用 receivedData 做进一步处理
//     Serial.println("Processing received data...");
//   }
// }
