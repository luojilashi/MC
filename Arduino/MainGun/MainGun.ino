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
#include <Servo.h>


#define LOW_GRADE_1   750
#define LOW_GRADE_2   1150
#define MID_GRADE_1   1400
#define MID_GRADE_2   1600
#define HIGH_GRADE_1  1850
#define HIGH_GRADE_2  2250
Servo ServoS[2];
int anglePos[2];

int POS_MIN = 600;
int POS_MILL = 1500;
int POS_MAX = 2400;


int pos = 0;
int i = 0;
int start = 4;//1 宸� 2 鍙� 3 鍋� 4 涓�
const int gunControl = 9;//涓荤偖淇″彿
unsigned long checkTime = 0;
unsigned long dayTime = 20;
const int postMove = 10;
void setup()
{
	Serial.begin(57600);//
	pinMode(gunControl, INPUT);//
	pinMode(2, INPUT);//
	pinMode(3, OUTPUT);//
	digitalWrite(3 ,HIGH);
	delay(100);
	int value  = digitalRead(2);
	if(HIGH == value)
	{
		POS_MIN = 500;
		POS_MILL = 1350;
		POS_MAX = 2100;
	}
	ServoS[0].attach(10);
	ServoS[1].attach(11);
	anglePos[0] = POS_MILL;
	anglePos[1] = POS_MILL;
	ServoS[0].writeMicroseconds(POS_MILL);
	ServoS[1].writeMicroseconds(POS_MILL);
	delay(400);
}

// Add the main program code into the continuous loop() function
void loop()
{
	pos = pulseIn(gunControl, HIGH);
  Serial.println(POS_MILL);
	if (pos > LOW_GRADE_1 && pos < LOW_GRADE_2)
	{
		//鍙宠浆,鏃堕棿璁板綍
		if (start != 1)
			checkTime = millis();

		start = 1;
		i = i - postMove;
	}
	else if (pos > HIGH_GRADE_1 && pos < HIGH_GRADE_2)
	{
		//宸﹁浆,鏃堕棿璁板綍
		if (start != 2)
			checkTime = millis();

		start = 2;
		i = i + postMove;

	}
	else if (pos > MID_GRADE_1 && pos < MID_GRADE_2)
	{
		//3 鍋� 4 涓�
		if (start != 3 && start != 4)
		{
			//鐘舵�佸垏鎹�
			//澶�500ms 鏆傚仠锛屽弽涔嬪洖涓�
			if (millis() - checkTime > 500)
			{
				start = 3;
			}
			else
			{
				start = 4;
				i = 0;
			}
			//鏆傚仠鎴栧洖涓椂闂磋褰�
			checkTime = millis();
		}
	}

	if (i >= 3600)
		i = 0;

	if (i < 0)
		i = 3600;

	switch (start)
	{
	case 1://宸�
	case 2://鍙�
	case 3://鏆傚仠
    controlServoFront(i);
		controlServoBack(i);
		break;
	case 4://鍥炰腑
    controlServoFront(0);
		controlServoBack(1800);
		break;
	default:
		break;
	}

	delay(dayTime);
}

void controlServoFront(int Fpos)
{
	//鍓嶆湁鏁�0-135 225-360
	if (Fpos >= 0 && Fpos <= 1350)
	{
		//0-135
		//鍙宠浆
		controlServo(0, map(Fpos, 0, 1350, POS_MILL, POS_MIN));
	}
	else if (Fpos >= 2250 && Fpos <= 3600)
	{
		//225-360
		//宸﹁浆
		controlServo(0, map(Fpos, 2250, 3600, POS_MAX, POS_MILL));
	}
	else if (Fpos > 1800 && Fpos < 2250)
	{
		//宸︽鍖�
		controlServo(0, POS_MAX);
	}
	else if (Fpos > 1350 && Fpos < 1800)
	{
		//鍙虫鍖�
		controlServo(0, POS_MIN);
	}
	else
	{
		//鏃犳晥
		;
	}
}

void controlServoBack(int Bpos)
{

	//鍚庢湁鏁� 45-315
	if (Bpos >= 450 && Bpos <= 3150)
		controlServo(1, map(Bpos, 450, 3150, POS_MAX, POS_MIN));
	else if (Bpos > 3150 && Bpos < 3600)
		controlServo(1, POS_MIN);
	else if (Bpos > 0 && Bpos < 450)
		controlServo(1, POS_MAX);
	else
		;
}

int controlServo(int gunIndex, int pos)
{

	int temp = ((pos - anglePos[gunIndex]) / postMove);

	if (temp > 0)
		temp = anglePos[gunIndex] + postMove;
	else if (temp < 0)
		temp = anglePos[gunIndex] - postMove;
	else
		temp = pos;

	anglePos[gunIndex] = temp;
	ServoS[gunIndex].writeMicroseconds(temp);

	return  temp;
}


