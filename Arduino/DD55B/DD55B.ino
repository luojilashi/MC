// Visual Micro is in vMicro>General>Tutorial Mode
//
/*
Name:       DD55B.ino
Created:	2018/6/5 23:16:26
Author:     MSI\MingCH
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
// The setup() function runs once each time the micro-controller starts
#include <Servo.h>    //Servo.h

#define LOW_GRADE_1		750
#define LOW_GRADE_2		1150
#define MID_GRADE_1		1400
#define MID_GRADE_2		1600
#define HIGH_GRADE_1	1850
#define HIGH_GRADE_2	2250
#define TIME_OUT		25000L
#define AI		0X1
#define AII		0X2
#define AIII	0X4
#define AIV		0X8

Servo servoFront;     //主炮
Servo myservo;
const int poutSignal = 8;//舱门舵机
const int poutGun = 11;//主炮舵机
const int pinControl = 9;//控制信号
const int songControl = 12;//声音信号
const int gunControl = 10;//主炮信号
unsigned long checkGunTime = 0;
unsigned long checkCMTime = 0;
unsigned long acTime = 0;
int controlState = 0;// 控制状态 0 关闭 1 开 2 发射
int gunState = 0;//主炮状态 0 回中 1 左 2 右 3 停
int soundState = 0;//声音状态 0 ... 1 汽笛 2 随机 3 停
int CMState = 0;
int FSState = 0;
unsigned long control = 0;	//控制信号
int pos = 1000;          //舵机脉冲
int gunpos = 1500;

int MissileNumber = 1;
const int MissileSize = 16;
const int cA0 = 2;
const int cA1 = 3;
const int cA2 = 4;
const int cA3 = 5;
const int cINH = 6;
const int cIE = 7;

uint8_t bA0 = LOW;
uint8_t bA1 = LOW;
uint8_t bA2 = LOW;
uint8_t bA3 = LOW;
bool bProtect = true;

void setup()
{
	Serial.begin(9600);//设置波特率
	mp3_set_serial(Serial);    //set Serial for DFPlayer-mini mp3 module
	mp3_set_volume(25);
	for (int i = 2; i < 8; i++)
	{
		pinMode(i, OUTPUT);
	}
	digitalWrite(cIE, HIGH);
	digitalWrite(cINH, HIGH);
	digitalWrite(cA0, LOW);
	digitalWrite(cA1, LOW);
	digitalWrite(cA2, LOW);
	digitalWrite(cA3, LOW);
	pinMode(songControl, INPUT);//声音
	pinMode(pinControl, INPUT);//控制
	pinMode(gunControl, INPUT);
	myservo.attach(poutSignal);		//舱门
	servoFront.attach(poutGun);		//主炮
	myservo.writeMicroseconds(pos);//初始化
	servoFront.writeMicroseconds(gunpos);//初始化

	delay(5);
	control = pulseIn(pinControl, HIGH, TIME_OUT);
	if (!control)
		controlState = -1;

	delay(5);
	control = pulseIn(gunControl, HIGH, TIME_OUT);
	if (!control)
		gunState = -1;

	delay(5);
	control = pulseIn(songControl, HIGH, TIME_OUT);
	if (!control)
		soundState = -1;

	delay(100);
	bProtect = true;

}

// Add the main program code into the continuous loop() function
void loop()
{
	int time = 50 - millis() + acTime;
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
	/*else
	{
		if (millis() - checkGTime > 5000)
		{
			control = pulseIn(gunControl, HIGH, TIME_OUT);
			if (control > LOW_GRADE_1 && control < HIGH_GRADE_2)
			{
				getGunStart();

				HandleGun();
			}
			else
			{
				checkGTime = millis();
			}
		}
	}*/

	if (controlState != -1)
	{
		getControlStart();

		HandleControl();
	}
	/*else
	{
		if (millis() - checkCTime > 5000)
		{
			control = pulseIn(gunControl, HIGH, TIME_OUT);
			if (control > LOW_GRADE_1 && control < HIGH_GRADE_2)
			{
				getControlStart();

				HandleControl();
			}
			else
			{
				checkCTime = millis();
			}
		}
	}*/

	if (soundState != -1)
	{
		getSoundStart();

		soundState = setSong(soundState);
	}
	/*else
	{
		if (millis() - checkSTime > 5000)
		{
			control = pulseIn(songControl, HIGH, TIME_OUT);
			if (control > LOW_GRADE_1 && control < HIGH_GRADE_2)
			{
				getSoundStart();

				soundState = setSong(soundState);
			}
			else
			{
				checkSTime = millis();
			}
		}
	}*/
}

int LaunchMissile(int no)
{
	if (no > 0 && no <= MissileSize)
	{
		bA0 = ((no - 1)&AI) != AI ? LOW : HIGH;
		bA1 = ((no - 1)&AII) != AII ? LOW : HIGH;
		bA2 = ((no - 1)&AIII) != AIII ? LOW : HIGH;
		bA3 = ((no - 1)&AIV) != AIV ? LOW : HIGH;
		no++;
		//Serial.print("MissileNumber ");
		//Serial.print(MissileNumber);
		//Serial.print("|| ");
		//Serial.print(bA0);
		//Serial.print(" ");
		//Serial.print(bA1);
		//Serial.print(" ");
		//Serial.print(bA2);
		//Serial.print(" ");
		//Serial.print(bA3);
		//Serial.println(no);

		digitalWrite(cA0, bA0);
		digitalWrite(cA1, bA1);
		digitalWrite(cA2, bA2);
		digitalWrite(cA3, bA3);
		digitalWrite(cINH, LOW);
		delay(3000);
	}
	//关闭发射写保护
	digitalWrite(cINH, HIGH);
	return no;
}

int setSong(int st)
{
	if (controlState > 0 && !bProtect)
		st =  0;
	
	switch (st & 0X0F)
	{
	case 0://...
		   Serial.println("setSong...");
		break;
	case 1://1 汽笛
		   Serial.println("setSong 1");
		mp3_play(2);
		delay(50);
		//mp3_single_loop(true);
		break;
	case 2://随机
		   Serial.println("setSong 2");
		mp3_next();
		delay(50);
		//mp3_single_loop(false);
		break;
	case 3://停
		   Serial.println("setSong 3");
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

void getControlStart()
{
	//delay(4);
	control = pulseIn(pinControl, HIGH);
	//关闭
	//0 关闭 1 开启 2 发射
	if (control > LOW_GRADE_1 && control < LOW_GRADE_2)//900 - 1150
		controlState = 0;
	else if (control > MID_GRADE_1 && control < MID_GRADE_2)
		controlState = 1;
	else if (control > HIGH_GRADE_1 && control < HIGH_GRADE_2)
		controlState = 2;
	else if (control > HIGH_GRADE_2 || control < LOW_GRADE_1)
	{
		controlState = -1;
		bProtect = true;
	}
	else
		;

	if (controlState == 0)
	{
		bProtect = false;
	}

	//Serial.print("soundState");
	//Serial.println(control);
}

void HandleGun()
{

	if (controlState != 3)
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

	/*Serial.print("gun ");
	Serial.print(gunpos);
	Serial.print(" gunSt ");
	Serial.println(gunState);*/
}

void HandleControl()
{
	if (bProtect)
		return;

	switch (controlState)
	{
	case 0:
		//关
		digitalWrite(cINH, HIGH);
		FSState = 0;
		if (CMState && pos > 1000)
		{
			pos = pos - 25;
			myservo.writeMicroseconds(pos);
			delay(20);
			pos = pos - 25;
			myservo.writeMicroseconds(pos);
			CMState = 3;
		}
		else if (CMState == 3 && pos <= 1000)
		{
			CMState = 0;
			if (pos != 1000)
			{
				pos = 1000;
				myservo.writeMicroseconds(pos);
			}
		}
		checkCMTime = millis();

		break;
	case 1:
	{
		//开舱门
		digitalWrite(cINH, HIGH);
		if (CMState != 2 && pos < 2000)
		{
			if (!CMState)
			{
				mp3_play(1);
			}

			CMState = 1;
			pos = pos + 25;
			myservo.writeMicroseconds(pos);
			delay(20);
			pos = pos + 25;
			myservo.writeMicroseconds(pos);
			checkCMTime = millis();
			FSState = 0;
		}
		else if (CMState == 1 && (millis() - checkCMTime) > 1000 && pos >= 2000)
		{
			CMState = 2;
			FSState = 1;

			if (pos != 2000)
			{
				pos = 2000;
				myservo.writeMicroseconds(2000);
			}
		}

		break;
	}
	case 2://发射
	{
		if (CMState != 2)
			break;

		//发射
		if (FSState == 1)
		{
			MissileNumber = LaunchMissile(MissileNumber);
		}

		break;
	}
	default:
		break;
	}
}
