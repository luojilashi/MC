#include <SoftwareSerial.h>

String comdata = ""; //String 定义一个空的字符串
int STATE=1;
SoftwareSerial cSerial(2, 3);
void setup()
{
    Serial.begin(9600);

      cSerial.begin(9600);
  cSerial.println("Hello, world?");
}
 
void loop()
{
  char data[10] = {0};
  data[0]=0XF0;
  data[1]='q';
  data[2]='w';
  data[3]='e';
  data[4]='r';
  data[5]='t';
  data[6]='y';
  data[7]='u';
  data[8]=0xFF;
  cSerial.println(data);
  delay(2000);
}