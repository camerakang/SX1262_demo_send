#include "SX1262_device.h"

SPIClass radio_spi_900M(HSPI);
SPISettings spiSettings_900M(2000000, MSBFIRST, SPI_MODE0);
SX1262 radio_900M = new Module(NSS_900, IRQ_900, RST_900, BUSY_900, radio_spi_900M, spiSettings_900M);
volatile bool operationDone = false;
int transmissionState = RADIOLIB_ERR_NONE;
bool transmitFlag = false;
extern "C" ICACHE_RAM_ATTR void setFlag(void)
{
  // we got a packet, set the flag
  operationDone = true;
}


void SX1262_init()
{

    radio_spi_900M.begin(SCK_900, MISO_900, MOSI_900, NSS_900);
    // 使用温度补偿晶振
    radio_900M.XTAL = true;
    // initialize SX1262 with default settings
    Serial.print(F("[SX1262] Initializing ... "));
    int state = radio_900M.beginFSK(915.0);
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {

        while (true)
        {
            Serial.print(F("9XX failed, code "));
            Serial.println(state);
            delay(1000);
        }
    }
    radio_900M.setDio1Action(setFlag);
    radio_900M.setRfSwitchPins(14, 18);
#if defined(INITIATING_NODE)
    // send the first packet on this node
    Serial.print(F("[SX1262] Sending first packet ... "));
    transmissionState = radio_900M.startTransmit("Hello World!");
    transmitFlag = true;
#else
    // start listening for LoRa packets on this node
    Serial.print(F("[SX1262] Starting to listen ... "));
    state = radio_900M.startReceive();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }
#endif
}

/**
 * 与远程设备通信，发送数据并接收确认信息
 * 
 * @param data 要发送的数据，为一个字符指针
 * @param length 要发送的数据长度，以字节为单位
 * @param ack 引用类型，用于接收返回的确认信息
 * @return bool 表示通信是否成功
 * 
 * 本函数首先尝试启动无线电模块进行数据传输如果启动传输成功，
 * 则等待数据传输完成。传输完成后，等待并接收返回的确认信息。
 * 根据接收结果返回通信成功与否。
 */
bool communicate(const char *data, size_t length, String &ack)
{
    // 启动数据传输，如果启动失败则返回错误代码
    int transmissionState = radio_900M.startTransmit(data, length);
    if (transmissionState != RADIOLIB_ERR_NONE)
    {
        // 输出错误信息并返回false
        Serial.print(F("Transmit failed, code "));
        Serial.println(transmissionState);
        return false;
    }

    // 输出传输开始信息
    Serial.print("Transmitting::");
    // 记录开始时间，用于超时判断
    unsigned long startTime = millis();
    // 等待传输完成，超时时间为3秒
    while ((!operationDone) && (millis() - startTime < 3000))
    {
        // 等待传输完成
    }
    Serial.println(F(data));
    // 判断是否传输超时
    if (!operationDone)
    {
        // 输出超时信息并返回false
        Serial.println(F("Transmit timeout"));
        return false;
    }

    // 传输成功后，重置操作完成标志，并准备接收数据
    operationDone = false;
    radio_900M.startReceive();

    // 记录开始时间，用于超时判断
    startTime = millis();
    // 等待接收完成，超时时间为3秒
    while ((!operationDone) && (millis() - startTime < 3000))
    {
        // 等待接收完成
    }

    // 判断是否接收超时
    if (!operationDone)
    {
        // 输出超时信息并返回false
        Serial.println(F("Receive timeout"));
        return false;
    }

    // 接收成功后，重置操作完成标志
    operationDone = false;

    // 读取数据，如果读取成功则表示通信成功
    int state = radio_900M.readData(ack);
    if (state == RADIOLIB_ERR_NONE)
    {
        // 输出成功信息并返回true
        Serial.print(F("ACK received!    "));
        return true;
    }
    else
    {
        // 输出错误信息并返回false
        Serial.print(F("Receive failed, code "));
        Serial.println(state);
        return false;
    }
}




bool handleRadioCommunication(const char* ackData, String& receivedData)
{
  bool hasReceivedData = false;  // 用于指示是否接收到数据

  // 检查之前的操作是否完成
  if (operationDone)
  {
    // 重置标志
    operationDone = false;

    if (transmitFlag)
    {
      // 之前的操作是传输，监听响应
      if (transmissionState == RADIOLIB_ERR_NONE)
      {
        Serial.println(F("transmission finished!"));
      }
      else
      {
        Serial.print(F("failed, code "));
        Serial.println(transmissionState);
      }

      // 开始接收
      radio_900M.startReceive();
      Serial.println(F("[SX1262] start Receive ... "));
      transmitFlag = false;
    }
    else
    {
      // 之前的操作是接收
      int state = radio_900M.readData(receivedData);

      if (state == RADIOLIB_ERR_NONE)
      {
        // 成功接收到数据
        Serial.println(F("[SX1262] Received packet!"));
        Serial.print(F("[SX1262] Data:\t\t"));
        Serial.println(receivedData);
        Serial.print(F("[SX1262] RSSI:\t\t"));
        Serial.print(radio_900M.getRSSI());
        Serial.println(F(" dBm"));
        Serial.print(F("[SX1262] SNR:\t\t"));
        Serial.print(radio_900M.getSNR());
        Serial.println(F(" dB"));

        hasReceivedData = true;  // 标记已接收到数据

        // 等待一段时间再发送
        delay(10);

        // 发送ACK数据
        Serial.print(F("[SX1262] Sending another packet ... "));
        transmissionState = radio_900M.startTransmit(ackData);
        transmitFlag = true;
      }
    }
  }

  return hasReceivedData;  // 返回是否接收到数据的标志
}
