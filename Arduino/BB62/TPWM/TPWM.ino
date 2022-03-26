// Visual Micro is in vMicro>General>Tutorial Mode
//
/*
Name:       TPWM.ino
Created:	2018/7/3 19:06:05
Author:     MSI\MingCH
*/

// Define User Types below here or use a .h file
//

// Define Function Prototypes that use User Types below here or use a .h file
//

// Define Functions below here or use other .ino or cpp files
//

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
int HZ = 100;
#define LOW_GRADE_1 900
#define LOW_GRADE_2 1100
#define MID_GRADE_1 1400
#define MID_GRADE_2 1600
#define HIGH_GRADE_1 1900
#define HIGH_GRADE_2 2100
const short POS_MIN = 120 * (HZ / 50);
const short POS_MILL = 260 * (HZ / 50);
const short POS_MAX = 400 * (HZ / 50);
const byte PWM_PIN = 7;
const byte gun_PIN = 0;

float moveangle = 0.1;
float i = 0;
byte start = 0; // 0 左 1 右 2 停 3 中
byte digital_Pin[2] = {0, 0};
float anglePos[16] = {0};
unsigned long checkTime[2] = {0, 0};
volatile int pwm_value[2] = {0, 0};
volatile int prev_time[2] = {0, 0};

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup()
{
	Serial.begin(9600);
	Serial.println("16 channel PWM test!");

	pinMode(PWM_PIN, INPUT_PULLUP);
	pinMode(gun_PIN, INPUT_PULLUP);
	// when pin D2 goes high, call the rising function
	digital_Pin[0] = digitalPinToInterrupt(PWM_PIN);
	digital_Pin[1] = digitalPinToInterrupt(gun_PIN);
	attachInterrupt(digital_Pin[0], rising_0, RISING);
	attachInterrupt(digital_Pin[1], rising_1, RISING);
	for (int i = 0; i < 16; i++)
		anglePos[i] = POS_MILL;

	pwm.begin();
	pwm.setPWMFreq(HZ); // This is the maximum PWM frequency
	delay(500);
}

void rising_0()
{
	attachInterrupt(digital_Pin[0], falling_0, FALLING);
	prev_time[0] = micros();
}

void falling_0()
{
	attachInterrupt(digital_Pin[0], rising_0, RISING);
	pwm_value[0] = micros() - prev_time[0];
	if (pwm_value[0] > HIGH_GRADE_2)
		return;
	// Serial.print("pwm_value0-");
	//  Serial.println(pwm_value[0]);
}

void rising_1()
{
	attachInterrupt(digital_Pin[1], falling_1, FALLING);
	prev_time[1] = micros();
}

void falling_1()
{
	attachInterrupt(digital_Pin[1], rising_1, RISING);
	pwm_value[1] = micros() - prev_time[1];
	if (pwm_value[1] > HIGH_GRADE_2)
		return;
	// Serial.print("pwm_value1-");
	// Serial.println(pwm_value[1]);
}

void loadStart()
{
	if (pwm_value[0] > LOW_GRADE_1 && pwm_value[0] < LOW_GRADE_2)
	{
		//右转,时间记录
		if (start != 0)
			checkTime[0] = micros();

		start = 0;
		i = i - 0.5;
	}
	else if (pwm_value[0] > HIGH_GRADE_1 && pwm_value[0] < HIGH_GRADE_2)
	{
		//左转,时间记录
		if (start != 1)
			checkTime[0] = micros();

		start = 1;
		i = i + 0.5;
	}
	else if (pwm_value[0] > MID_GRADE_1 && pwm_value[0] < MID_GRADE_2)
	{
		// 2 停 3 中
		if (start != 2 && start != 3)
		{
			//状态切换
			//大500ms 暂停，反之回中
			if (micros() - checkTime[0] > 500000)
			{
				start = 2;
			}
			else
			{
				start = 3;
				i = 0;
			}
			//暂停或回中时间记录
			checkTime[0] = micros();
		}
	}

	if (i >= 3600)
		i = 0;

	if (i < 0)
		i = 3600;
}

// Add the main program code into the continuous loop() function
void loop()
{
	loadStart();
	// Serial.println(start);

	// return;
	switch (start)
	{
	case 0: //左
	case 1: //右
	case 2: //暂停
		controlServoFront(i);
		controlServoBack(i);
		break;
	case 3: //回中
		controlServoFront(0);
		controlServoBack(1800);
		break;
	default:
		break;
	}
	// Serial.println(i);
}

void controlServoFront(float Fpos)
{
	//前有效0-135 225-360
	if (Fpos >= 0 && Fpos <= 1350)
	{
		// Serial.println(map(Fpos, 0, 1350, POS_MILL, POS_MIN));
		// 0-135
		//右转
		controlServo(0, map(Fpos, 0, 1350, POS_MILL, POS_MIN));
		controlServo(4, map(Fpos, 0, 1350, POS_MILL, POS_MIN));
	}
	else if (Fpos >= 2250 && Fpos <= 3600)
	{
		// 225-360
		//左转
		controlServo(0, map(Fpos, 2250, 3600, POS_MAX, POS_MILL));
		controlServo(0, map(Fpos, 2250, 3600, POS_MAX, POS_MILL));
	}
	else if (Fpos > 1800 && Fpos < 2250)
	{
		//左死区
		controlServo(0, POS_MAX);
		controlServo(4, POS_MAX);
	}
	else if (Fpos > 1350 && Fpos < 1800)
	{
		//右死区
		controlServo(0, POS_MIN);
		controlServo(4, POS_MIN);
	}
	else
	{
		//无效
		;
	}
}

void controlServoBack(float Bpos)
{
	//后有效 45-315
	if (Bpos >= 450 && Bpos <= 3150)
		controlServo(8, map(Bpos, 450, 3150, POS_MAX, POS_MIN));
	else if (Bpos > 3150 && Bpos < 3600)
		controlServo(8, POS_MAX);
	else if (Bpos > 0 && Bpos < 450)
		controlServo(8, POS_MIN);
	else
		;
}

int controlServo(int gunIndex, int pos)
{
	// Serial.print("anglePos ");
	// Serial.print(anglePos[gunIndex]);
	// Serial.print("pos ");
	Serial.println(pos);
	float temp = pos - anglePos[gunIndex];

	if (temp > 0)
		temp = anglePos[gunIndex] + moveangle;
	else if (temp < 0)
		temp = anglePos[gunIndex] - moveangle;
	else
		temp = pos;

	if (POS_MAX < temp || POS_MIN > temp)
		return temp;

	anglePos[gunIndex] = temp;

	pwm.setPWM(gunIndex, 0, temp);

	return temp;
}