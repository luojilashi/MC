// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       water.ino
    Created:	2018/9/19 21:58:21
    Author:     DESKTOP-VSGKPOG\luoji
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//
#include <Servo.h>    // 声明调用Servo.h库
Servo myservo;        // 创建一个舵机对象
#define MID_GRADE		1500
#define LOW_GRADE_1		900
#define LOW_GRADE_2		1150
#define MID_GRADE_1		1400
#define MID_GRADE_2		1600
#define HIGH_GRADE_1	1900
#define HIGH_GRADE_2	2100
int checkTime = 600;
unsigned long checkFullTime = 0;
unsigned long checkEmptyTime = 0;
const int intControl = 5;//声音信号
const int outControl = 6;//主炮信号
int fullState = 0;
int emptyState = 0;
// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	pinMode(intControl, INPUT);
	myservo.attach(outControl);
	myservo.writeMicroseconds(MID_GRADE);
	checkFullTime = millis();
	checkEmptyTime = millis();
}

// Add the main program code into the continuous loop() function
void loop()
{
	int cState = getState();
	switch (cState)
	{
	case 1://上浮 排水
		if (getWaterFull())
			setPump();
		else
			setPump(cState);
		break;
	case 2://下潜 进水 
		if (getWaterEmpty())
			setPump();
		else
			setPump(cState);
		break;
	default:
		setPump();
		break;
	}
	setPump(1);

	delay(5);
}

int getState()
{
	delay(5);
	int control = pulseIn(intControl, HIGH);
	if (control > LOW_GRADE_1 && control < LOW_GRADE_2)
	{
		//上浮 排水
		return 1;
	}
	else if (control > MID_GRADE_1 && control < MID_GRADE_2)
	{
		return 0;
	}
	else if (control > HIGH_GRADE_1 && control < HIGH_GRADE_2)
	{
		//下潜 进水 
		return 2;
	}
	else
	{
		return 0;
	}
}

void setPump(int pumpState = 0)
{
	switch (pumpState)
	{
	case 0:
		myservo.writeMicroseconds(MID_GRADE);
		break;
	case 1://排水
		myservo.writeMicroseconds(LOW_GRADE_2);
		break;
	case 2://进水
		myservo.writeMicroseconds(HIGH_GRADE_1);
		delay(10);
		myservo.writeMicroseconds(MID_GRADE);
		delay(10);
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
	int hi = analogRead(A0);

	switch (fullState)
	{
	case 1://full
		if (hi < 200 && (mills - checkFullTime)>checkTime)
		{
			fullState=0;
			checkFullTime = mills;
		}
		else if (hi > 200)
		{
			checkFullTime = mills;
		}
		break;
	case 0:
		if (hi > 200 && (mills - checkFullTime)>checkTime)
		{
			fullState = 1;
			checkFullTime = mills;
		}
		else if (hi < 200)
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
	int hi = analogRead(A1);

	switch (emptyState)
	{
	case 1://empty
		if (hi < 200 && (mills - checkEmptyTime)>checkTime)
		{
			emptyState = 0;
			checkEmptyTime = mills;
		}
		else if (hi > 200)
		{
			checkEmptyTime = mills;
		}
		break;
	case 0:
		if (hi > 200 && (mills - checkEmptyTime)>checkTime)
		{
			emptyState = 1;
			checkEmptyTime = mills;
		}
		else if (hi > 200)
		{
			checkEmptyTime = mills;
		}
		break;
	default:
		break;
	}

	return emptyState;
}