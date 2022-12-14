# Lab 1: MARIAN DVOŘÁČEK

### Morse code

1. Listing of C code which repeats one "dot" and one "comma" (BTW, in Morse code it is letter `A`) on a LED. Always use syntax highlighting, meaningful comments, and follow C guidelines:

```c
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
```

2. Scheme of Morse code application, i.e. connection of AVR device, LED, resistor, and supply voltage. The image can be drawn on a computer or by hand. Always name all components and their values!

   ![scheme](https://github.com/Dvoracma/Digital-electronics-2-2022/blob/main/labs/lab1/lab%201.png)
