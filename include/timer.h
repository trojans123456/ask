#ifndef __TIMER_H
#define __TIMER_H

#include "default.h"

C_API_BEGIN

declear_handler(hTimer);

enum loop_type
{
    LOOP_ONCE,
    LOOP_PERIOD
};

typedef void (*timeout_event_ptr)(hTimer h,void *ctx);

hTimer timeout_event_create(int tm_ms);
void timeout_event_destroy(hTimer h);

void timeout_event_set_cb(hTimer h,timeout_event_ptr func,void *ctx);

int timeout_event_add(hTimer h);
int timeout_event_set(hTimer h, int msecs);
int timeout_event_cancel(hTimer h);
int timeout_event_remain(hTimer h);

int timer_start(hTimer h);
int timer_stop(hTimer h);


C_API_END


#endif
