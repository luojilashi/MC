
#define DEF_NODE_SST 0XF0
#define DEF_NODE_END 0XFF

uint8_t inBuffer[10];   // 接收指令缓冲区
int bufferIndex = 0;    // 接收指令字符计数
uint8_t ch;             // 临时单个字符存储
bool recv_flag = false; // 指令接收成功标志位
int CmdState;           // 指令接收状态

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello");
}

void Load_Node_Data()
{
  for (int i = 0; i < 10 && !recv_flag; i++)
  {
    Uart_Command_Rev();
  }

  if (recv_flag)
  {
    Serial.print(inBuffer[0]);
    Serial.print(inBuffer[1]);
    Serial.print(inBuffer[2]);
    Serial.print(inBuffer[3]);
    Serial.print(inBuffer[4]);
    Serial.print(inBuffer[5]);
    Serial.print(inBuffer[6]);
    Serial.write("\n");

    switch (Serial.print(inBuffer[1]))
    {
    case 1:
    {
    }
    break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      break;

    default:
      break;
    }

    memset(inBuffer, 0x00, sizeof(inBuffer));
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
      // Serial.println("end");
    }
    break;
    }
  }
}

void loop()
{
  //  put your main code here, to run repeatedly:
  Load_Node_Data();
}