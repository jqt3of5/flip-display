
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#ifdef PICO_DEFAULT_LED_PIN
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif

const char * letter_order = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789&?! ";
const int end_stop_adcs[] = {0, 1, 2};
const int end_stop_adc_pin[] = {26, 27, 28};

const int motor_pins[3][4] =
        {
        {2, 4, 3, 5},
        {2, 4, 3, 5},
        {2, 4, 3, 5}
        };

const int stepsPerRevolution = 2038; //Actual  2037.8864
const int preset_positions = 40;
const int stepsPerPosition = stepsPerRevolution/preset_positions;

//The current step position for each motor relative to the endstop.
int current_steps[3] = {-1, -1, -1};
//the wires that are currently energized for each motor. Ideally to keep a smoother running
int current_wires[3] = {0, 0, 0};

void nextStep(int motor)
{
    gpio_put(motor_pins[motor][(current_wires[motor] + 4 - 1) % 4], false);
    gpio_put(motor_pins[motor][current_wires[motor] % 4], true);
    gpio_put(motor_pins[motor][(current_wires[motor] + 1) % 4], true);
    current_wires[motor] += 1;
    current_wires[motor] = current_wires[motor] % 4;



    sleep_ms(10);
}

void disableMotors()
{
    for (int motor = 0; motor < 3; ++motor)
    {
        for(int wire = 0; wire < 4; ++wire)
        {
            gpio_put(motor_pins[motor][wire], false);
        }
   }
}

//TODO: Dynamically update this based on calibration values
int zeroThreshold = 2048;
bool isZero(int endStop)
{
    adc_select_input(end_stop_adcs[endStop]);
    auto result = adc_read();
    if (result > zeroThreshold)
    {
        return true;
    }

    return false;
}

bool zeroAll()
{
    for (int step = 0; step < 2*stepsPerRevolution; step += 1)
    {
        bool allZero = true;
        for (int motor = 0; motor < 3; ++motor)
        {
            if (isZero(motor))
            {
                current_steps[motor] = 0;
                continue;
            }
            allZero = false;
            nextStep(motor);
        }

        if (allZero)
        {
            return true;
        }
    }

    //End stop for some motor not found
    return false;
}

bool setPositionAll(uint8_t a, uint8_t b, uint8_t c)
{
    int targetSteps[] = {a*stepsPerPosition, b*stepsPerPosition, c*stepsPerPosition};
    int stepsRemaining[3] = {0};

    //Calculate the number of steps for each letter
    for (int motor = 0; motor < 3; ++motor)
    {
        //Not zeroed since power on
        if (current_steps[motor] < 0)
        {
            continue;
        }

        //using modular arithmetic, find the number of steps needed to get to the target letter
       stepsRemaining[motor] = (targetSteps[motor] - current_steps[motor] + stepsPerRevolution) % stepsPerRevolution;
    }

    //Just putting an end stop on the number of steps.
    //step the number of times required for each motor to reach the target
    for (int step = 0; step <  stepsPerPosition; step += 1) {

        for (int motor = 0; motor < 3; ++motor)
        {
            if (stepsRemaining[motor] > 0)
            {
                nextStep(motor);
                stepsRemaining[motor] -= 1;
            }
        }
    }

    //Don't need to hold position after moving
    disableMotors();
    return true;
}

int main() {
    stdio_init_all();
    adc_init();

    //TODO: implement serial shift protocol
    //TODO: Handle Multiple motors

    // Make sure GPIO is high-impedance, no pullups etc
    for (int i = 0; i < 3; ++i)
    {
        adc_gpio_init(end_stop_adc_pin[i]);
    }

    for (int j = 0; j < 3; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            gpio_init(motor_pins[j][i]);
            gpio_set_dir(motor_pins[j][i], GPIO_OUT);
            gpio_put(motor_pins[j][i], false);
        }
    }

    zeroAll();

    while (1) {

        sleep_ms(500);
    }
    return 0;
}
