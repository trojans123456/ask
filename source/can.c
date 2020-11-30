/**
#    标准帧  127bit
#----------------------------------------
#|     |<- 仲裁域   ->|<- 控制域      ->| 数据域|    CRC                   ->|
#| SOF | 标识符 | RTR | IDE | R0 | DLC | data | CRC | crc分隔符 | ACK | EOF |
#-------------------------------------------------------------------------------
#| 1   |  11   |  1  |  1  | 1  |  4  | 0-64 | 15  |  1       |  2  |  7  |
#-----------------------------------------------------------------------------
#
#| SOF | 标识符 | SRR | IDE | 扩展 | R1 | R0 | DLC | ----
#|  1  |  11   |  1  |  1  | 18   | 1 |  1  |  4
#
# RTR : 远程发送请求位  数据帧为 显性  远程帧为隐性
# SRR ： 代替远程请求位 = 0 因为在扩展帧的再RTR的位置
# IDE ： 标识符扩展位  标准显性  扩展帧为隐性
R0 R1保留位

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#endif

#include "task.h"
#include "list.h"
#include "can.h"

#define CAN_BUFSIZE 1024

typedef struct
{
    int fd;
    int is_detach;
    cb_can_rxdata_func cbRxData;
    void *ctx;
    char msgRecvBuf[CAN_BUFSIZE];
    int msgRecvOffset;
    struct dl_list list;
}can_t;

typedef struct
{
    hMutex mtx;
    hThread thread;
    int ref;
    int first;
    struct dl_list can_head;
}can_priv_t;
static can_priv_t can_priv;

hCan can_open(const char *devname,int detach)
{
    if(!devname)
        return NULL;

    can_t *can = (can_t*)calloc(1,sizeof(can_t));
    if(!can)
        return NULL;

    struct sockaddr_can addr;
    struct ifreq ifr;

    can->fd = socket(PF_CAN,SOCK_RAW,CAN_RAW);
    if(can->fd < 0)
    {
        free(can);
        return NULL;
    }

    strcpy(ifr.ifr_name,devname);
    if(ioctl(can->fd,SIOCGIFINDEX,&ifr) < 0)
    {
        close(can->fd);
        free(can);
        return NULL;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if((bind(can->fd,(struct sockaddr*)&addr,sizeof(addr))) < 0)
    {
        close(can->fd);
        free(can);
        return NULL;
    }

    if(detach > 0)
    {
        can->is_detach = 1;
        dl_list_init(&can->list);
    }


    return (hCan)can;
}

int can_close(hCan h)
{
    can_t *can = (can_t*)h;
    if(!can)
        return -1;

    close(can->fd);
    can->is_detach = 0;
    free(can);
    return 0;
}

void can_set_filter(hCan h, struct can_filter *filter, int count)
{
    can_t *can = (can_t*)h;
    if(can && filter)
    {
        setsockopt(can->fd,SOL_CAN_RAW,CAN_RAW_FILTER,filter,count);
    }
}

void can_disable_loopback(hCan h)
{
    can_t *can = (can_t*)h;
    if(can)
    {
        int loopback = 0;
        setsockopt(can->fd,SOL_CAN_RAW,CAN_RAW_LOOPBACK,&loopback,sizeof(loopback));
    }
}

void can_disable_recivefilter(hCan h)
{
    can_t *can = (can_t*)h;
    if(can)
    {
        setsockopt(can->fd,SOL_CAN_RAW,CAN_RAW_FILTER,NULL,0);
    }
}

/** 写 */
int can_write(hCan h, void *pdata, int len)
{
    can_t *can = (can_t*)h;
    if(!can || !pdata || (len == 0))
        return -1;

    int cur = 0;
    int written = 0;
    while(cur < len)
    {
        written = write(can->fd,pdata + cur,len - cur);
        if(written < 0)
        {
            /*被中断 或资源不足*/
            if(errno == EINTR ||
               errno == EAGAIN)
            {
                continue;
            }
            return -1;
        }
        if(written == 0)
            break;

        cur += written;
    }
    return cur;
}

/** 读 */
int can_read(hCan h,void *pdata,int len)
{
    can_t *can = (can_t*)h;
    if(!can || !pdata || (len <= 0))
        return -1;
    int cur = 0,read_len = 0;

    while(cur < len)
    {
        read_len = read(can->fd,pdata + cur,len - cur);
        if(read_len < 0)
        {
            if(errno == EINTR ||
               errno == EAGAIN)
            {
                continue;
            }
            return -1;
        }
        if(read_len == 0)
            break;

        cur += read_len;
    }
    return cur;
}

/*********** detach ********************/
int can_setRxDataCallBack(hCan h, cb_can_rxdata_func cbrxdata, void *ctx)
{
    can_t *can = (can_t *)h;
    if(!can)
        return -1;
    if(can->is_detach <= 0)
        return -1;
    can->cbRxData = cbrxdata;
    can->ctx = ctx;

    return 0;
}

void *can_recv_task(hThread h,void *ctx)
{
    int ret,len;
    fd_set readfds;
    //int timeout = 1000;/* 1s */
    can_t *can = NULL;
    int maxfd = 0;
    struct timeval tm = {30,0};
    while(lapi_thread_isrunning(h))
    {
        FD_ZERO(&readfds);
        lapi_mutex_lock(can_priv.mtx);
        dl_list_for_each(can,&can_priv.can_head,can_t,list)
        {
            if(can)
            {
                FD_SET(can->fd,&readfds);
                maxfd = MAX(maxfd,can->fd);
            }
        }
        lapi_mutex_unlock(can_priv.mtx);

        ret = select(maxfd + 1,&readfds,NULL,NULL,&tm);
        if(ret < 0)
        {
            if(errno == EINTR) /* 信号中断*/
            {
                continue;
            }
            break;
        }
        if(ret == 0)
        {
            /* timeout */
            continue;
        }

        lapi_mutex_lock(can_priv.mtx);

        dl_list_for_each(can,&can_priv.can_head,can_t,list)
        {
            if(can && FD_ISSET(can->fd,&readfds))
            {
                memset(&can->msgRecvBuf[can->msgRecvOffset],0,CAN_BUFSIZE - can->msgRecvOffset);

                len = read(can->fd, &can->msgRecvBuf[can->msgRecvOffset], CAN_BUFSIZE - can->msgRecvOffset);

                if(len > 0)
                {
                    int rxLen = 0;
                    can->msgRecvOffset += len;
                    //printf("%s %d len=%d offset=%d \n",__FUNCTION__,__LINE__,len,comm->msgRecvOffset);
                    if( can->cbRxData)
                        rxLen = can->cbRxData((hCan)can, can->msgRecvBuf,can->msgRecvOffset, can->ctx);
                    //printf("%s %d hasProcLen=%d \n",__FUNCTION__,__LINE__,rxLen);
                    if(rxLen > 0)
                    {
                        memmove(can->msgRecvBuf,&can->msgRecvBuf[rxLen],can->msgRecvOffset - rxLen);
                        can->msgRecvOffset -= rxLen;
                    }
                }
            }
        }

        lapi_mutex_unlock(can_priv.mtx);
    }

    return NULL;
}

int can_start(hCan h)
{
    can_t *can = (can_t*)h;
    if(!can)
        return -1;
    if(can->is_detach <= 0)
        return -1;

    if(can_priv.first == 0)
    {
        dl_list_init(&can_priv.can_head);
        can_priv.mtx = lapi_mutex_create();
    }

    lapi_mutex_lock(can_priv.mtx);
    dl_list_add_tail(&can_priv.can_head,&can->list);
    can_priv.ref++;
    lapi_mutex_unlock(can_priv.mtx);

    if(can_priv.first == 0)
    {
        can_priv.first = 1;
        can_priv.thread = lapi_thread_create(can_recv_task,NULL,1 << 10 <<10);
    }

    return 0;
}

int can_stop(hCan h)
{
    can_t *can = (can_t*)h;
    if(!can)
        return -1;
    if(can->is_detach <= 0)
        return -1;

    lapi_mutex_lock(can_priv.mtx);

    dl_list_del(&can->list);
    can_priv.ref--;
    if(can_priv.ref == 0)
    {
        lapi_thread_destroy(can_priv.thread);
        can_priv.thread = NULL;
        can_priv.first = 0;
    }

    lapi_mutex_unlock(can_priv.mtx);

    if(can_priv.ref == 0)
    {
        lapi_mutex_destroy(can_priv.mtx);
    }

    return 0;
}
