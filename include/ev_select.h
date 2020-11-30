#ifndef __EV_SELECT_H
#define __EV_SELECT_H

#include "default.h"
#include "event.h"

C_API_BEGIN

#if defined(USED_SELECT)
extern struct event_ops select_ops;
#endif

C_API_END

#endif
