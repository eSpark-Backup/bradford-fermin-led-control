#include "pico/stdlib.h"
#include <stdio.h>

// GPIO Pin Definitions
#define LIMIT_SW 17                // Limit switch
#define LIMIT_SW_LED 19            // Limit switch LED
#define LED_1 12                   // RGB LED
#define LED_BRD_SWITCH 18          // Reset switch/LED board switch

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

// Function Prototypes
static void led_init(void);
static void reset_callback(uint gpio, uint32_t events);
static void idle_state_function(void);
static void neo_pixel_animation_1(void);
static void neo_pixel_animation_2(void);
static void state_transition(void);
static void gpio_callback(uint gpio, uint32_t events);

// Initialize GPIOs
static void led_init(void) {
    gpio_init(LIMIT_SW);
    gpio_set_dir(LIMIT_SW, GPIO_IN);
    gpio_pull_up(LIMIT_SW);

    gpio_init(LIMIT_SW_LED);
    gpio_set_dir(LIMIT_SW_LED, GPIO_OUT);

    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);

    gpio_init(LED_BRD_SWITCH);
    gpio_set_dir(LED_BRD_SWITCH, GPIO_IN);
    gpio_pull_up(LED_BRD_SWITCH);
}


// Idle State Function
static void idle_state_function(void) {
    gpio_put(LED_1, true);
    printf("Idle \n");

}

// Animation Functions
static void neo_pixel_animation_1(void) {
    gpio_put(LED_1, false);
  //  button_count = 0;
    printf("Animation 1\n");
}

static void neo_pixel_animation_2(void) {
    // Implement animation logic
    gpio_put(LED_1, false);
  //  button_count = 0;
    printf("Animation 2\n");
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
    if (gpio == LIMIT_SW  ) {
        printf("gpio called\n\r");
        time_window_active =true;
        static uint64_t last_time = 0;  // Debounce timer
        gpio_put(LIMIT_SW_LED, true); //turn on limit switch led indicating button pressed
        uint64_t now = time_us_64();
        
        if (now - last_time > 200000 && time_window_active) { // 200ms debounce
            if (button_count <2){
            button_count++;}
            printf("Button pressed! Count: %d\n", button_count);
        }
         last_time = now;
    }
    else if( gpio == LED_BRD_SWITCH){
            printf("reset called\n\r");
            current_state = IDLE;
            button_count = 0;
            printf("System reset.\n");
        
    }
}

// Main Function
int main(void) {
    stdio_init_all();
    sleep_ms(2000); // Allow time for serial initialization
    led_init();
    printf("LED initialized\n\r");

    gpio_set_irq_enabled_with_callback(LIMIT_SW, GPIO_IRQ_EDGE_FALL, true, &shared_callback);
    gpio_set_irq_enabled_with_callback(LED_BRD_SWITCH, GPIO_IRQ_EDGE_FALL, true, &shared_callback);

       
     while (true) {
        state_transition();

        switch (current_state) {
            case IDLE:
                idle_state_function();
                break;
            case ANIMATION_1:
                neo_pixel_animation_1();
                break;
            case ANIMATION_2:
                neo_pixel_animation_2();
                break;

        }
        gpio_put(LIMIT_SW_LED, false);
        sleep_ms(1000); // Prevent CPU overload
    }
}
