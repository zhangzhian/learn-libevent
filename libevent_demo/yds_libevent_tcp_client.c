#include <unistd.h>
#include <string.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h> 

void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0}; 
    bufferevent_read(bev, buf, sizeof(buf));
	printf("[client]rece server data\n");
    printf("[client]server say: %s\n", buf);
}
 
void write_cb(struct bufferevent *bev, void *arg)
{
   printf("[client]write_cb\n"); 
}
 
void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if (events & BEV_EVENT_EOF)
    {
        printf("[client]connection close\n");  
    }
    else if(events & BEV_EVENT_ERROR)   
    {
        printf("[client]connection error\n");
    }
    else if(events & BEV_EVENT_CONNECTED)
    {
        printf("[client]connection success\n");
        return;
    }
    
    bufferevent_free(bev);
    printf("[client]bufferevent free\n");
}
 
void send_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[1024] = {0}; 
    struct bufferevent* bev = (struct bufferevent*)arg;
    read(fd, buf, sizeof(buf));
    bufferevent_write(bev, buf, strlen(buf)+1);
}

int main(int argc, const char* argv[])
{
	struct event_base *base;
	struct bufferevent* bev;
	struct sockaddr_in serv;
	struct event* ev;

	base = event_base_new();

	if (!base) {
		printf("[client]Couldn't open event base\n");
		return 1;
	}

	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	//连接服务器
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(6666);
	//解析 IP 地址
    evutil_inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);
	//连接
    bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));
	//设置回调
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    bufferevent_enable(bev, EV_READ);

	// 创建一个事件
	//STDIN_FILENO：接收键盘的输入
    ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, 
                                 send_cb, bev);
	event_add(ev, NULL);
    
    event_base_dispatch(base);

	//释放
	bufferevent_free(bev);
	event_free(ev);
	event_base_free(base);

}