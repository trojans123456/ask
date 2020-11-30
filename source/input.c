#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/input.h>
#include <errno.h>
#endif
#include "task.h"
#include "list.h"
#include "input.h"

typedef struct
{
    int fd;
    cb_input_rxdata_func cbRxData;
    void *ctx;
    struct input_event msgbuffer;
    struct dl_list list;
}input_t;

typedef struct
{
    hMutex mtx;
    hThread thread;
    int ref;
    int first;
    struct dl_list input_head;
}input_priv_t;
static input_priv_t input_priv;

hInput input_open(const char *name)
{
    if(!name)
        return NULL;
    input_t *input = (input_t*)calloc(1,sizeof(input_t));
    if(!input)
        return NULL;

    input->fd = open(name,O_RDONLY);
    if(input->fd < 0)
    {
        free(input);
        return NULL;
    }

    return (hInput)input;
}

int input_close(hInput h)
{
    input_t *input = (input_t*)h;
    if(!input)
        return -1;

    close(input->fd);
    free(input);

    return 0;
}

int input_setRxDataFunc(hInput h, cb_input_rxdata_func func, void *ctx)
{
    input_t *comm = (input_t *)h;
    if(!comm)
        return -1;
    comm->cbRxData = func;
    comm->ctx = ctx;

    return 0;
}


void *input_recv_task(hThread h,void *p)
{
    int ret,len;
    fd_set readfds;
    //int timeout = 1000;/* 1s */
    input_t *comm = NULL;
    int maxfd = 0;
    struct timeval tm = {30,0};
    while(lapi_thread_isrunning(h))
    {
        FD_ZERO(&readfds);
        lapi_mutex_lock(input_priv.mtx);
        dl_list_for_each(comm,&input_priv.input_head,input_t,list)
        {
            if(comm)
            {
                FD_SET(comm->fd,&readfds);
                maxfd = MAX(maxfd,comm->fd);
            }
        }
        lapi_mutex_unlock(input_priv.mtx);

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

        lapi_mutex_lock(input_priv.mtx);

        dl_list_for_each(comm,&input_priv.input_head,input_t,list)
        {
            if(comm && FD_ISSET(comm->fd,&readfds))
            {

                len = read(comm->fd, &comm->msgbuffer, sizeof(comm->msgbuffer));

                if(len > 0)
                {
                    printf("msg = %d\n",comm->msgbuffer.type);
                }
            }
        }

        lapi_mutex_unlock(input_priv.mtx);
    }

    return NULL;
}

int input_start(hInput h)
{
    input_t *comm = (input_t*)h;
    if(!comm)
        return -1;


    if(input_priv.first == 0)
    {
        dl_list_init(&input_priv.input_head);
        input_priv.mtx = lapi_mutex_create();
    }

    lapi_mutex_lock(input_priv.mtx);
    dl_list_add_tail(&input_priv.input_head,&comm->list);
    input_priv.ref++;
    lapi_mutex_unlock(input_priv.mtx);

    if(input_priv.first == 0)
    {
        input_priv.first = 1;
        input_priv.thread = lapi_thread_create(input_recv_task,NULL,1 << 10 <<10);
    }

    return 0;
}

int input_stop(hInput h)
{
    input_t *comm = (input_t*)h;
    if(!comm)
        return -1;


    lapi_mutex_lock(input_priv.mtx);

    dl_list_del(&comm->list);
    input_priv.ref--;
    if(input_priv.ref == 0)
    {
        lapi_thread_destroy(input_priv.thread);
        input_priv.thread = NULL;
        input_priv.first = 0;
    }

    lapi_mutex_unlock(input_priv.mtx);

    if(input_priv.ref == 0)
    {
        lapi_mutex_destroy(input_priv.mtx);
    }

    return 0;
}
