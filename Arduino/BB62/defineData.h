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
#define _println_int(...)      \
    {                          \
        Serial1.begin(115200); \
    }
#define _println_log(...)             \
    {                                 \
        Serial1.println(__VA_ARGS__); \
    }
#define _print_log(...)             \
    {                               \
        Serial1.print(__VA_ARGS__); \
    }
#else
#define _println_log(...) \
    {                     \
    }
#define _print_log(...) \
    {                   \
    }
#define _println_int() \
    {                  \
    }
#endif

#define _START_LOOP          \
    checkTime[0] = micros(); \
    digitalWrite(13, HIGH);

#define _END_LOOP delay_time(50);

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

#define REGISTERED_FUNC(indexNo, pin) \
    digital_Pin[indexNo] = pin;       \
    pinMode(pin, INPUT_PULLUP);       \
    attachInterrupt(pin, rising_##indexNo, RISING);

byte digital_Pin[4] = {0, 0, 0, 0};
uint32_t pwm_value[4] = {0, 0, 0, 0};
uint32_t prev_time[4] = {0, 0, 0, 0};
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

void delay_time(unsigned long ms)
{
    uint32_t start = micros() - checkTime[0];
    // _println_log(start);
    digitalWrite(13, LOW);
    start = ms - start / 1000;
    if (start > 0)
        delay(start);
    checkTime[0] = micros();
}
