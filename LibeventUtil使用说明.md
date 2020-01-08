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



## 四、添加Tcp Server事件



## 五、添加Tcp Client事件





## 六、添加Http/Https Server事件





## 七、添加Http/Https Client事件





## 八、其他



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

