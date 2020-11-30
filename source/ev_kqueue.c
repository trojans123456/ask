#if defined(USED_KQUEUE)

#include <sys/types.h>
#ifdef __linux__
#include <kqueue/sys/event.h>
#endif
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include "ev_kqueue.h"

typedef struct aeApiState {
    int kqfd;
    struct kevent events[EVENT_SETSIZE];
} evApiState;

static int evApiCreate(evEventLoop *eventLoop) {
    evApiState *state = malloc(sizeof(evApiState));

    if (!state) return -1;
    state->kqfd = kqueue();
    if (state->kqfd == -1) return -1;
    eventLoop->apidata = state;

    return 0;
}

static void evApiFree(evEventLoop *eventLoop) {
    evApiState *state = eventLoop->apidata;

    close(state->kqfd);
    free(state);
}

static int evApiAddEvent(evEventLoop *eventLoop, int fd, int mask) {
    evApiState *state = eventLoop->apidata;
    struct kevent ke;

    if (mask & EVENT_READABLE) {
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(state->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
    }
    if (mask & EVENT_WRITABLE) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(state->kqfd, &ke, 1, NULL, 0, NULL) == -1) return -1;
    }
    return 0;
}

static void evApiDelEvent(evEventLoop *eventLoop, int fd, int mask) {
    evApiState *state = eventLoop->apidata;
    struct kevent ke;

    if (mask & EVENT_READABLE) {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(state->kqfd, &ke, 1, NULL, 0, NULL);
    }
    if (mask & EVENT_WRITABLE) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        kevent(state->kqfd, &ke, 1, NULL, 0, NULL);
    }
}

static int evApiPoll(evEventLoop *eventLoop, struct timeval *tvp) {
    evApiState *state = eventLoop->apidata;
    int retval, numevents = 0;

    if (tvp != NULL) {
        struct timespec timeout;
        timeout.tv_sec = tvp->tv_sec;
        timeout.tv_nsec = tvp->tv_usec * 1000;
        retval = kevent(state->kqfd, NULL, 0, state->events, EVENT_SETSIZE, &timeout);
    } else {
        retval = kevent(state->kqfd, NULL, 0, state->events, EVENT_SETSIZE, NULL);
    }

    if (retval > 0) {
        int j;

        numevents = retval;
        for(j = 0; j < numevents; j++) {
            int mask = 0;
            struct kevent *e = state->events+j;

            if (e->filter == EVFILT_READ) mask |= EVENT_READABLE;
            if (e->filter == EVFILT_WRITE) mask |= EVENT_WRITABLE;
            eventLoop->fired[j].fd = e->ident;
            eventLoop->fired[j].mask = mask;
        }
    }
    return numevents;
}

static char *evApiName(void) {
    return "kqueue";
}

struct event_ops kqueue_ops =
{
    evApiCreate,
    evApiFree,
    evApiAddEvent,
    evApiDelEvent,
    evApiPoll,
    evApiName
};

#endif
