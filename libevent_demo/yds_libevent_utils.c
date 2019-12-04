/** ###################################################################
**     Filename  : yds_libevent_utils.c
**     Project   : 
**     Processor : ARM7
**     Component : FDA
**     Version   : 
**     Compiler  : 
**     Date/Time : 2019/12/03, 20:10
**     Abstract  :
**         This is libevent utils 
**     Settings  :
**
**     Contents  :
**
**     Copyright : 1997 - 2010 Freescale Semiconductor, Inc. All Rights Reserved.
**
** ###################################################################*/
/** ===========================include=============================== **/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h> 
#include <event2/util.h>
#include <event2/thread.h>
#include "yds_libevent_utils.h"
/** ===========================include=============================== **/

/** ==========================globale================================ **/
static struct event_base *evBase = NULL;
static struct event_config *evConf = NULL;
static struct evconnlistener *listener = NULL;

/** ==========================func================================ **/
static void void_cb(evutil_socket_t fd, short event, void *arg)
{
	static int num = 0;
	printf("[Libevent]void_cb %d \n",++num);
	
}


// 读缓冲区回调
static void read_cb(struct bufferevent *bev, void *arg)
{
	//printf("[Libevent]read_cb\n");
	evTcpContext* context = (evTcpContext*)arg;
	//读取evbuffer
	struct evbuffer *input = bufferevent_get_input(bev);
	printf("[Libevent]read_cb while size %d\n",evbuffer_get_length(input));	
	
	UINT8 *data = NULL;

	while (1)
	{
		int size = context->decode(input,&data);
		if (size < 0)
			break;
		if (size == 0)
			continue;
	
		
		//printf("[Libevent]");
		//int i;
		//for ( i = 0; i < size; i++)
		//{
		//	printf("[%c]",data[i]);
		//}
		//printf("\n");
		
		//完整数据
		context->cb->readcb(bev, data, size);

		if(data == NULL) free(data);
		data = NULL;
	}	
}
 
// 写缓冲区回调
static void write_cb(struct bufferevent *bev, void *arg)
{
	evTcpContext* context = (evTcpContext*)arg;
	context->cb->writecb(bev, NULL);
}

static void event_cb(struct bufferevent *bev, short events, void *arg)
{
	printf("[Libevent]events %02x\n",events);
	evTcpContext* context = (evTcpContext*)arg;

	if (events & BEV_EVENT_EOF)
    {
		context->cb->eventcb(bev,CONN_CLOSE,NULL);
    }
    else if(events & BEV_EVENT_ERROR)   
    {
		context->cb->eventcb(bev,CONN_ERROR,NULL);
       
    }else if(events & BEV_EVENT_CONNECTED)
    {
		context->cb->eventcb(bev,CONN_SUCCESS,NULL);
        return;
    }
    
    bufferevent_free(bev);    
}

static void cb_listener(
        struct evconnlistener *listener, 
        evutil_socket_t fd, 
        struct sockaddr *addr, 
        int len, void *ptr)
{
   printf("[Libevent]connect new client\n");
   
   // 通信操作
   // 添加新事件
   struct bufferevent *bev;
   bev = bufferevent_socket_new(evBase, fd, BEV_OPT_CLOSE_ON_FREE);

   if (!bev) {
		fprintf(stderr, "[server]error constructing bufferevent!");
		event_base_loopbreak(evBase);
		return;
	}
   printf("[Libevent]constructing bufferevent\n");
   // 给bufferevent缓冲区设置回调
   bufferevent_setcb(bev,read_cb,write_cb, event_cb, ptr);
   bufferevent_enable(bev, EV_READ);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	printf("[Libevent]Got an error %d (%s) on the listener.Shutting down.\n", 
		err, evutil_socket_error_to_string(err));
	//event_base_loopexit(base, NULL);
	evconnlistener_free(listener);
	listener = NULL;
}

int yds_libevent_init(){

	int ret = 0;
	struct event *ev = NULL;	
	struct timeval tv = {5,0};
	struct timeval tv_interval = {1,0};
	printf("[Libevent] evthread_use_pthreads %d\n",evthread_use_pthreads());

	//创建带配置的event_base
	evConf = event_config_new();//创建event_config

	event_config_set_max_dispatch_interval(evConf, &tv_interval, -1, 0);

	evBase = event_base_new_with_config(evConf);

	if (!evBase) {
		printf("[Libevent]Couldn't open event base\n");
		ret = -1;
		return ret;
	}

	ev = event_new(evBase, -1, EV_PERSIST, void_cb, event_self_cbarg());
	event_add(ev, &tv);
	return ret;
}


int yds_libevent_addTimerEvent(YDS_TIMER_TYPE_E eFlag, struct timeval delayTime, void (*cb)(evutil_socket_t, short, void *)){
	struct event *ev_timeout = NULL;
	struct event *ev_now_timeout = NULL;
	struct timeval tv = {0,0};
	int res;

	if (eFlag == TIMER_TIMEOUT)
	{
		ev_timeout = event_new(evBase, -1, EV_PERSIST, cb, event_self_cbarg());
		res = event_add(ev_timeout, &delayTime);
	}
	if (eFlag == TIMER_NOW_TIMEOUT)
	{
		ev_now_timeout = evtimer_new(evBase, cb, event_self_cbarg());
		ev_timeout = event_new(evBase, -1, EV_PERSIST, cb, event_self_cbarg());

		res = event_add(ev_now_timeout, &tv);//立即执行一次，然后定时

		if (res == 0)
		{
			res = event_add(ev_timeout, &delayTime);
		}
	}

	return res;
}



int yds_libevent_addTcpServerEvent(int port,
							  tcp_decode decode,
							  bufferEvCB cb){
	struct sockaddr_in serv;
	static evTcpContext context; 
	static bufferEvCB mcb = {0};
	static tcp_decode mdecode;

	if (evBase == NULL)
	{
		return -1;
	}

	// init server 
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

	mcb.readcb = cb.readcb;
	mcb.writecb = cb.writecb;
	mcb.eventcb = cb.eventcb;

	mdecode = decode;

	context.cb = &mcb;
	context.decode = mdecode;
	context.tcp = TCP_SERVER;

	// 创建套接字
    // 绑定
    // 接收连接请求  
	//LEV_OPT_CLOSE_ON_FREE 如果设置了这个选项，释放连接监听器会关闭底层套接字。 
	//LEV_OPT_REUSEABLE 某些平台在默认情况下，关闭某监听套接字后，要过一会儿其他套接字才可以绑定到同一个端口。
	//设置这个标志会让 libevent 标记套接字是可重用的，这样一旦关闭，可以立即打开其他套接字，在相同端口进行监听。
	listener = evconnlistener_new_bind(evBase, cb_listener, &context, 
                                  LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                  36, (struct sockaddr*)&serv, sizeof(serv));


	if (!listener) {
		printf("[Libevent]Couldn't create listener\n");
		return -1;
	}else
	{
		printf("[Libevent]create listener\n");
	}

	//error处理
	evconnlistener_set_error_cb(listener, accept_error_cb);

	return 0;
}

struct bufferevent* yds_libevent_addTcpClientEvent(const char* ip, int port,
							  tcp_decode decode,
							  bufferEvCB cb){
	struct bufferevent* bev = NULL;
	struct sockaddr_in serv;
	static evTcpContext context; 
	static bufferEvCB mcb;
	static tcp_decode mdecode;

	if (evBase == NULL)
	{
		return bev;
	}

	bev = bufferevent_socket_new(evBase, -1, BEV_OPT_CLOSE_ON_FREE);

	if (bev == NULL)
	{
		return bev;
	}
	//连接服务器
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
	//解析 IP 地址
    evutil_inet_pton(AF_INET, ip, &serv.sin_addr.s_addr);
	//连接
    bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));

	mcb.eventcb = cb.eventcb;
	mcb.readcb = cb.readcb;
	mcb.writecb = cb.writecb;

	mdecode = decode;

	context.cb = &mcb;
	context.decode = mdecode;

	context.tcp = TCP_CLIENT;

	//设置回调
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, &context);
    bufferevent_enable(bev, EV_READ);

	return bev;
}

struct event* yds_libevent_addManualActiveEvent(void (*cb)(evutil_socket_t, short, void *)){
	
	struct event *ev = NULL;

	ev = event_new(evBase, -1, EV_READ | EV_WRITE, cb, NULL);

	event_add(ev, NULL);

	return ev;
}

int yds_libevent_removeEvent(struct event *ev){
	return event_del(ev);
}

int yds_libevent_run(){

	int ret = -1;

	if (evConf!=NULL)
	{
		ret = event_base_dispatch(evBase);
	}
	return ret;
}

void yds_libevent_getBase(struct event_base **ev){
	ev = &evBase;
}

void yds_libevent_deinit(){

	if (listener != NULL)
	{
		 evconnlistener_free(listener);
		 listener = NULL;
	}

	if (evConf!=NULL)
	{
		event_config_free(evConf);
		evConf = NULL;
	}
	if (evBase!=NULL)
	{
		event_base_free(evBase);
		evBase = NULL;
	}
	
}

