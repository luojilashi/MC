#pragma once
#define _DEBUG_ARD 1
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

#define _START_LOOP checkTime[0] = micros();
#define _END_LOOP delay_time(40);

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
unsigned long pitch_time[12] = {0};

#define DATA(type, name, def) \
    bool send##name = false;  \
    type m_##name = def;      \
    const type &get_##name()  \
    {                         \
        return m_##name;      \
    };                        \
    void set_##name(type var) \
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

double reverse_mapdouble(double y, double in_min, double in_max, double out_min, double out_max)
{
    return (y - out_min) * (in_max - in_min) / (out_max - out_min) + in_min;
}

double LinearEquationIn2Unknowns(double x, double a, double b)
{
    return x * a + b;
}

void loadStart(int index)
{
    if (pwm_value[index] > LOW_GRADE_1 && pwm_value[index] < LOW_GRADE_2)
    {
        // 右转,时间记录
        if (get_start() != '0')
            checkTime[3] = millis();

        set_start('0');
    }
    else if (pwm_value[index] > HIGH_GRADE_1 && pwm_value[index] < HIGH_GRADE_2)
    {
        // 左转,时间记录
        if (get_start() != '1')
            checkTime[3] = millis();

        set_start('1');
    }
    else if (pwm_value[index] > MID_GRADE_1 && pwm_value[index] < MID_GRADE_2)
    {
        // 2 停 3 中
        if (get_start() != '2' && get_start() != '3')
        {
            // 状态切换
            // 大200ms 暂停，反之回中
            if (millis() - checkTime[3] > 200)
            {
                set_start('2');
            }
            else
            {
                set_start('3');
            }
            // 暂停或回中时间记录
            checkTime[3] = millis();
        }
    }
}

void delay_time(unsigned long ms)
{
    uint32_t start = micros() - checkTime[0];
    _println_log(start);
    start = ms - start / 1000;
    if (start > 0)
        delay(start);
    checkTime[0] = micros();
}