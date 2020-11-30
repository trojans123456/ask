#if defined(USED_SELECT)

#ifdef __linux__
#include <sys/select.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ev_select.h"


typedef struct evApiState {
    fd_set rfds, wfds;
    /* We need to have a copy of the fd sets as it's not safe to reuse
     * FD sets after select(). */
    fd_set _rfds, _wfds;
} evApiState;

static int evApiCreate(evEventLoop *eventLoop) {
    evApiState *state = malloc(sizeof(evApiState));

    if (!state) return -1;
    FD_ZERO(&state->rfds);
    FD_ZERO(&state->wfds);
    eventLoop->apidata = state;
    return 0;
}

static void evApiFree(evEventLoop *eventLoop) {
    free(eventLoop->apidata);
}

static int evApiAddEvent(evEventLoop *eventLoop, int fd, int mask) {
    evApiState *state = eventLoop->apidata;

    if (mask & EVENT_READABLE) FD_SET(fd,&state->rfds);
    if (mask & EVENT_WRITABLE) FD_SET(fd,&state->wfds);
    return 0;
}

static void evApiDelEvent(evEventLoop *eventLoop, int fd, int mask) {
    evApiState *state = eventLoop->apidata;

    if (mask & EVENT_READABLE) FD_CLR(fd,&state->rfds);
    if (mask & EVENT_WRITABLE) FD_CLR(fd,&state->wfds);
}

static int evApiPoll(evEventLoop *eventLoop, struct timeval *tvp) {
    evApiState *state = eventLoop->apidata;
    int retval, j, numevents = 0;

    memcpy(&state->_rfds,&state->rfds,sizeof(fd_set));
    memcpy(&state->_wfds,&state->wfds,sizeof(fd_set));

    retval = select(eventLoop->maxfd+1,
                &state->_rfds,&state->_wfds,NULL,tvp);
    if (retval > 0) {
        for (j = 0; j <= eventLoop->maxfd; j++) {
            int mask = 0;
            evFileEvent *fe = &eventLoop->events[j];

            if (fe->mask == EVENT_NONE) continue;
            if (fe->mask & EVENT_READABLE && FD_ISSET(j,&state->_rfds))
                mask |= EVENT_READABLE;
            if (fe->mask & EVENT_WRITABLE && FD_ISSET(j,&state->_wfds))
                mask |= EVENT_WRITABLE;
            eventLoop->fired[numevents].fd = j;
            eventLoop->fired[numevents].mask = mask;
            numevents++;
        }
    }
    return numevents;
}

static char *evApiName(void) {
    return "select";
}

struct event_ops select_ops =
{
    evApiCreate,
    evApiFree,
    evApiAddEvent,
    evApiDelEvent,
    evApiPoll,
    evApiName
};

#endif
