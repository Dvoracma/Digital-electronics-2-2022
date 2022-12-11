#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <stdlib.h>         // C library. Needed for number conversions
#include <lcd.h>            // Peter Fleury's LCD library
#include <uart.h>           // Peter Fleury's UART library
#include <util/delay.h>     // Functions for busy-wait delay loops
#include <string.h>         //standard library for strings


#define SW   PD2            // Pin D2  - Digital pin for button on Joystick
#define LED  PB5            // Pin D13 - LED indicate
#define PINX PC0            // Pin A0  - Analog pin for X coordinate of Joystick
#define PINY PC1            // Pin A1  - Analog pin for Y coordinate of Joystick

// global variables

//PWM limit values
const uint16_t min_servo_v = 61; // min PWM value for vertical servo
const uint16_t min_servo_h = 61; // min PWM value for horizontal servo
const uint16_t max_servo_v = 126; // max PWM value for vertical servo
const uint16_t max_servo_h = 126; // max PWM value for horizontal servo

const uint16_t min_v_servo_angle = 0; // min angle of vertical servo in deegrees
const uint16_t max_v_servo_angle = 180; // max angle of vertical servo in deegrees
const uint16_t min_h_servo_angle = 0; // min angle of horizontal servo in deegrees
const uint16_t max_h_servo_angle = 180; // max angle of horizontal servo in deegrees

uint16_t servo_v;                 // PWM value variable for vartical servo
uint16_t servo_h;                 // PWM value variable for horizontal servo

/**********************************************************************
 * Function: convertAngleToDeegrees()
 * Purpose:  Convert value in PWM to angle in deegrees.
 * Input(s): PWM_value - Actual value of PWM.
 *           PWM_min_value - Minimal possible value of PWM.
 *           PWM_max_value - Maximal possible value of PWM.
 *           min_angle_of_servo - minimal possible angle of servo rotation
 *           max_angle_of_servo - maximal possible angle of servo rotation
 * Returns:  angle in deegrees
 **********************************************************************/
uint16_t convertAngleToDeegrees(uint16_t PWM_value, uint16_t PWM_min_value, uint16_t PWM_max_value, uint16_t min_angle_of_servo, uint16_t max_angle_of_servo)
{
    uint16_t countOfSteps;          // available count of PWM steps
    int16_t actualPWMValue;         // actual PWM step
    float oneStepAngle;             // angle of one PWM step
    float convertedAngle;           // converted PWM step to angle

    countOfSteps = PWM_max_value - PWM_min_value;
    oneStepAngle = (float) max_angle_of_servo / (float) countOfSteps;
    actualPWMValue = PWM_value - PWM_min_value;

    if (actualPWMValue >= 0 && actualPWMValue <= PWM_max_value)
    {
        convertedAngle = (float) actualPWMValue * oneStepAngle;
    }
    else if (actualPWMValue < 0)
    {
        convertedAngle = min_angle_of_servo;
    }
    else
    {
        convertedAngle = max_angle_of_servo;
    }
    
    
    return (uint16_t) round(convertedAngle);
}

/* Main function -----------------------------------------------------*/
/**********************************************************************
 * Define pins, initialize USART and LCD display, setting ADC conversion
 * Using Timer/Counter1, intterrupts and starting infinit loop
 **********************************************************************/
int main(void)
{
    GPIO_mode_input_pullup(&DDRD, SW);              // Set pin for Joystick button, where on-board LED is connected as input with pullup resistor
    GPIO_mode_input_nopullup(&DDRC, PINX);            // Set pin X coordinate of Joystick, where on-board LED is connected as input with pullup resistor
    GPIO_mode_input_nopullup(&DDRC, PINY);            // Set pin Y coordinate of Joystick, where on-board LED is connected as input with pullup resistor
    GPIO_mode_output(&DDRB, LED);                   // Set pin for LED, where on-board LED is connected as output

    // set PWM init values
    servo_v = min_servo_v;                           // init PWM value for vartical servo
    servo_h = min_servo_h;                           // init PWM value for horizontal servo

    // uart_init(UART_BAUD_SELECT(9600, F_CPU));       // Initialize USART to asynchronous, 8N1, 9600
    lcd_init(LCD_DISP_ON);                          // Initialize LCD display without any cursor
    lcd_gotoxy(0,0);

    // lcd init
    lcd_puts("ANGLE V: 0 deg");
    lcd_gotoxy(0,1);
    lcd_puts("ANGLE H: 0 deg");

    // Configure Analog-to-Digital Convertion unit
    // Select ADC voltage reference to "AVcc with external capacitor at AREF pin"
    ADMUX |=  (1<<REFS0); 
    ADMUX &= ~(1<<REFS1);       

    // Select input channel ADC0 (voltage divider pin)
    ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0));        

    // Enable ADC module
    ADCSRA |= (1<<ADEN);

    // Enable conversion complete interrupt
    ADCSRA |= (1<<ADIE);

    // Set clock prescaler to 128
    ADCSRA |= ((1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2));    
    
    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 33 ms and enable overflow interrupt
    TIM2_overflow_16ms();                      
    TIM2_overflow_interrupt_enable(); 

    //PWM settings
    // reset timer settings
    TCCR1A = 0;
    TCCR1B = 0;
    // set PWM pin as output
    DDRB |= (1<<PB2) | (1<<PB1);
    // set timer mode Phase Correct PWM, 10 bit
    TCCR1A &= ~(1<<WGM13)| (1<<WGM12); TCCR1A |= (1<<WGM11) | (1<<WGM10);       // Phase Correct PWM 10 bit with TOP=0x03FF
    // set output mode
    TCCR1A |= (1<<COM1A1);                  // non-inverting mode for OCR1A
    TCCR1A |= (1<<COM1B1);                  // non-inverting mode for OCR1B
    // set prescaller
    TCCR1B &= ~((1<<CS11) | (1<<CS10)); TCCR1B |= (1<<CS12);        // 256          
   
    // Enables interrupts by setting the global interrupt mask
    sei();

    
    // Infinite loop
    while (1)       
    {          
    // All actions are performed in this loop    
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Use single conversion mode and start conversion every 100 ms.
 **********************************************************************/

ISR(TIMER2_OVF_vect)
{
    ADCSRA |= (1<<ADSC);                             // Start ADC conversion 
                                   
}

/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Display converted value on LCD screen.
 **********************************************************************/

ISR(ADC_vect)
{
    GPIO_read(&PIND, SW);                           // Reading data from Joystick Button
    GPIO_read(&PINC, PINX);                         // Reading value (voltage) from Joystick X coordinate
    GPIO_read(&PINC, PINY);                         // Reading value (voltage) from Joystick Y coordinate
    GPIO_write_low(&PORTB, LED);                    // Turning off LED port or low level
        
    uint16_t value;                                 // Constant which shows 2 direction for ADC (0-1024)          | uint16_t range is 0 to 32 767
    char angle[3];                                  // Constant which shows angle string on LCD
    //char string[4];                               // String for converted numbers by itoa() | ADC conversion
    //char str[4];
    
    if (!GPIO_read(&PIND, SW))                      // Joystick button reading condition
    {
        GPIO_write_high(&PORTB, LED);               // Turning on the LED (just indicate that button is pressed)
    }

    switch (ADMUX)                                  // Important condition, which needs to define ports between ADC0 and ADC1 for ADC Conversion (it's all a last digit) 
    {
        case 0b01000000:                            // Turning on the port ADC0 that has amount 0100 0000        
        value = ADC;                                // Read converted value

        //itoa(value, string, 10);                  // Convert VALUE to STRING at Decimal 
        if (value > 900)                            // Condition if we are moving to the Right side on LCD
        {
            GPIO_write_high(&PORTB, LED);           // Turning on the LED

            if(servo_v < max_servo_v)               // increase angle of vertical servo
            {
                lcd_gotoxy(9,0);                    // show vertical angle on LCD
                lcd_puts("       ");
                lcd_gotoxy(9,0);
                servo_v++;
                itoa(convertAngleToDeegrees(servo_v, min_servo_v, max_servo_v, min_v_servo_angle, max_v_servo_angle), angle, 10);
                lcd_puts(strcat(angle, " deg"));                                        
            }
        }
        if (value < 100)                            
        {
            GPIO_write_high(&PORTB, LED);           // Turning on the LED

            if(servo_v > min_servo_v)               // decrease angle of vertical servo
            {
                lcd_gotoxy(9,0);                    // show vertical angle on LCD
                lcd_puts("       ");
                lcd_gotoxy(9,0);
                servo_v--;
                itoa(convertAngleToDeegrees(servo_v, min_servo_v, max_servo_v, min_v_servo_angle, max_v_servo_angle), angle, 10);
                lcd_puts(strcat(angle, " deg"));                                        
            }            
        }
        ADMUX = 0b01000001;                         // At the end of the loop, change port ADC0 to ADC1

        //itoa(line, str, 10);                      // Convert LINE to STR at Decimal
        //uart_puts("Line is: ");                   // Write on UART
        //uart_puts(str);                           // Display the row on UART        
        break;                                      // Stop the first condition of CASE


        case 0b01000001:                            // Turning on the port ADC1 that has amount 0100 0001
        value = ADC;                                // Read converted value
       
        //itoa(value, string, 10);   
        if (value > 900)                            
        {
            GPIO_write_high(&PORTB, LED);           // Turning on the LED
            
            if(servo_h < max_servo_h)               // increase angle of horizontal servo
            {
                lcd_gotoxy(9,1);                    // show horizontal angle on LCD
                lcd_puts("       ");
                lcd_gotoxy(9,1);
                servo_h++;
                itoa(convertAngleToDeegrees(servo_h, min_servo_h, max_servo_h, min_h_servo_angle, max_h_servo_angle), angle, 10);
                lcd_puts(strcat(angle, " deg"));                                        
            }
        }
        if (value < 100)
        {   GPIO_write_high(&PORTB, LED);           // Turning on the LED
            
            if(servo_h > min_servo_h)               // decrease angle of horizontal servo
            {
                lcd_gotoxy(9,1);                    // show horizontal angle on LCD
                lcd_puts("       ");
                lcd_gotoxy(9,1);
                servo_h--;
                itoa(convertAngleToDeegrees(servo_h, min_servo_h, max_servo_h, min_h_servo_angle, max_h_servo_angle), angle, 10);
                lcd_puts(strcat(angle, " deg"));                                        
            }
        }        
        ADMUX = 0b01000000;                         // Again change port from ADC1 to ADC0

        //itoa(column, str, 10);
        //uart_puts("      Column is: ");
        //uart_puts(str);                           // Display the column on UART
        //uart_puts(" \r\n");                       // Go to the new line and move the cursor in the beginning position of line
        break;                                      // Stop the second condition of CASE

        default:                                    // Each case should have the default condition which is empty
        break;
    }
        OCR1A = servo_v;                            // generate PWM for vertical servo
        OCR1B = servo_h;                            // generate PWM for horizontal servo
        _delay_ms(50);                              // wait to servo process the command 
}