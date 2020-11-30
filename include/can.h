#ifndef __CAN_H
#define __CAN_H

#ifdef __linux__
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

#include "default.h"

C_API_BEGIN

declear_handler(hCan);

#define CAN_FILTER_STD_INIT(id) {(id),(CAN_SFF_MASK)}
#define CAN_FILTER_EXT_INIT(id) {(id),(CAN_EFF_MASK)}
#define CAN_FILTER_USR_INIT(id,mask) {(id |= CAN_EFF_FLAG),(mask &= ~CAN_ERR_FLAG)}
//过滤反转
#define CAN_FILTER_USR_INV_INIT(id,mask) {(id |= (CAN_EFF_FLAG | CAN_INV_FILTER)),(mask &= ~CAN_ERR_FLAG)}


hCan can_open(const char *devname,int detach);

int can_close(hCan h);

/**
  struct can_filter filter[] =
  {
    CAN_FILTER_STD_INIT(0x123),
    CAN_FILTER_EXT_INIT(0x456),
    CAN_FILTER_USR_INIT(0x789,0x700)
  };
*/
void can_set_filter(hCan h,struct can_filter *filter,int count);

/** 禁止回环 */
void can_disable_loopback(hCan h);
/** 禁止接收过滤规则 */
void can_disable_recivefilter(hCan h);

/** 写 */
int can_write(hCan h, void *pdata, int len);
/** 读 */
int can_read(hCan h, void *pdata, int len);

/*********** detach ********************/

typedef int (*cb_can_rxdata_func)(hCan h, char *pdata, int len, void *ctx);
int can_setRxDataCallBack(hCan h, cb_can_rxdata_func cbrxdata, void *ctx);

int can_start(hCan h);
int can_stop(hCan h);


C_API_END

#endif
