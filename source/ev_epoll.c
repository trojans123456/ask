#if defined(USED_EPOLL)


#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include "ev_epoll.h"

typedef struct evApiState
{
    int epfd;
    struct epoll_event events[EVENT_SETSIZE];
}evApiState;

/**
 * @brief evApiCreate  创建一个 epoll
 * @param eventLoop
 * @return
 */
static int evApiCreate(evEventLoop *eventLoop) {
    evApiState *state = malloc(sizeof(evApiState));

    if (!state) return -1;
    state->epfd = epoll_create(EVENT_SETSIZE);/* 1024 */ /* 1024 is just an hint for the kernel */
    if (state->epfd == -1) return -1;
    eventLoop->apidata = state;
    return 0;
}


static void evApiFree(evEventLoop *eventLoop) {
    evApiState *state = eventLoop->apidata;

    close(state->epfd);
    free(state);
}

static int evApiAddEvent(evEventLoop *eventLoop, int fd, int mask) {
    evApiState *state = eventLoop->apidata;
    struct epoll_event ee;
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    int op = eventLoop->events[fd].mask == EVENT_NONE ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    mask |= eventLoop->events[fd].mask; /* Merge old events */
    if (mask & EVENT_READABLE) ee.events |= EPOLLIN;
    if (mask & EVENT_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (epoll_ctl(state->epfd,op,fd,&ee) == -1) return -1;
    return 0;
}

static void evApiDelEvent(evEventLoop *eventLoop, int fd, int delmask) {
    evApiState *state = eventLoop->apidata;
    struct epoll_event ee;
    int mask = eventLoop->events[fd].mask & (~delmask);

    ee.events = 0;
    if (mask & EVENT_READABLE) ee.events |= EPOLLIN;
    if (mask & EVENT_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; /* avoid valgrind warning */
    ee.data.fd = fd;
    if (mask != EVENT_NONE) {
        epoll_ctl(state->epfd,EPOLL_CTL_MOD,fd,&ee);
    } else {
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for
         * EPOLL_CTL_DEL. */
        epoll_ctl(state->epfd,EPOLL_CTL_DEL,fd,&ee);
    }
}

static int evApiPoll(evEventLoop *eventLoop, struct timeval *tvp) {
    evApiState *state = eventLoop->apidata;
    int retval, numevents = 0;

    retval = epoll_wait(state->epfd,state->events,EVENT_SETSIZE,
            tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);
    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = state->events+j;

            if (e->events & EPOLLIN) mask |= EVENT_READABLE;
            if (e->events & EPOLLOUT) mask |= EVENT_WRITABLE;
            eventLoop->fired[j].fd = e->data.fd;
            eventLoop->fired[j].mask = mask;
        }
    }
    return numevents;
}

static char *evApiName(void) {
    return "epoll";
}

struct event_ops epoll_ops =
{
    evApiCreate,
    evApiFree,
    evApiAddEvent,
    evApiDelEvent,
    evApiPoll,
    evApiName
};

#endif
