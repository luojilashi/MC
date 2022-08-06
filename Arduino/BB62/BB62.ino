// Visual Micro is in vMicro>General>Tutorial Mode
//
/*
  Name:       TPWM.ino
  Created:  2018/7/3 19:06:05
  Author:     MSI\MingCH
*/

// Define User Types below here or use a .h file
//

// Define Function Prototypes that use User Types below here or use a .h file
//

// Define Functions below here or use other .ino or cpp files
//

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "defineData.h"
int HZ = 50;
const double POS_D = 0.1729;
const double POS_MIN = 700 * (HZ / 50) * POS_D;
const double POS_MILL = 1520 * (HZ / 50) * POS_D;
const double POS_MAX = 2300 * (HZ / 50) * POS_D;
const byte PWM_PIN = 2;
const byte gun_PIN = 3;
#ifdef _DEBUG_ARD
double moveangle = 1;// 0.035 no log
#else
double moveangle = 0.15;
#endif

double moveipos = moveangle * 3600 / POS_MAX;
double ipos = 0;
double anglePos[16] = {0};

////////////////////////////////////////////

uint8_t inBuffer[10];   // 接收指令缓冲区
int bufferIndex = 0;    // 接收指令字符计数
uint8_t ch;             // 临时单个字符存储
bool recv_flag = false; // 指令接收成功标志位
int CmdState;           // 指令接收状态
////////////////////////////////////////////////////////////

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

RISING_FUNC(0)
FALLING_FUNC(0)
RISING_FUNC(1)
FALLING_FUNC(1)

void setup()
{
  Serial.begin(9600);
  Serial.println("16 channel PWM test!");

  pinMode(PWM_PIN, INPUT_PULLUP);
  pinMode(gun_PIN, INPUT_PULLUP);
  // when pin D2 goes high, call the rising function
  digital_Pin[0] = digitalPinToInterrupt(PWM_PIN);
  digital_Pin[1] = digitalPinToInterrupt(gun_PIN);
  attachInterrupt(digital_Pin[0], rising_0, RISING);
  attachInterrupt(digital_Pin[1], rising_1, RISING);
  for (int i = 0; i < 16; i++)
    anglePos[i] = POS_MILL;

  pinMode(7, OUTPUT);
  pwm.begin();
  pwm.setPWMFreq(HZ); // This is the maximum PWM frequency
  delay(500);
}

// Add the main program code into the continuous loop() function
void loop()
{
  //loadStart(0);
  //Load_pos();
  Load_Node_Data();
   //_println_log(micros() - checkTime[2]);
   //checkTime[2] = micros();

  switch (Getstart())
  {
    case '0': //左
    case '1': //右
    case '2': //暂停
      controlServoFront(ipos);
      controlServoBack(ipos);
      break;
    case '3': //回中
      controlServoFront(0);
      controlServoBack(1800);
      break;
    default:
      break;
  }
  //_println_log(anglePos[0]);
  sendDataPwm();
}

void TEXT_FUNC()
{
  // loadStart(0);
  controlServoFront(900);
  controlServoBack(900);
  sendDataPwm();
}

void controlServoFront(double Fpos)
{
  //前有效0-135 225-360
  double pos = POS_MAX;
  if (Fpos >= 0 && Fpos <= 1350)
  {
    // Serial.println(mapdouble(Fpos, 0, 1350, POS_MILL, POS_MIN));
    // 0-135
    //右转
    pos = mapdouble(Fpos, 0, 1350, POS_MILL, POS_MIN);
    controlServo(0, pos);
    controlServo(4, pos);
  }
  else if (Fpos >= 2250 && Fpos <= 3600)
  {
    // 225-360
    //左转
    pos = mapdouble(Fpos, 2250, 3600, POS_MAX, POS_MILL);
    controlServo(0, pos);
    controlServo(4, pos);
  }
  else if (Fpos > 1800 && Fpos < 2250)
  {
    //左死区
    controlServo(0, pos);
    controlServo(4, pos);
  }
  else if (Fpos > 1350 && Fpos < 1800)
  {
    //右死区
    controlServo(0, POS_MIN);
    controlServo(4, POS_MIN);
  }
  else
  {
    //无效
    ;
  }
  // controlServo(1, pos);
  // controlServo(2, pos);
  // controlServo(3, pos);
  // controlServo(5, pos);
  // controlServo(6, pos);
  // controlServo(7, pos);
  // controlServo(9, pos);
  // controlServo(10, pos);
}

void controlServoBack(double Bpos)
{
  //后有效 45-315
  if (Bpos >= 450 && Bpos <= 3150)
    controlServo(8, mapdouble(Bpos, 450, 3150, POS_MAX, POS_MIN) );
  else if (Bpos > 3150 && Bpos < 3600)
    controlServo(8, POS_MIN);
  else if (Bpos > 0 && Bpos < 450)
    controlServo(8, POS_MAX);
  else
    ;
}

void sendDataPwm()
{
  for (int i = 0; i < 16; i++)
    pwm.setPWM(i, 0, anglePos[i]);
}

void controlServo(const int &gunIndex, const double &pos)
{
  double temp = pos - anglePos[gunIndex];

  if (temp > 0)
    temp = anglePos[gunIndex] + moveangle;
  else if (temp < 0)
    temp = anglePos[gunIndex] - moveangle;
  else
    temp = pos;

  if (POS_MAX < temp || POS_MIN > temp)
    return;

  anglePos[gunIndex] = temp;
}

void Load_pos()
{
  {
    switch (Getstart())
    {
      case '0': // 左
        ipos = ipos - moveipos;
        break;
      case '1': // 右
        ipos = ipos + moveipos;
        break;
      case '2': // 暂停
        break;
      case '3': // 回中
        ipos = 0;
        break;
      default:
        break;
    }
    if (ipos >= 3600)
      ipos = 0;

    if (ipos < 0)
      ipos = 3600;
  }
}

void Load_Node_Data()
{
  // for (int i = 0; i < 2; i++)
  // {
  //  Uart_Command_Rev();
  //  if (recv_flag)
  //  {
  //    break;
  //  }
  // }
  Uart_Command_Rev();
  if (recv_flag)
  {
    //     for (int i = 0; i < 10; i++)
    //     {
    //      Serial.print("inBuffer[");
    //      Serial.print(i);
    //      Serial.print("]:{");
    //      Serial.print(inBuffer[i]);
    //      Serial.println("}   ");
    //     }

    switch (inBuffer[1])
    {
      case '1': //  主炮转向
        {
          Setstart(inBuffer[2]);
        }
        break;
      case '2':
      case '3':
      case '4':
      case '5':
        break;

      default:
        break;
    }
    checkTime[3] = micros();
    memset(inBuffer, 0x00, sizeof(inBuffer));
  }
  // 获取炮转状态
  Load_pos();

  if (micros() - checkTime[3] > 5000000)
  {
    // 5s 内无信号
    Setstart('3');
    // _println_log("TRS lost error");
  }
  recv_flag = false;
}

void Uart_Command_Rev()
{

  while (Serial.available() > 0)
  {
    ch = Serial.read();
    // _print_log("CmdState:[");
    // _print_log(CmdState);
    // _print_log("]ch:[");
    //_print_log(ch);
    //_println_log("]");
    switch (CmdState)
    {
      /******** start *********/
      case 0:
        {
          if (DEF_NODE_SST == ch)
          {
            bufferIndex = 0;
            CmdState = 1;
          }
        }
        break;
      /******** data *********/
      case 1:
        {
          if (DEF_NODE_END == ch)
          {
            CmdState = 2;
          }
          bufferIndex++;
          inBuffer[bufferIndex] = ch;
        }
        break;
      /******** end *********/
      case 2:
        {
          bufferIndex = 0;
          recv_flag = true;
          CmdState = 0;
        }
        break;
    }
  }
}
