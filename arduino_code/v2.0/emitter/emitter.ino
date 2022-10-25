#include <VirtualWire.h>
#include "AHT10.h"

uint8_t readStatus = 0;
float temp_humid[2];
AHT10 myAHT10(AHT10_ADDRESS_0X38);

void setup() {
  Serial.begin(9600);

  while (myAHT10.begin() != true)
  {
    delay(5000);
  }

  //vw_set_rx_pin(10);
  vw_setup(2000);
}

void loop() {
  readStatus = myAHT10.readRawData();
  if (readStatus != AHT10_ERROR)
  {
    temp_humid[0] = myAHT10.readTemperature(AHT10_USE_READ_DATA);
    temp_humid[1] = myAHT10.readHumidity(AHT10_USE_READ_DATA);
  }
  else
  {
    myAHT10.softReset();//reset 1-success, 0-failed
  }

  vw_send((byte *) &temp_humid, sizeof(temp_humid));
  vw_wait_tx();

  delay(1000);
}
