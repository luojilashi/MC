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
const int intControl = 3;//控制
const int outControl = 4;//水泵
int checkFullOutTime = 200;
int checkEmptyOutTime = 800;
int checkV = 75;
int checkVLow = 100;
int fullState = 0;
int emptyState = 0;
bool isStop = true;
bool isDepthLow = false;
bool isVoltageLow = false;
bool isDanger = false;
// The setup() function runs once each time the micro-controller starts
void setup()
{
  Serial.begin(9600);
  pinMode(intControl, INPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  myservo.attach(outControl);
  myservo.writeMicroseconds(MID_GRADE);
  checkFullTime = millis();
  checkEmptyTime = millis();
}

// Add the main program code into the continuous loop() function
void loop()
{
  isVoltageLow = analogRead(A1) < 660;
  isDepthLow = analogRead(A0) > 310;
  isDanger = isDanger || isVoltageLow || isDepthLow;
  Serial.println(analogRead(A0));
  
  int cState = getState();

  getWaterFull();
  getWaterEmpty();
  // TODO
  isDanger = false;
  if (isDanger)
    cState = 4;

  
  
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
    isStop = true;
    myservo.writeMicroseconds(LOW_GRADE_2);
    break;
  case 2://进水
    if(isStop)
    {
      //myservo.writeMicroseconds(HIGH_GRADE_1);
      //delay(50);
      //myservo.writeMicroseconds(MID_GRADE);
      //delay(50);
      myservo.writeMicroseconds(HIGH_GRADE_1);
      isStop = false;
    }
    myservo.writeMicroseconds(HIGH_GRADE_1);
    break;
  default:
    myservo.writeMicroseconds(MID_GRADE);
    break;
  }
}

int getWaterFull()
{
  auto mills = millis();
  int hi = analogRead(A2);
  
  switch (fullState)
  {
  case 1://full
    if (hi < checkV && (mills - checkFullTime)>checkFullOutTime)
    {
      fullState=0;
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
      fullState = 1;
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
  
  return fullState;
}

int getWaterEmpty()
{
  auto mills = millis();
  int hi = analogRead(A3);
  
  switch (emptyState)
  {
  case 0:
    if (hi < checkVLow && (mills - checkEmptyTime)>checkEmptyOutTime)
    {
      emptyState = 1;
      checkEmptyTime = mills;
    }
    else if (hi >= checkVLow)
    {
      checkEmptyTime = mills;
    }
    break;
  case 1://empty
    if (hi >= checkV && (mills - checkEmptyTime)>checkEmptyOutTime)
    {
      emptyState = 0;
      checkEmptyTime = mills;
    }
    else if (hi < checkV)
    {
      checkEmptyTime = mills;
    }
    break;
  default:
    break;
  }
  return emptyState;
}

void drainOffWater()
{
  if (fullState)
    setPump(0);
  else
    setPump(1);
}

void inletWater()
{
  if (emptyState)
    setPump(0);
  else
    setPump(2);
}
