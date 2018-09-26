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

const int intControl = 5;//声音信号
const int outControl = 6;//主炮信号
// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(9600);
	pinMode(intControl, INPUT);
	myservo.attach(outControl);
	myservo.writeMicroseconds(MID_GRADE);
}

// Add the main program code into the continuous loop() function
void loop()
{
	int control = pulseIn(10, HIGH);
	int hi = analogRead(A0);
	int gi = analogRead(A1);
	Serial.print("A0:");
	Serial.print(hi);
	Serial.print("  A1:");
	Serial.println(control);

	if (control<2050&&control > 1600)
	{
		if(hi<200)
		{
			myservo.writeMicroseconds(1070);
		}
		else
		{
			myservo.writeMicroseconds(1500);
		}
	}
	else if (control < LOW_GRADE_2 && control>LOW_GRADE_1)
	{
		if (myservo.readMicroseconds() < 1900)
		{
			myservo.writeMicroseconds(1930);
			delay(100);
			myservo.writeMicroseconds(1500);
			delay(100);
			myservo.writeMicroseconds(1930);
		}		
	}
	else if(control<MID_GRADE_2 && control>MID_GRADE_1)
	{
		myservo.writeMicroseconds(1500);
	}

	delay(50);

}

int getStart()
{
	delay(10);
	int control = pulseIn(intControl, HIGH);
	if (control > LOW_GRADE_1 && control < LOW_GRADE_2)
	{
		return 1;
	}
	else if (control > MID_GRADE_1 && control < MID_GRADE_2)
	{
		return 2;
	}
	else if (control > HIGH_GRADE_1 && control < HIGH_GRADE_2)
	{
		return 3;
	}
	else
	{
		return 0;
	}
}

