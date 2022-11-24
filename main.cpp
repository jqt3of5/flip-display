
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
//    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
//    for (int i = 0; i < 3; ++i)
//    {
//        adc_gpio_init(end_stop_adc_pin[i]);
//    }
    i2c_init(i2c_default, 400 * 1000);

    endstop_init();
    stepper_init();

    stepper_setPositionAll(1,2,3, 4);

    return 0;
}
