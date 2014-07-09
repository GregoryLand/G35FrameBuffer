#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef G35TimerOne_H
#define G35TimerOne_H

const uint16_t CLOCKCYCLES_IN_30_MICROSECONDS = (F_CPU / 1000000) * 30;
const uint16_t SHORT_PULSE_TIME               = CLOCKCYCLES_IN_30_MICROSECONDS / 3;
const uint16_t LONG_PULSE_TIME                = SHORT_PULSE_TIME * 2;
const uint16_t SEND_A_ZERO                    = SHORT_PULSE_TIME;
const uint16_t SEND_A_ONE                     = LONG_PULSE_TIME;
const uint16_t HOLD_LOW                       = CLOCKCYCLES_IN_30_MICROSECONDS;

void stopTimer();
void startTimer();
void SetupTimerOne();

#endif
