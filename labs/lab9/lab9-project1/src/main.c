#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <stdlib.h>         // C library. Needed for number conversions
#include <lcd.h>            // Peter Fleury's LCD library
#include <uart.h>           // Peter Fleury's UART library
#include <util/delay.h>     // Functions for busy-wait delay loops

#define SW   PD2            // Pin D2  - Digital pin for button on Joystick
#define LED  PB5            // Pin D13 - LED indicate
#define PINX PC0            // Pin A0  - Analog pin for X coordinate of Joystick
#define PINY PC1            // Pin A1  - Analog pin for Y coordinate of Joystick

#define SW1  PB2            // Pin D10 - Digital pin for button on encoder
#define DT   PB3            // Pin D11 - Digital pin for DT encoder pin
#define CLK  PB4            // Pin D12 - Digital pin for CLK encoder pin


/* Main function -----------------------------------------------------*/
/**********************************************************************
 * Define pins, initialize USART and LCD display, setting ADC conversion
 * Using Timer/Counter1, intterrupts and starting infinit loop
 **********************************************************************/

/*Global variables----------------------------------------------------*/
    uint8_t symbol = 0x2a;                      // Symbol value
    uint8_t line = 0;                           // Constant for LCD lines (0-15)    | uint8_t range is 0 to 255
    uint8_t column = 0;                         // Constant for LCD columns (0-1)
    uint8_t pinALast;                           // Last value of CLK pin on the encoder

int main(void)
{
    GPIO_mode_input_pullup(&DDRD, SW);              // Set pin for Joystick button, where on-board LED is connected as input with pullup resistor
    GPIO_mode_input_pullup(&DDRC, PINX);            // Set pin X coordinate of Joystick, where on-board LED is connected as input with pullup resistor
    GPIO_mode_input_pullup(&DDRC, PINY);            // Set pin Y coordinate of Joystick, where on-board LED is connected as input with pullup resistor
    GPIO_mode_output(&DDRB, LED);                   // Set pin for LED, where on-board LED is connected as output

    GPIO_mode_input_pullup(&DDRB, SW1);             // Set pin for Encoder button
    GPIO_mode_input_nopullup(&DDRB, DT);            // Set DT pin for Encoder 
    GPIO_mode_input_nopullup(&DDRB, CLK);           // Set CLK pin for Encoder 

    pinALast = GPIO_read(&PINB, CLK);               // Remembers the last encoder position

    uart_init(UART_BAUD_SELECT(9600, F_CPU));       // Initialize USART to asynchronous, 8N1, 9600
    lcd_init(LCD_DISP_ON);                          // Initialize LCD display without any cursor

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
    TIM1_overflow_33ms();                       
    TIM1_overflow_interrupt_enable();           
   
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

ISR(TIMER1_OVF_vect)
{
    ADCSRA |= (1<<ADSC);                            // Start ADC conversion     
    
    uint8_t aVal;                                   // Actual value of CLK pin
    aVal = GPIO_read(&PINB, CLK);                   // Reading actual position of encoder

    if (!GPIO_read(&PINB, SW1))                     // Encoder button reading condition
        {
            GPIO_write_high(&PORTB, LED);           // Turning on the LED (just indicate that button on encoder is pressed)        
            symbol = 0x21;                          // First value, which is a symbol `!`
        }

    if (aVal != pinALast)                           // The knob is rotating, if the knob is rotating, we need to determine direction, we do that by reading pin B
        {
            if (GPIO_read(&PINB, DT) != aVal)       // Means pin A Changed first - We're Rotating Clockwise
            {
                if(symbol < 0xff)                   // The condition when the symbol can reach the maximum value, i.e. 255
                {
                    symbol++;                       // Incrementing SYMBOL by 1, it's actually just next symbol from ASCII table
                }
                else 
                {
                    symbol = 0x21;                  // Start value, because there are a lot of user symbols on the beginning (range is 21 to 255)
                }
            } 
            else                                    // Otherwise B changed first and we're moving CCW
            {
                if (symbol > 0x21)                  // Moving on another side
                {
                    symbol--;                       // Reduction SYMBOL by 1, which is previous symbol from the table
                }
                else 
                {
                symbol = 0xff;                      // End value (255)
                }
            }
            pinALast = aVal;                        // Assigning the read value to the last one to determine the direction of rotation
        }
        
    lcd_gotoxy(line, column);                       // Going to x, y position on LCD
    lcd_putc(symbol);                               // Writing the definite symbol         
        
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
    
    static uint8_t marker = 0;                      // One time using constant for position of first symbol 
    
    uint16_t value;                                 // Constant which shows 2 direction for ADC (0-1024)          | uint16_t range is 0 to 32 767
    char string[4];                                 // String for converted numbers by itoa() | ADC conversion
    char str[4];                                    // String for converted numbers by itoa() | UART printing

    if (marker == 0)                                // Inicialized only ones when program is started
    {
        marker = 1;                                 // Incrementing MARKER to value which will never reach 
        lcd_gotoxy(line, column);                   // Going to x,y position on LCD, 16x2
        lcd_putc(symbol);                           // Writes a symbol to the designated cell 
    }

    if (!GPIO_read(&PIND, SW))                      // Joystick button reading condition
    {
        GPIO_write_high(&PORTB, LED);               // Turning on the LED (just indicate that button is pressed)

        lcd_gotoxy(line, column);           
        lcd_putc(0xef);                             // Writing the definite symbol
    }

    switch (ADMUX)                                  // Important condition, which needs to define ports between ADC0 and ADC1 for ADC Conversion (it's all a last digit) 
    {
        case 0b01000000:                            // Turning on the port ADC0 that has amount 0100 0000        
        value = ADC;                                // Read converted value

        itoa(value, string, 10);                    // Convert VALUE to STRING at Decimal 
        if (value > 900)                            // Condition if we are moving to the Right side on LCD
        {
            GPIO_write_high(&PORTB, LED);           // Turning on the LED

            lcd_clrscr();                           // Clear LCD display
            _delay_ms(50);
                                                    // Incrementing LINE by 1
            if (line < 16)                          // Condition of movement on a row to the right
            {
                line++;
                lcd_gotoxy(line, column);
                lcd_putc(symbol);                
            }
                      
            _delay_ms(50);                          // Delay to return the button to its original position
        }
        if (value < 100)                            // Condition if we are moving to the Left side on LCD
        {
            GPIO_write_high(&PORTB, LED);

            lcd_clrscr();
            _delay_ms(50);
                                                    // Reduction LINE by 1
            if (line > 0)                          // Condition of movement on a row to the left
            {
                line--;
                lcd_gotoxy(line, column);
                lcd_putc(symbol);                
            }
             
            _delay_ms(50);
        }
        ADMUX = 0b01000001;                         // At the end of the loop, change port ADC0 to ADC1

        itoa(line, str, 10);                        // Convert LINE to STR at Decimal
        uart_puts("Line is: ");                     // Write on UART
        uart_puts(str);                             // Display the row on UART        
        break;                                      // Stop the first condition of CASE


        case 0b01000001:                            // Turning on the port ADC1 that has amount 0100 0001
        value = ADC;                                // Read converted value
       
        itoa(value, string, 10);   
        if (value > 900)                            // Condition if we are moving to the down on LCD
        {
            GPIO_write_high(&PORTB, LED);

            lcd_clrscr();
            _delay_ms(50);
                                                    // Incrementing COLUMN by 1. Actually LCD has just 2 columns, which is 0 and 1
            if (column < 2)                         // Condition if we are changing column on LCD
            {
                column++;
                lcd_gotoxy(line, column);
                lcd_putc(symbol);                
            }
                     
            _delay_ms(50);
        }
        if (value < 100)                            // Condition if we are moving to the up on LCD
        {
            GPIO_write_high(&PORTB, LED);

            lcd_clrscr();
            _delay_ms(50);
                                                    // Reduction COLUMN by 1
            if (column > 0)
            {
                column--;
                lcd_gotoxy(line, column);
                lcd_putc(symbol);                
            }
                       
            _delay_ms(50);
        }        
        ADMUX = 0b01000000;                         // Again change port from ADC1 to ADC0

        itoa(column, str, 10);
        uart_puts("      Column is: ");
        uart_puts(str);                             // Display the column on UART
        uart_puts(" \r\n");                         // Go to the new line and move the cursor in the beginning position of line
        break;                                      // Stop the second condition of CASE

        default:                                    // Each case should have the default condition which is empty
        break;
    }
}