/**
 *
 *1.单独监听,可用一个线程监听所有的串口数据
 *
 *2.事件监听,将串口作为一个事件放到事件监听队列中(需要启动 事件引擎)
**/

#include <stdio.h>

#include "list.h"
#include "serial.h"

typedef struct
{
    int fd_tty;
    CommAttribute attr;
    cb_com_rxdata_func cbRxData;
    void *ctx;
    char *msgRecvBuf;
    int msgRecvOffset;
    int timeout;
    struct dl_list list;
}comm_t;
