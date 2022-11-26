//
// Created by jqt3o on 11/23/2022.
//

#include <hardware/gpio.h>
#include <pico/time.h>
#include "stepper.h"
#include "endstop.h"


const int motor_pins[4][4] =
        {
                {2, 4, 3, 5},
                {2, 4, 3, 5},
                {2, 4, 3, 5},
                {2, 4, 3, 5}
        };

const int stepsPerRevolution = 2038; //Actual  2037.8864
const int preset_positions = 11;
const int stepsPerPosition = stepsPerRevolution/preset_positions;

//The current step position for each motor relative to the endstop.
int current_steps[4] = {-1, -1, -1, -1};
//the wires that are currently energized for each motor. Ideally to keep a smoother running
int current_wires[4] = {0, 0, 0, 0};

void stepper_init()
{
    uint16_t thresholds[4] = {2048, 2048, 2048, 2048};
    endstop_init(thresholds);

    for (int j = 0; j < 3; ++j)
    {
        for(int i = 0; i < 4; ++i)
        {
            gpio_init(motor_pins[j][i]);
            gpio_set_dir(motor_pins[j][i], GPIO_OUT);
            gpio_put(motor_pins[j][i], false);
        }
    }

    stepper_zeroAll();
}

void stepper_nextStep(int motor)
{
    gpio_put(motor_pins[motor][(current_wires[motor] + 4 - 1) % 4], false);
    gpio_put(motor_pins[motor][current_wires[motor] % 4], true);
    gpio_put(motor_pins[motor][(current_wires[motor] + 1) % 4], true);
    current_wires[motor] += 1;
    current_wires[motor] = current_wires[motor] % 4;

    sleep_ms(10);
}

void stepper_disableMotors()
{
    for (int motor = 0; motor < 4; ++motor)
    {
        for(int wire = 0; wire < 4; ++wire)
        {
            gpio_put(motor_pins[motor][wire], false);
        }
    }
}


bool stepper_zeroAll()
{
    for (int step = 0; step < 2*stepsPerRevolution; step += 1)
    {
        bool allZero = true;
        for (int motor = 0; motor < 4; ++motor)
        {
            if (endstop_isZero(motor))
            {
                current_steps[motor] = 0;
                continue;
            }
            allZero = false;
            stepper_nextStep(motor);
        }

        if (allZero)
        {
            return true;
        }
    }

    //End stop for some motor not found
    return false;
}

bool stepper_setPositionAll(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    //Calc steps relative to endstop
    int targetSteps[] = {a*stepsPerPosition, b*stepsPerPosition, c*stepsPerPosition, d*stepsPerPosition};
    int stepsRemaining[3] = {0};

    //Calculate the number of steps for each letter
    for (int motor = 0; motor < 4; ++motor)
    {
        //the motor will be zeroed first, so we can assume current_steps is 0
        if (current_steps[motor] < 0)
        {
            stepsRemaining[motor] = targetSteps[motor];
            continue;
        }

        //using modular arithmetic, find the number of steps needed to get to the target letter
        stepsRemaining[motor] = (targetSteps[motor] - current_steps[motor] + stepsPerRevolution) % stepsPerRevolution;
    }

    //Just putting a limit on the number of steps.
    //step the number of times required for each motor to reach the target
    for (int step = 0; step < 2*stepsPerPosition; step += 1)
    {
        for (int motor = 0; motor < 4; ++motor)
        {
            //The motor might not be zeroed, or might need to be zeroed again
            if (current_steps[motor] < 0)
            {
                if (endstop_isZero(motor))
                {
                    //motor is zeroed
                    current_steps[motor] = 0;
                    continue;
                }
                stepper_nextStep(motor);
            }
            else if (stepsRemaining[motor] > 0)
            {
                stepper_nextStep(motor);
                stepsRemaining[motor] -= 1;
            }
        }
    }

    //Don't need to hold position after moving
    stepper_disableMotors();
    return true;
}
