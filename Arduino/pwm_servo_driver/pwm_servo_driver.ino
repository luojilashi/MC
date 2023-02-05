#include <SoftwareSerial.h>
#include "defineData.h"

RISING_FUNC(4)
FALLING_FUNC(4)
RISING_FUNC(5)
FALLING_FUNC(5)
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
    pwm_value[i] = 0;
    prev_time[i] = 0;
    checkTime[i] = 0.0;

    pinMode(i, INPUT_PULLUP);
    digital_Pin[i] = digitalPinToInterrupt(i);
  }

  REGISTERED_FUNC(4)
  REGISTERED_FUNC(5)
  REGISTERED_FUNC(2)
  REGISTERED_FUNC(3)

  Serial.println("Start serial");
}

void loop()
{
  // loadStart();
  //Serial.println(Getstart());
  TextFunc();
  // LoadData();
}

void TextFunc() {
  Serial.print("St_0[");
  Serial.print(pwm_value[0]);
  Serial.print("]");
  Serial.print("   St_1[");
  Serial.print(pwm_value[1]);
  Serial.print("]");
  Serial.print("   St_2[");
  Serial.print(pwm_value[2]);
  Serial.print("]");
  Serial.print("   St_3[");
  Serial.print(pwm_value[3]);
  Serial.println("]");
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
  delay(50);
  sendstart = false;
  checkTime[1] = micros();
  // Serial.println(data);
}
