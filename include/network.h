#ifndef __NETWORK_H
#define __NETWORK_H

#include "default.h"

C_API_BEGIN

declear_handler(hSock);

#define TX_BUFSIZE  1024
/* ipaddr */
typedef struct
{
    int port;
    char ip[64];
}IPAddress;

/** ready事件 */
enum
{
    SOCK_NONE = 0,
    SOCK_DATA,		// 有数据过来
    SOCK_NEW_SOCKET,	// 有新连接过来（针对监听sox）
    SOCK_CONNECT_OK,	// 连接成功
    SOCK_CLOSE,		// 连接关闭
    SOCK_RESEND,		// 有重发数据
};

/** 根据底层事件，判断sox处于具体哪种ready事件 */
enum
{
    SOCK_READABLE,
    SOCK_WRITEABLE,
    SOCK_EXCEPTION
};

/*创建句柄*/
hSock sock_tcp(const IPAddress *local_addr,int bufsize,int detach);
hSock sock_udp(const IPAddress *local_addr,int detach);

void sock_close(hSock h);
void sock_close_async(hSock h); /*异步关闭*/

/* 监听 连接*/
int sock_listen(hSock h,int max);
hSock sock_accept(hSock h,int bufsize);
int sock_connect(hSock h,const IPAddress *remote_addr);


/* 发送接收 */
int sock_sendto(hSock h, const char *buf, int len, const IPAddress *rmt_addr);
int sock_recvfrom(hSock h, char *buf, int len, IPAddress *rmt_addr);
int sock_send(hSock h, const char *buf, int len);
int sock_send_async(hSock h, char *buf, int len);	//异步发送
int sock_recv(hSock h, char *buf, int len);

int sock_setsndbuf(hSock h, int size);
int sock_setrcvbuf(hSock h, int size);

int sock_getrmtaddr(hSock h, IPAddress *addr);
int sock_getlocaladdr(hSock h, IPAddress *addr);

void sock_setctx(hSock h, void *context);
void *sock_getctx(hSock h);

int sock_get_fd(hSock h);
int sock_detach_fd(hSock h);//取出fd，之后sox中不再有fd

int sock_get_tcptxbs(hSock h);			// 获取TCP可靠发送缓冲大小
int sock_get_tcptxbs_used(hSock h);		// 获取TCP可靠发送缓冲已用大小

int sock_get_fd(hSock h);

uint64_t sock_htonll(uint64_t val);
uint64_t sock_ntohll(uint64_t val);

/******************** detach ************************/

int sock_start(hSock h);
int sock_stop(hSock h);



/** 获取ready事件 */
int sock_ready(hSock h);
/** 清空ready状态 */
void sock_ready_reset(hSock h);


/** 判断sox是否需要设置读集 */
int sock_is_reading(hSock h);
/** 判断sox是否需要设置写集 */
 int sock_is_writing(hSock h);

/** 验证socket的事件，将事件细分 */
void sock_check_event(hSock h, int ev);

typedef void (*cb_recv_data_ptr)(hSock h,void *ctx);
int sock_setRxDataCallBack(hSock h,cb_recv_data_ptr func,void *ctx);



C_API_END

#endif // __NETWORK_H
