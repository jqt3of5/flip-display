//
// Created by jqt3o on 11/23/2022.
//

#ifndef FLIPDISPLAY_ENDSTOP_H
#define FLIPDISPLAY_ENDSTOP_H

void endstop_init(uint16_t thresholds[4]);
bool endstop_isZero(int endStop);

#endif //FLIPDISPLAY_ENDSTOP_H
