//
// Created by jqt3o on 11/23/2022.
//

#include <boards/pico.h>
#include <hardware/gpio.h>
#include "endstop.h"
#include <hardware/i2c.h>
#include <cstdio>

const uint8_t read_addr = 0b1001000;
const uint8_t write_addr = 0b1001000;

const uint8_t POINTER_CONFIG = 0b00000001;
const uint8_t POINTER_CONVERSION = 0b00000000;

#define ONE_SHOT (1<<15)
#define MUX2 (1<<14)
#define MUX1 (1<<13)
#define MUX0 (1<<12)
#define PGA2 (1<<11)
#define PGA1 (1<<10)
#define PGA0 (1<<9)
#define MODE (1<<8)
#define DR2 (1<<7)
#define DR1 (1<<6)
#define DR0 (1<<5)
#define COMP_MODE (1<<4)
#define COMP_POL (1<<3)
#define COMP_LAT (1<<2)
#define COMP_QUE1 (1<<1)
#define COMP_QUE0 (1<<0)

#define PGA_6_144 (0)
#define PGA_4_096 PGA0
#define PGA_2_048 PGA1
#define PGA_1_024 (PGA1 | PGA0)
#define PGA_0_512 PGA2
#define PGA_0_256 (PGA2 | PGA1 | PGA0)

#define AIN0 (MUX2)
#define AIN1 (MUX2 | MUX0)
#define AIN2 (MUX2 | MUX1)
#define AIN3 (MUX2 | MUX1 | MUX0)

#define SPS128 (DR2)

#define DISABLE_COMP (COMP_QUE1 | COMP_QUE0)

#define SDA 0
#define SCL 1
#define i2c_port i2c0

uint16_t endstop_thresholds[4] = {2048, 2048, 2048, 2048};

uint16_t endstop_readConfig()
{
    uint8_t config[2] = {POINTER_CONFIG};
    //Write to config
    i2c_write_blocking(i2c_port, write_addr, config, 1, true);

    i2c_read_blocking(i2c_port, read_addr, config, 2, false);

    return config[1] << 8 | config[0];
}

void endstop_writeConfig(uint16_t config)
{
    //Write to config
    uint8_t toWrite[3] ={
            POINTER_CONFIG, //Write to pointer reg
            ((uint8_t*)&config)[1],
            ((uint8_t*)&config)[0],
    };
    i2c_write_blocking(i2c_port, write_addr, toWrite, 3, false);
}

uint16_t endstop_readConversion()
{
    //write to pointer register so we can read from conversion register
    i2c_write_blocking(i2c_port, write_addr, &POINTER_CONVERSION, 1, false);

    uint8_t readConversion[2] = {0};
    i2c_read_blocking(i2c_port, read_addr, readConversion, 2, false);

//    uint8_t swap = readConversion[0];
//    readConversion[0] = readConversion[1];
//    readConversion[1] = swap;

    return readConversion[1] << 8 | readConversion[0];
}

void endstop_startConversion(int ain)
{
    switch(ain)
    {
        case 0:
            endstop_writeConfig(ONE_SHOT | AIN0 | PGA_4_096 | MODE | SPS128 | DISABLE_COMP);
            break;
        case 1:
            endstop_writeConfig(ONE_SHOT | AIN1 | PGA_4_096 | MODE | SPS128 | DISABLE_COMP);
            break;
        case 2:
            endstop_writeConfig(ONE_SHOT | AIN2 | PGA_4_096 | MODE | SPS128 | DISABLE_COMP);
            break;
        case 3:
            endstop_writeConfig(ONE_SHOT | AIN3 | PGA_4_096 | MODE | SPS128 | DISABLE_COMP);
            break;
        default:
            break;
    }
}

void endstop_waitForConversion()
{
    while (!(endstop_readConfig() & ONE_SHOT));
}

void endstop_init(uint16_t thresholds[4])
{
    for (int i = 0; i < 4; ++i)
    {
        endstop_thresholds[i] = thresholds[i];
    }

    i2c_init(i2c_port, 400 * 1000);

    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    gpio_put(PICO_DEFAULT_LED_PIN, true);
    sleep_ms(500);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(500);

    endstop_writeConfig(ONE_SHOT | AIN0 | PGA_4_096 | MODE | SPS128 | DISABLE_COMP);
    printf("Wrote config\n");

    auto config = endstop_readConfig();

    for (int i = 0; i < 16; ++i)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms((config& (1<<i)) ? 1000 : 250);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(1000);
    }

    printf("Config: %X\n", config);

}
bool endstop_isZero(int endStop)
{
    if (endStop >= 4)
    {
        return false;
    }

    endstop_startConversion(endStop);
    printf("Conversion Started for endstop: %d\n", endStop);

//    gpio_put(PICO_DEFAULT_LED_PIN, true);
//    sleep_ms(250);
//    gpio_put(PICO_DEFAULT_LED_PIN, false);
//    sleep_ms(250);

    endstop_waitForConversion();
    printf("Conversion Finished\n");

//    gpio_put(PICO_DEFAULT_LED_PIN, true);
//    sleep_ms(250);
//    gpio_put(PICO_DEFAULT_LED_PIN, false);
//    sleep_ms(250);

    auto result = endstop_readConversion();
    printf("Conversion Read %d\n", result);

    for (int i = 0; i < 16; ++i)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms((result& (1<<i)) ? 1000 : 250);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(1000);
    }

    if (result > endstop_thresholds[endStop])
    {
        return true;
    }

    return false;
}