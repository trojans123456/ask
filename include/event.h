#ifndef __EVENT_H
#define __EVENT_H

#include "default.h"

C_API_BEGIN

declear_handler(hEventLoop);

#define EVENT_MASK_NONE     0
#define EVENT_MASK_READABLE 1
#define EVENT_MASK_WRITABLE 2

typedef void (*file_event_proc)(hEventLoop h,int fd,int mask,void *priv);
typedef void (*time_event_proc)(hEventLoop h,int id,void *priv);

typedef struct file_event
{
    int mask;
    file_event_proc *read_proc;
    file_event_proc *write_proc;
    void *priv_data;
}FileEvent_t;

typedef struct time_event
{
    int id; /* time event identifier*/
    long when_sec;/* seconds */
    long when_ms;/* milliseconds */
    time_event_proc *time_proc;
    void *priv_data;
    struct time_event *next;
}TimeEvent_t;



hEventLoop *event_loop_create();
void event_loop_destroy(hEventLoop h);

void event_loop_stop(hEventLoop h);

/** create event and add to event loop */
int create_file_event(hEventLoop h,FileEvent_t *file_ev);
void delete_file_event(hEventLoop h,int fd,int mask);

int create_time_event(hEventLoop h,TimeEvent_t *time_ev);
int delete_time_event(hEventLoop h,int id);

int process_events(hEventLoop h);
int event_wait(int fd,int mask,long long milliseconds);

void event_main(hEventLoop h);



C_API_END

#endif // __EVENT_h
