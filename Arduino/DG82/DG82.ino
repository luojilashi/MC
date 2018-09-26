// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
Name:       DG82.ino
Created:  2018/7/21 23:49:16
Author:     DESKTOP-VSGKPOG\luoji
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts


// Define Functions below here or use other .ino or cpp files
//
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <Servo.h>    //Servo.


#define LOW_GRADE_1   750
#define LOW_GRADE_2   1150
#define MID_GRADE_1   1400
#define MID_GRADE_2   1600
#define HIGH_GRADE_1  1850
#define HIGH_GRADE_2  2250
#define TIME_OUT    25000L

Servo servoFront;     //主炮
const int poutGun = 10;//主炮舵机
const int songControl = 5;//声音信号
const int gunControl = 6;//主炮信号
unsigned long checkGunTime = 0;
unsigned long acTime = 0;
int gunState = 0;//主炮状态 0 回中 1 左 2 右 3 停
int soundState = 0;//声音状态 0 ... 1 汽笛 2 随机 3 停
unsigned long control = 0;  //控制信号
int gunpos = 1500;

void setup()
{
	Serial.begin(9600);//设置波特率
	mp3_set_serial(Serial);    //set Serial for DFPlayer-mini mp3 module
	mp3_set_volume(25);

	pinMode(songControl, INPUT);//声音
	pinMode(gunControl, INPUT);
	servoFront.attach(poutGun);   //主炮
	servoFront.writeMicroseconds(gunpos);//初始化

	delay(5);
	control = pulseIn(gunControl, HIGH);
	if (!control)
		gunState = -1;

	delay(5);
	control = pulseIn(songControl, HIGH, TIME_OUT);
	if (!control)
		soundState = -1;

	delay(100);

}

// Add the main program code into the continuous loop() function
void loop()
{
	int time = 40 - millis() + acTime;
	if (time < 0)
		time = 0;
	else
		delay(time);

	acTime = millis();

	if (gunState != -1)
	{
		getGunStart();

		HandleGun();
	}
	else
	{

		control = pulseIn(gunControl, HIGH, TIME_OUT);
		if (!control)
			gunState = -1;
		else
			gunState = 0;

	}

	if (soundState != -1)
	{
		getSoundStart();

		soundState = setSong(soundState);
	}
	else
	{
		control = pulseIn(songControl, HIGH, TIME_OUT);
		if (!control)
			soundState = -1;
		else
			soundState = 0;
	}
}



int setSong(int st)
{

	switch (st & 0X0F)
	{
	case 0://...
	//Serial.println("setSong...");
		break;
	case 1://1 汽笛
	//Serial.println("setSong 1");
		mp3_play(2);
		delay(200);
		mp3_single_loop(true);
		break;
	case 2://随机
	//Serial.println("setSong 2");
		mp3_next();
		delay(200);
		mp3_single_loop(false);
		break;
	case 3://停
	//Serial.println("setSong 3");
		mp3_stop();
		break;
	default:
		//Serial.println("setSong ???");
		mp3_stop();
		break;
	}
	return (st & 0XF0);
}

void getGunStart()
{

	//主炮
	//0 回中 1 左转 2 右转 3 停
	//delay(4);
	control = pulseIn(gunControl, HIGH);
	if (control > LOW_GRADE_1 && control < LOW_GRADE_2)//900 - 1150
	{
		//时间记录
		if (gunState != 1)
			checkGunTime = millis();

		gunState = 1;
	}
	else if (control > MID_GRADE_1 && control < MID_GRADE_2)
	{
		//中/停
		if (gunState != 3 && gunState != 0)
		{
			//中
			//大于500ms 为停
			if ((millis() - checkGunTime) > 200)
				gunState = 3;
			else
				gunState = 0;
			//时间记录
			checkGunTime = millis();
		}
	}
	else if (control > HIGH_GRADE_1 && control < HIGH_GRADE_2)
	{
		//左转
		if (gunState != 2)
			checkGunTime = millis();

		gunState = 2;
	}
	else if (control > HIGH_GRADE_2 || control < LOW_GRADE_1)
	{
		gunState = 3;
	}
	else
		;

}

void getSoundStart()
{

	//delay(4);
	control = pulseIn(songControl, HIGH);

	//声音信号
	//状态 0 ... 1 汽笛 2 随机 3 停
	if ((soundState & 0XF0) != 0X10 && control > LOW_GRADE_1 && control < LOW_GRADE_2) //900 - 1150
	{
		soundState = 0X11;
	}
	else if ((soundState & 0XF0) != 0X30 && control > MID_GRADE_1 && control < MID_GRADE_2)
	{
		soundState = 0X33;
	}
	else if ((soundState & 0XF0) != 0X20 && control > HIGH_GRADE_1 && control < HIGH_GRADE_2)
	{
		soundState = 0X22;
	}
	else if (control > HIGH_GRADE_2 || control < LOW_GRADE_1)
	{
		soundState = 0;
	}
	else
		;

}


void HandleGun()
{

	//在发射状态下无效
	switch (gunState)
	{
	case 0://回中
	{
		if (gunpos > 1510)
		{
			//Serial.print("1:");
			gunpos = gunpos - 20;
			servoFront.writeMicroseconds(gunpos);
		}
		else if (gunpos < 1490)
		{
			//Serial.print("2:");
			gunpos = gunpos + 20;
			servoFront.writeMicroseconds(gunpos);
		}
		else
		{
			//Serial.print("3:");
			if (gunpos != 1500)
			{
				gunState = 3;
				gunpos = 1500;
				servoFront.writeMicroseconds(1500);
			}
		}
		break;
	}
	case 1://左转
	{
		if (gunpos >= 2400)
		{
			if (gunpos != 2400)
			{
				gunpos = 2400;
				servoFront.writeMicroseconds(gunpos);
			}
		}
		else
		{
			gunpos = gunpos + 15;
			servoFront.writeMicroseconds(gunpos);
		}
		break;
	}
	case 2://右转
	{
		if (gunpos <= 600)
		{
			if (gunpos != 600)
			{
				gunpos = 600;
				servoFront.writeMicroseconds(gunpos);
			}
		}
		else
		{
			gunpos = gunpos - 15;
			servoFront.writeMicroseconds(gunpos);
		}
		break;
	}
	case 3://停
		break;
	case -1://不动
		break;
	default:
		servoFront.writeMicroseconds(1500);
		break;
	}
}

