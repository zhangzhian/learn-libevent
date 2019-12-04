
#include <string.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h> 
// ���������ص�
static void read_cb(struct bufferevent *bev, void *arg)
{
	//����buff
    //char buf[1024] = {0};   
    //bufferevent_read(bev, buf, sizeof(buf));

	//��ȡevbuffer
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t size = evbuffer_get_length(input);
	
	char *buf = (char *)malloc(size);   
	evbuffer_remove(input, buf, size);

	printf("[server]rece client data\n");
	printf("[server]client say: %s ,size = %d\n", buf, size);
 
}
 
// д�������ص�
static void write_cb(struct bufferevent *bev, void *arg)
{
    printf("[server]write_cb\n"); 
}
 
// �¼�
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
   // ͨ�Ų���
   // ������¼�
   struct bufferevent *bev;
   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
 
   if (!bev) {
		fprintf(stderr, "[server]error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}

   // ��bufferevent���������ûص�
   bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

   bufferevent_enable(bev, EV_READ);
   
   // ����һ���¼�
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

    // �����׽���
    // ��
    // ������������  
	//LEV_OPT_CLOSE_ON_FREE ������������ѡ��ͷ����Ӽ�������رյײ��׽��֡� 
	//LEV_OPT_REUSEABLE ĳЩƽ̨��Ĭ������£��ر�ĳ�����׽��ֺ�Ҫ��һ��������׽��ֲſ��԰󶨵�ͬһ���˿ڡ�
	//���������־���� libevent ����׽����ǿ����õģ�����һ���رգ����������������׽��֣�����ͬ�˿ڽ��м�����
    listener = evconnlistener_new_bind(base, cb_listener, base, 
                                  LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                  36, (struct sockaddr*)&serv, sizeof(serv));
 
	if (!listener) {
		perror("[server]Couldn't create listener");
		return 1;
	}

	//error����
	evconnlistener_set_error_cb(listener, accept_error_cb);

    event_base_dispatch(base);
 
	//�ͷ�
    evconnlistener_free(listener);
    event_base_free(base);
 
    return 0;
}