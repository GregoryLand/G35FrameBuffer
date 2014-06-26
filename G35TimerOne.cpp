#include "G35TimerOne.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

void stopTimer()
{
  // Set the clock for the counter to none
  TCCR1B &= ~(1 << CS10 | 1 << CS11 | 1 << CS12);
}
void startTimer()
{
  // Set the clock for the counter to no prescaling mode
  // Set Multiplier for Timer to one to one with the cpu clock
  // Hardware manual calls this No prescaling mode
  TCCR1B |= (1 << CS10 | 0 << CS11 | 0 << CS12);
}

void SetupTimerOne()
{
  // Make sure Timer isn't running
  stopTimer();
  
  // Clear Interupts since we are unsure of the state of them
  TIFR1 &= ~(1 << TOV1 | 1 << OCF1A | 1 << OCF1B | 1 << ICF1 ); 
    
  // Set values to put timer in FastPWM mode witn ICR1 as TOP
  TCCR1A &= ~(1 << WGM10);
  TCCR1A |= 1 << WGM11;
  TCCR1B |= 1 << WGM12;
  TCCR1B |= 1 << WGM13;
  
  // Setup compare output units to set pins 9 and 10 high when counter matches them
  // and low again when counter resets
  TCCR1A |= 1 << COM1A0;
  TCCR1A |= 1 << COM1A1;
  TCCR1A |= 1 << COM1B0;
  TCCR1A |= 1 << COM1B1;
  
  // Enable outputs on the ports
  DDRB |= 1 << PORTB1;
  DDRB |= 1 << PORTB2;
  
  // Set IRC1 to hold the top value for our timer
  // Since ICR1/OCR1A/OCR1B are 16 bit registers we have to make sure
  // we dont get interrupted so disable interrupts
  cli();
  ICR1  = CLOCKCYCLES_IN_30_MICROSECONDS;
  OCR1A = HOLD_LOW;
  OCR1B = HOLD_LOW;
  sei();
  
  // Setup our interrupts masks correctly
  TIMSK1 &= ~(1 << ICIE1 );  // Disable Input capture interrupt
  TIMSK1 &= ~(1 << OCIE1B);  // Disable interrupt for OCIE1B compare
  TIMSK1 &= ~(1 << OCIE1A);  // Disable interrupt for OCiE1A compare
  TIMSK1 |= 1 << TOIE1;      // Enable interrupt for Overflow/Top value when in fastpwm
  
  // Clear Timer Flags
  stopTimer();
}
