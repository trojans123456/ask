#ifndef __EV_EPOLL_H
#define __EV_EPOLL_H

#include "default.h"
#include "event.h"

C_API_BEGIN

#if defined(USED_EPOLL)
extern struct event_ops epoll_ops;
#endif

C_API_END

#endif
