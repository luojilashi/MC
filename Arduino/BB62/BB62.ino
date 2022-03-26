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
#define DEF_NODE_SST 0XF0
#define DEF_NODE_END 0XFF

const short POS_MIN = 120 * (HZ / 50);
const short POS_MILL = 260 * (HZ / 50);
const short POS_MAX = 400 * (HZ / 50);
const byte PWM_PIN = 2;
const byte gun_PIN = 3;

float moveangle = 0.1;
float ipos = 0;
byte start = 0; // 0 左 1 右 2 停 3 中
byte digital_Pin[2] = {0, 0};
float anglePos[16] = {0};
unsigned long checkTime[2] = {0, 0};
volatile int pwm_value[2] = {0, 0};
volatile int prev_time[2] = {0, 0};

////////////////////////////////////////////

uint8_t inBuffer[10];	// 接收指令缓冲区
int bufferIndex = 0;	// 接收指令字符计数
uint8_t ch;				// 临时单个字符存储
bool recv_flag = false; // 指令接收成功标志位
int CmdState;			// 指令接收状态
////////////////////////////////////////////////////////////

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
}

void loadStart()
{
	if (pwm_value[0] > LOW_GRADE_1 && pwm_value[0] < LOW_GRADE_2)
	{
		//右转,时间记录
		if (start != 0)
			checkTime[0] = micros();

		start = 0;
		ipos = ipos - 0.5;
	}
	else if (pwm_value[0] > HIGH_GRADE_1 && pwm_value[0] < HIGH_GRADE_2)
	{
		//左转,时间记录
		if (start != 1)
			checkTime[0] = micros();

		start = 1;
		ipos = ipos + 0.5;
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
				ipos = 0;
			}
			//暂停或回中时间记录
			checkTime[0] = micros();
		}
	}

	if (ipos >= 3600)
		ipos = 0;

	if (ipos < 0)
		ipos = 3600;
}

// Add the main program code into the continuous loop() function
void loop()
{
	// loadStart();
	Load_Node_Data();
	Serial.println(start);

	// return;
	switch (start)
	{
	case 0: //左
	case 1: //右
	case 2: //暂停
		controlServoFront(ipos);
		controlServoBack(ipos);
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

void Load_Node_Data()
{
	for (int i = 0; i < 10; i++)
	{
		Uart_Command_Rev();
		if (recv_flag)
		{
			// Serial.println("end");
			break;
		}
	}

	if (recv_flag)
	{
		// for (int i = 0; i < 10; i++)
		// {
		// 	Serial.print("inBuffer[");
		// 	Serial.print(i);
		// 	Serial.print("]:{");
		// 	Serial.print(inBuffer[i]);
		// 	Serial.println("}   ");
		// }

		switch (inBuffer[1])
		{
		case '1':
		{
			switch (inBuffer[2])
			{
			case '0':
			{
				//右转,时间记录
				if (start != 0)
					checkTime[0] = micros();

				start = 0;
				ipos = ipos - 0.5;
			}
			break;
			case '1':
			{
				//左转,时间记录
				if (start != 1)
					checkTime[0] = micros();

				start = 1;
				ipos = ipos + 0.5;
			}
			break;
			case '2':
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
						ipos = 0;
					}
					//暂停或回中时间记录
					checkTime[0] = micros();
				}
			}
			break;
			default:
				break;
			}
			if (ipos >= 3600)
				ipos = 0;

			if (ipos < 0)
				ipos = 3600;
		}
		break;
		case 3:
		case 4:
		case 5:
		case 6:
			break;

		default:
			break;
		}
		checkTime[1] = micros();
		memset(inBuffer, 0x00, sizeof(inBuffer));
	}

	if (micros() - checkTime[1] > 5000000)
	{
		// 5s 内无信号
		start = 3;
		Serial.println("TRS lost error");

	}

	recv_flag = false;
}

void Uart_Command_Rev()
{

	while (Serial.available() > 0)
	{
		ch = Serial.read();
		// Serial.print("CmdState:[");
		// Serial.print(CmdState);
		// Serial.print("]ch:[");
		// Serial.print(ch);
		// Serial.println("]");
		switch (CmdState)
		{
		/******** start *********/
		case 0:
		{
			if (DEF_NODE_SST == ch)
			{
				bufferIndex = 0;
				CmdState = 1;
			}
		}
		break;
		/******** data *********/
		case 1:
		{
			if (DEF_NODE_END == ch)
			{
				CmdState = 2;
			}
			bufferIndex++;
			inBuffer[bufferIndex] = ch;
		}
		break;
		/******** end *********/
		case 2:
		{
			bufferIndex = 0;
			recv_flag = true;
			CmdState = 0;
		}
		break;
		}
	}
}
