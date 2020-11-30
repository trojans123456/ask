#ifndef __EV_KQUEUE_H
#define __EV_KQUEUE_H

#include "default.h"
#include "event.h"

C_API_BEGIN

#if defined(USED_KQUEUE)
extern struct event_ops kqueue_ops;
#endif

C_API_END


#endif
