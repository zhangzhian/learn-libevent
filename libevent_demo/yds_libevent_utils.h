/** ###################################################################
**     Filename  : yds_libevent_utils.h
**     Project   : 
**     Processor : ARM7
**     Component : 
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

#ifndef libevent_utils__h
#define libevent_utils__h

#ifdef __cplusplus
extern "C"
{
#endif
/** ===========================include=============================== **/
#include <sys/types.h>
#include <fcntl.h> 
#include "yds_rte.h"
#include "yds_rte_api.h"

#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/buffer.h> 
#include <event2/util.h>
#include <event2/thread.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include <event2/bufferevent_ssl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
/** ===========================include=============================== **/

/** ===========================define================================ **/

/*
** ===================================================================
**     Method      :  yds_libevent_active_event
**
**     Description :
**         libevent manual active event
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_active_event(ev, res) \
	event_active((ev), (res), (0));

/*
** ===================================================================
**     Method      :  yds_libevent_removeEvent
**
**     Description :
**         libevent remove Event
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_removeEvent(ev) \
	event_del((ev))

/*
** ===================================================================
**     Method      :  yds_libevent_tcpSendData
**
**     Description :
**         libevent Tcp close Connect
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_tcpConnectClose(bufev)	\
	bufferevent_free((bufev))

/*
** ===================================================================
**     Method      :  yds_libevent_tcpSendData
**
**     Description :
**         libevent Tcp send data
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_tcpSendData(bufev, data, size)	\
	bufferevent_write((bufev), (data),  (size))

/*
** ===================================================================
**     Method      :  yds_libevent_removeEvbufferData
**
**     Description :
**         libevent remove evbuffer data
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_removeEvbufferData(buf, len) \
	evbuffer_drain((buf),(len))

/*
** ===================================================================
**     Method      :  yds_libevent_getEvbufferData
**
**     Description :
**         libevent get evbuffer data, and del
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
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
**     Parameters  : 
**                 : 
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
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
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
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_getEvbuffLength(buf) \
	evbuffer_get_length((buf))

/*
** ===================================================================
**     Method      :  yds_libevent_getHttpParamValue
**
**     Description :
**         libevent get http param value by key
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_getHttpParamValue(headers,key) \
	evhttp_find_header((headers),(key))

/*
** ===================================================================
**     Method      :  yds_libevent_sendHttpError
**
**     Description :
**         libevent send http error
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
#define yds_libevent_sendHttpError(req,error,reason) \
	evhttp_send_error((req),(error),(reason))

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


typedef enum
{
	TIMER_ONCE,					//wait ... exec
    TIMER_TIMEOUT,				//wait ... exec ... wait
}YDS_TIMER_TYPE_E;

typedef enum
{
    CONN_SUCCESS,				//connection succeeded
    CONN_CLOSE,					//connection closed
	CONN_ERROR,					//connection error
}YDS_CONN_RESULT_E;

typedef enum
{
    TCP_CLIENT,				//clent
	TCP_SERVER,				//server
}YDS_TCP_TYPE_E;

typedef enum
{
    POST_JSON,		
	POST_FORMAT,
	POST_HEADER,
	POST_FILE,
	POST_BUFF,
}YDS_HTTP_POST_TYPE_E;

typedef enum 
{ 
	HTTP,
	HTTPS 
} YDS_HTTP_TYPE_E;

/**
   A callback function for an event.
 */
typedef void (*timer_cb)(evutil_socket_t, short, void *);

/**
   A callback function for an event.
 */
typedef void (*manual_active_event_cb)(evutil_socket_t, short, void *);

/**
   An decode data func for a evbuffer.
*/
typedef int (*tcp_decode_func)(struct evbuffer *buff, UINT8 **data);

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
	tcp_read_data_cb			readcb;
	tcp_write_data_cb			writecb;
	tcp_event_data_cb			eventcb;				//YDS_CONN_RESULT_E
	tcp_connlistener_err_cb		connlistrnercb;
} ydsTcpEventCb;

typedef struct
{
	YDS_TCP_TYPE_E			tcp;
	tcp_decode_func			decode;
	ydsTcpEventCb			*cb;

} ydsTcpContext;

typedef struct
{
	YDS_HTTP_TYPE_E		type;
	const char			*certificate_chain;				// if https
	const char			*private_key;					// if https
	UINT8				size;
	CHAR				*uri[255];
}ydsEvHttpServerInfo;

typedef struct
{
	struct evkeyvalq	*params;  
	const char			*uri;
	const char			*uriWithParam;
	YDS_HTTP_REQ_TYPE	reqType;						//GET POST ...
	struct evbuffer		*evbuff;						//if POST
}ydsEvHttpServerUriRequestInfo;

typedef struct
{
	int					code;
	const char			*reason;
	const char			*buffData;	
}ydsEvHttpServerUriResponseInfo;

typedef void (*http_server_request_cb)(struct evhttp_request			*request, 
									   ydsEvHttpServerUriRequestInfo	*info);

typedef  struct  {
	const char *key;
	const char *value;
	//CURLformoption value_type;
} ydsPostFormatData;

typedef  struct  {
	const char *key;
	const char *value;
} ydsHeaderData;

typedef struct
{
	const char				*requestUri; 
	int						retries; 
	struct timeval			timeoutConn;		//	connect timeout
	struct timeval			timeoutWrite;		//	write timeout
	struct timeval			timeoutRead;		//	read timeout
	YDS_HTTP_REQ_TYPE		type;				//	get pos ...
	YDS_HTTP_POST_TYPE_E	postType;
	ydsPostFormatData		*format;
	ydsHeaderData			*header;
	int						formatSize;
	int						headerSize;
	const char				*postData;
	const char				*filePath;
}ydsEvHttpClientUriRequestInfo;

typedef struct
{
	struct evbuffer		*buffData; 
	int					code;
    const char			*reason;
}ydsEvHttpClientUriResponseInfo;

typedef void (*http_client_request_cb)(struct evhttp_request			*request, 
									   ydsEvHttpClientUriResponseInfo	*info);

typedef struct
{
   struct bufferevent			*bev;
   http_client_request_cb		cb;
}ydsEvHttpClientRequestDone;

/** ==========================func================================ **/

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
**     Returns     : None
** ===================================================================
*/
int yds_libevent_init();

/*
** ===================================================================
**     Method      :  yds_libevent_addTimerEvent
**
**     Description :
**         libevent add Timer Event
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_addTimerEvent(struct event			**ev, 
							   YDS_TIMER_TYPE_E		eFlag,
							   struct timeval		delayTime,							   
							   timer_cb				cb);



/*
** ===================================================================
**     Method      :  yds_libevent_addTcpServer
**
**     Description :
**         libevent add Tcp Server Event
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_addTcpServerEvent(int				port, 
								   int				backlog,
								   tcp_decode_func	decode, 
								   ydsTcpEventCb	cb);

/*
** ===================================================================
**     Method      :  yds_libevent_addTcpServer
**
**     Description :
**         libevent add Tcp Client Event
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
struct bufferevent* yds_libevent_addTcpClientEvent(const char			*ip, 
												   int					port, 
												   tcp_decode_func		decode, 
												   ydsTcpEventCb		cb);

/*
** ===================================================================
**     Method      :  yds_libevent_addManualActiveEvent
**
**     Description :
**         libevent add Manual Active Event
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_addManualActiveEvent(struct event				**ev, 
									  manual_active_event_cb	cb);

/*
** ===================================================================
**     Method      :  yds_libevent_addHttpServer
**
**     Description :
**         libevent add http server
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_addHttpServer(int						port, 
							   ydsEvHttpServerInfo		*info,
							   http_server_request_cb	cb);

/*
** ===================================================================
**     Method      :  yds_libevent_addHttpRequest
**
**     Description :
**         libevent add http client request
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_addHttpClientRequest(ydsEvHttpClientUriRequestInfo		*info, 
									  http_client_request_cb			cb);

/*
** ===================================================================
**     Method      :  yds_libevent_sendHttpResponse
**
**     Description :
**         libevent send http server response
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_sendHttpResponse(struct evhttp_request				*request,
								   ydsEvHttpServerUriResponseInfo	*responseInfo);

/*
** ===================================================================
**     Method      :  yds_libevent_removeEvent
**
**     Description :
**         libevent run
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int yds_libevent_run();

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


#ifdef __cplusplus
}
#endif

#endif