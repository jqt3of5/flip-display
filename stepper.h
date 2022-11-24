//
// Created by jqt3o on 11/23/2022.
//

#ifndef FLIPDISPLAY_STEPPER_H
#define FLIPDISPLAY_STEPPER_H

void stepper_init();

void stepper_nextStep(int motor);
void stepper_disableMotors();
bool stepper_zeroAll();
bool stepper_setPositionAll(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

#endif //FLIPDISPLAY_STEPPER_H
