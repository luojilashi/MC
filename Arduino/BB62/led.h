#pragma once
#include "Arduino.h"
#include "PCF8575.h"

// Set i2c address
PCF8575 led(0x20);

void load_all_PCF8575()
{
    // Set pinMode to OUTPUT
    led.pinMode(P0, OUTPUT);
    led.pinMode(P1, OUTPUT);
    led.pinMode(P2, OUTPUT);
    led.pinMode(P3, OUTPUT);
    led.pinMode(P4, OUTPUT);
    led.pinMode(P5, OUTPUT);
    led.pinMode(P6, OUTPUT);
    led.pinMode(P7, OUTPUT);
    led.pinMode(P8, OUTPUT);
    led.pinMode(P9, OUTPUT);
    led.pinMode(P10, OUTPUT);
    led.pinMode(P11, OUTPUT);
    led.pinMode(P12, OUTPUT);
    led.pinMode(P13, OUTPUT);
    led.pinMode(P14, OUTPUT);
    led.pinMode(P15, OUTPUT);
}
