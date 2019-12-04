#include <string.h>
#include <unistd.h>
#include <pthread.h> 
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h> 
#include <event2/util.h>
#include <event2/thread.h>
#include "yds_libevent_utils.h"
#include "yds_libevent_decode.h"
//***********************************************************************************
static int numCalls = 0;
static int numCalls_now = 0;
struct timeval lasttime;
struct timeval lasttime_now;

static void timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *ev = (struct event *)arg;
	struct timeval newtime,tv_diff;
	double elapsed;

	evutil_gettimeofday(&newtime, NULL);
	evutil_timersub(&newtime, &lasttime, &tv_diff);

	elapsed = tv_diff.tv_sec +  (tv_diff.tv_usec / 1.0e6);

	lasttime = newtime;
	printf("[%.3f] 1 timeout_cb %d \n",elapsed,++numCalls);

	//if (numCalls == 5)
	//{
	//	yds_libevent_removeEvent(ev);
	//}
	
}
static void timeout_now_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *ev = (struct event *)arg;
	struct timeval newtime,tv_diff;
	double elapsed;
	evutil_gettimeofday(&newtime, NULL);
	evutil_timersub(&newtime, &lasttime_now, &tv_diff);
	elapsed = tv_diff.tv_sec +  (tv_diff.tv_usec / 1.0e6);

	lasttime_now = newtime;
	printf("[%.3f] 2 timeout_cb %d \n",elapsed,++numCalls_now);
	//sleep(2);
	//printf(" %d \n",event);
}


void* yds_libevent_test1(void *arg)
{
	struct timeval tv_test = {1,0};

	yds_libevent_init();

	//获取时间
    //evutil_gettimeofday(&lasttime, NULL);

	//yds_libevent_addTimerEvent(TIMER_TIMEOUT,tv_test,timeout_cb);
	yds_libevent_run();

}
// 读缓冲区回调
static void read_cb_ser(struct bufferevent *bev, void *arg)
{
	//读到buff
    char buf[1024] = {0};   
    bufferevent_read(bev, buf, sizeof(buf));

	printf("[server]rece client data\n");
	printf("[server]client say: %s\n", buf);

	bufferevent_write(bev, buf, sizeof(buf));
 
}
 
// 写缓冲区回调
static void write_cb_ser(struct bufferevent *bev, void *arg)
{
    printf("[server]write_cb\n"); 
}
 
// 事件
static void event_cb_ser(struct bufferevent *bev, short events, void *arg)
{
	printf("[server]event_cb\n"); 
    if (events == CONN_CLOSE)
    {
        printf("[server]connection close\n");  
    }
    else if(events == CONN_ERROR)   
    {
        printf("[server]connection error\n");
    }else if(events == CONN_SUCCESS)
    {
        printf("[server]connection success\n");
    }
 
}// 读缓冲区回调

static void read_cb(struct bufferevent *bev, void *arg)
{
	//读到buff
    char buf[1024] = {0};   
    bufferevent_read(bev, buf, sizeof(buf));

	printf("[client]rece client data\n");
	printf("[client]client say: %s\n", buf);

	//bufferevent_write(bev, buf, sizeof(buf));
 
}

// 读缓冲区回调

static void read_cb_tcp(struct bufferevent *bev, UINT8 *data, int length)
{
	printf("[client]rece client data %d \n",length);
	int i;
	for ( i = 0; i < length; i++)
	{
		printf("[%c]",data[i]);
	}
	printf("\n");
}
 
 
// 写缓冲区回调
static void write_cb(struct bufferevent *bev, void *arg)
{
	 printf("[client]write_cb\n"); 
}

// 事件
static void event_cb(struct bufferevent *bev, short events, void *arg)
{
	printf("[client]event_cb\n"); 
    if (events == CONN_CLOSE)
    {
        printf("[client]connection close\n");  
    }
    else if(events == CONN_ERROR)   
    {
        printf("[client]connection error\n");
    }else if(events == CONN_SUCCESS)
    {
        printf("[client]connection success\n");

		//char* buf = "abc#START* hello#END*#START* hello#END*"; 
		UINT8 buff[41] = {0};
		UINT16 u16Data = 19;
		memcpy(buff,"abc#START*",strlen("abc#START*"));
		buff[10] = (UINT8)(u16Data >> 8);
		buff[11] = (UINT8)u16Data;

		//memcpy(buff + 10,&u16Data,2);
		memcpy(buff + 12,"hello#END#START*",strlen("hello#END#START*"));
		buff[28] = (UINT8)(u16Data >> 8);
		buff[29] = (UINT8)u16Data;
		memcpy(buff + 30,"hello#END",strlen("hello#END"));
		//printf("[client]buff %s\n",buff);
		//int i;
		//for ( i = 0; i < 41; i++)
		//{
		//	printf("[%c]",buff[i]);
		//}
		//printf("\n");
		yds_libevent_tcpSendData(bev, buff, sizeof(buff));
		//bufferevent_write(bev, buff, sizeof(buff));
    }
 
}

//static int tcp_decode_func(struct evbuffer *buff, UINT8 **data){
//	printf("tcp_decod\n");
//}

void yds_libevent_test2()
{
	struct timeval tv_test = {1,0};
	//evTcpDataFormat format = {FORMAT_HZ_PROTOBUF,10,"123"};

	bufferEvCB cb = {read_cb_tcp,write_cb_ser,event_cb_ser};
	bufferEvCB cb_1 = {read_cb_tcp,write_cb,event_cb};
	sleep(3);

	//struct event_base *ev = NULL;
	//yds_libevent_getBase(&ev);
	evutil_gettimeofday(&lasttime_now, NULL);
	//yds_libevent_addTimerEvent(TIMER_NOW_TIMEOUT,tv_test,timeout_now_cb);
	yds_libevent_addTcpServerEvent(6666,tcp_decode_hz_protobuf,cb);
	
	sleep(1);
	
	struct bufferevent* bev = yds_libevent_addTcpClientEvent("127.0.0.1",6666,tcp_decode_hz_protobuf,cb_1);

	//struct event* ev = yds_libevent_addManualActiveEvent(timeout_now_cb);
	//
	//sleep(3);

	//event_active(ev, EV_WRITE, 0);
}


int main(int argc, char **argv)
{
	struct timeval tv_test = {1,0};
	static pthread_t testTask1 = NULL;
	static pthread_t testTask2 = NULL;
	
	int err1 = pthread_create(&testTask1, NULL,yds_libevent_test1, NULL);
	int err2 = pthread_create(&testTask2, NULL, yds_libevent_test2, NULL);
	
	while (1)
	{
		sleep(10);
	}
	
	
}
//***********************************************************************************