
/***********************************************************************
 * 
 * Blink a LED in Arduino-style and use function from the delay library.
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2022 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Defines -----------------------------------------------------------*/
#define LED_GREEN PB5   // PB5 is AVR pin where green on-board LED
#define LED_RED PB0 
                        // is connected
#define SHORT_DELAY 250 // Delay in milliseconds
#define LONG_DELAY 750 // Delay in milliseconds
#define SPACE_DELAY 1500 // Delay in milliseconds
#ifndef F_CPU
# define F_CPU 16000000 // CPU frequency in Hz required for delay funcs
#endif

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>     // AVR device-specific IO definitions
#include <util/delay.h> // Functions for busy-wait delay loops


// -----
// This part is needed to use Arduino functions but also physical pin
// names. We are using Arduino-style just to simplify the first lab.
#include "Arduino.h"
#define PB0 8          // In Arduino world, PB0 is called "8"
#define PB5 13          // In Arduino world, PB5 is called "13"
// -----


/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Toggle one LED and use delay library.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    uint8_t led_value = LOW;  // Local variable to keep LED status
    uint8_t last_delay = LOW; // Local variable to keep delay status

    // Set pin where on-board LED is connected as output
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_RED, OUTPUT);


    // Infinite loop
    while (1)
    {
        // Change LED value
        if (led_value == LOW)
            led_value = HIGH;
        else
            led_value = LOW;

        // Pause several milliseconds
        if(led_value == HIGH && last_delay == HIGH)
          {
            last_delay = LOW;
            _delay_ms(SHORT_DELAY);
          }
        else if(led_value == HIGH && last_delay == LOW)
          {
            last_delay = HIGH;
            _delay_ms(LONG_DELAY);
          }
        else
          _delay_ms(SHORT_DELAY);
        // Turn ON/OFF on-board LED
        digitalWrite(LED_GREEN, led_value);
        digitalWrite(LED_RED, led_value);
    }

    // Will never reach this
    return 0;
}
