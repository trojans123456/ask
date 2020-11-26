#include <stdio.h>
#include <string.h>
#include "task.h"
#include "network.h"



void sock_close_cb(hSock h,void *ctx)
{
    sock_stop(h);
    sock_close(h);
}

void sock_read_cb(hSock h,void *ctx)
{
    char buffer[32] = "";
    sock_recv(h,buffer,sizeof(buffer));
    printf("buffer = %s\n",buffer);

    /* client */
    sock_send(h,"xxx",strlen("xxx") +1);
}

void sock_accept_cb(hSock h,void *ctx)
{
    /* tcp */
#if 0
    IPAddress addr;
    hSock new_sock = sock_accept(h,1024);
    if(!new_sock)
        return ;

    sock_getrmtaddr(new_sock,&addr);

    ready_cb_ptr rdy = {0};
    rdy.close_callback = sock_close_cb;
    rdy.read_callback = sock_read_cb;

    sock_set_detach(new_sock);
    sock_setRxDataCallBack(new_sock,&rdy);

    int ret = sock_start(new_sock);
    if(ret < 0)
    {
        sock_close(new_sock);
    }
#endif


    hSock new_sock = sock_accept(h,0);
    if(!new_sock)
        return ;

    ready_cb_ptr rdy = {0};
    rdy.close_callback = sock_close_cb;
    rdy.read_callback = sock_read_cb;
    sock_set_detach(new_sock);
    sock_setRxDataCallBack(new_sock,&rdy);

    int ret = sock_start(new_sock);
    if(ret <0)
        sock_close(new_sock);
}

int srv_main(int argc,char *argv[])
{
    IPAddress addr;
    addr.port = 6800;
    memcpy(addr.ip,"0.0.0.0",sizeof(addr.ip));

    hSock sock = sock_tcp(&addr,1024,1);
    if(!sock)
        return -1;

    sock_listen(sock,10);
    //sock_connect(sock,&addr);

    ready_cb_ptr srv_ready;
    srv_ready.accept_callback = sock_accept_cb;
    srv_ready.close_callback = sock_close_cb;
    srv_ready.read_callback = sock_read_cb;
    srv_ready.connect_callback = NULL;
    srv_ready.priv = NULL;

    sock_set_reused(sock);
    sock_setRxDataCallBack(sock,&srv_ready);

    sock_start(sock);
    printf("$$$$$$$$\n");


    while(1);
}

int cli_main(int argc,char *argv[])
{
    IPAddress addr;
    addr.port = 6800;
    memcpy(addr.ip,"172.23.233.108",sizeof(addr.ip));

    hSock sock = sock_tcp(NULL,1024,1);
    if(!sock)
        return -1;

    sock_connect(sock,&addr);
    ready_cb_ptr cli_ready;
    cli_ready.close_callback = sock_close_cb;
    cli_ready.read_callback = sock_read_cb;
    cli_ready.priv = NULL;

    sock_setRxDataCallBack(sock,&cli_ready);

    sock_start(sock);

    while(1);
}

int main(int argc,char *argv[])
{
    hSock sock = sock_unix("unix_test",1,1);
    if(!sock)
        return -1;

    sock_listen(sock,10);

    ready_cb_ptr unix_ready;
    unix_ready.close_callback = sock_close_cb;
    unix_ready.accept_callback = sock_accept_cb;
    unix_ready.read_callback = sock_read_cb;

    sock_setRxDataCallBack(sock,&unix_ready);

    sock_start(sock);

    while(1) ;
}
