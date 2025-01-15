#include "pico/stdlib.h"
#include <stdio.h>

#define LIMIT_SW 17
#define LIMIT_SW_LED 19

volatile bool led_on = false;   // Flag to indicate LED state
volatile uint64_t press_time = 0;   // Timestamp of the button press

// Initialize the GPIO for the button and LED
static void led_init(void) {
    gpio_init(LIMIT_SW);
    gpio_set_dir(LIMIT_SW, GPIO_IN);
    gpio_pull_up(LIMIT_SW);             // Enable internal pull-up resistor
    gpio_init(LIMIT_SW_LED);
    gpio_set_dir(LIMIT_SW_LED, GPIO_OUT);
    gpio_put(LIMIT_SW_LED, false);      // Ensure LED starts off
}

// Interrupt callback function for switch press
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == LIMIT_SW) {
        if (gpio_get(LIMIT_SW) == 0) {          // Switch pulled low (button pressed)
            led_on = true;                      //  flag to turn the LED on
            press_time = time_us_64();          // button press time
            printf("Switch pressed! LED ON\n");
        }
    }
}

int main(void) {
    
    stdio_init_all();
    sleep_ms(2000); 
    led_init();                             // Initialize LED and button


    gpio_set_irq_enabled_with_callback(LIMIT_SW, GPIO_IRQ_EDGE_FALL, true, &gpio_callback); // interrupt for falling edge

    while (true) {
        if (led_on) {
            gpio_put(LIMIT_SW_LED, true);              
            if (time_us_64() - press_time > 4000000) {              // if 4 seconds have elapsed
                gpio_put(LIMIT_SW_LED, false);                       // Turn off the LED
                led_on = false;                                  // Reset the flag
                printf("LED OFF\n");
            }
        }
        sleep_ms(10); 
    }

    return 0;
}
