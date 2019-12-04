#include <event2/event.h>
#include <event2/util.h>

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
	printf("[%.3f]timeout_cb %d \n",elapsed,++numCalls);
	
}

static void now_timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *ev = (struct event *)arg;
	struct timeval tv = {3,0};
	struct timeval newtime,tv_diff;
	double elapsed;

	evutil_gettimeofday(&newtime, NULL);
	evutil_timersub(&newtime, &lasttime_now, &tv_diff);
	elapsed = tv_diff.tv_sec +  (tv_diff.tv_usec / 1.0e6);

	lasttime_now = newtime;
	printf("[%.3f]now_timeout_cb %d \n",elapsed,++numCalls_now);

	//每次回调都将当前先del，再次添加
	//if (!event_pending(ev,EV_PERSIST|EV_TIMEOUT,NULL))
	//{
	//	printf("if\n");
	//	event_del(ev);
	//	event_add(ev, &tv);
	//}

	//添加新的event
	//if (!event_pending(ev,EV_PERSIST|EV_TIMEOUT,NULL))
	//{
	//	struct event_base *evBase = event_get_base(ev);
	//	event_del(ev);
	//	event_free(ev);
	//
	//	ev = event_new(evBase, -1, EV_PERSIST|EV_TIMEOUT, now_timeout_cb, event_self_cbarg());
	//	event_add(ev, &tv);
	//}
}

int main(int argc, char **argv)
{
	struct event_base *evBase = NULL;
	struct event_config *evConf = NULL;
	struct event *ev_now_timeout = NULL;
	struct event *ev_timeout = NULL;
	struct timeval tv = {0,0};
	struct timeval tv_now = {0,0};
	//创建简单的event_base
	evBase = event_base_new();

	//创建带配置的event_base
	evConf = event_config_new();//创建event_config
	evBase = event_base_new_with_config(evConf);

	//创建event
	//传递自己event_self_cbarg()
	ev_now_timeout = evtimer_new(evBase, now_timeout_cb, event_self_cbarg());

	//设置时间
	tv.tv_sec = 1;
	tv.tv_usec = 500 * 1000;
	ev_timeout = event_new(evBase, -1, EV_PERSIST|EV_TIMEOUT, timeout_cb, event_self_cbarg());

	//添加event
	event_add(ev_now_timeout, &tv_now);//立即执行一次，然后定时
	event_add(ev_timeout, &tv);

	//获取时间
    evutil_gettimeofday(&lasttime, NULL);
	evutil_gettimeofday(&lasttime_now, NULL);
	//循环
	//event_base_loop(evBase, 0);
	event_base_dispatch(evBase);
	
	//释放
	event_free(ev_timeout);
	event_free(ev_now_timeout);
	event_config_free(evConf);
	event_base_free(evBase);
}

