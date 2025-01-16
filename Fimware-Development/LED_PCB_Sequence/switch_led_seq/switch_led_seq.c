#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ws2812.pio.h"

// GPIO Pin Definitions
#define LIMIT_SW 17                // Limit switch
#define LIMIT_SW_LED 16            // Limit switch LED
#define LED_1 12                   // RGB LED
#define LED_2 11                   // RGB LED2
#define LED_BRD_SWITCH 18          // Reset switch/LED board switch

//config
#define NUM_LEDS 10          // Number of LEDs in the strip
#define IS_RGBW false        // Set true if the LED strip supports RGBW


// Constants
const uint64_t DURATION = 3000000; // 50 seconds in microseconds

// States
typedef enum {
    IDLE,
    ANIMATION_1,
    ANIMATION_2,
} STATE;

// Variables
STATE current_state = IDLE;
uint8_t button_count = 0;
bool time_window_active = false;
uint64_t start_time = 0;
volatile bool limit_sw_triggered = false;    // Flag for LIMIT_SW
volatile bool led_brd_sw_triggered = false; // Flag for LED_BRD_SWITCH

// Function Prototypes
static void task_in_progress(PIO pio, uint sm, int num_leds);
static void task_done(PIO pio, uint sm, int num_leds);
static void idle_state(PIO pio, uint sm, int num_leds);
static uint32_t hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v);
static void led_init(void);
static void reset_callback(uint gpio, uint32_t events);
static void state_transition(void);
static void gpio_callback(uint gpio, uint32_t events);

static void put_pixel(uint32_t pixel_grb) {
    // Send the 24-bit GRB color data to the LED strip
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u); // Shift left to align with WS2812 protocol
}

// Clear all LEDs
static void clear_leds(PIO pio, uint sm, int num_leds) {
    for (int i = 0; i < num_leds; i++) {
        put_pixel(0x000000);
    }
    sleep_ms(50);
}

// Animation 1: Task in Progress (Running Dot)
static void task_in_progress(PIO pio, uint sm, int num_leds) {
        while (!limit_sw_triggered && !led_brd_sw_triggered) { // Exit if any interrupt is triggered
            button_count = 0;
            for (int i = 0; i < num_leds; i++) {
                for (int j = 0; j < num_leds; j++) {
                    put_pixel(j == i ? 0x00FF00 : 0x000000); // Green dot
                }
                sleep_ms(100);
            }
            for (int i = num_leds - 1; i >= 0; i--) {
                for (int j = 0; j < num_leds; j++) {
                    put_pixel(j == i ? 0x00FF00 : 0x000000); // Green dot
                }
                sleep_ms(100);
            }
         }
        limit_sw_triggered = false;   // Reset the flag after breaking the loop
        led_brd_sw_triggered = false; // Reset the flag after breaking the loop
}

// Animation 2: Task Done (Success Flash)
static void task_done(PIO pio, uint sm, int num_leds) {
    while (!limit_sw_triggered && !led_brd_sw_triggered) { // Exit if any interrupt is triggered
        button_count = 0;
        for (int i = 0; i < 3; i++) { // Flash three times
            if (limit_sw_triggered || led_brd_sw_triggered) break; // Check before each iteration
            for (int j = 0; j < num_leds; j++) {
                put_pixel(0x00FF00); // All green
            }
            sleep_ms(200);
            clear_leds(pio, sm, num_leds);
            sleep_ms(200);
        }
    }
    limit_sw_triggered = false;   // Reset the flag after breaking the loop
    led_brd_sw_triggered = false; // Reset the flag after breaking the loop
}

// Animation 3: Idle State (Pulsing Glow)
static void idle_state(PIO pio, uint sm, int num_leds) {
    // while(1){
        button_count = 0;
        for (int brightness = 0; brightness <= 255; brightness += 5) {
            uint32_t color = ((0x0000FF * brightness / 255) & 0x0000FF); // Blue pulsing
            for (int i = 0; i < num_leds; i++) {
                put_pixel(color);
            }
            sleep_ms(30);
        }
        for (int brightness = 255; brightness >= 0; brightness -= 5) {
            uint32_t color = ((0x0000FF * brightness / 255) & 0x0000FF); // Blue pulsing
            for (int i = 0; i < num_leds; i++) {
                put_pixel(color);
            }
            sleep_ms(30);
        }
    // }
}

// HSV to RGB Conversion
static uint32_t hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    uint8_t r, g, b;
    switch (region) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    return (r << 16) | (g << 8) | b; // Combine into GRB format
}
// Initialize GPIOs
static void led_init(void) {
    gpio_init(LIMIT_SW);
    gpio_set_dir(LIMIT_SW, GPIO_IN);
    gpio_pull_up(LIMIT_SW);

    gpio_init(LIMIT_SW_LED);
    gpio_set_dir(LIMIT_SW_LED, GPIO_OUT);

    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);

    gpio_init(LED_2);
    gpio_set_dir(LED_2, GPIO_OUT);

    gpio_init(LED_BRD_SWITCH);
    gpio_set_dir(LED_BRD_SWITCH, GPIO_IN);
    gpio_pull_up(LED_BRD_SWITCH);
}

// State Transition
static void state_transition(void) {
    switch (button_count) {
        case 0:
            current_state = IDLE;
            break;
        case 1:
            current_state = ANIMATION_1;
            break;
        case 2:
            current_state = ANIMATION_2;
            break;

        default:
            button_count = 0; // Reset count if it exceeds the defined states
            current_state = IDLE;
            break;
    }
}

// GPIO Callback for Button
static void shared_callback(uint gpio, uint32_t events) {
    if (gpio == LIMIT_SW) {
        time_window_active = true;
        static uint64_t last_time = 0;  // Debounce timer
        gpio_put(LIMIT_SW_LED, true); // Turn on limit switch LED indicating button pressed
        uint64_t now = time_us_64();

        if (now - last_time > 200000 && time_window_active) { // 200ms debounce
            if (button_count < 2) {
                button_count++;
            }
            limit_sw_triggered = true; // Set flag for LIMIT_SW
            printf("Button pressed! Count: %d\n", button_count);
        }
        last_time = now;
    } else if (gpio == LED_BRD_SWITCH) {
        printf("Reset called\n\r");
        current_state = IDLE;
        button_count = 0;
        led_brd_sw_triggered = true; // Set flag for LED_BRD_SWITCH
        printf("System reset.\n");
    }
}

// Main Function
int main(void) {
    stdio_init_all();
    sleep_ms(2000); // Allow time for serial initialization
    PIO pio = pio0;
    int sm = 0;
    led_init();
    printf("LED initialized\n\r");

    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), LED_1, 800000, IS_RGBW);
    
    gpio_set_irq_enabled_with_callback(LIMIT_SW, GPIO_IRQ_EDGE_FALL, true, &shared_callback);
    gpio_set_irq_enabled_with_callback(LED_BRD_SWITCH, GPIO_IRQ_EDGE_FALL, true, &shared_callback);

       
    while (true) {
    state_transition();

    switch (current_state) {
        case IDLE:
            idle_state(pio, sm, NUM_LEDS);       // Idle animation
            break;
        case ANIMATION_1:
            task_in_progress(pio, sm, NUM_LEDS); // Task in progress animation
            break;
        case ANIMATION_2:
            task_done(pio, sm, NUM_LEDS);        // Task done animation
            break;
    }
    gpio_put(LIMIT_SW_LED, false);
    limit_sw_triggered = false;   // Reset LIMIT_SW flag
    led_brd_sw_triggered = false; // Reset LED_BRD_SWITCH flag
    sleep_ms(1000); // Prevent CPU overload
    }
}
