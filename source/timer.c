#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "list.h"
#include "task.h"
#include "timer.h"

typedef struct timeout_event
{
    struct dl_list list;
    uint8_t pending;/*添加一个timeout false时删除*/
    timeout_event_ptr cb;
    void *ctx;
    struct timeval time;/*超时时间*/
    enum loop_type type;
}timeout_event_t;

typedef struct
{
    struct dl_list timer_head;
    hMutex mtx;
    hThread thread;
    int ref;
    int first;
}timer_priv_t;
static timer_priv_t timer_priv;

static int tv_diff(struct timeval *t1,struct timeval *t2)
{
    return (t1->tv_sec - t2->tv_sec) * 1000 +
            (t1->tv_usec - t2->tv_usec) / 1000;
}

void event_gettime(struct timeval *tv)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    //printf("ts = %d\n",ts.tv_sec);
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000;
}

hTimer timeout_event_create(int tm_ms)
{
    struct timeout_event *tm_ev = NULL;
    tm_ev = (timeout_event_t*)calloc(1,sizeof(timeout_event_t));
    if(!tm_ev)
        return NULL;

    dl_list_init(&tm_ev->list);
    tm_ev->pending = 0;
    tm_ev->cb = NULL;
    tm_ev->time.tv_sec = (tm_ms / 1000);
    tm_ev->time.tv_usec = (tm_ms % 1000) * 1000;

    return (hTimer)tm_ev;
}

void timeout_event_destroy(hTimer h)
{
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(tm_ev)
    {
        timeout_event_cancel(h);
        tm_ev->pending = 1;
        free(tm_ev);
    }
}

void timeout_event_set_cb(hTimer h, timeout_event_ptr func, void *ctx)
{
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(tm_ev)
    {
        tm_ev->cb = func;
        tm_ev->ctx = ctx;
    }
}

int timeout_event_add(hTimer h)
{
    struct timeout_event *tmp = NULL;
    struct dl_list *head = NULL;

    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(!tm_ev)
        return -1;

    if(tm_ev->pending > 0)
        return -1;

    head = &timer_priv.timer_head;


    dl_list_for_each(tmp,&timer_priv.timer_head,timeout_event_t,list)
    {
        if(tv_diff(&tmp->time,&tm_ev->time) > 0)
        {
            head = &tmp->list;
            break;
        }
    }

    dl_list_add_tail(head,&tm_ev->list);
    tm_ev->pending = 1;


    return 0;
}

int timeout_event_set(hTimer h, int msecs)
{
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    struct timeval *time = &tm_ev->time;

    if(tm_ev->pending > 0)
    {
        timeout_event_cancel(h);
    }

    event_gettime(time);

    time->tv_sec += msecs / 1000;
    time->tv_usec += (msecs % 1000) * 1000;

    if(time->tv_usec > 1000000)
    {
        time->tv_sec++;
        time->tv_usec -= 1000000;
    }

    return timeout_event_add(h);
}

int timeout_event_cancel(hTimer h)
{
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(!tm_ev)
        return -1;
    if(tm_ev->pending <= 0)
        return -1;
    //printf("del list....\n");
    dl_list_del(&tm_ev->list);
    tm_ev->pending = 0;

    return 0;
}

int timeout_event_remain(hTimer h)
{
    struct timeval now;
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(!tm_ev)
        return -1;
    if(tm_ev->pending <= 0)
        return 0;

    event_gettime(&now);

    return tv_diff(&tm_ev->time,&now);
}

int event_get_next_timeout(struct dl_list *tm_head,struct timeval *tv)
{
    struct timeout_event *timeout;
    int diff;

    if(!tm_head)
        return -1;

    if(dl_list_empty(tm_head))
        return -1;

    timeout = dl_list_first(tm_head,timeout_event_t,list);
    diff = tv_diff(&timeout->time,tv);
    if(diff < 0)
        return -1;

    return diff;
}

void event_process_timeouts(struct dl_list *tm_head,struct timeval *tv)
{
    timeout_event_t *t;
    if(!tm_head)
        return ;

    while(!dl_list_empty(tm_head))
    {
        t = dl_list_first(tm_head,timeout_event_t,list);
        if(tv_diff(&t->time,tv) > 0)
        {
            //printf("tv...\n");
            break;
        }

        timeout_event_cancel((hTimer)t);

        if(t->cb)
        {
            t->cb((hTimer)t,t->ctx);
        }
    }
}

void *timer_task(hThread h,void *ctx)
{
    int next_time = 1;
    struct timeval tv;


    while(lapi_thread_isrunning(h))
    {
        event_gettime(&tv);
        event_process_timeouts(&timer_priv.timer_head,&tv);



        next_time = event_get_next_timeout(&timer_priv.timer_head,&tv);
        if(next_time < 0)
            next_time = 1000;
        //printf("next_time = %d\n",next_time);
        lapi_sleep(next_time);
    }

    return NULL;
}



int timer_start(hTimer h)
{
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(!tm_ev)
        return -1;

    if(timer_priv.first == 0)
    {
        timer_priv.mtx = lapi_mutex_create();
        dl_list_init(&timer_priv.timer_head);
    }

    lapi_mutex_lock(timer_priv.mtx);
    timer_priv.ref++;
    timeout_event_add(h);
    lapi_mutex_unlock(timer_priv.mtx);

    if(timer_priv.first == 0)
    {
        timer_priv.first = 1;
        timer_priv.thread = lapi_thread_create(timer_task,NULL,1 << 10 << 10);
    }

    return 0;
}

int timer_stop(hTimer h)
{
    timeout_event_t *tm_ev = (timeout_event_t*)h;
    if(!tm_ev)
        return -1;

    lapi_mutex_lock(timer_priv.mtx);
    timeout_event_destroy(h);
    timer_priv.ref--;
    if(timer_priv.ref == 0)
    {
        lapi_thread_destroy(timer_priv.thread);
        timer_priv.thread = NULL;
        timer_priv.first = 0;
    }
    lapi_mutex_unlock(timer_priv.mtx);

    if(timer_priv.ref == 0)
    {
        lapi_mutex_destroy(timer_priv.mtx);
    }
    return 0;
}




