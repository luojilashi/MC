// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
Name:       water.ino
Created:  2018/9/19 21:58:21
Author:     DESKTOP-VSGKPOG\luoji
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//
#include <Servo.h>    // 声明调用Servo.h库
#define MAX_DEPTH 300
#define DEF_DEPT_VALUE 250
#define MID_GRADE   1500
#define LOW_GRADE_1   900
#define LOW_GRADE_2   1150
#define MID_GRADE_1   1400
#define MID_GRADE_2   1600
#define HIGH_GRADE_1  1900
#define HIGH_GRADE_2  2100
Servo myservo;        // 创建一个舵机对象

unsigned long checkFullTime = 0;
unsigned long checkEmptyTime = 0;
const unsigned char intControl = 3;//控制
const unsigned char outFull = 8;// 满水
const unsigned char outEmpty = 7;// 空水
const unsigned char lowV = 6;// 低电压 
const unsigned char outControl = 10;//水泵
int checkFullOutTime = 50;
int checkEmptyOutTime = 800;
int checkV = 75;
int checkVLow = 130;
unsigned char m_fullState = 0;
unsigned char m_emptyState = 0;
bool isDepthLow = false;
bool isVoltageLow = false;
bool isDanger = false;
double DeptTemp = 0;
// The setup() function runs once each time the micro-controller starts
void setup()
{
  Serial.begin(9600);
  pinMode(intControl, INPUT);
  pinMode(outFull, OUTPUT);
  pinMode(outEmpty, OUTPUT);
  pinMode(lowV, OUTPUT);
  digitalWrite(outFull, LOW);
  digitalWrite(outEmpty, LOW);
  digitalWrite(lowV, LOW);
  myservo.attach(outControl);
  myservo.writeMicroseconds(MID_GRADE);
  checkFullTime = millis();
  checkEmptyTime = millis();
  DeptTemp = double(DEF_DEPT_VALUE-160) / 1024;
}

// Add the main program code into the continuous loop() function
void loop()
{
  isVoltageLow = analogRead(A0) < 690;
  isDepthLow = false;//analogRead(A1) > MAX_DEPTH;
  isDanger = isDanger || isVoltageLow || isDepthLow;

  int cState = getState();
  //Serial.println(DeptTemp);
  getWaterFull();
  getWaterEmpty();

  digitalWrite(outFull, m_fullState);
  digitalWrite(outEmpty, m_emptyState);
  
  // TODO
  isDanger = false;
  if (isDanger)
  {
    isDanger = isVoltageLow|| (analogRead(A1) < 160);
    cState = 4;
  }
  //else if(analogRead(A1)>(DeptTemp*analogRead(A4)+160))
    //cState = 3;

  digitalWrite(lowV, isDanger || cState ==3 && millis() / 1000 % 2);
  //cState = 0;
  switch (cState)
  {
  case 1://上浮 排水
    drainOffWater();
    //Serial.println("UP");
    break;
  case 2://下潜 进水 
    inletWater();
    //Serial.println("DOWN");
    break;
  case 3:
    setPump(0);
    break;
  case 4:
    // 超过深度
    // 低电压上浮
    drainOffWater();
    break;
  default:
    setPump(0);
    break;
  }

  delay(5);
}

int getState()
{
  int state = 0;
  delay(5);
  int control = pulseIn(intControl, HIGH);

  if (control > LOW_GRADE_1 && control < LOW_GRADE_2)
  {
    //上浮 排水
    state = 1;
  }
  else if (control > MID_GRADE_1 && control < MID_GRADE_2)
  {
    state = 0;
  }
  else if (control > HIGH_GRADE_1 && control < HIGH_GRADE_2)
  {
    //下潜 进水 
    state = 2;
  }

  return state;
}

void setPump(int pumpState)
{
  switch (pumpState)
  {
  case 0:
    myservo.writeMicroseconds(MID_GRADE);
    break;
  case 1://排水
    myservo.writeMicroseconds(HIGH_GRADE_1);
    break;
  case 2://进水
    myservo.writeMicroseconds(LOW_GRADE_2);
    break;
  default:
    myservo.writeMicroseconds(MID_GRADE);
    return;
  }

  digitalWrite(outEmpty,m_emptyState|| !m_fullState && pumpState !=0 && millis() / 1000 % 2);
  
}

void getWaterFull()
{
  auto mills = millis();
  int hi = analogRead(A2);
  //Serial.println(m_fullState);
  switch (m_fullState)
  {
  case 1://full
    if (hi < checkV && (mills - checkFullTime)>checkFullOutTime)
    {
      m_fullState = 0;
      checkFullTime = mills;
    }
    else if (hi >= checkV)
    {
      checkFullTime = mills;
    }
    break;
  case 0:
    if (hi >= checkV && (mills - checkFullTime)>checkFullOutTime)
    {
      m_fullState = 1;
      checkFullTime = mills;
    }
    else if (hi < checkV)
    {
      checkFullTime = mills;
    }
    break;
  default:
    break;
  }
  //Serial.print("fullState");
  //Serial.println(m_fullState);
}

void getWaterEmpty()
{
  auto mills = millis();
  int he = analogRead(A3);
  //Serial.println(he);

  switch (m_emptyState)
  {
  case 0:
    if (he < checkVLow && (mills - checkEmptyTime)>checkEmptyOutTime)
    {
      m_emptyState = 1;
      checkEmptyTime = mills;
    }
    else if (he >= checkVLow)
    {
      checkEmptyTime = mills;
    }
    break;
  case 1://empty
    if (he >= checkVLow && (mills - checkEmptyTime)>checkEmptyOutTime)
    {
      m_emptyState = 0;
      checkEmptyTime = mills;
    }
    else if (he < checkVLow)
    {
      checkEmptyTime = mills;
    }
    break;
  default:
    break;
  }

  //Serial.print("emptyState");
  //Serial.println(m_emptyState);
}

void drainOffWater()
{
  if (m_emptyState)
    setPump(0);
  else
    setPump(1);
}

void inletWater()
{
  if (m_fullState)
    setPump(0);
  else
    setPump(2);
}

