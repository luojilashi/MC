#pragma once
// #define _DEBUG_ARD 1
#define LOW_GRADE_1 900
#define LOW_GRADE_2 1100
#define MID_GRADE_1 1400
#define MID_GRADE_2 1600
#define HIGH_GRADE_1 1900
#define HIGH_GRADE_2 2100
#define DEF_NODE_SST 0XF0
#define DEF_NODE_END 0XFF

#ifdef _DEBUG_ARD
#define _println_log(var) Serial.println(var);
#define _print_log(var) Serial.print(var);
#else
#define _println_log(var)
#define _print_log(var)
#endif

#define RISING_FUNC(indexNo)                                               \
    void rising_##indexNo()                                                \
    {                                                                      \
        attachInterrupt(digital_Pin[indexNo], falling_##indexNo, FALLING); \
        prev_time[indexNo] = micros();                                     \
    }

#define FALLING_FUNC(indexNo)                                            \
    void falling_##indexNo()                                             \
    {                                                                    \
        attachInterrupt(digital_Pin[indexNo], rising_##indexNo, RISING); \
        pwm_value[indexNo] = micros() - prev_time[indexNo];              \
        if (pwm_value[indexNo] > HIGH_GRADE_2)                           \
            return;                                                      \
    }

#define REGISTERED_FUNC(indexNo) attachInterrupt(digital_Pin[indexNo], rising_##indexNo, RISING);

byte digital_Pin[4] = {0, 0, 0, 0};
volatile int pwm_value[4] = {0, 0, 0, 0};
volatile int prev_time[4] = {0, 0, 0, 0};
unsigned long checkTime[4] = {0, 0, 0, 0};

#define DATA(type, name, def) \
    bool send##name = false;  \
    type m_##name = def;      \
    const type &Get##name()   \
    {                         \
        return m_##name;      \
    };                        \
    void Set##name(type var)  \
    {                         \
        if (var == m_##name)  \
            return;           \
        m_##name = var;       \
        send##name = true;    \
    };

DATA(char, start, '0') // 0 左 1 右 2 停 3 中

double mapdouble(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double LinearEquationIn2Unknowns(double x, double a, double b)
{
    return x*a+b;
}

void loadStart(int index)
{
    if (pwm_value[index] > LOW_GRADE_1 && pwm_value[index] < LOW_GRADE_2)
    {
        // 右转,时间记录
        if (Getstart() != '0')
            checkTime[3] = micros();

        Setstart('0');
    }
    else if (pwm_value[index] > HIGH_GRADE_1 && pwm_value[index] < HIGH_GRADE_2)
    {
        // 左转,时间记录
        if (Getstart() != '1')
            checkTime[index] = micros();

        Setstart('1');
    }
    else if (pwm_value[index] > MID_GRADE_1 && pwm_value[index] < MID_GRADE_2)
    {
        // 2 停 3 中
        if (Getstart() != '2' && Getstart() != '3')
        {
            // 状态切换
            // 大200ms 暂停，反之回中
            if (micros() - checkTime[index] > 200000)
            {
                Setstart('2');
            }
            else
            {
                Setstart('3');
            }
            // 暂停或回中时间记录
            checkTime[index] = micros();
        }
    }
}
