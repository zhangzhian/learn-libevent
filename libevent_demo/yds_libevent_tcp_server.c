
#include <string.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h> 
// 读缓冲区回调
static void read_cb(struct bufferevent *bev, void *arg)
{
	//读到buff
    //char buf[1024] = {0};   
    //bufferevent_read(bev, buf, sizeof(buf));

	//读取evbuffer
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t size = evbuffer_get_length(input);
	
	char *buf = (char *)malloc(size);   
	evbuffer_remove(input, buf, size);

	printf("[server]rece client data\n");
	printf("[server]client say: %s ,size = %d\n", buf, size);
 
}
 
// 写缓冲区回调
static void write_cb(struct bufferevent *bev, void *arg)
{
    printf("[server]write_cb\n"); 
}
 
// 事件
static void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if (events & BEV_EVENT_EOF)
    {
        printf("[server]connection close\n");  
    }
    else if(events & BEV_EVENT_ERROR)   
    {
        printf("[server]connection error\n");
    }else if(events & BEV_EVENT_CONNECTED)
    {
        printf("[server]connection success\n");
        return;
    }
    
    bufferevent_free(bev);    
    printf("[server]bufferevent free\n"); 
}
 
static void send_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[1024] = {0}; 
    struct bufferevent* bev = (struct bufferevent*)arg;

    read(fd, buf, sizeof(buf));

	struct evbuffer *output = bufferevent_get_output(bev);
	evbuffer_add(output, buf, strlen(buf)+1);

    //bufferevent_write(bev, buf, strlen(buf)+1);
}

static void cb_listener(
        struct evconnlistener *listener, 
        evutil_socket_t fd, 
        struct sockaddr *addr, 
        int len, void *ptr)
{
   printf("[server]connect new client\n");
 
   struct event_base* base = (struct event_base*)ptr;
   // 通信操作
   // 添加新事件
   struct bufferevent *bev;
   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
 
   if (!bev) {
		fprintf(stderr, "[server]error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}

   // 给bufferevent缓冲区设置回调
   bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

   bufferevent_enable(bev, EV_READ);
   
   // 创建一个事件
   struct event* ev = event_new(base, STDIN_FILENO, 
                                 EV_READ | EV_PERSIST, 
                                 send_cb, bev);
   event_add(ev, NULL);
   
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();
	printf("[Server]Got an error %d (%s) on the listener.Shutting down.\n", 
		err, evutil_socket_error_to_string(err));
	event_base_loopexit(base, NULL);
}

 
int main(int argc, const char* argv[])
{
    struct sockaddr_in serv;
	struct event_base* base;
	struct evconnlistener* listener;

	// init server 
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(6666);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    
    base = event_base_new();

	if (!base) {
		printf("[server]Couldn't open event base\n");
		return 1;
	}

    // 创建套接字
    // 绑定
    // 接收连接请求  
	//LEV_OPT_CLOSE_ON_FREE 如果设置了这个选项，释放连接监听器会关闭底层套接字。 
	//LEV_OPT_REUSEABLE 某些平台在默认情况下，关闭某监听套接字后，要过一会儿其他套接字才可以绑定到同一个端口。
	//设置这个标志会让 libevent 标记套接字是可重用的，这样一旦关闭，可以立即打开其他套接字，在相同端口进行监听。
    listener = evconnlistener_new_bind(base, cb_listener, base, 
                                  LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                  36, (struct sockaddr*)&serv, sizeof(serv));
 
	if (!listener) {
		perror("[server]Couldn't create listener");
		return 1;
	}

	//error处理
	evconnlistener_set_error_cb(listener, accept_error_cb);

    event_base_dispatch(base);
 
	//释放
    evconnlistener_free(listener);
    event_base_free(base);
 
    return 0;
}