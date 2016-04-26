#ifndef __STATE_H
#define __STATE_H

enum state {
  kWaitingForShortPress = 0,
  kDebouncingShortPress,
  kWaitingForLongPress,
  kDebouncingLongPress,
  kTiming,
};

enum state current_state(void);
void set_state(enum state s);

#endif /* __STATE_H */
