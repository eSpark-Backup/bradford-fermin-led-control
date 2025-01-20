#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ws2812.pio.h"


// GPIO Pin Definitions
#define LIMIT_SW 17                // Limit switch
#define LIMIT_SW_LED 19            // Limit switch LED
#define LED_1 12                   // RGB LED
#define LED_2 11                   // RGB LED2
#define LED_BRD_SWITCH 18          // Reset switch/LED board switch

// Config
#define NUM_LEDS 10          // Number of LEDs in the strip
#define IS_RGBW false        // Set true if the LED strip supports RGBW
#define WINDOW_TIME  1500000 // Button press window time in microseconds(1.5 sec)git

// States
typedef enum {
    IDLE,
    PRINTING,
    PRINT_FINISHED,
    PAUSE,
    ERROR
} STATE;

STATE current_state = IDLE;
uint8_t button_count = 1;
bool press_window = false;
uint64_t press_window_start_time = 0;
volatile bool limit_sw_triggered = false;    // Flag for LIMIT_SW
volatile bool led_brd_sw_triggered = false; // Flag for LED_BRD_SWITCH

// Animation functions
static void idle_animation(PIO pio, uint sm, int num_leds);
static void printing_animation(PIO pio, uint sm, int num_leds);
static void print_finished_animation(PIO pio, uint sm, int num_leds);
static void pause_animation(PIO pio, uint sm, int num_leds);
static void error_animation(PIO pio, uint sm, int num_leds);

// Setup functions
static void put_pixel(uint32_t pixel_grb);
static uint32_t hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v);
static void clear_leds(PIO pio, uint sm, int num_leds);
static void led_init(void);
static void state_transition(void);

// ISR functions
static void shared_callback(uint gpio, uint32_t events);

// Utility functions
static bool debounce(uint gpio);

// Send color to LED
static void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Clear all LEDs
static void clear_leds(PIO pio, uint sm, int num_leds) {
    for (int i = 0; i < num_leds; i++) {
        put_pixel(0x000000);
    }
    sleep_ms(50);
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

static void idle_animation(PIO pio, uint sm, int num_leds) {
    while (current_state == IDLE && !limit_sw_triggered && !led_brd_sw_triggered) {
       // for (int brightness = 0; brightness <= 255; brightness += 5) {
            int brightness =255;
            uint32_t color = 0xFFFFFF * brightness / 255;           //static white
            for (int i = 0; i < num_leds; i++) put_pixel(color);
            sleep_ms(30);
        //}
        // for (int brightness = 255; brightness >= 0; brightness -= 5) {
        //     uint32_t color = 0xFFFFFF * brightness / 255;
        //     for (int i = 0; i < num_leds; i++) put_pixel(color);
        //     sleep_ms(30);
        // }
    }
}

static void printing_animation(PIO pio, uint sm, int num_leds) {
    while (current_state == PRINTING && !limit_sw_triggered && !led_brd_sw_triggered) {
        for (int brightness = 0; brightness <= 255; brightness += 5) {          //fade in/out green
            uint32_t color = 0xFF0000 * brightness / 255;
            for (int i = 0; i < num_leds; i++) put_pixel(color);
            sleep_ms(30);
        }
        for (int brightness = 255; brightness >= 0; brightness -= 5) {
            uint32_t color = 0xFF0000 * brightness / 255;
            for (int i = 0; i < num_leds; i++) put_pixel(color);
            sleep_ms(30);
        }
    }
}

static void print_finished_animation(PIO pio, uint sm, int num_leds) {
    while (current_state == PRINT_FINISHED  && !limit_sw_triggered && !led_brd_sw_triggered) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < num_leds; j++) put_pixel(0x0000FF);         //blinking blue
            sleep_ms(100);
            clear_leds(pio, sm, num_leds);
            sleep_ms(100);
        }
        sleep_ms(250);
    }
}

static void pause_animation(PIO pio, uint sm, int num_leds) {
    while (current_state == PAUSE  && !limit_sw_triggered && !led_brd_sw_triggered) {
        for (int i = 0; i < num_leds; i++) put_pixel(0xFFFF00);                 //blinking yellow
        sleep_ms(500);                      
        clear_leds(pio, sm, num_leds);  
        sleep_ms(500);
    }
}

static void error_animation(PIO pio, uint sm, int num_leds) {
    while (current_state == ERROR  && !limit_sw_triggered && !led_brd_sw_triggered) {
        for (int i = 0; i < num_leds; i++) put_pixel(0x00FF00);                 //blinking red
        sleep_ms(250);  
        clear_leds(pio, sm, num_leds);
        sleep_ms(250);
    }
}

// Initialize GPIOs
static void led_init(void) {
    gpio_init(LIMIT_SW);
    gpio_set_dir(LIMIT_SW, GPIO_IN);
    gpio_pull_up(LIMIT_SW);

    gpio_init(LIMIT_SW_LED);
    gpio_set_dir(LIMIT_SW_LED, GPIO_OUT);

    gpio_init(LED_BRD_SWITCH);
    gpio_set_dir(LED_BRD_SWITCH, GPIO_IN);
    gpio_pull_up(LED_BRD_SWITCH);
}

// Handle state transitions
static void state_transition(void) {
    switch (button_count) {
        case 2: current_state = PRINTING; break;
        case 3: current_state = PRINT_FINISHED; break;
        case 4: current_state = IDLE; break;
        case 5: current_state = PAUSE; break;
        case 6: current_state = ERROR; break;
        default: current_state = IDLE; break;
    }
 
}

// Debounce helper
static bool debounce(uint gpio) {
    static uint64_t last_time = 0;
    uint64_t now = time_us_64();
    if (now - last_time > 130000) { // 130ms debounce
        last_time = now;
        return true;
    }
    return false;
}

// Shared GPIO callback
static void shared_callback(uint gpio, uint32_t events) {
    if (gpio == LIMIT_SW && debounce(gpio)) {
        gpio_put(LIMIT_SW_LED, true);
        if (!press_window) {
            press_window = true;
            press_window_start_time = time_us_64();
            button_count = 1;

        }
               
        if (time_us_64() - press_window_start_time < WINDOW_TIME) {
            button_count++;
            printf("Button pressed! Count: %d\n", button_count);
            limit_sw_triggered = true; // Set flag for LIMIT_SW

        } else {
            press_window = false;

        }
    } else if (gpio == LED_BRD_SWITCH && debounce(gpio)) {
        printf("Reset pressed. Returning to IDLE state.\n");
        current_state = IDLE;
        button_count = 1;
        led_brd_sw_triggered = true; // Set flag for LED_BRD_SWITCH

    }
}

// Main Function
int main(void) {
    stdio_init_all();
    sleep_ms(2000); // Allow time for serial initialization
    PIO pio = pio0;
    int sm = 0;
    led_init();

    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), LED_1, 600000, IS_RGBW);

    gpio_set_irq_enabled_with_callback(LIMIT_SW, GPIO_IRQ_EDGE_FALL, true, &shared_callback);
    gpio_set_irq_enabled_with_callback(LED_BRD_SWITCH, GPIO_IRQ_EDGE_FALL, true, &shared_callback);

    while (true) {
            state_transition();
            printf("state: %d\n\r" , current_state);
        switch (current_state) {
            case IDLE: idle_animation(pio, sm, NUM_LEDS); break;
            case PRINTING: printing_animation(pio, sm, NUM_LEDS); break;
            case PRINT_FINISHED: print_finished_animation(pio, sm, NUM_LEDS); break;
            case PAUSE: pause_animation(pio, sm, NUM_LEDS); break;
            case ERROR: error_animation(pio, sm, NUM_LEDS); break;
        }
        limit_sw_triggered = false;   // Reset LIMIT_SW flag
        led_brd_sw_triggered = false;
        gpio_put(LIMIT_SW_LED, false);

    }
}
