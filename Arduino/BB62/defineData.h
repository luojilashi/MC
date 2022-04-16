#pragma once

#define LOW_GRADE_1 900
#define LOW_GRADE_2 1100
#define MID_GRADE_1 1400
#define MID_GRADE_2 1600
#define HIGH_GRADE_1 1900
#define HIGH_GRADE_2 2100
#define DEF_NODE_SST 0XF0
#define DEF_NODE_END 0XFF

#define RISING_FUNC(indexNo)                                           \
  void rising_##indexNo()                                              \
  {                                                                    \
    attachInterrupt(digital_Pin[indexNo], falling_##indexNo, FALLING); \
    prev_time[indexNo] = micros();                                     \
  }

#define FALLING_FUNC(indexNo)                                        \
  void falling_##indexNo()                                           \
  {                                                                  \
    attachInterrupt(digital_Pin[indexNo], rising_##indexNo, RISING); \
    pwm_value[indexNo] = micros() - prev_time[indexNo];              \
    if (pwm_value[indexNo] > HIGH_GRADE_2)                           \
      return;                                                        \
  }

#define REGISTERED_FUNC(indexNo) attachInterrupt(digital_Pin[indexNo], rising_##indexNo, RISING);

byte digital_Pin[4] = {0, 0, 0, 0};
volatile int pwm_value[4] = {0, 0, 0, 0};
volatile int prev_time[4] = {0, 0, 0, 0};
