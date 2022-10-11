/***********************************************************************
 * 
 * Blink LEDs in Arduino-style and use function from the delay library.
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2022 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Defines -----------------------------------------------------------*/
#define LED_GREEN PB5   // PB5 is AVR pin where green on-board LED
                        // is connected
#define LED_RED PB0     // External active-low LED
#define SHORT_DELAY 250 // Delay in milliseconds
#ifndef F_CPU
# define F_CPU 16000000 // CPU frequency in Hz required for delay funcs
#endif

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>     // AVR device-specific IO definitions
#include <util/delay.h> // Functions for busy-wait delay loops
#include <gpio.h>


// -----
// This part is needed to use Arduino functions but also physical pin
// names. We are using Arduino-style just to simplify the first lab.
#define PB5 5          // In Arduino world, PB5 is called "13"
#define PB0 0
// -----


/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Toggle LEDs and use delay library.
 * Returns:  none
 **********************************************************************/
int main(void)
{
    
    // Set pin where on-board LED is connected as output
    GPIO_mode_output(&DDRB, LED_GREEN);  // Set output mode in DDRB reg
    // Set second pin as output
    GPIO_mode_output(&DDRB, LED_RED);  // Set output mode in DDRB reg

    // Infinite loop
    while (1)
    {
        // Turn ON/OFF on-board LED ...
        GPIO_write_toggle(&PORTB, LED_GREEN);   // Toggle output in PORTB reg
        // ... and external LED as well
        GPIO_write_toggle(&PORTB, LED_RED);   // Toggle output in PORTB reg

        // Pause several milliseconds
        _delay_ms(SHORT_DELAY);

      
    }

    // Will never reach this
    return 0;
}