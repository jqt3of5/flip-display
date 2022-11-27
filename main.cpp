
#include <stdio.h>
#include <hardware/i2c.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "stepper.h"
#include "endstop.h"

#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif

const char * letter_order = "0123456789 ";

int main() {
    stdio_init_all();

    sleep_ms(5000);
    printf("Starting Up");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    stepper_init();

    stepper_setPositionAll(1,2,3, 4);

    return 0;
}
