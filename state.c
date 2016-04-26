#include <msp430.h>

#include "state.h"

enum state gState = kWaitingForShortPress;

enum state raw_current_state(void) {
  return gState;
}

enum state current_state(void) {
  __disable_interrupt();
  enum state s = gState;
  __enable_interrupt();
  return s;
}

void set_state(enum state s) {
  __disable_interrupt();
  gState = s;
  __enable_interrupt();
}
