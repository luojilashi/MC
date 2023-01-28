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
#include <SoftwareSerial.h>
#include "defineData.h"

const double POS_MIN = 700;
const double POS_MILL = 1500;
const double POS_MAX = 2300;
const int GUN_PITCH_MAX = 400;
const int GUN_PITCH_MIN = -50;
const byte PWM_PIN = 2;
const byte gun_PIN = 3;
const int m_angle_value[12][2] = {{1374, 2248}, {1874, 1068}, {1731, 922}, {1196, 2015}, {1895, 1185}, {1783, 887}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

#ifdef _DEBUG_ARD
const double moveangle = 2; // 0.035 no log
#else
double moveangle = 3;
#endif
const double moveipos = moveangle * 1.1;

////////////////////////////////

double m_ipos = 0;
double m_anglePos[32] = {0};
uint32_t m_angleChangeBit = 0;
int m_anglePitch[12] = {0};
char m_strdata[256] = "";
////////////////////////////////////////////
char strTempdata[16] = "";

uint8_t m_inBuffer[10];   // 接收指令缓冲区
int m_bufferIndex = 0;    // 接收指令字符计数
uint8_t m_ch;             // 临时单个字符存储
bool m_recv_flag = false; // 指令接收成功标志位
int m_cmdState;           // 指令接收状态
////////////////////////////////////////////////////////////

SoftwareSerial SoftSerial(4, 5); // RX, TX
RISING_FUNC(0)
FALLING_FUNC(0)
RISING_FUNC(1)
FALLING_FUNC(1)

void setup()
{
  Serial.begin(115200);
  SoftSerial.begin(115200);
  Serial.println("16 channel PWM test!");

  pinMode(PWM_PIN, INPUT_PULLUP);
  pinMode(gun_PIN, INPUT_PULLUP);
  // when pin D2 goes high, call the rising function
  digital_Pin[0] = digitalPinToInterrupt(PWM_PIN);
  digital_Pin[1] = digitalPinToInterrupt(gun_PIN);
  attachInterrupt(digital_Pin[0], rising_0, RISING);
  attachInterrupt(digital_Pin[1], rising_1, RISING);
  set_start('3');
  for (int i = 0; i < 32; i++)
    m_anglePos[i] = POS_MILL;

  sendDataPwm();
  pinMode(7, OUTPUT);
  delay(100);
}

// Add the main program code into the continuous loop() function
void loop()
{
  _START_LOOP
  load_node_data();
  load_main_gun();
  TEXT_FUNC();
  sendDataPwm();
  _END_LOOP
}

void TEXT_FUNC()
{
#ifdef _DEBUG_ARD

#else

#endif
  set_start('3');
  double asd = micros() / 30000;
  int iii = int(asd) % (GUN_PITCH_MAX * 2);
  if (iii > GUN_PITCH_MAX)
  {
    // set_start('1');
    //  gun_pitch('a', GUN_PITCH_MAX, false);
    //  gun_pitch('b', GUN_PITCH_MAX, false);
    //  gun_pitch('C', GUN_PITCH_MAX, false);
  }
  else
  {
    // gun_pitch('a', GUN_PITCH_MIN, false);
    // gun_pitch('b', GUN_PITCH_MIN, false);
  }

  //  gun_pitch('a',GUN_PITCH_MIN, false);
}

void controlServoFront(double Fpos, uint8_t bitF = 0b11)
{
  // 前有效0-135 225-360
  double pos = POS_MAX;
  if (Fpos >= 0 && Fpos <= 1350)
  {
    // Serial.println(mapdouble(Fpos, 0, 1350, POS_MILL, POS_MIN));
    // 0-135
    // 右转
    pos = mapdouble(Fpos, 0, 1350, POS_MILL, POS_MIN);
    if (bitRead(bitF, 0))
      controlServoDelay(0, pos, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, pos, moveangle);
  }
  else if (Fpos >= 2250 && Fpos <= 3600)
  {
    // 225-360
    // 左转
    pos = mapdouble(Fpos, 2250, 3600, POS_MAX, POS_MILL);
    if (bitRead(bitF, 0))
      controlServoDelay(0, pos, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, pos, moveangle);
  }
  else if (Fpos > 1800 && Fpos < 2250)
  {
    // 左死区
    if (bitRead(bitF, 0))
      controlServoDelay(0, POS_MAX, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, POS_MAX, moveangle);
  }
  else if (Fpos > 1350 && Fpos < 1800)
  {
    // 右死区
    if (bitRead(bitF, 0))
      controlServoDelay(0, POS_MIN, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, POS_MIN, moveangle);
  }
  else
  {
    // 无效
    ;
  }
}

void controlServoBack(double Bpos)
{
  // 后有效 45-315
  if (Bpos >= 450 && Bpos <= 3150)
    controlServoDelay(8, mapdouble(Bpos, 450, 3150, POS_MAX, POS_MIN), moveangle);
  else if (Bpos > 3150 && Bpos < 3600)
    controlServoDelay(8, POS_MIN, moveangle);
  else if (Bpos > 0 && Bpos < 450)
    controlServoDelay(8, POS_MAX, moveangle);
  else
    ;
}

void controlServoA(double Fpos)
{
  controlServoFront(Fpos, 0b1);
}
void controlServoB(double Fpos)
{
  controlServoFront(Fpos, 0b10);
}
void controlServoC(double Fpos)
{
  controlServoBack(Fpos);
}

void sendDataPwm()
{
  memset(m_strdata, 0x0, sizeof(m_strdata));
  for (size_t i = 0; i < 32; i++)
  {
    if (!bitRead(m_angleChangeBit, i))
      continue;

    memset(strTempdata, 0x00, sizeof(strTempdata));
    sprintf(strTempdata, "#%dP%d", i + 1, (int)m_anglePos[i]);
    strcat(m_strdata, strTempdata);
  }
  SoftSerial.println(m_strdata);
  _println_log(m_strdata);
  m_angleChangeBit = 0;
}

void load_main_gun()
{
  load_pos();
  switch (get_start())
  {
  case '0': // 左
  case '1': // 右
    controlServoFront(m_ipos);
    controlServoBack(m_ipos);
    break;
  case '2': // 暂停
    break;
  case '3': // 回中
    controlServoFront(0);
    controlServoBack(1800);
    break;
  default:
    break;
  }
  //_println_log(anglePos[0]);
}

void controlServoDelay(const int &gunIndex, const double &pos, double delay)
{
  double temp = pos - m_anglePos[gunIndex];
  if (temp > 0)
    temp = m_anglePos[gunIndex] + delay;
  else if (temp < 0)
    temp = m_anglePos[gunIndex] - delay;
  else
    temp = pos;

  if (POS_MAX < temp || POS_MIN > temp)
    return;

  if (m_anglePos[gunIndex] != temp)
    bitSet(m_angleChangeBit, gunIndex);
  m_anglePos[gunIndex] = temp;
}

void controlServo(const int &gunIndex, const double &pos)
{
  if (POS_MAX < pos || POS_MIN > pos)
    return;

  if (m_anglePos[gunIndex] != pos)
    bitSet(m_angleChangeBit, gunIndex);
  m_anglePos[gunIndex] = pos;
}

void load_pos()
{
  {
    switch (get_start())
    {
    case '0': // 左
      m_ipos = m_ipos - moveipos;
      break;
    case '1': // 右
      m_ipos = m_ipos + moveipos;
      break;
    case '2': // 暂停
      break;
    case '3': // 回中
      m_ipos = 0;
      break;
    default:
      break;
    }
    if (m_ipos >= 3600)
      m_ipos = 0;

    if (m_ipos < 0)
      m_ipos = 3600;
  }
}

void load_node_data()
{
  // for (int i = 0; i < 2; i++)
  // {
  //  Uart_Command_Rev();
  //  if (recv_flag)
  //  {
  //    break;
  //  }
  // }
  uart_command_rev();
  if (m_recv_flag)
  {
    //     for (int i = 0; i < 10; i++)
    //     {
    //      Serial.print("inBuffer[");
    //      Serial.print(i);
    //      Serial.print("]:{");
    //      Serial.print(inBuffer[i]);
    //      Serial.println("}   ");
    //     }

    switch (m_inBuffer[1])
    {
    case '1': //  主炮转向
    {
      set_start(m_inBuffer[2]);
      // load_main_gun();
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
    checkTime[2] = millis();
    memset(m_inBuffer, 0x00, sizeof(m_inBuffer));
  }

  if (millis() - checkTime[2] > 5000)
  {
    // 5s 内无信号
    // set_start('3');
    // _println_log("TRS lost error");
  }
  m_recv_flag = false;
}

void uart_command_rev()
{

  while (Serial.available() > 0)
  {
    m_ch = Serial.read();
    // _print_log("CmdState:[");
    // _print_log(CmdState);
    // _print_log("]ch:[");
    //_print_log(ch);
    //_println_log("]");
    switch (m_cmdState)
    {
    /******** start *********/
    case 0:
    {
      if (DEF_NODE_SST == m_ch)
      {
        m_bufferIndex = 0;
        m_cmdState = 1;
      }
    }
    break;
    /******** data *********/
    case 1:
    {
      if (DEF_NODE_END == m_ch)
      {
        m_cmdState = 2;
      }
      m_bufferIndex++;
      m_inBuffer[m_bufferIndex] = m_ch;
    }
    break;
    /******** end *********/
    case 2:
    {
      m_bufferIndex = 0;
      m_recv_flag = true;
      m_cmdState = 0;
    }
    break;
    }
  }
}

void funcasync(double angle, int i, int a, int timer)
{
  double pospitch = mapdouble(angle, GUN_PITCH_MIN, GUN_PITCH_MAX, m_angle_value[i][0], m_angle_value[i][1]);
  if (m_anglePitch[i] == angle)
  {
    controlServoDelay(a, pospitch, 10);
    pitch_time[i] = millis();
  }
  else if ((millis() - pitch_time[i]) > timer)
  {
    controlServoDelay(a, pospitch, 10);
    m_anglePitch[i] = angle;
  }

  // _println_log(m_anglePos[a]);
};

void gun_pitch(char st, double angle, bool sync)
{
  auto func = [&](int i, int a)
  {
    m_anglePitch[i] = angle;
    double pospitch = mapdouble(angle, GUN_PITCH_MIN, GUN_PITCH_MAX, m_angle_value[i][0], m_angle_value[i][1]);
    controlServoDelay(a, pospitch, 10);
  };
  switch (st)
  {
  case 'a':
  {
    if (sync)
    {
      func(0, 1);
      func(1, 2);
      func(2, 3);
    }
    else
    {
      func(0, 1);
      funcasync(angle, 1, 2, 4000);
      funcasync(angle, 2, 3, 2000);
    }
  }
  break;
  case 'b':
  {
    if (sync)
    {
      func(3, 5);
      func(4, 6);
      func(5, 7);
    }
    else
    {
      func(4, 6);
      funcasync(angle, 3, 5, 4000);
      funcasync(angle, 5, 7, 2000);
    }
  }
  break;
  case 'c':
  {
    if (sync)
    {
      func(6, 9);
      func(7, 10);
      func(8, 11);
    }
    else
    {
      func(8, 11);
      funcasync(angle, 6, 9, 4000);
      funcasync(angle, 7, 10, 2000);
    }
  }
  break;
  case 'd':
    break;
  default:
    break;
  }
}
