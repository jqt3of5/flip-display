
#include <stdio.h>
#include <hardware/i2c.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "stepper.h"
#include "endstop.h"

#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif


int main() {
    stdio_init_all();

    sleep_ms(5000);
    printf("Starting Up");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    stepper_init();

    uint64_t time = 0;
    while(true) {
        for (int i = 0; i < 60; ++i)
        {
            //Check for button clicks
            sleep_ms(1000);
        }

        time += 1;

        uint minutes = time % 60;
        uint hours = (time / 60) % 12;

        //Need to double check number position
        stepper_setPositionAll(hours / 10,hours % 10,minutes / 10, minutes % 10);
    }
    return 0;
}
