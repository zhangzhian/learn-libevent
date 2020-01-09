# LibeventUtil使用说明

## 一、概述

LibeventUtil包括4个文件

- yds_libevent_utils.h

- yds_libevent_utils.c

- yds_libevent_decode.h

- yds_libevent_decode.c

其中\*.h为接口文件，供使用者调用，\*.c为源码文件，utils为核心工具类，decode为tcp协议解析的工具类。

开发目的是简化libevent的使用流程，屏蔽libevent使用细节，使用者可快速将其应用到项目中。

## 二、基本流程

将4个源码文件添加到项目中。

### 初始化

```c
/*
** ===================================================================
**     Method      :  yds_libevent_init
**
**     Description :
**         libevent init
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_init()
```

初始化Libevent。添加了多线程支持，如下log，pthreads_ret = 1为开启多线程支持成功。

```c
YDS_LOG(LOG_LEVEL_INFO,"[Libevent] evthread_use_pthreads %d\n",pthreads_ret);
```

### 运行

```c
/*
** ===================================================================
**     Method      :  yds_libevent_run
**
**     Description :
**         libevent start run
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_run();
```

在Libevent init 后即可调用run函数。

使用run函数后会阻塞当前线程，建议和init函数一起开启新线程中使用，如下。

```c
static pthread_t g_libevent = NULL; /*http server threadId*/

/*
** ===================================================================
**     Method      :  yds_rdt_libeventStart 
**
**     Description :
**        libevent start process
**     Parameters  : None
**     Returns     : None
** ===================================================================
*/
VOID *yds_rdt_libeventStart(VOID){

	int ret = yds_libevent_init();

	if (ret != YDS_LIBEVENT_NOERR)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[RDT]libevent init error %d\n", ret);
	}

	ret = yds_libevent_run();

	if (ret != YDS_LIBEVENT_NOERR)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[RDT]libevent run error %d\n", ret);
	}
}

/*
** ===================================================================
**     Method      :  yds_rdt_libeventStart
**
**     Description :
**         init libevent 
**     Parameters  : None
**     Returns     : None
** ===================================================================
*/
VOID yds_rdt_libeventInit(){
	
	INT16 pthread_err;

    pthread_err = pthread_create(&g_libevent,NULL,yds_rdt_libeventStart,NULL);

    if (pthread_err != 0)
    {
        YDS_LOG(LOG_LEVEL_INFO,"[RDT]libevent thread failed, errno: %d\n", errnoGet());
    }
    else
    {
        YDS_LOG(LOG_LEVEL_INFO,"[RDT]libevent thread create success\n");
    }
}

```

### 添加事件

初始化Libevent完成后，即可添加事件。

> 在添加事件需要确保初始化Libevent完成。（若添加事件的线程和初始化的线程同时创建，可以在添加事件的线程开始处sleep 1秒，确保初始化完成后再进行添加。）

事件详见下面三到七章节

### 反初始化

```c
/*
** ===================================================================
**     Method      :  yds_libevent_deinit
**
**     Description :
**         libevent deinit
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
void yds_libevent_deinit();
```

在程序结束的时候调用deinit进行资源释放，会尽可能释放Libevent相关的所有资源。

## 三、添加Timer事件

添加timer事件使用addTimerEvent函数。

```c
typedef enum
{
	TIMER_ONCE,					//wait ... exec
    TIMER_TIMEOUT,				//wait ... exec ... wait
	TIMER_RTC_ONCE,				//wait ... exec rtc
	TIMER_RTC_TIMEOUT,			//wait ... exec ... wait rtc
}YDS_TIMER_TYPE_E;

/*
** ===================================================================
**     Method      :  yds_libevent_addTimerEvent
**
**     Description :
**         libevent add Timer Event
**     Parameters  : ev: an timer event
**                 : eFlag: timer type
**                 : delayTime: timer delay time
**                 : cb: timer callback
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_addTimerEvent(struct event			**ev, 
							   YDS_TIMER_TYPE_E		eFlag,
							   struct timeval		delayTime,
							   timer_cb				cb);
```

ev：当前事件

delayTime：延时时间，定义如下

```c
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};
```

eFlag：事件的类型

- TIMER_ONCE：delayTime后执行一次

- TIMER_TIMEOUT：delayTime为周期执行

- TIMER_RTC_ONCE：delayTime后执行一次，系统休眠不会影响执行

- TIMER_RTC_TIMEOUT：delayTime为周期执行，系统休眠不会影响执行

cb：执行回调，定义如下：

```c
/**
   A callback function for an event.
 */
typedef void (*timer_cb)(evutil_socket_t fd, short what, void *arg);
```

使用示例如下：

```c
struct event *evMda;
struct timeval tvMda = {1,0};//1s

//add mda manage event	
	ret = yds_libevent_addTimerEvent(&evMda,TIMER_TIMEOUT,tvMda,yds_rdt_timer_mda_cb);
	if (ret != YDS_LIBEVENT_NOERR)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[RDT]add mda manage event error %d\n", ret);
	}
```

```c
static void yds_rdt_timer_mda_cb(evutil_socket_t fd, short event, void *arg)
{
    ...//do somehting 
}
```

## 四、添加Tcp Server事件

添加Tcp Server事件使用addTcpServerEvent函数。

```c
/*
** ===================================================================
**     Method      :  yds_libevent_addTcpServer
**
**     Description :
**         libevent add Tcp Server Event
**     Parameters  : port: Tcp server listener port
**                 : backlog: Passed to the listen() call to determine the length of the
**							  acceptable connection backlog.  Set to -1 for a reasonable default.
**                 : decode: decode receve data func
**                 : cb: Tcp event callback
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_addTcpServerEvent(int				port, 
								   int				backlog,
								   tcp_decode_func	decode, 
								   ydsTcpEventCb	cb);

```

port：需要监听的端口

backlog：设置支持的连接数

decode：接收到的数据解码函数，请查看"解码"章节

cb：回调函数，定义如下：

```c
/**
   An read callback for a bufferevent.
*/
typedef void (*tcp_read_data_cb)(struct bufferevent *bev, UINT8 *data, int length);

/**
   A write callback for a bufferevent.
 */
typedef void (*tcp_write_data_cb)(struct bufferevent *bev, void *ctx);

/**
   An event/error callback for a bufferevent.
*/
typedef void (*tcp_event_data_cb)(struct bufferevent *bev, short what, void *ctx);

/**
   A callback that we invoke when a listener encounters a non-retriable error.
 */
typedef void (*tcp_connlistener_err_cb)(struct evconnlistener *conn, void *ctx);

typedef struct
{
	tcp_read_data_cb			readcb; //read data callback
	tcp_write_data_cb			writecb; //write data callback
	tcp_event_data_cb			eventcb; //event data callback, see YDS_CONN_RESULT_E
	tcp_connlistener_err_cb		connlistrnercb;
} ydsTcpEventCb;
```

其中：

- readcb为读数据回调
- writecb为写数据回调
- eventcb为连接事件回调
- connlistrnercb为监听器错误回调

在eventcb的what参数，可以判断连接状态。返回如下：

关于读写回调中对数据的操作请查看"数据操作"章节

```c
typedef enum
{
    CONN_SUCCESS,				//connection succeeded
    CONN_CLOSE,					//connection closed
	CONN_ERROR,					//connection error
}YDS_CONN_RESUT_E;
```

使用示例如下：

```c
ydsTcpEventCb cb = {read_cb_ser,write_cb_ser,event_cb_ser,connlistener_err_deal};

yds_libevent_addTcpServerEvent(6666,1,tcp_decode_all,cb);
```

```c
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
 
}

// 读缓冲区回调
static void read_cb_ser(struct bufferevent *bev, UINT8 *data, int length)
{
	printf("[server]rece client data %d \n",length);
}

static void connlistener_err_deal(struct evconnlistener	*listener, 
								          void					*ctx)
{
	int err = EVUTIL_SOCKET_ERROR();
	YDS_LOG(LOG_LEVEL_INFO,"[server]Got an error %d (%s) on the Tcplistener.Shutting down.\n", err, evutil_socket_error_to_string(err));

	evconnlistener_free(listener);
	listener = NULL;
}
```

## 五、添加Tcp Client事件

添加Tcp Client事件使用addTcpClientEvent函数。

```c
/*
** ===================================================================
**     Method      :  yds_libevent_addTcpServer
**
**     Description :
**         libevent add Tcp Client Event
**     Parameters  : ip: Tcp server connect ip
**                 : port: Tcp server connect port
**                 : decode: decode receve data func
**                 : cb: Tcp event callback
**     Returns     : the bufferevent structure
** ===================================================================
*/
struct bufferevent* yds_libevent_addTcpClientEvent(const char			*ip, 
												   int					port, 
												   tcp_decode_func		decode, 
												   ydsTcpEventCb		cb);
```

ip：需要连接的ip地址

port：需要连接的端口

decode：接收到的数据解码函数，请查看"解码"章节

cb：回调函数，同“添加Tcp Server事件”章节cb的定义

struct bufferevent*：返回连接后的bufferevent事件

使用示例：

```c
ydsTcpEventCb cb_client= {read_cb_tcp,write_cb,event_cb,NULL};
struct bufferevent* bev = yds_libevent_addTcpClientEvent("127.0.0.1",6666,tcp_decode_all,cb_client);
```

```c
// 读缓冲区回调

static void read_cb(struct bufferevent *bev, void *arg)
{
	printf("[client]rece client data\n");
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
    }
 
}
```

## 六、添加Http/Https Server事件

添加Http/Https Server事件使用addHttpServer函数。

```c
/*
** ===================================================================
**     Method      :  yds_libevent_addHttpServer
**
**     Description :
**         libevent add http server
**     Parameters  : port: Http server connect port
**                 : info: Http server conf info
**                 : cb: Http server uri event callback
**                 : 
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_addHttpServer(int						port, 
							   ydsEvHttpServerInfo		*info,
							   http_server_request_cb	cb);

```

port：监听的端口号

info：Http server的一些配置信息，定义如下：

```c
typedef enum 
{ 
	HTTP,
	HTTPS 
} YDS_HTTP_TYPE_E;

typedef struct
{
	YDS_HTTP_TYPE_E		type;							// YDS_HTTP_TYPE_E
	const char			*certificate_chain;				// if https
	const char			*private_key;					// if https
	UINT8				size;							// uri number
	CHAR				*uri[255]; 			// need listener uri list, uri "/getinfo"
}ydsEvHttpServerInfo;
```

- type：HTTP 还是 HTTPS
- certificate_chain：证书
- private_key：私钥
- size：uri数组长度
- uri：uri数组

cb：Http serve配置好的客户端请求的回调，定义如下：

```c
typedef struct
{
	struct evkeyvalq	*params;						//request uri params data
	const char			*uri;							//request uri
	const char			*uriWithParam;					//request uri include params
	YDS_HTTP_REQ_TYPE	reqType;						//YDS_HTTP_REQ_TYPE :GET POST ...
	struct evbuffer		*evbuff;						//if POST
}ydsEvHttpServerUriRequestInfo;

typedef void (*http_server_request_cb)(struct evhttp_request			*request, 
									   ydsEvHttpServerUriRequestInfo	*info);
```

request：当前的请求结构体

info：请求的一些信息，可供使用者便捷使用

- params：如果url带参数，返回的参数结构体，使用如下函数或取key对应的value

```c
/*
** ===================================================================
**     Method      :  yds_libevent_getHttpParamValue
**
**     Description :
**         libevent get http param value by key
**     Parameters  : headers the evkeyvalq object in which to find the header
**                 : key the name of the header to find
**                 : 
**                 : 
**     Returns     : a pointer to the value for the header or NULL 
**					 if the headercould not be found.
** ===================================================================
*/
#define yds_libevent_getHttpParamValue(headers,key) \
	evhttp_find_header((headers),(key))
```

- uri：不带参数的uri
- uriWithParam：带参数的uri
- reqType：查看“HTTP请求类型”章节
- evbuff：数据所在的buff，操作请查看“数据操作”章节

Server给客户端回复数据，请查看“Http请求回复”

使用示例：

```c
	char uri1[255] = {"/test"};
	char uri2[255] = {"/test123"};

	ydsEvHttpServerInfo info_s;
	info_s.type = HTTP;
	info_s.uri[0] = uri1;
	info_s.uri[1] = uri2;
	info_s.size = 2;

	yds_libevent_addHttpServer(8888, &info_s,request_cb);
```

```c
static void request_cb(struct evhttp_request *request, 
					   ydsEvHttpServerUriRequestInfo *info){

	printf("server\n");
	printf("%s\n",info->uri);
	printf("%s\n",info->uriWithParam);
	//得到a所对应的value
	if (info->params != NULL)
	{
		const char *a_data = yds_libevent_getHttpParamValue(info->params, "a");

		printf("a=%s\n",a_data);
	}

	if (info->evbuff != NULL)
	{
		char data[1024] = {0};

		yds_libevent_getEvbufferAllData(info->evbuff,data);
	
		printf("%s\n",data);
	}

	ydsEvHttpServerUriResponseInfo infoResp = {0};
	infoResp.buffData = "hello world";
	infoResp.code = 200;
	infoResp.reason = "sucess";

	yds_libevent_sendHttpResponse(request,&infoResp);

	printf("server end\n");

}

```

## 七、添加Http/Https Client事件

添加Http/Https Client事件使用addHttpClientRequest函数。

```c
/*
** ===================================================================
**     Method      :  yds_libevent_addHttpRequest
**
**     Description :
**         libevent add http client request
**     Parameters  : info: Http client req conf info
**                 : cb: Http client uri event callback
**                 : 
**                 : 
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_addHttpClientRequest(ydsEvHttpClientUriRequestInfo		*info, 
									  http_client_request_cb			cb);
```

info：请求的配置信息

```c
typedef struct
{
	const char				*requestUrl;				//the whole uri that need requset server 
	int						retries;					//retry number
	struct timeval			timeoutConn;				//connect timeout
	struct timeval			timeoutWrite;				//write timeout
	struct timeval			timeoutRead;				//read timeout
	YDS_HTTP_REQ_TYPE		type;						//get post ...
	YDS_HTTP_POST_TYPE_E	postType;					//YDS_HTTP_REQ_TYPE
	ydsPostFormatData		*format;					//if POST_FORMAT
	ydsHeaderData			*header;					//if need add extra header 
	int						formatSize;					//format number 
	int						headerSize;					//header number
	const char				*postData;					//if POST_BUFF
	const char				*filePath;					//if POST_FILE
}ydsEvHttpClientUriRequestInfo;
```

- requestUrl：请求url
- retries：重试次数
- timeoutConn：连接超时时间
- timeoutWrite：写超时
- timeoutRead：读超时
- type：YDS_HTTP_REQ_TYPE
- postType：YDS_HTTP_POST_TYPE_E post请求类型

```c
typedef enum
{
    POST_JSON,		
	POST_FORMAT,
	POST_HEADER,
	POST_FILE,
	POST_BUFF,
}YDS_HTTP_POST_TYPE_E;
```

分为对应json数据，formart方式，header方式，file方式，body buff方式。

- format：formart数据，对应POST_FORMAT和POST_HEADER
- header：header数据，需要额外在header中添加的数据
- formatSize：formart数据个数
- headerSize：header数据个数
- postData：post数据，对应POST_JSON和POST_BUFF
- filePath：文件路径，对应POST_FILE

cb：请求完成回调，如下：

```c
typedef struct
{
	struct evbuffer		*buffData;						//http server response buff data
	int					code;							//http server response cede
    const char			*reason;						//http server response reason
}ydsEvHttpClientUriResponseInfo;

typedef void (*http_client_request_cb)(struct evhttp_request			*request, 
									   ydsEvHttpClientUriResponseInfo	*info);
```

request：当前Client请求

info：同ydsEvHttpServerUriResponseInfo

示例：

```c
    printf("GET\n");
	ydsEvHttpClientUriRequestInfo info;
	info.requestUri = "http://127.0.0.1:8888/test?a=123";
	info.retries = 3;
	info.timeoutConn.tv_sec = 10;
	info.timeoutWrite.tv_sec = 10;
	info.timeoutRead.tv_sec = 10;
	info.type = YDS_HTTP_REQ_GET;
	info.headerSize = 0;	
	printf("GET client ret: %d\n",  yds_libevent_addHttpClientRequest(&info,client_request_cb));


	printf("POST FILE\n");
	info.type = YDS_HTTP_REQ_POST;
	info.filePath = "/home/zza/libevent/demo/test.txt";
	info.postType = POST_FILE;
	yds_libevent_addHttpClientRequest(&info,client_request_cb);

	printf("POST POST JSON\n");
	info.postType = POST_JSON;
	info.postData = "{\"a\":\"b\"}\0";
	yds_libevent_addHttpClientRequest(&info,client_request_cb);

	printf("POST POST HEADER\n");
	ydsPostFormatData formatData[1] = {0};
	formatData[0].key = "zza";
	formatData[0].value = "123456";
	info.postType = POST_HEADER;
	info.formatSize = 1;
	info.format = formatData;
	yds_libevent_addHttpClientRequest(&info,client_request_cb);

	printf("POST POST FORMAT\n");
	info.postType = POST_FORMAT;
	yds_libevent_addHttpClientRequest(&info,client_request_cb);

	printf("POST POST BUFF\n");
	info.postType = POST_BUFF;
	info.postData = "123456789";
	yds_libevent_addHttpClientRequest(&info,client_request_cb);

```

```c
static void client_request_cb(struct evhttp_request *request, 
							ydsEvHttpClientUriResponseInfo *info){
	
    if (info->buffData != NULL)
	{
		char data[1024] = {0};
		yds_libevent_getEvbufferAllData(info->buffData,data);	
		printf("%s\n",data);
	}
    
	printf("%d\n",info->code);
	printf("%s\n",info->reason);
}
```

## 八、解码

解码回调的定义如下：

```c
/**
   An decode data func for a evbuffer.
*/
typedef int (*tcp_decode_func)(struct evbuffer *buff, UINT8 **data);
```

在yds_libevent_utils.h中提供了解码的通用函数：

```c
typedef enum
{
    TCP_DATA_NORAML,				//normal
	TCP_DATA_DOUBLING,				//Doubling
}YDS_TCP_DATA_TYPE_E;

typedef enum
{
    TCP_SIZE_BIGBYTE,				//BigByte
	TCP_SIZE_LITTLEBYTE,			//LittleByte
}YDS_TCP_SIZE_MODULE_E;

typedef enum
{
    TCP_SIZE_ALL,				
	TCP_SIZE_ONLY_DATA,	
}YDS_TCP_SIZE_TYPE_E;

typedef struct
{
	const char				*header;
	const char				*end;
	UINT8					sizeLength;	//data size length 1,2,...
	UINT16					sizeIndex;	//data size index ,form protocol start
	YDS_TCP_SIZE_MODULE_E	sizeModule;	//big-endian or little-endian
	YDS_TCP_SIZE_TYPE_E		sizeType;
	UINT16					otherLength;//not include sizeLength, headerLength, endLength and dataLength 
	YDS_TCP_DATA_TYPE_E		type;
	
} evUniversalForamt;	
/*
** ===================================================================
**     Method      :  tcp_decode_universal
**
**     Description :
**         tcp decode universal func
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int tcp_decode_universal(struct evbuffer *buff, UINT8 **data, evUniversalForamt format);
```

获取所有数据的函数：

```c
/*
** ===================================================================
**     Method      :  tcp_decode_universal
**
**     Description :
**         tcp decode universal func
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int tcp_decode_all(struct evbuffer *buff, UINT8 **data);
```

tcp_decode_universal函数需要在外层进行设置，然后提作为tcp_decode_func函数提供给Tcp服务端或客户端。

例如：

```c
/*
** ===================================================================
**     Method      :  tcp_decode_hz_protobuf
**
**     Description :
**         tcp_decode_universal use demo
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int tcp_decode_hz_protobuf(struct evbuffer *buff, UINT8 **data){
	
	evUniversalForamt format;
	const char *header = "#START*";
	const char *end = "#END*";

	format.header = header;
	format.end = end;
	format.sizeIndex = 7;
	format.sizeLength = 2;
	format.sizeModule = TCP_SIZE_BIGBYTE;
	format.type = TCP_DATA_DOUBLING;
	format.sizeType = TCP_SIZE_ONLY_DATA;
	format.otherLength = 0;
	return tcp_decode_universal(buff,data,format);
}
```

## 九、数据操作

### bufferevent数据操作

数据写：

```c
/*
** ===================================================================
**     Method      :  yds_libevent_tcpSendData
**
**     Description :
**         libevent Tcp send data
**     Parameters  : bufev the bufferevent to be written to
**                 : data a pointer to the data to be written
**                 : size the length of the data, in bytes
**                 : 
**     Returns     : 0 if successful, or -1 if an error occurred
** ===================================================================
*/
#define yds_libevent_tcpSendData(bufev, data, size)	\
	bufferevent_write((bufev), (data),  (size))
```

数据删：

```c
/*
** ===================================================================
**     Method      :  yds_libevent_removeEvbufferData
**
**     Description :
**         libevent remove evbuffer data
**     Parameters  : buf the evbuffer to be drained 
**                 : len the number of bytes to drain from the beginning of the buffer
**                 : 
**                 : 
**     Returns     : 0 on success, -1 on failure.
** ===================================================================
*/
#define yds_libevent_removeEvbufferData(buf, len) \
	evbuffer_drain((buf),(len))
```

### evbuffer数据操作

yds_libevent_getEvbufferData：获取数据，删除获取的数据

yds_libevent_getEvbufferAllData：获取所有数据，删除获取的数据

yds_libevent_copyEvbufferData：复制数据，不删

yds_libevent_getEvbuffLength：获取数据长度

```c

/*
** ===================================================================
**     Method      :  yds_libevent_getEvbufferData
**
**     Description :
**         libevent get len size evbuffer data, and del
**     Parameters  : buf the evbuffer to be read from
**                 : data the destination buffer to store the result
**                 : len the maximum size of the destination buffer
**                 : 
**     Returns     : the number of bytes read, or -1 if we can't drain the buffer.
** ===================================================================
*/
#define yds_libevent_getEvbufferData(buf, data, len) \
	evbuffer_remove((buf),(data),(len))

/*
** ===================================================================
**     Method      :  yds_libevent_getEvbufferAllData
**
**     Description :
**          libevent get all evbuffer data, and del
**     Parameters  : buf the evbuffer to be read from
**                 : data the destination buffer to store the result
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_getEvbufferAllData(buf, data) \
	evbuffer_remove((buf),(data),(evbuffer_get_length((buf))))

/*
** ===================================================================
**     Method      :  yds_libevent_copyEvbufferData
**
**     Description :
**         libevent copy evbuffer data, not del
**     Parameters  : buf the evbuffer to be read from
**                 : data_out the destination buffer to store the result
**                 : datlen the maximum size of the destination buffer
**                 : 
**     Returns     : the number of bytes read, or -1 if we can't drain the buffer.
** ===================================================================
*/
#define yds_libevent_copyEvbufferData(buf, data, len) \
	evbuffer_copyout((buf),(data),(len))

/*
** ===================================================================
**     Method      :  yds_libevent_getEvbuffLength
**
**     Description :
**         libevent get evbuff data length
**     Parameters  : buf pointer to the evbuffer
**                 : 
**                 : 
**                 : 
**     Returns     : the number of bytes stored in the evbuffer
** ===================================================================
*/
#define yds_libevent_getEvbuffLength(buf) \
	evbuffer_get_length((buf))
```

## 十、Http请求回复

Server给客户端回复数据，通常在http_server_request_cb中使用，使用函数如下：

```c
/*
** ===================================================================
**     Method      :  yds_libevent_sendHttpResponse
**
**     Description :
**         libevent send http server response
**     Parameters  : request: Http requset struct evhttp_request
**                 : responseInfo: Http need response info data
**                 : 
**                 : 
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_sendHttpResponse(struct evhttp_request				*request,
								   ydsEvHttpServerUriResponseInfo	*responseInfo);
```

request：回复数据的请求

responseInfo：回复的数据，定义如下：

```c
typedef struct
{
	int					code;							//response client code  200,404,500....
	const char			*reason;						//response client reason, header
	const char			*buffData;						//response client buff data, body
}ydsEvHttpServerUriResponseInfo;
```

code：返回码，例如：200,404,500....

reason：原因，例如：“ok”，“fail”

buffData：数据，根据实际请求定义返回

使用示例

```c
	ydsEvHttpServerUriResponseInfo infoResp = {0};
	infoResp.buffData = "hello world";
	infoResp.code = 200;
	infoResp.reason = "sucess";

	yds_libevent_sendHttpResponse(request,&infoResp);
```

如果请求发生错误，可以使用下列函数回复发送错误：

```c
/*
** ===================================================================
**     Method      :  yds_libevent_sendHttpError
**
**     Description :
**         libevent send http error
**     Parameters  : req a request object
**                 : error the HTTP error code
**                 : reason a brief explanation of the error.
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_sendHttpError(req,error,reason) \
	evhttp_send_error((req),(error),(reason))
```



## 十、其他



### 返回码

```c
#define YDS_LIBEVENT_NOERR							0
#define YDS_LIBEVENT_ERROR							-1
#define YDS_LIBEVENT_INIT_EVENT_BASE_ERR			-2
#define YDS_LIBEVENT_EVENT_NULL_ERR					-3
#define YDS_LIBEVENT_LISTENER_TCP_ERR				-4
#define YDS_LIBEVENT_HTTP_SERVER_EXITES_ERR			-5
#define YDS_LIBEVENT_HTTP_SERVER_EVHTTP_ERR			-6
#define YDS_LIBEVENT_HTTP_SERVER_SSL_EC_KEY_ERR		-7
#define YDS_LIBEVENT_HTTP_SERVER_SSL_ECDH_ERR		-8
#define YDS_LIBEVENT_HTTP_SERVER_SSL_SET_CERT_ERR	-9
#define YDS_LIBEVENT_HTTP_SERVER_BIND_SOCKET_ERR	-10
#define YDS_LIBEVENT_HTTP_SERVER_LISTEN_SOCKET_ERR	-11
#define YDS_LIBEVENT_HTTP_CLIENT_URI_PARSE_ERR		-12
#define YDS_LIBEVENT_HTTP_CLIENT_URI_SCHEME_ERR		-13
#define YDS_LIBEVENT_HTTP_CLIENT_URI_HOST_ERR		-14
#define YDS_LIBEVENT_HTTP_CLIENT_SSL_CTX_ERR		-15
#define YDS_LIBEVENT_HTTP_CLIENT_SSL_ERR			-16
#define YDS_LIBEVENT_HTTP_CLIENT_BUFFEREVENT_ERR	-17
#define YDS_LIBEVENT_HTTP_CLIENT_CONNECTION_ERR		-18
#define YDS_LIBEVENT_HTTP_CLIENT_REQUEST_ERR		-19
#define YDS_LIBEVENT_HTTP_CLIENT_GET_BUFF_ERR		-20
#define YDS_LIBEVENT_HTTP_CLIENT_FILE_OPEN_ERR		-21

```

### HTTP请求类型

```c
#define YDS_HTTP_REQ_TYPE		enum evhttp_cmd_type

#define YDS_HTTP_REQ_GET 		EVHTTP_REQ_GET    
#define YDS_HTTP_REQ_POST 		EVHTTP_REQ_POST 
#define YDS_HTTP_REQ_HEADE		EVHTTP_REQ_HEAD 
#define YDS_HTTP_REQ_PUT 		EVHTTP_REQ_PUT
#define YDS_HTTP_REQ_DELETE 	EVHTTP_REQ_DELETE
#define YDS_HTTP_REQ_OPTIONS 	EVHTTP_REQ_OPTIONS
#define YDS_HTTP_REQ_TRACE		EVHTTP_REQ_TRACE 
#define YDS_HTTP_REQ_CONNECT 	EVHTTP_REQ_CONNECT 
#define YDS_HTTP_REQ_PATCH 		EVHTTP_REQ_PATCH
```

### 手动激活Event

```c
/**
   A callback function for an event.
 */
typedef void (*manual_active_event_cb)(evutil_socket_t fd, short what, void *arg);
/*
** ===================================================================
**     Method      :  yds_libevent_addManualActiveEvent
**
**     Description :
**         libevent add Manual Active Event
**     Parameters  : ev: an manual active event
**                 : cb: manual active event callback
**                 : 
**                 : 
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_addManualActiveEvent(struct event				**ev, 
									  manual_active_event_cb	cb);

/*
** ===================================================================
**     Method      :  yds_libevent_active_event
**
**     Description :
**         libevent manual active event
**     Parameters  : ev an event to make active.
**                 : res a set of flags to pass to the event's callback.
**                 : ncalls @param ncalls an obsolete argument: this is ignored.
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_active_event(ev, res) \
	event_active((ev), (res), (0));
```

yds_libevent_addManualActiveEvent：添加手动激活的事件

yds_libevent_active_event：手动激活事件

### 删除事件

```c
/*
** ===================================================================
**     Method      :  yds_libevent_removeEvent
**
**     Description :
**         libevent remove Event
**     Parameters  : ev: an event struct to be removed from the working set
**                 : 
**                 : 
**                 : 
**     Returns     : return 0 if successful, or -1 if an error occurred
** ===================================================================
*/
#define yds_libevent_removeEvent(ev) \
	event_del((ev))

```

删除指定的事件

### 关闭Tcp连接

```c
/*
** ===================================================================
**     Method      :  yds_libevent_tcpSendData
**
**     Description :
**         libevent Tcp close Connect
**     Parameters  : bufev the bufferevent structure to be freed.
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_tcpConnectClose(bufev)	\
	bufferevent_free((bufev))

```

### 获得event_base

```c
/*
** ===================================================================
**     Method      :  yds_libevent_getBase
**
**     Description :
**         libevent get base
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
void yds_libevent_getBase(struct event_base **ev);
```

### 关闭Tcp Server

 ```c
/*
** ===================================================================
**     Method      :  yds_libevent_closeTcpServer
**
**     Description :
**         libevent close tcp server
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
void yds_libevent_closeTcpServer();
 ```

### 关闭HttpServer

```c
/*
** ===================================================================
**     Method      :  yds_libevent_closeHttpServer
**
**     Description :
**         libevent close http server
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
void yds_libevent_closeHttpServer();
```

