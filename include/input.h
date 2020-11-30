#ifndef __INPUT_H
#define __INPUT_H

#include "default.h"

C_API_BEGIN

declear_handler(hInput);

/** cat /proc/bus/input/devices 查看事件类型*/
hInput input_open(const char *name);

int input_close(hInput h);

typedef void (*cb_input_rxdata_func)(hInput h,void *data);
int input_setRxDataFunc(hInput h,cb_input_rxdata_func func,void *ctx);

int input_start(hInput h);
int input_stop(hInput h);

C_API_END

#endif
