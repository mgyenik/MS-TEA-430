#include <msp430.h>

#include "gpio.h"

void gpio_setup(void) {
  // Button is input.
  P1DIR &= ~(1 << 3);

  // Button pull-up.
  P1OUT |= (1 << 3);
  P1REN |= (1 << 3);

  // Red LED is output.
  P1DIR |= (1 << 0);

  // Green LED is output.
  P1DIR |= (1 << 6);
}

void toggle_led(void) {
  P1OUT ^= (1 << 6);
}
