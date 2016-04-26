#ifndef __STATE_H
#define __STATE_H

enum state {
  kWaitingForShortPress = 0,
  kDebouncingShortPress,
  kWaitingForLongPress,
  kTiming,
};

enum state raw_current_state(void);
enum state current_state(void);
void set_state(enum state s);

#endif /* __STATE_H */
