#ifndef __TIMER_H
#define __TIMER_H

#include <stdbool.h>

enum timer_channel {
  kChannel0 = 0,
  kChannel1 = 1,
};

enum timer_mode {
  kContinuous,
  kOneShot,
};

struct timer_ctl {
  enum timer_mode mode;
  int (*handler)(void);
};


void timer_setup(void);
void timer_start(enum timer_channel c, int time);
void timer_stop(enum timer_channel c);
void timer_set_mode(enum timer_channel c, enum timer_mode m);
void timer_set_handler(enum timer_channel c, int(*h)(void));

#endif /* __TIMER_H */
