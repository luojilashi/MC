#include <SoftwareSerial.h>
#include "defineData.h"

RISING_FUNC(0)
FALLING_FUNC(0)
RISING_FUNC(1)
FALLING_FUNC(1)
RISING_FUNC(2)
FALLING_FUNC(2)
RISING_FUNC(3)
FALLING_FUNC(3)

SoftwareSerial cSerial(4, 5); // rx tx
void setup()
{
  Serial.begin(9600);
  cSerial.begin(9600);

  for (int i = 0; i < 4; i++)
  {
    pinMode(i, INPUT_PULLUP);
    digital_Pin[i] = digitalPinToInterrupt(i);
  }

  REGISTERED_FUNC(0)
  REGISTERED_FUNC(1)
  REGISTERED_FUNC(2)
  REGISTERED_FUNC(3)

  cSerial.println("Start serial");
}

void loop()
{
  loadStart();
  // Serial.println(Getstart());
  LoadData();
}

void LoadData()
{
  if ((micros() - checkTime[1]) > 1000000)
    return sendData();

  if (sendstart)
    return sendData();
}

void sendData()
{
  char data[10] = {0};
  data[0] = DEF_NODE_SST;
  data[1] = '1';
  data[2] = Getstart();
  data[3] = 'e';
  data[4] = 'r';
  data[5] = 't';
  data[6] = 'y';
  data[7] = 'u';
  data[8] = DEF_NODE_END;

  cSerial.println(data);
  sendstart = false;
  checkTime[1] = micros();
  // Serial.println(data);
}
