#include <msp430.h>

#include "timer.h"
#include "gpio.h"

static struct timer_ctl g_timer_ctls[2];

void timer_setup(void) {
  // Clear timer A, Select clock source 0 (LFXTAL)
  TACTL = TACLR | TASSEL0;
}

void timer_start(enum timer_channel c, int time) {
  switch (c) {
    case kChannel0:
      // Enable the compare ISR on TimerA CCR0
      TACCTL0 |= CCIE;
      TACCR0 = time;
      break;
    case kChannel1:
      // Enable the compare ISR on TimerA CCR0
      TACCTL1 |= CCIE;
      TACCR1 = time;
      break;
  }
}

void timer_stop(enum timer_channel c) {
  switch (c) {
    case kChannel0:
      // Disable the compare ISR on TimerA CCR0
      TACCTL0 &= ~CCIE;
      break;
    case kChannel1:
      // Disable the compare ISR on TimerA CCR0
      TACCTL1 &= ~CCIE;
      break;
  }
}

void timer_set_mode(enum timer_channel c, enum timer_mode m) {
  __disable_interrupt();
  g_timer_ctls[c].mode = m;
  __enable_interrupt();
}

void timer_set_handler(enum timer_channel c, int(*h)(void)) {
  __disable_interrupt();
  g_timer_ctls[c].handler = h;
  __enable_interrupt();
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void) {
  if (g_timer_ctls[kChannel0].mode == kOneShot) {
    timer_stop(kChannel0);
  }
  if (g_timer_ctls[kChannel0].handler) {
      toggle_led();
      if (g_timer_ctls[kChannel0].handler() != 0) {
        // Wake the processor when this ISR exits
        __bic_SR_register_on_exit(LPM0_bits);
      }
  }
}

#pragma vector=TIMERA1_VECTOR
__interrupt void TIMERA1_ISR(void) {
  if (g_timer_ctls[kChannel1].mode == kOneShot) {
    timer_stop(kChannel1);
  }
  if (g_timer_ctls[kChannel1].handler) {
      toggle_led();
      if (g_timer_ctls[kChannel1].handler() != 0) {
        // Wake the processor when this ISR exits
        __bic_SR_register_on_exit(LPM0_bits);
      }
  }
}
