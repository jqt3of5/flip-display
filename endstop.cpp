//
// Created by jqt3o on 11/23/2022.
//

#include <boards/pico.h>
#include <hardware/gpio.h>
#include "endstop.h"
#include <hardware/i2c.h>

const uint8_t read_addr = 0b10010001;
const uint8_t write_addr = 0b10010000;

void endstop_init()
{
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    uint8_t config[3] ={0b00000001, 0b10000100, 0b10000011 };
    i2c_write_blocking(i2c_default, write_addr, config, 3, false);
}
//TODO: Dynamically update this based on calibration values
int zeroThreshold = 2048;
bool endstop_isZero(int endStop)
{
//    adc_select_input(end_stop_adcs[endStop]);
//    auto result = adc_read();

    uint8_t pointer = 0;
    i2c_write_blocking(i2c_default, write_addr, &pointer, 1, false);

    uint8_t readConversion[2] = {0};
    i2c_read_blocking(i2c_default, read_addr, readConversion, 2, false);

    if (result > zeroThreshold)
    {
        return true;
    }

    return false;
}