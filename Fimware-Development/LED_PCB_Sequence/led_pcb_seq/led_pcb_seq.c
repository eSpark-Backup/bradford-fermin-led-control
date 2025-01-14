#include "pico/stdlib.h"
#include <stdio.h>

#define LIMIT_SW 17
#define LIMIT_SW_LED 16

// Initialize the GPIO for the button and LED
static void led_init(void) { 
    gpio_init(LIMIT_SW);
    gpio_set_dir(LIMIT_SW, GPIO_IN);
    gpio_pull_up(LIMIT_SW);   // Enable internal pull-up resistor
    gpio_init(LIMIT_SW_LED);
    gpio_set_dir(LIMIT_SW_LED, GPIO_OUT);
    gpio_put(LIMIT_SW_LED, false); // Ensure LED starts off
}

// Interrupt callback function for switch press
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == LIMIT_SW) {
        if (gpio_get(LIMIT_SW) == 0) { // Switch pulled low (button pressed)
            gpio_put(LIMIT_SW_LED, true); // Turn on the LED
            printf("Switch pressed! LED ON\n");
        } else { // Switch released (button not pressed)
            gpio_put(LIMIT_SW_LED, false); // Turn off the LED
            printf("Switch released! LED OFF\n");
        }
    }
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000); // Wait for serial connection to initialize
    printf("Program started. Waiting for switch press...\n");

    led_init(); // Initialize LED and button

    // Enable interrupt for both rising and falling edges (button press/release)
    gpio_set_irq_enabled_with_callback(LIMIT_SW, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while (true) {
        printf("running \n\r");
        sleep_ms(1000); // Sleep to reduce CPU usage
    }

    return 0; 
}
