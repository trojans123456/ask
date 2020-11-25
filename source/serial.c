/**
 *
 *1.单独监听,可用一个线程监听所有的串口数据
 *
 *2.事件监听,将串口作为一个事件放到事件监听队列中(需要启动 事件引擎)
**/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#endif // __linux__

#include "task.h"
#include "list.h"
#include "serial.h"

#define COM_BUFSIZE     1024
typedef struct
{
    int fd_tty;
    int is_detach;
    CommAttribute attr;
    cb_com_rxdata_func cbRxData;
    void *ctx;
    char msgRecvBuf[COM_BUFSIZE];
    int msgRecvOffset;
    //int timeout;
    struct dl_list list;
}comm_t;

/* for thread listen */
typedef struct
{
    struct dl_list s_comms;/*链表头*/
    hThread thread;
    int ref;
    int first;
    hMutex s_comm_mtx;
}com_priv_t;
static com_priv_t com_priv = {0};

int purge_usb_com(int tty_fd, int dw_flags)
{
    switch (dw_flags)
    {
        case COMM_PURGE_TXABORT:
            tcflow(tty_fd, TCOOFF);
            break;
        case COMM_PURGE_RXABORT:
            tcflow(tty_fd, TCIOFF);
            break;
        case COMM_PURGE_TXCLEAR:
            tcflush(tty_fd, TCOFLUSH);
            break;
        case COMM_PURGE_RXCLEAR:
            tcflush(tty_fd, TCIFLUSH);
            break;
        default:

            return -1;
    }

    return 0;
}

int serial_purge(hSerial h,int dw_flags)
{
    comm_t *com = (comm_t*)h;
    if(!com)
        return -1;

    return purge_usb_com(com->fd_tty,dw_flags);
}

int serial_setAttr(hSerial h,const CommAttribute *attr)
{
    struct termios opt;
    comm_t *comm = (comm_t*)h;
    if(!comm || !attr)
        return -1;

    memcpy(&comm->attr,attr,sizeof(CommAttribute));

    memset(&opt, 0, sizeof(struct termios));
    tcgetattr(comm->fd_tty, &opt);
    cfmakeraw(&opt);			/* set raw mode	*/
    opt.c_iflag = IGNBRK;
    switch (attr->baudrate)
    {
        case 50:
            cfsetispeed(&opt, B50);
            cfsetospeed(&opt, B50);
            break;
        case 75:
            cfsetispeed(&opt, B75);
            cfsetospeed(&opt, B75);
            break;
        case 110:
            cfsetispeed(&opt, B110);
            cfsetospeed(&opt, B110);
            break;
        case 134:
            cfsetispeed(&opt, B134);
            cfsetospeed(&opt, B134);
            break;
        case 150:
            cfsetispeed(&opt, B150);
            cfsetospeed(&opt, B150);
            break;
        case 200:
            cfsetispeed(&opt, B200);
            cfsetospeed(&opt, B200);
            break;
        case 300:
            cfsetispeed(&opt, B300);
            cfsetospeed(&opt, B300);
            break;
        case 600:
            cfsetispeed(&opt, B600);
            cfsetospeed(&opt, B600);
            break;
        case 1200:
            cfsetispeed(&opt, B1200);
            cfsetospeed(&opt, B1200);
            break;
        case 1800:
            cfsetispeed(&opt, B1800);
            cfsetospeed(&opt, B1800);
            break;
        case 2400:
            cfsetispeed(&opt, B2400);
            cfsetospeed(&opt, B2400);
            break;
        case 4800:
            cfsetispeed(&opt, B4800);
            cfsetospeed(&opt, B4800);
            break;
        case 9600:
            cfsetispeed(&opt, B9600);
            cfsetospeed(&opt, B9600);
            break;
        case 19200:
            cfsetispeed(&opt, B19200);
            cfsetospeed(&opt, B19200);
            break;
        case 38400:
            cfsetispeed(&opt, B38400);
            cfsetospeed(&opt, B38400);
            break;
        case 57600:
            cfsetispeed(&opt, B57600);
            cfsetospeed(&opt, B57600);
            break;
        case 115200:
            cfsetispeed(&opt, B115200);
            cfsetospeed(&opt, B115200);
            break;
        case 230400:
            cfsetispeed(&opt, B230400);
            cfsetospeed(&opt, B230400);
            break;
        case 460800:
            cfsetispeed(&opt, B460800);
            cfsetospeed(&opt, B460800);
            break;
        case 500000:
            cfsetispeed(&opt, B500000);
            cfsetospeed(&opt, B500000);
            break;
        case 576000:
            cfsetispeed(&opt, B576000);
            cfsetospeed(&opt, B576000);
            break;
        case 921600:
            cfsetispeed(&opt, B921600);
            cfsetospeed(&opt, B921600);
            break;
        case 1000000:
            cfsetispeed(&opt, B1000000);
            cfsetospeed(&opt, B1000000);
            break;
        case 1152000:
            cfsetispeed(&opt, B1152000);
            cfsetospeed(&opt, B1152000);
            break;
        case 1500000:
            cfsetispeed(&opt, B1500000);
            cfsetospeed(&opt, B1500000);
            break;
        case 2000000:
            cfsetispeed(&opt, B2000000);
            cfsetospeed(&opt, B2000000);
            break;
        case 2500000:
            cfsetispeed(&opt, B2500000);
            cfsetospeed(&opt, B2500000);
            break;
        case 3000000:
            cfsetispeed(&opt, B3000000);
            cfsetospeed(&opt, B3000000);
            break;
        case 3500000:
            cfsetispeed(&opt, B3500000);
            cfsetospeed(&opt, B3500000);
            break;
        case 4000000:
            cfsetispeed(&opt, B4000000);
            cfsetospeed(&opt, B4000000);
            break;
        default:

            return -1;
    }


    switch (attr->parity)
    {
        case COMM_PARITY_NONE:		    /* none			*/
            opt.c_cflag &= ~PARENB;	/* disable parity	*/
            opt.c_iflag &= ~INPCK;	/* disable parity check	*/
            break;
        case COMM_PARITY_ODD:		/* odd			*/
            opt.c_cflag |= PARENB;	/* enable parity	*/
            opt.c_cflag |= PARODD;	/* odd			*/
            opt.c_iflag |= INPCK;	/* enable parity check	*/
            break;
        case COMM_PARITY_EVEN:		/* even			*/
            opt.c_cflag |= PARENB;	/* enable parity	*/
            opt.c_cflag &= ~PARODD;	/* even			*/
            opt.c_iflag |= INPCK;	/* enable parity check	*/
        default:

            return -1;
    }

    opt.c_cflag &= ~CSIZE;
    switch (attr->dataBits)
    {
        case 5:
            opt.c_cflag |= CS5;
            break;
        case 6:
            opt.c_cflag |= CS6;
            break;
        case 7:
            opt.c_cflag |= CS7;
            break;
        case 8:
            opt.c_cflag |= CS8;
            break;
        default:

            return -1;
    }

    opt.c_cflag &= ~CSTOPB;
    switch (attr->stopBits)
    {
        case COMM_STOPBIT_1:
            opt.c_cflag &= ~CSTOPB;
            break;
/*		case COMM_STOPBIT_1_5:
            break;
*/
        case COMM_STOPBITS_2:
            opt.c_cflag |= CSTOPB;
            break;
        default:

            return -1;
    }
    opt.c_cc[VTIME]	= 0;
    opt.c_cc[VMIN]	= 1;			/* block until data arrive */

    tcflush(comm->fd_tty, TCIOFLUSH);
    if (tcsetattr(comm->fd_tty, TCSANOW, &opt) < 0)
    {

        return -1;
    }

    //清空收发缓存，避免收到残留的数据
    purge_usb_com(comm->fd_tty, COMM_PURGE_TXCLEAR);
    purge_usb_com(comm->fd_tty, COMM_PURGE_RXCLEAR);

    return 0;
}

hSerial serial_open(const char *tty_dev,uint8_t detach)
{
    int i,fd_tty;
    comm_t *com = NULL;
    if(!tty_dev)
        return NULL;

    for(i = 0;i < 2;i++)
    {
        fd_tty = open(tty_dev,O_RDWR | O_NOCTTY | O_NDELAY);
        if(fd_tty < 0)
        {
            lapi_sleep(5 * 1000);
                continue;
        }
        break;
    }

    if(i == 2)
        return NULL;

    com = (comm_t *)calloc(1,sizeof(comm_t));
    if(!com)
    {
        close(fd_tty);
        return NULL;
    }

    com->fd_tty = fd_tty;

    if(detach > 0)
    {
        dl_list_init(&com->list);
        com->is_detach = 1;
    }

    return (hSerial)com;
}

int serial_close(hSerial h)
{
    comm_t *com = (comm_t*)h;
    if(!com)
        return -1;
    if(com_priv.s_comm_mtx)
    {
        lapi_mutex_lock(com_priv.s_comm_mtx);
    }

    close(com->fd_tty);
    free(com);
    com = NULL;

    if(com_priv.s_comm_mtx)
    {
        lapi_mutex_unlock(com_priv.s_comm_mtx);
    }

    return 0;
}

int serial_write(hSerial h,void *pdata,int len)
{
    comm_t *com = (comm_t*)h;
    if(!com || !pdata || (len == 0))
        return -1;

    int cur = 0;
    int written = 0;
    while(cur < len)
    {
        written = write(com->fd_tty,pdata + cur,len - cur);
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

int serial_read(hSerial h,void *pdata,int len)
{
    comm_t *comm = (comm_t*)h;
    if(!comm || !pdata || (len <= 0))
        return -1;
    int cur = 0,read_len = 0;

    while(cur < len)
    {
        read_len = read(comm->fd_tty,pdata + cur,len - cur);
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

int serial_setRxDataCallBack(hSerial h,cb_com_rxdata_func cbrxdata,void *ctx)
{
    comm_t *comm = (comm_t *)h;
    if(!comm)
        return -1;
    if(comm->is_detach <= 0)
        return -1;
    comm->cbRxData = cbrxdata;
    comm->ctx = ctx;

    return 0;
}

void *comm_recv_task(hThread h,void *p)
{
    int ret,len;
    fd_set readfds;
    //int timeout = 1000;/* 1s */
    comm_t *comm = NULL;
    int maxfd = 0;
    struct timeval tm = {30,0};
    while(lapi_thread_isrunning(h))
    {
        FD_ZERO(&readfds);
        lapi_mutex_lock(com_priv.s_comm_mtx);
        dl_list_for_each(comm,&com_priv.s_comms,comm_t,list)
        {
            if(comm)
            {
                FD_SET(comm->fd_tty,&readfds);
                maxfd = MAX(maxfd,comm->fd_tty);
            }
        }
        lapi_mutex_unlock(com_priv.s_comm_mtx);

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

        lapi_mutex_lock(com_priv.s_comm_mtx);

        dl_list_for_each(comm,&com_priv.s_comms,comm_t,list)
        {
            if(comm && FD_ISSET(comm->fd_tty,&readfds))
            {
                memset(&comm->msgRecvBuf[comm->msgRecvOffset],0,COM_BUFSIZE - comm->msgRecvOffset);

                len = read(comm->fd_tty, &comm->msgRecvBuf[comm->msgRecvOffset], COM_BUFSIZE - comm->msgRecvOffset);

                if(len > 0)
                {
                    int rxLen = 0;
                    comm->msgRecvOffset += len;
                    //printf("%s %d len=%d offset=%d \n",__FUNCTION__,__LINE__,len,comm->msgRecvOffset);
                    if( comm->cbRxData)
                        rxLen = comm->cbRxData((hSerial)comm, comm->msgRecvBuf,comm->msgRecvOffset, comm->ctx);
                    //printf("%s %d hasProcLen=%d \n",__FUNCTION__,__LINE__,rxLen);
                    if(rxLen > 0)
                    {
                        memmove(comm->msgRecvBuf,&comm->msgRecvBuf[rxLen],comm->msgRecvOffset - rxLen);
                        comm->msgRecvOffset -= rxLen;
                    }
                }
            }
        }

        lapi_mutex_unlock(com_priv.s_comm_mtx);
    }

    return NULL;
}

int serial_start(hSerial h)
{
    comm_t *comm = (comm_t*)h;
    if(!comm)
        return -1;
    if(comm->is_detach <= 0)
        return -1;

    if(com_priv.first == 0)
    {
        dl_list_init(&com_priv.s_comms);
        com_priv.s_comm_mtx = lapi_mutex_create();
    }

    lapi_mutex_lock(com_priv.s_comm_mtx);
    dl_list_add_tail(&com_priv.s_comms,&comm->list);
    com_priv.ref++;
    lapi_mutex_unlock(com_priv.s_comm_mtx);

    if(com_priv.first == 0)
    {
        com_priv.first = 1;
        com_priv.thread = lapi_thread_create(comm_recv_task,NULL,1 << 10 <<10);
    }

    return 0;
}

int serial_stop(hSerial h)
{
    comm_t *comm = (comm_t*)h;
    if(!comm)
        return -1;
    if(comm->is_detach <= 0)
        return -1;

    lapi_mutex_lock(com_priv.s_comm_mtx);

    dl_list_del(&comm->list);
    com_priv.ref--;
    if(com_priv.ref == 0)
    {
        lapi_thread_destroy(com_priv.thread);
        com_priv.thread = NULL;
        com_priv.first = 0;
    }

    lapi_mutex_unlock(com_priv.s_comm_mtx);

    return 0;
}
