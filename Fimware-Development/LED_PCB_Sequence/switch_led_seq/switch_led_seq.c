#include "pico/stdlib.h"
#include <stdio.h>

#define LED_DELAY_MS 100
#define LIMIT_SW 17
#define LIMIT_SW_LED 16
#define LED_1 12

const uint8_t limit_sw = LIMIT_SW;
const uint8_t limit_sw_led = LIMIT_SW_LED;

volatile uint8_t button_count = 0; // Button press counter
volatile bool time_window_active = true; // Flag for active time window
volatile uint64_t start_time; // Start time of the time window
const int64_t duration = 8000000;          // Duration in microseconds (8 seconds)

// Initialize the GPIO for the LED
static void led_init(void) {
  
    gpio_init(LIMIT_SW);
    gpio_set_dir(LIMIT_SW, GPIO_IN);
    gpio_pull_up(LIMIT_SW);   
    gpio_init(LIMIT_SW_LED);
    gpio_set_dir(LIMIT_SW_LED, GPIO_OUT);
    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);
    
}

// led state change
static void pico_set_led(bool led_on , const int gpio_num) {

    gpio_put(gpio_num, led_on);

}


void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == LIMIT_SW && time_window_active) {
        button_count++; // Increment the button press count
        printf("Button pressed! Count: %d\n", button_count);
    }
}


void main() {

    stdio_init_all();
    sleep_ms(2000);
    led_init();
    printf("LED initialized\n\r");
    
    
    gpio_set_irq_enabled_with_callback(LIMIT_SW, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    while(true) {
            time_window_active = true;
             while (time_window_active) {
            // Check if the time window has elapsed
                if (time_us_64() - start_time > duration) {
                    time_window_active = false; // End the time window
                }
             }
            
    
       
    }
}

