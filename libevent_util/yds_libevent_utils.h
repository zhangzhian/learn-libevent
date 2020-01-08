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
#include <sys/timerfd.h>
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
**     Parameters  : ev an event to make active.
**                 : res a set of flags to pass to the event's callback.
**                 : ncalls @param ncalls an obsolete argument: this is ignored.
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
**     Parameters  : ev: an event struct to be removed from the working set
**                 : 
**                 : 
**                 : 
**     Returns     : return 0 if successful, or -1 if an error occurred
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
**     Parameters  : bufev the bufferevent structure to be freed.
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
**     Parameters  : bufev the bufferevent to be written to
**                 : data a pointer to the data to be written
**                 : size the length of the data, in bytes
**                 : 
**     Returns     : 0 if successful, or -1 if an error occurred
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
**     Parameters  : buf the evbuffer to be drained 
**                 : len the number of bytes to drain from the beginning of the buffer
**                 : 
**                 : 
**     Returns     : 0 on success, -1 on failure.
** ===================================================================
*/
#define yds_libevent_removeEvbufferData(buf, len) \
	evbuffer_drain((buf),(len))

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
	TIMER_RTC_ONCE,				//wait ... exec rtc
	TIMER_RTC_TIMEOUT,			//wait ... exec ... wait rtc
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
}YDS_TCP_TYPE_E;			//inner

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
typedef void (*timer_cb)(evutil_socket_t fd, short what, void *arg);

/**
   A callback function for an event.
 */
typedef void (*manual_active_event_cb)(evutil_socket_t fd, short what, void *arg);

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
	tcp_read_data_cb			readcb;					//read data callback
	tcp_write_data_cb			writecb;				//write data callback
	tcp_event_data_cb			eventcb;				//event data callback, see YDS_CONN_RESULT_E
	tcp_connlistener_err_cb		connlistrnercb;
} ydsTcpEventCb;

typedef struct
{
	YDS_TCP_TYPE_E			tcp;						
	tcp_decode_func			decode;
	ydsTcpEventCb			*cb;

} ydsTcpContext;										//inner

typedef struct
{
	YDS_HTTP_TYPE_E		type;							// YDS_HTTP_TYPE_E
	const char			*certificate_chain;				// if https
	const char			*private_key;					// if https
	UINT8				size;							// uri number
	CHAR				*uri[255];						// need listener uri list, uri "/getinfo"
}ydsEvHttpServerInfo;

typedef struct
{
	struct evkeyvalq	*params;						//request uri params data
	const char			*uri;							//request uri
	const char			*uriWithParam;					//request uri include params
	YDS_HTTP_REQ_TYPE	reqType;						//YDS_HTTP_REQ_TYPE :GET POST ...
	struct evbuffer		*evbuff;						//if POST
}ydsEvHttpServerUriRequestInfo;

typedef struct
{
	int					code;							//response client code  200,404,500....
	const char			*reason;						//response client reason, header
	const char			*buffData;						//response client buff data, body
}ydsEvHttpServerUriResponseInfo;

typedef void (*http_server_request_cb)(struct evhttp_request			*request, 
									   ydsEvHttpServerUriRequestInfo	*info);

typedef  struct  {
	const char *key;									//key
	const char *value;									//value
	//CURLformoption value_type;
} ydsPostFormatData;

typedef  struct  {
	const char *key;									//key
	const char *value;									//value
} ydsHeaderData;

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

typedef struct
{
	struct evbuffer		*buffData;						//http server response buff data
	int					code;							//http server response cede
    const char			*reason;						//http server response reason
}ydsEvHttpClientUriResponseInfo;

typedef void (*http_client_request_cb)(struct evhttp_request			*request, 
									   ydsEvHttpClientUriResponseInfo	*info);

typedef struct
{
   struct bufferevent			*bev;				
   http_client_request_cb		cb;
}ydsEvHttpClientRequestDone;							//inner

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
**     Returns     : 0 if successful, or < -1 if an error occurred
** ===================================================================
*/
int yds_libevent_init();

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