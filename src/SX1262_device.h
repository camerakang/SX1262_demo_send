#ifndef __SX1262_DEVICE_H__
#define __SX1262_DEVICE_H__
#include <RadioLib.h>
#define MISO_900 8
#define MOSI_900 9
#define NSS_900 10
#define SCK_900 11

#define IRQ_900 12
#define RST_900 15
#define TX_900 -1
#define RX_900 14
#define BUSY_900 13

extern SX1262 radio_900M;
extern volatile bool operationDone ;
extern int transmissionState ;
extern bool transmitFlag ;
extern "C" ICACHE_RAM_ATTR void setFlag(void);


void SX1262_init();
bool communicate(const char *data, size_t length, String &ack);
bool handleRadioCommunication(const char* ackData, String& receivedData);

#endif // __SX1262_DEVICE_H__
