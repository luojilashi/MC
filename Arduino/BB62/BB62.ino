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
#include <SoftwareSerial.h>
#include "defineData.h"

#define GREEN_LED A5
#define SEARCH_LED A6
#define HALL_LED 24
#define WARN_LED A7

const double POS_MIN = 580;
const double POS_MILL = 1500;
const double POS_MAX = 2420;
const int GUN_PITCH_MAX = 400;
const int GUN_PITCH_MIN = -50;

const double ANGLE_0 = 0;
const double ANGLE_1 = 520;
const double ANGLE_2 = 1280;
const double ANGLE_3 = 1800;
const double ANGLE_4 = 2320;
const double ANGLE_5 = 3080;
const double ANGLE_6 = 3600;

// RC pwm 信号
const byte PWM_PIN = 5;
const byte gun_PIN = 6;
const byte gupitch_PIN = 7;
// ttl 数据
const byte soft_rx_PIN = 2;
const byte soft_tx_PIN = 3;

// led灯
const byte led_PIN = 13;

const int m_angle_value[12][10] =
    {{1290, 1380, 1540, 1660, 1760, 1760, 1950, 2020, 2070, 2150},
     {1870, 1810, 1620, 1530, 1410, 1310, 1230, 1180, 1120, 1060},
     {1710, 1680, 1530, 1440, 1330, 1250, 1140, 1080, 1000, 920},
     {1280, 1360, 1470, 1590, 1680, 1770, 1860, 1940, 2030, 2120},
     {1890, 1800, 1680, 1590, 1500, 1410, 1350, 1270, 1200, 1130},
     {1720, 1650, 1510, 1390, 1270, 1160, 1070, 980, 900, 800},
     {1950, 1820, 1571, 1455, 1385, 1323, 1182, 1140, 1077, 980},
     {1729, 1645, 1482, 1372, 1310, 1260, 1127, 1070, 1026, 924},
     {1326, 1428, 1623, 1738, 1798, 1850, 1980, 2045, 2105, 2218},
     {0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000},
     {0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000},
     {0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000}};

#ifdef _DEBUG_ARD
const double moveangle = 10; // 0.035 no log
#else
double moveangle = 10;
#endif
const double moveipos = moveangle;

////////////////////////////////

double m_ipos = 0;
double m_anglePos[32] = {0};
uint32_t m_angleChangeBit = 0;
uint32_t m_anglePitch[12] = {0};
char m_strdata[256] = "";
////////////////////////////////////////////
char strTempdata[16] = "";

uint8_t m_inBuffer[10];   // 接收指令缓冲区
int m_bufferIndex = 0;    // 接收指令字符计数
uint8_t m_ch;             // 临时单个字符存储
bool m_recv_flag = false; // 指令接收成功标志位
int m_cmdState;           // 指令接收状态
////////////////////////////////////////////////////////////

SoftwareSerial SoftSerial(soft_rx_PIN, soft_tx_PIN); // RX, TX
RISING_FUNC(0)
FALLING_FUNC(0)
RISING_FUNC(1)
FALLING_FUNC(1)
RISING_FUNC(2)
FALLING_FUNC(2)
DATA(char, start, '3')      // 0 左 1 右 2 停 3 中
DATA(uint32_t, pitch, 1500) // 俯仰
DATA(int, other_start, 0)   // 其他状态+
DATA(uint32_t, led, 0);

void setup()
{
  delay(500);
  _println_int();
  Serial1.begin(115200);
  //   when pin D2 goes high, call the rising function
  REGISTERED_FUNC(0, PWM_PIN);
  REGISTERED_FUNC(1, gun_PIN);
  REGISTERED_FUNC(2, gupitch_PIN);
  attachFunc();

  pinMode(led_PIN, OUTPUT);
  pinMode(HALL_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(SEARCH_LED, OUTPUT);
  pinMode(WARN_LED, OUTPUT);

  for (int i = 0; i < 32; i++)
    m_anglePos[i] = POS_MILL;

  set_start('3');
  sendDataPwm();
  delay(500);
}

// Add the main program code into the continuous loop() function
void loop()
{
  _START_LOOP
  TEXT_FUNC();
  // load_node_data();
  loadled();
  load_main_gun();
  sendDataPwm();
  _END_LOOP
}

void TEXT_FUNC()
{
#ifdef _DEBUG_ARD
  // _print_log("get_start:");
  // _println_log(get_start());

  // _print_log("get_pitch:");
  // (get_pitch());_println_log

  // _print_log("get_other_start:");
  _println_log(get_other_start());

  // _print_log("pwm_value0:");
  // _print_log(pwm_value[0]);

  // _print_log(" pwm_value1:");
  // _print_log(pwm_value[1]);

  // _print_log(" pwm_value2:");
  // _print_log(pwm_value[2]);

  // _print_log(" pwm_value3:");
  // _println_log(pwm_value[3]);

  // double asd = micros() / 30000;
  // int iii = int(asd) % (GUN_PITCH_MAX * 2);
  // if (iii > GUN_PITCH_MAX)
  // {
  //   // set_start('1');
  //   //  gun_pitch('a', GUN_PITCH_MAX, false);
  //   //  gun_pitch('b', GUN_PITCH_MAX, false);
  //   //  gun_pitch('C', GUN_PITCH_MAX, false);
  // }
  // else
  // {
  //   // gun_pitch('a', GUN_PITCH_MIN, false);
  //   // gun_pitch('b', GUN_PITCH_MIN, false);
  // }

  //  gun_pitch('a',GUN_PITCH_MIN, false);
  // digitalWrite(24, HIGH); // 走廊灯
  // digitalWrite(A5, HIGH);// 安全灯

#else

#endif
}

void loadled()
{
  digitalWrite(GREEN_LED, bitRead(get_led(), 0) ? HIGH : LOW);
  digitalWrite(WARN_LED, bitRead(get_led(), 1) ? HIGH : LOW);
  digitalWrite(HALL_LED, bitRead(get_led(), 2) ? HIGH : LOW);
  digitalWrite(SEARCH_LED, bitRead(get_led(), 3) ? HIGH : LOW);
}

void controlServoFront(double Fpos, uint8_t bitF = 0b11)
{
  // 前有效0-135 225-360
  double pos = POS_MAX;
  if (Fpos >= ANGLE_0 && Fpos <= ANGLE_2)
  {
    // Serial.println(mapdouble(Fpos, 0, 1350, POS_MILL, POS_MIN));
    // 0-135
    // 右转
    pos = mapdouble(Fpos, ANGLE_0, ANGLE_2, POS_MILL, POS_MIN);
    if (bitRead(bitF, 0))
      controlServoDelay(0, pos, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, pos, moveangle);
  }
  else if (Fpos >= ANGLE_4 && Fpos <= ANGLE_6)
  {
    // 225-360
    // 左转
    pos = mapdouble(Fpos, ANGLE_4, ANGLE_6, POS_MAX, POS_MILL);
    if (bitRead(bitF, 0))
      controlServoDelay(0, pos, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, pos, moveangle);
  }
  else if (Fpos > ANGLE_3 && Fpos < ANGLE_4)
  {
    // 左死区
    if (bitRead(bitF, 0))
      controlServoDelay(0, POS_MAX, moveangle);
    if (bitRead(bitF, 1))
      controlServoDelay(4, POS_MAX, moveangle);
  }
  else if (Fpos > ANGLE_2 && Fpos < ANGLE_3)
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
  if (Bpos >= ANGLE_1 && Bpos <= ANGLE_5)
    controlServoDelay(8, mapdouble(Bpos, ANGLE_1, ANGLE_5, POS_MIN, POS_MAX), moveangle);
  else if (Bpos > ANGLE_5 && Bpos < ANGLE_6)
    controlServoDelay(8, POS_MAX, moveangle);
  else if (Bpos > ANGLE_0 && Bpos < ANGLE_1)
    controlServoDelay(8, POS_MIN, moveangle);
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
  _println_log(m_strdata);
  Serial1.println(m_strdata);
  m_angleChangeBit = 0;
}

void load_main_gun()
{
  load_pos();
  switch (get_start())
  {
  case '0': // 左
  case '1': // 右
  case '2': // 暂停
    controlServoFront(m_ipos);
    controlServoBack(m_ipos);
    break;

  case '3': // 回中
    controlServoFront(0);
    controlServoBack(1800);
    break;
  default:
    break;
  }
  int pospitch = 0;
  if (m_pitch < LOW_GRADE_2)
    pospitch = 0;
  else if (m_pitch > HIGH_GRADE_1)
    pospitch = GUN_PITCH_MAX;
  else
    pospitch = mapdouble(m_pitch, LOW_GRADE_2, HIGH_GRADE_1, 0, GUN_PITCH_MAX);

  gun_pitch('a', pospitch, false);
  gun_pitch('b', pospitch, false);
  gun_pitch('c', pospitch, false);
}

void controlServoDelay(const int gunIndex, double pos, double delay)
{
  pos = int(pos / delay) * delay;
  int temp = pos - m_anglePos[gunIndex];
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

void func(double angle, byte i, byte a)
{
  //_print_log("func ");
  //_print_log(angle);
  m_anglePitch[i] = int(angle / 10) * 10;
  //_print_log(" func/10 ");
  //_print_log(m_anglePitch[i]);
  uint32_t pospitch = 0;
  if (angle < 0 && angle >= GUN_PITCH_MIN)
  {
    pospitch = mapdouble(angle, GUN_PITCH_MIN, GUN_PITCH_MAX, m_angle_value[i][0], m_angle_value[i][1]);
  }
  else if (angle > GUN_PITCH_MAX)
  {
    pospitch = m_angle_value[i][9];
  }
  else if (angle < GUN_PITCH_MIN)
  {
    pospitch = m_angle_value[i][0];
  }
  else
  {
    int index = angle / 50 + 1;
    pospitch = mapdouble(angle, GUN_PITCH_MIN, GUN_PITCH_MAX, m_angle_value[i][index], m_angle_value[i][index + 1]);
    //_print_log(" index ");
    //_print_log(index);
  }

  //_print_log(" pospitch ");
  // _print_log(pospitch);
  controlServoDelay(a, pospitch, 10);
  // _print_log(" m_anglePos ");
  //_println_log(m_anglePos[a]);
};

void funcasync(double angle, byte i, byte a, unsigned long timer)
{
  angle = int(angle / 10) * 10;
  uint32_t pospitch = 0;
  if (angle < 0 && angle >= GUN_PITCH_MIN)
  {
    pospitch = mapdouble(angle, GUN_PITCH_MIN, GUN_PITCH_MAX, m_angle_value[i][0], m_angle_value[i][1]);
  }
  else if (angle > GUN_PITCH_MAX)
  {
    pospitch = m_angle_value[i][9];
  }
  else if (angle < GUN_PITCH_MIN)
  {
    pospitch = m_angle_value[i][0];
  }
  else
  {
    int index = angle / 50 + 1;
    pospitch = mapdouble(angle, GUN_PITCH_MIN, GUN_PITCH_MAX, m_angle_value[i][index], m_angle_value[i][index + 1]);
  }

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
};

void gun_pitch(char st, double angle, bool sync)
{
  switch (st)
  {
  case 'a':
  {
    if (sync)
    {
      func(angle, 0, 1);
      func(angle, 1, 2);
      func(angle, 2, 3);
    }
    else
    {
      func(angle, 0, 1);
      funcasync(angle, 1, 2, 2500);
      funcasync(angle, 2, 3, 1250);
    }
  }
  break;
  case 'b':
  {
    if (sync)
    {
      func(angle, 3, 5);
      func(angle, 4, 6);
      func(angle, 5, 7);
    }
    else
    {
      func(angle, 4, 6);
      funcasync(angle, 3, 5, 2500);
      funcasync(angle, 5, 7, 1250);
    }
  }
  break;
  case 'c':
  {
    if (sync)
    {
      func(angle, 6, 9);
      func(angle, 7, 10);
      func(angle, 8, 11);
    }
    else
    {
      func(angle, 8, 11);
      funcasync(angle, 6, 9, 2500);
      funcasync(angle, 7, 10, 1250);
    }
  }
  break;
  case 'd':
    break;
  default:
    break;
  }
}

void load_start()
{
  _print_log("pwm_value ");
  _println_log(pwm_value[0]);
  if (pwm_value[0] > LOW_GRADE_1 && pwm_value[0] < LOW_GRADE_2)
  {
    // 右转,时间记录
    if (get_start() != '0')
      checkTime[3] = millis();

    set_start('0');
  }
  else if (pwm_value[0] > HIGH_GRADE_1 && pwm_value[0] < HIGH_GRADE_2)
  {
    // 左转,时间记录
    if (get_start() != '1')
      checkTime[3] = millis();

    set_start('1');
  }
  else if (pwm_value[0] > MID_GRADE_1 && pwm_value[0] < MID_GRADE_2)
  {
    // 2 停 3 中
    if (get_start() != '2' && get_start() != '3')
    {
      // 状态切换
      // 大200ms 暂停，反之回中
      if (millis() - checkTime[3] > 200)
      {
        set_start('2');
      }
      else
      {
        set_start('3');
      }
      // 暂停或回中时间记录
      checkTime[3] = millis();
    }
  }
  // 俯仰
  if (pwm_value[1] > LOW_GRADE_1 && pwm_value[1] < HIGH_GRADE_2)
  {
    set_pitch(pwm_value[1]);
  }

  // 其他功能
  if (pwm_value[2] > LOW_GRADE_1 && pwm_value[2] < LOW_GRADE_2)
  {
    int ledstart = get_led();
    if (bitRead(ledstart, 5) != 0x1)
    {
      switch (get_other_start())
      {
      case 0:
        bitWrite(ledstart, 0, 1);
        set_other_start(get_other_start() + 1);
        break;
      case 1:
        bitWrite(ledstart, 0, 0);
        bitWrite(ledstart, 1, 1);
        set_other_start(get_other_start() + 1);
        break;
      case 2:
        bitWrite(ledstart, 1, 0);
        bitWrite(ledstart, 2, 1);
        set_other_start(get_other_start() + 1);
        break;
      case 3:
        bitWrite(ledstart, 0, 1);
        set_other_start(get_other_start() + 1);
        break;
      case 4:
        bitWrite(ledstart, 3, 1);
        set_other_start(get_other_start() + 1);
        break;
      default:
        set_other_start(0);
        ledstart = 0;
        break;
      }
    }

    bitSet(ledstart, 5);
    set_led(ledstart);
  }
  else if (pwm_value[2] > HIGH_GRADE_1 && pwm_value[2] < HIGH_GRADE_2)
  {
    set_other_start(-1);
    set_led(0);
    if ((millis() % 1000) > 500)
      set_led(0B10);
  }
  else if (pwm_value[2] > MID_GRADE_1 && pwm_value[2] < MID_GRADE_2)
  {
    if (get_other_start() < 0)
    {
      set_led(0);
      set_other_start(0);
    }

    int ledstart = get_led();
    bitWrite(ledstart, 5, 0);
    set_led(ledstart);
  }
}

void fallingfuc()
{
  attachInterrupt(13, risingfuc, RISING);
  load_start();
  digitalWrite(13, HIGH);
}

void risingfuc()
{
  attachInterrupt(13, fallingfuc, FALLING);
  load_start();
  digitalWrite(13, LOW);
}

void attachFunc()
{
  pinMode(13, INPUT_PULLUP);
  attachInterrupt(13, risingfuc, RISING);
  digitalWrite(13, LOW);
}