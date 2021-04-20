// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       u_boat.ino
    Created:  2020/5/2 18:24:29
    Author:     DESKTOP-VSGKPOG\luoji
*/
#define DEBUG_ARD
// Define User Types below here or use a .h file
//
#include <Servo.h>

// Define Function Prototypes that use User Types below here or use a .h file
//
#define LOW_VOLTAGE_VALUE 640
#define MAX_DEPTH 300
#define DEF_DEPT_VALUE 250
#define LOW_GRADE_1   750
#define LOW_GRADE_2   1150
#define MID_GRADE_1   1400
#define MID_GRADE_2   1600
#define HIGH_GRADE_1  1850
#define HIGH_GRADE_2  2250
#define INPUT_SER 9
Servo myservo;
Servo myservo1;
int Yg=0;
int pos = 0;
unsigned char m_state =2;
unsigned char m_state_1 = 2;
int VoltageValue = 0;
int PressureValue = 0;
int DepthValue = 0;
int DeptValueTemp = 0;
double DeptTemp = 0;
unsigned long checkTime = 0;
unsigned long checkTime_1 = 0;

bool isLoW = false;
bool Maximumdepth = false;
bool PredeterminedDepth = false;
unsigned char Re_buf[11], counter = 0;
unsigned char sign = 0;
float angle[3], T;
// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts
void setup()
{
  Serial.begin(9600);// 
  pinMode(A0, INPUT); 
  pinMode(A1, INPUT); 
  pinMode(A2, INPUT); 
  pinMode(A3, INPUT); 
  checkTime = millis();
  checkTime_1 = millis();

  myservo.attach(9);
  myservo1.attach(10);
  myservo.writeMicroseconds(1500);
  myservo1.writeMicroseconds(1500);
  DeptTemp = double(DEF_DEPT_VALUE-160)/1024;
}

// Add the main program code into the continuous loop() function
void loop()
{
	VoltageValue = analogRead(A0);
	PressureValue = analogRead(A1);
	DepthValue = analogRead(A2);
	GetAttitude();
	if (LOW_VOLTAGE_VALUE > VoltageValue || isLoW)
	{
		FrontWaterTank(1);
		isLoW = true;
		return;
	}

	DeptValueTemp = DeptTemp*DepthValue + 160;
	if (MAX_DEPTH < PressureValue || Maximumdepth)
	{
		FrontWaterTank(1);
		Maximumdepth = true;

		PressureValue = analogRead(A1);

		if (PressureValue < 160)
		{
			Maximumdepth = false;
		}

		return;
	}

	if (DeptValueTemp < PressureValue)
	{
		PredeterminedDepth = true;
	}
	else
	{
		PredeterminedDepth = false;
	}

	pos = pulseIn(INPUT_SER, HIGH);

	if (LOW_GRADE_2 >= pos) {

		// 上浮
		FrontWaterTank(1);

		if (angle[0] > 15)
			BackWaterTank(1);
	}
	else if (LOW_GRADE_2 < pos && HIGH_GRADE_1 >= pos)
	{
		FrontWaterTank(2);
		BackWaterTank(2);
	}
	else if (HIGH_GRADE_1 < pos)
	{
		if (!PredeterminedDepth)
		{
			FrontWaterTank(3);
			if (angle[0] < -15)
				BackWaterTank(3);
		}
		else
		{
			FrontWaterTank(2);
			BackWaterTank(2);
		}	
	}
	else
	{
		return;
	}
}

void FrontWaterTank(int state)
{
	if (m_state == state)
	{
		if (m_state == 1 || m_state == 3)
		{
			if (isTimeOut(30000))
			{
				myservo.writeMicroseconds(1500);
				return;
			}
		}
	}

	//  Serial.println("asd");
	switch (state)
	{
	case 1:
		if (myservo.readMicroseconds() != 1100)
		{
			myservo.writeMicroseconds(1100);
		}
		else if(angle[0]<5)
		{
			myservo.writeMicroseconds(1500);
		}
		
		break;

	case 2:
		if (angle[0] > 5)
		{
			FrontWaterTank(3);
			return;
		}
		checkTime = millis();
		myservo.writeMicroseconds(1500);
		break;

	case 3:
		if (myservo.readMicroseconds() != 1900)
		{
			myservo.writeMicroseconds(1900);
		}
		else if (angle[0]>-5)
		{
			myservo.writeMicroseconds(1500);
		}
		break;

	default:
		checkTime = millis();
		myservo.writeMicroseconds(1500);
		break;
	}
	checkTime = millis();
	m_state = state;
}
 
void BackWaterTank(int state)
{

	if (m_state_1 == state)
	{
		if (m_state_1 == 1 || m_state_1 == 3)
		{
			if (isBackTimeOut(30000))
			{
				myservo1.writeMicroseconds(1500);
				return;
			}
		}
	}

	//  Serial.println("asd");
	switch (state)
	{
	case 1:
			myservo1.writeMicroseconds(1100);
		break;
	case 2:
		checkTime_1 = millis();
		if (angle[0] < -5)
			myservo1.writeMicroseconds(1900);

		myservo1.writeMicroseconds(1500);
		//    Serial.println("LOW");
		break;

	case 3:
		myservo1.writeMicroseconds(1900);
		break;

	default:
		checkTime_1 = millis();
		myservo1.writeMicroseconds(1500);
		break;
	}
	checkTime_1 = millis();
	m_state_1 = state;
}

bool isTimeOut(int times)
{
	if ((checkTime + times) >= millis())
	{
		return false;
	}
	return true;
}

bool isBackTimeOut(int times)
{
	if ((checkTime_1 + times) >= millis())
	{
		return false;
	}
	return true;
}

void GetAttitude()
{
	if (sign)
	{
		sign = 0;
		if (Re_buf[0] == 0x55)      //检查帧头
		{
			switch (Re_buf[1])
			{
			case 0x53:
				angle[0] = (short(Re_buf[3] << 8 | Re_buf[2])) / 32768.0 * 180;
				angle[1] = (short(Re_buf[5] << 8 | Re_buf[4])) / 32768.0 * 180;
				angle[2] = (short(Re_buf[7] << 8 | Re_buf[6])) / 32768.0 * 180;
				T = (short(Re_buf[9] << 8 | Re_buf[8])) / 340.0 + 36.25;
				Serial.print("a:");
				Serial.print("angle:");
				Serial.print(angle[0]); Serial.print(" ");
				Serial.print(angle[1]); Serial.print(" ");
				Serial.print(angle[2]); Serial.print(" ");
				Serial.print("T:");
				Serial.println(T);
				break;
			}
		}
	}
}

void serialEvent() {
	while (Serial.available()) {

		//char inChar = (char)Serial.read(); Serial.print(inChar); //Output Original Data, use this code 

		Re_buf[counter] = (unsigned char)Serial.read();
		if (counter == 0 && Re_buf[0] != 0x55) return;      //第0号数据不是帧头              
		counter++;
		if (counter == 11)             //接收到11个数据
		{
			counter = 0;               //重新赋值，准备下一帧数据的接收 
			sign = 1;
		}

	}
}