// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       MainGun.ino
    Created:	2018/7/30 22:02:52
    Author:     DESKTOP-VSGKPOG\luoji
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts
#include <Servo.h>
#define LOW_GRADE_1		750
#define LOW_GRADE_2		1150
#define MID_GRADE_1		1400
#define MID_GRADE_2		1600
#define HIGH_GRADE_1	1850
#define HIGH_GRADE_2	2250
const int POS_MIN = 600;
const int POS_MILL = 1500;
const int POS_MAX = 2400;
int i = 0;
int pos = 0;
int Bpos = 0;
int Fpos = 0;
int start = 0;//1 左 2 右 3 停 4 中
unsigned long checkTime = 0;
Servo servoFront;
Servo servoBack;

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(56000);//
	pinMode(9, INPUT);//
	servoFront.attach(10);
	servoBack.attach(11);
	servoFront.writeMicroseconds(POS_MILL);
	servoBack.writeMicroseconds(POS_MILL);
	delay(500);
}

// Add the main program code into the continuous loop() function
void loop()
{
	pos = pulseIn(9, HIGH);

	if (pos > LOW_GRADE_1 && pos < LOW_GRADE_2)
	{
		//右转,时间记录
		if (start != 1)
			checkTime = millis();

		start = 1;
		i = i - 5;
	}
	else if (pos > HIGH_GRADE_1 && pos < HIGH_GRADE_2)
	{
		//左转,时间记录
		if (start != 2)
			checkTime = millis();

		start = 2;
		i = i + 5;
	}
	else if(pos > MID_GRADE_1 && pos < MID_GRADE_2)
	{
		//3 停 4 中
		if (start != 3 && start != 4)
		{
			//状态切换
			//大500ms 暂停，反之回中
			if (millis() - checkTime > 500)
				start = 3;
			else
				start = 4;
			//暂停或回中时间记录
			checkTime = millis();
		}
	}

	if (i >= 3600)
		i = 0;

	if (i < 0)
		i = 3600;

	switch (start)
	{
	case 1://左
		controlServoFront();
		controlServoBack();
		break;
	case 2://右
		controlServoFront();
		controlServoBack();
		break;
	case 3://暂停
		break;
	case 4://回中
		controlServoFront();
		controlServoBack();
		break;
	default:
		break;
	}

	delay(30);
}

void controlServoFront()
{


	//前有效0-135 225-360
	if (Fpos >= 0 && Fpos <= 1350)
	{
		//0-135
		//右转
		servoFront.writeMicroseconds(map(Fpos, 0, 1350, POS_MILL, POS_MIN));
	}
	else if (Fpos >= 2250 && Fpos <= 3600)
	{
		//225-360
		//左转
		servoFront.writeMicroseconds(map(Fpos, 2250, 3600, POS_MAX, POS_MILL));
	}
	else if(Fpos > 1350 && Fpos < 1800)
	{
		//130- 180 之间不动
		servoFront.writeMicroseconds(POS_MIN);
	}
	else if(Fpos > 1800 && Fpos < 2250)
	{
		servoFront.writeMicroseconds(POS_MAX);
	}
	else
		;
}

void controlServoBack()
{
	//后有效 45-315
	if (Bpos >= 450 && Bpos <= 3150)
	{
		servoBack.writeMicroseconds(map(Bpos, 450, 3150, POS_MAX, POS_MIN));
	}
	else if (Bpos > 0 && Bpos <450)
	{
		servoBack.writeMicroseconds(POS_MAX);
	}
	else if (Bpos > 3150 && Bpos < 3600)
	{
		servoBack.writeMicroseconds(POS_MIN);
	}
	else
		//无效
		;
}

int MainGunBack(int backPos)
{

}
