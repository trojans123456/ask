#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/un.h>
#endif
#include "task.h"
#include "buffer.h"
#include "network.h"

// 设置函数指针，当sox需要设置写集时调用
typedef void (*cb_sock_writing_func)(hSock h, void *ctx);

/* event callback */
typedef struct
{
    cb_sock_writing_func func; //设置写集
    void *ctx;
}writing_func_t;

/* sock 对象 */
typedef struct
{
    int fd;
    strbuf_t tx_buf;/*tcp发送缓冲区*/
    hMutex mut;
    int ready; /*ready事件*/
    int connected;/*是否已经连接*/
    int listening;/*当前是否监听状态*/
    int close_async;/*异步关闭*/
    int is_udp;
    int is_detach;
    writing_func_t wf;
    ready_cb_ptr cb_ready;
    IPAddress rmt_addr;
}sock_t;

static void set_writing(sock_t *s)
{
    if(s->wf.func)
    {
        s->wf.func((hSock)s,s->wf.ctx);
    }
}

static hSock init_sock(int fd,int bufsize,int connected)
{
    sock_t *s = NULL;

    s = (sock_t *)malloc(sizeof(sock_t));
    if (!s)
    {
        close(fd);
        return NULL;
    }
    memset(s, 0, sizeof(sock_t));

    // 设置非阻塞
    {
        int on = 1;
        ioctl(fd, FIONBIO, &on);
    }

    s->fd = fd;
    s->connected = connected;
    s->mut = lapi_mutex_create();

    strbuf_init(&s->tx_buf, bufsize);

    return (hSock)s;
}

hSock sock_tcp(const IPAddress *local_addr, int bufsize, int detach)
{
    int fd = -1;
    sock_t *s = NULL;
    fd = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0)
    {
        printf("socket fail, errno = %d\n", errno);
        return NULL;
    }

    /* set socket option */
    {
        int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on));
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    }

    if (local_addr)
    {
        int ret = 0;

        struct sockaddr_in tSvrINAddr;
        memset(&tSvrINAddr, 0, sizeof(tSvrINAddr));
        tSvrINAddr.sin_family = AF_INET;
        tSvrINAddr.sin_addr.s_addr = inet_addr(local_addr->ip);
        tSvrINAddr.sin_port = htons(local_addr->port);

        ret = bind(fd, (struct sockaddr *)&tSvrINAddr, sizeof(tSvrINAddr));
        if (ret < 0)
        {
            printf("bind[%s:%d] error, errno = %d\n", local_addr->ip, local_addr->port, errno);
            close(fd);
            return NULL;
        }
    }
    s = (sock_t*)init_sock(fd, bufsize, 0);
    if(!s)
        return NULL;

    s->is_detach = detach;

    return (hSock)s;
}

hSock sock_udp(const IPAddress *local_addr, int detach)
{
    int fd = -1;
    sock_t *s = NULL;

    fd = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0)
    {
        printf("socket fail, errno = %d\n", errno);
        return NULL;
    }

    if (local_addr)
    {
        int ret = 0;

        struct sockaddr_in tSvrINAddr;
        memset(&tSvrINAddr, 0, sizeof(tSvrINAddr));
        tSvrINAddr.sin_family = AF_INET;
        tSvrINAddr.sin_addr.s_addr = inet_addr(local_addr->ip);
        tSvrINAddr.sin_port = htons(local_addr->port);

        ret = bind(fd, (struct sockaddr *)&tSvrINAddr, sizeof(tSvrINAddr));
        if (ret < 0)
        {
            printf("bind[%s:%d] error, errno = %d\n", local_addr->ip, local_addr->port, errno);
            close(fd);
            return NULL;
        }
    }

    s = (sock_t *)init_sock(fd, 0, 1);
    if(!s)
        return NULL;
    s->is_detach = detach;
    s->is_udp = 1;

    return (hSock)s;
}

hSock sock_unix(const char *path,int is_server,int detach)
{
    sock_t *sock = NULL;
    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    if(!path)
        return NULL;
    if(strlen(path) >= sizeof(sun.sun_path))
        return NULL;

    strcpy(sun.sun_path,path);

    int fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(fd < 0)
        return NULL;
    if(is_server > 0)
    {
        /* bind */
        if(bind(fd,(struct sockaddr*)&sun,sizeof(sun)) < 0)
        {
            printf("bind error %s \n",strerror(errno));
            close(fd);
            return NULL;
        }
    }
    sock = (sock_t*)init_sock(fd,0,0);
    if(!sock)
        return NULL;

    sock->is_detach = detach;
    return (hSock)sock;
}

void sock_close(hSock h)
{
    sock_t *s = (sock_t*)h;
    if(s)
    {
        if(s->fd > 0)
            close(s->fd);
        lapi_mutex_destroy(s->mut);

        strbuf_release(&s->tx_buf);
        free(s);
    }
}

void sock_close_async(hSock h)
{
    sock_t *s = (sock_t*)h;
    if(s)
    {
        s->close_async = 1;
        if(s->is_detach > 0)
        {
            /* 异步关闭需要设置写集 */
            set_writing(s);
        }
    }
}

int sock_listen(hSock h, int max)
{
    int ret;
    sock_t *s = (sock_t *)h;

    if (!s)
    {
        return -1;
    }

    s->listening = 1;

    ret = (listen(s->fd, max));
    if (ret < 0)
    {
        printf("listen error, errno = %d\n", errno);
        return -1;
    }

    return 0;
}

hSock sock_accept(hSock h, int bufsize)
{
    struct sockaddr_in tAddr;
    unsigned int nLen;
    int new_fd;
    sock_t *s = (sock_t *)h;

    if (!s)
    {
        return NULL;
    }

    memset(&tAddr, 0, sizeof(tAddr));
    nLen = (unsigned int)sizeof(tAddr);

    new_fd = (int)accept(s->fd, (struct sockaddr*)&tAddr, &nLen);
    if (new_fd < 0)
    {
        printf("accept error, errno = %d\n", errno);
        return NULL;
    }

    return init_sock(new_fd, bufsize, 1);
}

int sock_connect(hSock h, const IPAddress *remote_addr)
{
    int ret;
    sock_t *s = (sock_t *)h;
    struct sockaddr_in tSvrINAddr;

    if (!s || !remote_addr)
    {
        return -1;
    }

    memcpy(&s->rmt_addr, remote_addr, sizeof(s->rmt_addr));

    // already connected
    if (s->connected)
    {
        return 0;
    }

    memset(&tSvrINAddr, 0, sizeof(tSvrINAddr));
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_addr.s_addr = inet_addr(remote_addr->ip);
    tSvrINAddr.sin_port = htons(remote_addr->port);

    ret = connect(s->fd, (struct sockaddr*)&tSvrINAddr, sizeof(tSvrINAddr));
    if (ret == -1)
    {
        if (errno == EWOULDBLOCK || errno == EINPROGRESS)
        {
            ret = 0;
        }
        else
        {
            printf("connect error, errno = %d\n", errno);
        }
    }

    if(ret == 0 && (s->is_detach > 0))
    {
        // 异步连接需要设置写集
        set_writing(s);
    }

    return ret;
}

int sock_sendto(hSock h, const char *buf, int len, const IPAddress *rmt_addr)
{
    int ret;
    struct sockaddr_in tAddr;
    sock_t *s = (sock_t *)h;

    if (!s || !buf || len <= 0 || !rmt_addr)
    {
        return -1;
    }


    memset(&tAddr, 0, sizeof(tAddr));
    tAddr.sin_family = AF_INET;
    tAddr.sin_addr.s_addr = inet_addr(rmt_addr->ip);
    tAddr.sin_port = htons(rmt_addr->port);

    ret = sendto(s->fd, buf, len, 0, (struct sockaddr*)&tAddr, sizeof(tAddr));
    if (ret < 0)
    {
        printf("sendto[%s:%d] error, errno = %d\n", rmt_addr->ip, rmt_addr->port, errno);
    }

    return ret;
}

int sock_recvfrom(hSock h, char *buf, int len, IPAddress *rmt_addr)
{
    int ret;
    struct sockaddr_in tAddr;
    unsigned int addrLen = (unsigned int)sizeof(tAddr);
    sock_t *s = (sock_t *)h;

    if (!s || !buf || len <= 0 || !rmt_addr)
    {
        return -1;
    }


    ret = recvfrom(s->fd, buf, len, 0, (struct sockaddr*)&tAddr, &addrLen);
    if (ret < 0)
    {
        if (errno == EAGAIN)
        {
            ret = 0;
        }
        else
        {
            printf("recvfrom error, errno = %d\n", errno);
        }
    }
    else
    {
        unsigned char *ip = (unsigned char *)&(tAddr.sin_addr.s_addr);
        sprintf(rmt_addr->ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        rmt_addr->port = (int)ntohs(tAddr.sin_port);
    }

    return ret;
}


static void tcp_resend(sock_t *s)
{
    strbuf_t *fb = &s->tx_buf;
    if (fb->len > 0)
    {
        //如果没有发送完呢？
        int ret = send(s->fd, fb->buf, fb->len, 0);
        if (ret > 0)
        {
            fb->consume(fb, ret);
        }
    }
}

static int sock_send_internal(sock_t *s, const char *buf, int len, int async)
{
    int ret;
    strbuf_t *fb = &s->tx_buf;

    // 若缓冲有数据，先发一下缓冲内的
    if (fb->len > 0)
    {
        tcp_resend(s);
    }

    // 若缓冲还有数据，或者需要异步发送
    if (fb->len > 0 || async)
    {
        if (fb->append(fb, buf, len) < 0)
        {
            printf("Tcp send too large.\n");
            return -1;
        }

        return len;
    }

    // 若缓冲中无数据，直接发送
    ret = send(s->fd, buf, len, 0);
    if (ret < 0)
    {
        // 这种情况是表明发不出去，也就是说发送字节数为0
        if (errno == EWOULDBLOCK)
        {
            ret = 0;
        }
        // 其他情况表示socket有错误
        else
        {

            printf("send error, fd=%d errno = %d\n",s->fd,  errno);
            return -1;
        }
    }

    // 发送字节数不到应发的长度，需要保存剩下的数据到缓冲内
    if (ret < len)
    {
        if (fb->append(fb, buf + ret, len - ret) < 0)
        {
            printf("Tcp send incomplete.\n");
            return ret;//返回实际发送长度
        }
    }

    return len;
}

int sock_send(hSock h, const char *buf, int len)
{
    int ret;
    sock_t *s = (sock_t *)h;

    if (s->is_udp)
    {
        return sock_sendto(h, buf, len, &s->rmt_addr);
    }

    if (!s || !buf || len <= 0)
    {
        return -1;
    }

    // 防止和resend竞争，需要加锁
    lapi_mutex_lock(s->mut);
    ret = sock_send_internal(s, buf, len, 0);
    lapi_mutex_unlock(s->mut);

    return ret;
}

int sock_send_async(hSock h, char *buf, int len)
{
    int ret;
    sock_t *s = (sock_t *)h;

    if (s->is_udp)
    {
        return sock_sendto(h, buf, len, &s->rmt_addr);
    }

    if (!s || !buf || len <= 0)
    {
        return -1;
    }

    // 防止和resend竞争，需要加锁
    lapi_mutex_lock(s->mut);
    ret = sock_send_internal(s, buf, len, 1);
    lapi_mutex_unlock(s->mut);

    return ret;
}

int sock_recv(hSock h, char *buf, int len)
{
    int recvlen = 0;
    sock_t *s = (sock_t *)h;

    if (!s || !buf || len <= 0)
    {
        return -1;
    }

    if (s->ready == SOCK_CLOSE)
    {
        return -1;
    }

    recvlen = recv(s->fd, buf, len, 0);
    if (recvlen < 0)
    {
        if (errno == EAGAIN)
        {
            recvlen = 0;
        }
        else
        {
            printf("recv error, errno = %d\n", errno);
        }
    }

    return recvlen;
}

int sock_setsndbuf(hSock h, int size)
{
    sock_t *s = (sock_t *)h;

    if (!s)
    {
        return -1;
    }

    setsockopt(s->fd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));

    return 0;
}

int sock_setrcvbuf(hSock h, int size)
{
    sock_t *s = (sock_t *)h;

    if (!s)
    {
        return -1;
    }

    setsockopt(s->fd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));

    return 0;
}

int sock_set_reused(hSock h)
{
    sock_t *s = (sock_t *)h;
    if(!s)
        return -1;

    const int one = 1;
    setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    return 0;
}

int sock_getrmtaddr(hSock h, IPAddress *addr)
{
    struct sockaddr_in tAddr;
    unsigned int nLen;
    int ret;
    sock_t *s = (sock_t *)h;
    unsigned char *ip;

    if (!s || !addr)
    {
        return -1;
    }

    memset(&tAddr, 0, sizeof(tAddr));
    nLen = (socklen_t)sizeof(tAddr);

    ret = getpeername(s->fd, (struct sockaddr*)&tAddr, &nLen);
    if (ret != 0)
    {
        return -1;
    }

    ip = (unsigned char *)&(tAddr.sin_addr.s_addr);
    sprintf(addr->ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    addr->port = (int)ntohs(tAddr.sin_port);

    return 0;
}

int sock_getlocaladdr(hSock h, IPAddress *addr)
{
    struct sockaddr_in tAddr;
    unsigned int nLen;
    int ret;
    sock_t *s = (sock_t *)h;
    unsigned char *ip;

    if (!s || !addr)
    {
        return -1;
    }

    memset(&tAddr, 0, sizeof(tAddr));
    nLen = (socklen_t)sizeof(tAddr);

    ret = getsockname(s->fd, (struct sockaddr*)&tAddr, &nLen);
    if (ret != 0)
    {
        return -1;
    }

    ip = (unsigned char *)&(tAddr.sin_addr.s_addr);
    sprintf(addr->ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    addr->port = (int)ntohs(tAddr.sin_port);

    return 0;
}

void sock_set_detach(hSock h)
{
    sock_t *s = (sock_t*)h;
    if(s)
    {
        s->is_detach = 1;
    }
}

int sock_get_fd(hSock h)
{
    sock_t *s = (sock_t*)h;
    return s ? s->fd : -1;
}

static uint8_t isBigEndian()
{
    int  i = 1;   			/* i = 0x00000001*/
    char  c = *(char  *)&i; 	/* 注意不能写成 char c = (char)i; */
    return  (int )c != i;
}

uint64_t sock_htonll(uint64_t val)
{
    if (isBigEndian() == 0 )
    {
        return (((uint64_t )htonl((int)((val << 32) >> 32))) << 32) | (uint64_t)htonl((int)(val >> 32));
    }
    else
    {
        return val;
    }
}

uint64_t sock_ntohll(uint64_t val)
{
    if (isBigEndian() == 0)
    {
        return (((uint64_t )htonl((int)((val << 32) >> 32))) << 32) | (uint32_t)htonl((int)(val >> 32));
    }
    else
    {
         return val;
    }
}

/*************** detach ***************/

int sock_ready(hSock h)
{
    sock_t *s = (sock_t*)h;
    if(!s)
        return -1;
    return s->ready;
}

void sock_ready_reset(hSock h)
{
    sock_t *s = (sock_t*)h;
    if(s)
    {
        s->ready = SOCK_NONE;
    }
}

int sock_set_writing_func(hSock h, cb_sock_writing_func func, void *ctx)
{
    sock_t *s = (sock_t*)h;
    if(!s)
        return -1;
    s->wf.func = func;
    s->wf.ctx = ctx;

    return 0;
}

// 检查socket rcvbuf里是否有数据
static int socket_avail(sock_t *s)
{
    int len = 0;

    ioctl(s->fd, FIONREAD, &len);

    return len;
}

void sock_check_event(hSock h, int ev)
{
    sock_t *s = (sock_t *)h;

    if (ev == SOCK_READABLE)
    {
        // 监听可读，说明有套接字连接过来
        if (s->listening)
        {
            s->ready = SOCK_NEW_SOCKET;
        }
        // 可读，但没有数据，说明对端关闭
        else if (socket_avail(s) <= 0)
        {
            s->ready = SOCK_CLOSE;
        }
        // 可读，有数据
        else
        {
            s->ready = SOCK_DATA;
        }
    }
    else if (ev == SOCK_WRITEABLE)
    {
        // 可写，检查异步关闭标识，此时回调CLOSE
        if (s->close_async)
        {
            s->ready = SOCK_CLOSE;
        }
        // 可写，并且是连接状态，可以重发TCP缓冲
        else if (s->connected)
        {
            s->ready = SOCK_RESEND;

            // 防止和send竞争，需要加锁
            lapi_mutex_lock(s->mut);
            tcp_resend(s);
            lapi_mutex_unlock(s->mut);
        }
        // 当前正在连接时发现可写，说明连接成功
        else
        {
            s->ready = SOCK_CONNECT_OK;
            s->connected = 1;
        }
    }
    else if (ev == SOCK_EXCEPTION)
    {
        s->ready = SOCK_CLOSE;
    }
}

int sock_is_reading(hSock h)
{
    sock_t *s = (sock_t *)h;

    if (!s)
    {
        return 0;
    }

    // 如果已连接上，或者是监听，则设置读集
    if (s->connected || s->listening)
    {
        return 1;
    }

    return 0;
}

int sock_is_writing(hSock h)
{
    sock_t *s = (sock_t *)h;

    if (!s)
    {
        return 0;
    }

    // TCP缓冲有数据，或者正在connect，或者设置了异步关闭标识，则需要设置写集
    if (s->tx_buf.len > 0 || !s->connected || s->close_async)
    {
        return 1;
    }

    return 0;
}


typedef struct ios__
{
    void (*destroy)(struct ios__ *ios);
    int (*add)(struct ios__ *ios,hSock h);
    int (*del)(struct ios__ *ios,hSock h);
    int (*wait)(struct ios__ *ios,int timeout);
    hSock *evarr; /** 产生事件的sox数组*/
    void *pri; /* pointer ep_box */
    hMutex mtx;
}ios_t;

typedef struct
{
    int epfd;
    int size;
    int num;
    struct epoll_event *evarr;
}ep_box;

static void i_destroy(ios_t *ios)
{
    if(ios)
    {
        ep_box *epb = (ep_box*)ios->pri;
        if(epb)
        {
            close(epb->epfd);
            if(epb->evarr)
                free(epb->evarr);
            free(epb);
        }
        if(ios->evarr)
            free(ios->evarr);

        lapi_mutex_destroy(ios->mtx);
    }
}

static int _modify_writing_set(ios_t *ios, hSock h, int wr)
{
    ep_box *epb = (ep_box *)ios->pri;
    struct epoll_event ev = {0};
    int fd = sock_get_fd(h);
    int ret = 0;

    ev.data.ptr = (void *)h;
    ev.events = EPOLLIN;//固定设置读集

    if (wr)
    {
        ev.events |= EPOLLOUT;
    }
    else
    {
        ev.events &= ~EPOLLOUT;
    }

    lapi_mutex_lock(ios->mtx);
    ret = epoll_ctl(epb->epfd, EPOLL_CTL_MOD, fd, &ev);
    lapi_mutex_unlock(ios->mtx);

// 	printf("_modify_writing_set %d\n", wr);

    return ret;
}

static void func_set_writing(hSock h,void *ctx)
{
    ios_t *ios = (ios_t*)ctx;
    if(ios)
    {
        _modify_writing_set(ios,h,1);
    }
}

static int i_add(ios_t *ios,hSock h)
{
    ep_box *epb = (ep_box*)ios->pri;
    struct epoll_event ev;
    int fd = sock_get_fd(h);
    int ret = 0;

    if(!ios)
        return -1;

    if(epb->num >= epb->size)
        return -1;

    ev.data.ptr = (void *)h;
    ev.events = EPOLLIN | EPOLLHUP;/*read */

    lapi_mutex_lock(ios->mtx);

    ret = epoll_ctl(epb->epfd,EPOLL_CTL_ADD,fd,&ev);
    printf("add ret = %d\n",ret);
    if(ret == 0)
    {
        printf("epb->num++\n");
        epb->num++;
    }
    lapi_mutex_unlock(ios->mtx);

    // 另外，设置监听写集函数
    sock_set_writing_func(h, func_set_writing, (void *)ios);

    return ret;
}


static int i_del(ios_t *ios, hSock h)
{
    ep_box *epb = (ep_box *)ios->pri;
    int fd = sock_get_fd(h);
    int ret = 0;

    if (!ios)
    {
        return -1;
    }

    lapi_mutex_lock(ios->mtx);

    ret = epoll_ctl(epb->epfd, EPOLL_CTL_DEL, fd, NULL);
    if (ret == 0)
    {
        epb->num--;
    }

    lapi_mutex_unlock(ios->mtx);

    return ret;
}

static int i_wait(ios_t *ios, int timeout)
{
    int ev = 0;
    ep_box *epb = (ep_box *)ios->pri;
    int i;

    if (!ios)
    {
        return -1;
    }

    ev = epoll_wait(epb->epfd, epb->evarr, epb->size, timeout);

    if (ev < 0)
    {
        printf("epoll_wait err[%d]\n", errno);
        return -1;
    }

    hSock sox;
    for (i = 0; i < ev; i++)
    {
        sox = (hSock)epb->evarr[i].data.ptr;

        if (epb->evarr[i].events & EPOLLERR)
        {
            sock_check_event(sox, SOCK_EXCEPTION);
        }

        if (epb->evarr[i].events & EPOLLHUP)
        {
            if (sock_is_reading(sox))
            {
                sock_check_event(sox, SOCK_EXCEPTION);
            }
        }

        if (epb->evarr[i].events & EPOLLIN)
        {
            sock_check_event(sox, SOCK_READABLE);
        }

        if (epb->evarr[i].events & EPOLLOUT)
        {
            sock_check_event(sox, SOCK_WRITEABLE);

            // 如果sox不再需要监听写集，则去掉写集
            if (!sock_is_writing(sox))
            {
                _modify_writing_set(ios, sox, 0);
            }
        }

        ios->evarr[i] = sox;
    }

    return ev;
}

int ios_epoll_init(ios_t *ios,int maxfds)
{
    ep_box *epb = NULL;
    if(!ios)
        return -1;

    ios->mtx = lapi_mutex_create();
    ios->destroy = i_destroy;
    ios->add = i_add;
    ios->del = i_del;
    ios->wait = i_wait;

    ios->evarr = (hSock*)calloc(maxfds,sizeof(hSock));
    if(!ios->evarr)
    {
        return -1;
    }
    epb = (ep_box *)calloc(1,sizeof(ep_box));
    if(!epb)
    {
        free(ios->evarr);
        return -1;
    }
    ios->pri = (void *)epb;

    epb->epfd = epoll_create(maxfds);
    if(epb->epfd < 0)
    {
        free(epb);
        free(ios->evarr);
        return -1;
    }

    epb->size = maxfds;
    epb->num = 0;
    epb->evarr = (struct epoll_event *)calloc(maxfds,sizeof(struct epoll_event));
    if(!epb->evarr)
    {
        close(epb->epfd);
        free(epb);
        free(ios->evarr);
        return -1;
    }

    return 0;
}

typedef struct
{
    ios_t ios;
    hThread thread;
    int ref;
    int first;
    hMutex mtx;
}sock_priv_t;

static sock_priv_t sock_priv;

void *sock_recv_task(hThread h,void *p)
{
    int i,ev;
    ios_t *ios = NULL;
    if(!p)
        return NULL;
    ios = (ios_t*)p;
    hSock s = NULL;
    sock_t *sock = NULL;
    while(lapi_thread_isrunning(h))
    {
        ev = ios->wait(ios,3600);

        if(ev <= 0)
        {
            lapi_sleep(10);
            continue;
        }

        for(i = 0;i < ev;i++)
        {
            s = ios->evarr[i];
            sock = (sock_t*)s;
            switch(sock_ready(s))
            {
            case SOCK_NEW_SOCKET:
            {
                if(sock->cb_ready.accept_callback)
                {
                    sock->cb_ready.accept_callback(s,sock->cb_ready.priv);
                }
            }
                break;
            case SOCK_CLOSE:
            {
                if(sock->cb_ready.close_callback)
                {
                    sock->cb_ready.close_callback(s,sock->cb_ready.close_callback);
                }
            }
                break;
            case SOCK_DATA:
            {
                if(sock->cb_ready.read_callback)
                {
                    sock->cb_ready.read_callback(s,sock->cb_ready.priv);
                }
            }
                break;
            default:
                break;
            }
        }
    }
    printf("2222222222\n");
    return NULL;
}

int sock_start(hSock h)
{
    printf("(((((\n");
    sock_t *sock = (sock_t*)h;
    if(!sock)
    {
        printf("sock is NULL\n");
        return -1;
    }

    printf("#######\n");
    if(sock->is_detach <= 0)
        return -1;
    printf("*****\n");
    if(sock_priv.first == 0)
    {
        sock_priv.mtx = lapi_mutex_create();
        ios_epoll_init(&sock_priv.ios,30);
    }
    printf("sock_priv = %d\n",sock_priv.first);
    lapi_mutex_lock(sock_priv.mtx);
    printf("sock add....\n");
    sock_priv.ios.add(&sock_priv.ios,h);

    sock_priv.ref++;
    lapi_mutex_unlock(sock_priv.mtx);
    printf("sock_priv = %d\n",sock_priv.first);
    if(sock_priv.first == 0)
    {
        sock_priv.first = 1;
        printf("3333\n");
        sock_priv.thread = lapi_thread_create(sock_recv_task,&sock_priv.ios,1 << 10 <<10);
    }

    return 0;
}

int sock_stop(hSock h)
{
    sock_t *sock = (sock_t*)h;
    if(!sock)
        return -1;
    if(sock->is_detach <= 0)
        return -1;

    lapi_mutex_lock(sock_priv.mtx);

    sock_priv.ios.del(&sock_priv.ios,h);

    sock_priv.ref--;
    if(sock_priv.ref == 0)
    {
        lapi_thread_destroy(sock_priv.thread);
        sock_priv.thread = NULL;
        sock_priv.first = 0;

        sock_priv.ios.destroy(&sock_priv.ios);
    }

    lapi_mutex_unlock(sock_priv.mtx);

    return 0;
}

int sock_setRxDataCallBack(hSock h, ready_cb_ptr *func)
{
    sock_t *s = (sock_t*)h;
    if(!s)
    {
        printf("set callback s empty\n");
        return -1;
    }


    if(s->is_detach <= 0)
    {
        printf("is_detach  = %d\n",s->is_detach);
        return -1;
    }

    s->cb_ready.accept_callback = func->accept_callback;
    s->cb_ready.close_callback = func->close_callback;
    s->cb_ready.connect_callback = func->connect_callback;
    s->cb_ready.read_callback = func->read_callback;
    s->cb_ready.priv = func->priv;
    return 0;
}
