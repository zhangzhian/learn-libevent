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
#include <signal.h>

#include "yds_libevent_utils.h"
/** ===========================include=============================== **/

/** ==========================globale================================ **/

#define YDS_LIBEVENT_HTTP_POST_PORMAT_BODY					"--------------------------\r\nContent-Disposition: form-data;name=\"%s\"\r\n\r\n%s\r\n"	
#define YDS_LIBEVENT_HTTP_POST_PORMAT_TAIL					"--------------------------\r\n"
#define YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_TYPE				"Content-Type"
#define YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_LENGTH			"Content-Length"
#define YDS_LIBEVENT_HTTP_POST_PORMAT_CONTENT_TYPE_VALUE	"multipart/form-data; boundary=--------------------------"
#define YDS_LIBEVENT_HTTP_POST_PORMAT_CONTENT_LENGTH_VALUE	"application/json;charset=UTF-8"

static struct event_base		*base			= NULL;
static struct event_config		*conf			= NULL;
static struct evconnlistener	*listenerTcp	= NULL;
static struct evhttp			*http			= NULL;
static struct event				*ev_term		= NULL;
/** ==========================func================================ **/
void yds_libevent_init_openssl();
void yds_libevent_deinit_openssl();

// void cb, avoid event base stop
static void void_cb(evutil_socket_t fd, short event, void *arg)
{
	static int num = 0;
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]libevent is running %d \n",++num);	
}


// tcp read buff cb
static void tcp_read_cb(struct bufferevent *bev, void *arg)
{
	int					size		=	0;
	UINT8				*data		=	NULL;
	ydsTcpContext		*context	=	(ydsTcpContext*)arg;
	struct evbuffer		*input		=	NULL;
	

	input = bufferevent_get_input(bev);	//get evbuffer;

	if (input == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]tcp read data evbuffer null\n");
		return;
	}

	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]tcp read data size %d\n",evbuffer_get_length(input));	

	while (TRUE)
	{
		size = context->decode(input,&data);
	
		if (size < 0)
			break;
		if (size == 0)
			continue;
		
		//real data
		context->cb->readcb(bev, data, size);
 
		if(data != NULL) {
			free(data);
			data = NULL;
		}
	}	
}
 
// tcp write buff cb
static void tcp_write_cb(struct bufferevent *bev, void *arg)
{
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]tcp_write_cb\n");
	ydsTcpContext* context = (ydsTcpContext*)arg;
	context->cb->writecb(bev, NULL);
}

// tcp evnet cb
static void tcp_event_cb(struct bufferevent *bev, short events, void *arg)
{
	ydsTcpContext* context = (ydsTcpContext*)arg;

	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]tcp events:%02x\n",events);

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
	bev = NULL;
}

//tcp listener cb
static void tcp_listener_cb(struct evconnlistener	*listener, 
							evutil_socket_t			fd, 
							struct sockaddr			*addr, 
							int						len, 
							void					*ptr)
{
	
	struct bufferevent		*bev     =	NULL;
	ydsTcpContext			*context =	(ydsTcpContext*)ptr;
	
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]connect new client\n");
	
	/**
	* direct create and connect
	* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	* not use bufferevent_socket_connect() and bufferevent_setfd()
	* but can not receve first conn event
	*/
	// new buffevent
	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	if (!bev) {
		YDS_LOG(LOG_LEVEL_INFO, "[Libevent]error constructing bufferevent!");
		return;
	}

	//bufferevent set cb,than connect, set fd
	bufferevent_setcb(bev,tcp_read_cb,tcp_write_cb, tcp_event_cb, ptr);
	
	bufferevent_socket_connect(bev, addr, sizeof(struct sockaddr));
    bufferevent_setfd(bev,fd);

	bufferevent_enable(bev, EV_READ);
}

//deal terminating signal
static void do_term_cb(int sig, short events, void *arg)
{
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Got %i, Terminating Libevent\n", sig);
	yds_libevent_deinit();
}


static int display_listen_sock(struct evhttp_bound_socket *handle)
{
	struct sockaddr_storage		ss;
	evutil_socket_t				fd;
	ev_socklen_t				socklen			= sizeof(ss);
	char						addrbuf[128];
	void						*inaddr;
	const char					*addr;
	int							got_port		= -1;
	char						uri_root[512];

	fd = evhttp_bound_socket_get_fd(handle);

	memset(&ss, 0, sizeof(ss));
	if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]getsockname() failed");
		return -1;
	}

	if (ss.ss_family == AF_INET) {
		got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
		inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
	} else if (ss.ss_family == AF_INET6) {
		got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
		inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
	} else {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Weird address family %d\n",
		    ss.ss_family);
		return -1;
	}

	addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf, sizeof(addrbuf));

	if (addr) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Listening on %s:%d\n", addr, got_port);
		evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d",addr,got_port);
	} else {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evutil_inet_ntop failed\n");
		return -1;
	}

	return 0;
}

//http universal cb
static void http_universal_cb(struct evhttp_request *req, void *arg)
{
	evhttp_send_error(req, 404, "url was not found");
}

// http server deal requset cb
static void http_deal_request_cb(struct evhttp_request *req, void *arg)
{
	const char						*cmdtype;
	struct evkeyvalq				*headers;
	struct evkeyval					*header;
	struct evbuffer					*buf_in;
	struct evhttp_uri				*decoded		= NULL;
    struct evkeyvalq				params;
	char							*decoded_path;
	const char						*path;
	const char						*uri;
	char							cbuf[1024]		= {0};

	http_server_request_cb			cb				= (http_server_request_cb)arg;
	ydsEvHttpServerUriRequestInfo	requestInfo		= {0};

	switch (evhttp_request_get_command(req)) {
		case EVHTTP_REQ_GET: cmdtype = "GET"; break;
		case EVHTTP_REQ_POST: cmdtype = "POST"; break;
		case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
		case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
		case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
		case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
		case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
		case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
		case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
		default: cmdtype = "unknown"; break;
	}

	//received a request for /test?a=123
	uri = evhttp_request_get_uri(req);
	
	requestInfo.uriWithParam = uri;
	requestInfo.reqType = evhttp_request_get_command(req);

	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Received a %s request for %s\nHeaders:\n", cmdtype, uri);

 	/*
	 *  Headers:
	 *  Host: 127.0.0.1
	 *  Connection: close
	 */
	headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) {
		YDS_LOG(LOG_LEVEL_INFO,"  %s: %s\n", header->key, header->value);
	}

	/*********************************/
	// parase URI
	decoded = evhttp_uri_parse(uri);

	if (!decoded) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]It's not a good URI decoded. Sending BADREQUEST\n");
	}

	// get path 
	// path: /test
	path = evhttp_uri_get_path(decoded);
	if (!path){
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]It's not a good URI path. Sending BADREQUEST\n");
	}
	requestInfo.uri = path;

	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]path: %s\n", path);
    
	// parse URI params
    // key-value format, q=value1, s=value2
    evhttp_parse_query(uri, &params);
	requestInfo.params = &params;

	/*********************************/

	buf_in = evhttp_request_get_input_buffer(req);

	if (buf_in==NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evBuf null\n");
	}
	requestInfo.evbuff = buf_in;

	cb(req, &requestInfo);
}

// http client requset done cb
static void http_request_done(struct evhttp_request *req, void *ctx)
{
	char							buffer[256];
	int								nread;
	struct evbuffer					*buf_in;
	ydsEvHttpClientUriResponseInfo	info = {0};
	ydsEvHttpClientRequestDone		*requestDonw = (ydsEvHttpClientRequestDone *) ctx;
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]http_request_done\n");
	//error deal，print
	if (!req || !evhttp_request_get_response_code(req)) {

		unsigned long oslerr;
		int printed_err = 0;
		int errcode = EVUTIL_SOCKET_ERROR();
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]some request failed - no idea which one though!\n");
		/* Print out the OpenSSL error queue that libevent
		 * squirreled away for us, if any. */
		while ((oslerr = bufferevent_get_openssl_error(requestDonw->bev))) {
			ERR_error_string_n(oslerr, buffer, sizeof(buffer));
			YDS_LOG(LOG_LEVEL_INFO,"[Libevent] %s\n", buffer);
			printed_err = 1;
		}
		/* If the OpenSSL error queue was empty, maybe it was a
		 * socket error; let's try printing that. */
		if (! printed_err)
			YDS_LOG(LOG_LEVEL_INFO,"[Libevent]socket error = %s (%d)\n",
				evutil_socket_error_to_string(errcode), errcode);
		requestDonw->cb(req,&info);
		return;
	}

	info.code = evhttp_request_get_response_code(req);
	info.reason = evhttp_request_get_response_code_line(req);

	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Response line: %d %s\n",info.code ,info.reason );

	//获取数据
	buf_in = evhttp_request_get_input_buffer(req);

	if (buf_in==NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evBuf null, err\n");
	}

	info.buffData = buf_in;

	requestDonw->cb(req,&info);
}

/**
 *	该回调负责创建新的SSL连接并将其包装在OpenSSL bufferevent中。
 *	这是我们实现https服务器而不是普通的http服务器的方式。
 */
static struct bufferevent* buffevent_ssl_cb(struct event_base *base, void *arg)
{ 
    struct bufferevent			*bev;
    SSL_CTX						*ctx = (SSL_CTX *) arg;
 
    bev = bufferevent_openssl_socket_new (base, 
		-1, 
		SSL_new (ctx), 
		BUFFEREVENT_SSL_ACCEPTING, 
		BEV_OPT_CLOSE_ON_FREE);

    return bev;
}

static int server_setup_certs (SSL_CTX *ctx, 
							   const char *certificate_chain, 
							   const char *private_key)
{ 
    YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Loading certificate chain from '%s'\n"
            "and private key from '%s'\n",
            certificate_chain, private_key);
 
    if (1 != SSL_CTX_use_certificate_chain_file (ctx, certificate_chain))
       return -1;
 
    if (1 != SSL_CTX_use_PrivateKey_file (ctx, private_key, SSL_FILETYPE_PEM))
       return -1;
 
    if (1 != SSL_CTX_check_private_key (ctx))
       return -1;
}

//init libevent
int yds_libevent_init(){

	int					ret				= 0;
	int					pthreads_ret	= 0;
	struct event		*ev_void		= NULL;
	//struct event		*ev_term		= NULL;
	struct timeval		tv				= {5,0};
	struct timeval		tv_max_interval = {1,0};
	
	//support pthreads
	pthreads_ret = evthread_use_pthreads();

	YDS_LOG(LOG_LEVEL_INFO,"[Libevent] evthread_use_pthreads %d\n",pthreads_ret);
	
	//create event_config
	conf = event_config_new();

	//max interval 1s
	event_config_set_max_dispatch_interval(conf, &tv_max_interval, -1, 0);

	//create event_base
	base = event_base_new_with_config(conf);

	if (!base) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Couldn't open event base\n");
		return YDS_LIBEVENT_INIT_EVENT_BASE_ERR;
	}
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent] Using Libevent with backend method %s.\n", event_base_get_method(base));
	//void event, avoid libevent stopas
	ev_void = event_new(base, -1, EV_PERSIST, void_cb, event_self_cbarg());
	event_add(ev_void, &tv);

	//Stop signal SIGTSTP
	ev_term = evsignal_new(base, SIGINT , do_term_cb, base);
	event_add(ev_term, NULL);
	return ret;
}

//add timer event
int yds_libevent_addTimerEvent(struct event			**ev, 
							   YDS_TIMER_TYPE_E		eFlag,
							   struct timeval		delayTime,							   
							   timer_cb				cb){


								   
	int					ret;
	int					tfd = 0;
	struct timeval		tv = {0,0};
	struct itimerspec	timeValue;
	ydsTimerContext     context;

	if (base == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event base not create\n");
		return YDS_LIBEVENT_EVENT_NULL_ERR;
	}

	if (eFlag == TIMER_ONCE)
	{
		*ev = evtimer_new(base, cb, event_self_cbarg());
		ret = event_add(*ev, &delayTime);			
	}

	if (eFlag == TIMER_TIMEOUT)
	{
		*ev = event_new(base, -1, EV_PERSIST, cb, event_self_cbarg());
		ret = event_add(*ev, &delayTime);		
	}

	if (eFlag == TIMER_RTC_ONCE)
	{
		tfd = timerfd_create(12, 0); //TFD_CLOEXEC or TFD_NONBLOCK

		if (tfd < 0)
		{
			return YDS_LIBEVENT_ERROR;
		}

		//it_value表示定时器第一次超时时间，it_interval表示之后的超时时间即每隔多长时间超时
		timeValue.it_value.tv_sec = (time_t)delayTime.tv_sec;
		timeValue.it_value.tv_nsec = delayTime.tv_usec;

		ret = timerfd_settime(tfd, 0, &timeValue, NULL);

		if (ret < 0)
		{
			return YDS_LIBEVENT_ERROR;
		}
		*ev = event_new(base, tfd, EV_READ, cb, event_self_cbarg());
		ret = event_add(*ev, NULL);		
	}
	if (eFlag == TIMER_RTC_TIMEOUT)
	{
		tfd = timerfd_create(12, 0); //TFD_CLOEXEC or TFD_NONBLOCK

		if (tfd < 0)
		{
			return YDS_LIBEVENT_ERROR;
		}

		//it_value表示定时器第一次超时时间，it_interval表示之后的超时时间即每隔多长时间超时
		timeValue.it_value.tv_sec = (time_t)delayTime.tv_sec;
		timeValue.it_value.tv_nsec = delayTime.tv_usec;	
		
	    //设置定时器周期
		timeValue.it_interval.tv_sec = (time_t)5;
		timeValue.it_interval.tv_nsec = 0;	

		ret = timerfd_settime(tfd, 0, &timeValue, NULL);

		if (ret < 0)
		{
			return YDS_LIBEVENT_ERROR;
		}
		
		*ev = event_new(base, tfd, EV_READ | EV_PERSIST, cb, event_self_cbarg());

		ret = event_add(*ev, NULL);		
	}
	return ret;
}

//add tcp server event 
int yds_libevent_addTcpServerEvent(int					port, 
								   int					backlog, 
								   tcp_decode_func		decode, 
								   ydsTcpEventCb		cb){

	struct sockaddr_in			serv;
	static ydsTcpContext		context; 
	static ydsTcpEventCb		mcb			= {0};
	static tcp_decode_func		mdecode;

	if (base == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event base not create\n");
		return YDS_LIBEVENT_EVENT_NULL_ERR;
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
	listenerTcp = evconnlistener_new_bind(base, tcp_listener_cb, &context, 
                                  LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                  backlog == 0 ? 1 : backlog , 
								  (struct sockaddr*)&serv, sizeof(serv));

	if (!listenerTcp) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Couldn't create listener\n");
		return YDS_LIBEVENT_LISTENER_TCP_ERR;
	}
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]create listener\n");
	
	//error处理  tcp_connlistener_error_cb
	evconnlistener_set_error_cb(listenerTcp, cb.connlistrnercb);

	return YDS_LIBEVENT_NOERR;
}



// add tcp client event
struct bufferevent* yds_libevent_addTcpClientEvent(const char			*ip, 
												   int					port, 
												   tcp_decode_func		decode, 
												   ydsTcpEventCb		cb){

	struct bufferevent*			bev			= NULL;
	struct sockaddr_in			serv;
	static ydsTcpContext		context; 
	static ydsTcpEventCb		mcb;
	static tcp_decode_func		mdecode;

	if (base == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event base not create\n");
		return bev;
	}

	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	if (bev == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]new bufferevent error \n");
		return bev;
	}

	//set cb
    bufferevent_setcb(bev, tcp_read_cb, tcp_write_cb, tcp_event_cb, &context);

	//connect server
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
	//parse IP address
    evutil_inet_pton(AF_INET, ip, &serv.sin_addr.s_addr);
	//connect
    bufferevent_socket_connect(bev, (struct sockaddr*)&serv, sizeof(serv));

	mcb.eventcb = cb.eventcb;
	mcb.readcb = cb.readcb;
	mcb.writecb = cb.writecb;

	mdecode = decode;

	context.cb = &mcb;
	context.decode = mdecode;

	context.tcp = TCP_CLIENT;

    bufferevent_enable(bev, EV_READ);

	return bev;
}

/* add manual active event
 * use yds_libevent_active_event func to active */
int yds_libevent_addManualActiveEvent(struct event				**ev,
									  manual_active_event_cb	cb){
	
	*ev = event_new(base, -1, EV_READ | EV_WRITE, cb, NULL);

	return event_add(*ev, NULL);
}

// add http or https server
int yds_libevent_addHttpServer(int								port, 
							   ydsEvHttpServerInfo				*info,
							   http_server_request_cb			cb){
	
	UINT8						i			= 0;
	INT8						ret			= 0;
	SSL_CTX						*ssl_ctx	= NULL;
	SSL							*ssl		= NULL;
	struct evhttp_bound_socket	*handle		= NULL;

	if (base == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event base not create\n");
		ret = YDS_LIBEVENT_EVENT_NULL_ERR;
		goto end;
	}

    if(http != NULL){
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Aleady start server\n");
		ret = YDS_LIBEVENT_HTTP_SERVER_EXITES_ERR;
		goto end;
	}

	if (info->type == HTTPS)
	{
		//init openssl
		yds_libevent_init_openssl();
	}

	// create evhttp
	http = evhttp_new(base);
	if (!http) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]couldn't create evhttp.\n");
		ret = YDS_LIBEVENT_HTTP_SERVER_EVHTTP_ERR;
		goto cleanup;	
	}

	if (info->type == HTTPS)
	{
		/******************************************/
		/* 创建SSL上下文环境 ，可以理解为 SSL句柄 */
		ssl_ctx = SSL_CTX_new (SSLv23_server_method ());
		SSL_CTX_set_options (ssl_ctx,
				SSL_OP_SINGLE_DH_USE |
				SSL_OP_SINGLE_ECDH_USE |
				SSL_OP_NO_SSLv2);
 
		/* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
		 * We just hardcode a single curve which is reasonably decent.
		 * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
		EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);

		if (! ecdh)
		{
			ret = YDS_LIBEVENT_HTTP_SERVER_SSL_EC_KEY_ERR;
			goto cleanup;
		}
		if (1 != SSL_CTX_set_tmp_ecdh (ssl_ctx, ecdh))
		{
			ret = YDS_LIBEVENT_HTTP_SERVER_SSL_ECDH_ERR;
			goto cleanup;
		}
 
		/* 设置服务器证书 和 服务器私钥 到 OPENSSL ctx上下文句柄中 */
		if (server_setup_certs (ssl_ctx, info->certificate_chain, info->private_key) == -1)
		{
			ret = YDS_LIBEVENT_HTTP_SERVER_SSL_SET_CERT_ERR;
			goto cleanup;
		}
		/* 
			使我们创建好的evhttp句柄 支持 SSL加密
			实际上，加密的动作和解密的动作都已经帮
			我们自动完成，我们拿到的数据就已经解密之后的

			设置用于为与给定evhttp对象的连接创建新的bufferevent的回调。
			您可以使用它来覆盖默认的bufferevent类型，
			例如，使此evhttp对象使用SSL缓冲区事件而不是未加密的事件。
			新的缓冲区事件必须在未设置fd的情况下进行分配。
		*/
		evhttp_set_bevcb (http, buffevent_ssl_cb, ssl_ctx);
		/******************************************/
	}

	for (i = 0;i< info->size;i++)
	{                
		evhttp_set_cb(http,(const char *)info->uri[i], http_deal_request_cb, cb);
	}

	//accept any request，set universal cb
	evhttp_set_gencb(http, http_universal_cb, NULL);

	//bind socket
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);

	if (!handle) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]couldn't bind to port %d. Exiting.\n", port);
		ret = YDS_LIBEVENT_HTTP_SERVER_BIND_SOCKET_ERR;
		goto cleanup;
	}
	//listen socket
	if (display_listen_sock(handle)) {
		ret = YDS_LIBEVENT_HTTP_SERVER_LISTEN_SOCKET_ERR;
		goto cleanup;
	}

	goto end;

cleanup:
	
	if (http){
		evhttp_free(http);
		http == NULL;
	}
	
	if (ssl_ctx){
		SSL_CTX_free(ssl_ctx);
	}
	
	if (info->type == HTTPS && ssl){
		SSL_free(ssl);
	}

	yds_libevent_deinit_openssl();

end:

	return ret;
}

// send http server response
int yds_libevent_sendHttpResponse(struct evhttp_request					*request,
								  ydsEvHttpServerUriResponseInfo		*responseInfo){

	struct evbuffer *buf_out = evbuffer_new();
	
	INT8 res = YDS_LIBEVENT_NOERR;

	if (responseInfo->buffData != NULL)
	{
		res = evbuffer_add_printf(buf_out,"%s",responseInfo->buffData);
	}

	if (res != YDS_LIBEVENT_ERROR)
	{
		evhttp_send_reply(request, responseInfo->code, responseInfo->reason, buf_out);
	}

	free(buf_out);

	return res;
}


int yds_libevent_addHttpClientRequest(ydsEvHttpClientUriRequestInfo			*info, 
									  http_client_request_cb				cb){

									
	int							i;
	int							result_req;
	int							ret				=	0;
	struct evhttp_uri			*http_uri		=	NULL;
	const char					*scheme;
	const char					*host;
	const char					*path;
	const char					*query;
	char						uri[256];					//not include http://ip:port
	int							port;
	BOOLEAN						isHttps			=	FALSE;

	struct bufferevent			*bev;
	struct evhttp_connection	*evcon			=	NULL;
	struct evhttp_request		*req;
	struct evkeyvalq			*output_headers;
	
	ydsEvHttpClientRequestDone	requestDone;
	SSL_CTX						*ssl_ctx		=	NULL;
	SSL							*ssl			=	NULL;
	

	//static http_client_request_cb	mcb;
	if (base == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event base not create\n");
		ret = YDS_LIBEVENT_EVENT_NULL_ERR;
		goto end;
	}

	http_uri = evhttp_uri_parse(info->requestUrl);

	if (http_uri == NULL) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]url parse error \n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_URI_PARSE_ERR;
		goto cleanup;
	}

	scheme = evhttp_uri_get_scheme(http_uri);
	//忽略大小写比较字符串
	if (scheme == NULL || (
		strcasecmp(scheme, "http") != 0 && 
		strcasecmp(scheme, "https") != 0 )) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]url must be http/https\n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_URI_SCHEME_ERR;
		goto cleanup;
	}

	if (strcasecmp(scheme, "https") == 0)
	{
		isHttps = TRUE;
	}

	host = evhttp_uri_get_host(http_uri);
	if (host == NULL) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]url must have a host\n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_URI_HOST_ERR;
		goto cleanup;
	}
	
	port = evhttp_uri_get_port(http_uri);
	if (port == -1) {
		port = isHttps ? 443 : 80;
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]port %d\n",port);
	}

	path = evhttp_uri_get_path(http_uri);
	if (strlen(path) == 0) {
		path = "/";
	}

	query = evhttp_uri_get_query(http_uri);

	if (query == NULL) {
		//将可变参数 “…” 按照format的格式格式化为字符串，然后再将其拷贝至str中。
		snprintf(uri, sizeof(uri) - 1, "%s", path);
	} else {
		snprintf(uri, sizeof(uri) - 1, "%s?%s", path, query);
	}
	uri[sizeof(uri) - 1] = '\0';
	
	if (isHttps)
	{
		yds_libevent_init_openssl();
		/* Create a new OpenSSL context */
		ssl_ctx = SSL_CTX_new(SSLv23_method());
		if (!ssl_ctx) {
			ret = YDS_LIBEVENT_HTTP_CLIENT_SSL_CTX_ERR;
			goto cleanup;
		}

		// Create OpenSSL bufferevent and stack evhttp on top of it
		ssl = SSL_new(ssl_ctx);
		if (ssl == NULL) {
			ret = YDS_LIBEVENT_HTTP_CLIENT_SSL_ERR;
			goto cleanup;
		}

		#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
			// Set hostname for SNI extension
			SSL_set_tlsext_host_name(ssl, host);
		#endif
	}

	if (!isHttps) {
		bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	}else
	{
		bev = bufferevent_openssl_socket_new(base, -1, ssl,
			BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);

		bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
	}
	
	if (bev == NULL) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]bufferevent_socket_new() failed\n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_BUFFEREVENT_ERR;
		goto cleanup;
	}
	
	evcon = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);

	if (evcon == NULL) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evhttp_connection_base_bufferevent_new() failed\n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_CONNECTION_ERR;
		goto cleanup;
	}

	//重试
	if (info->retries >= 0) {
		evhttp_connection_set_retries(evcon, info->retries);
	}

	//超时
	if (info->timeoutConn.tv_sec != 0 && info->timeoutConn.tv_usec != 0) {
		evhttp_connection_set_timeout_tv(evcon, &(info->timeoutConn));
	}

	//写超时
	if (info->timeoutWrite.tv_sec != 0 && info->timeoutWrite.tv_usec != 0) {
		bufferevent_set_timeouts(bev,NULL,&(info->timeoutWrite));
	}

	//读超时
	if (info->timeoutRead.tv_sec != 0 && info->timeoutRead.tv_usec != 0)
	{
		bufferevent_set_timeouts(bev,&(info->timeoutRead),NULL);
	}

	requestDone.bev = bev;

	requestDone.cb = cb;

	//回调
	req = evhttp_request_new(http_request_done, &requestDone);

	if (req == NULL) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evhttp_request_new() failed\n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_REQUEST_ERR;
		goto cleanup;
	}

	output_headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_headers, "Host", host);
	evhttp_add_header(output_headers, "Connection", "close");

	for (i = 0; i < info->headerSize; i++)
	{
		evhttp_add_header(output_headers, info->header[i].key, info->header[i].value);
	}

	switch (info->type)
	{
		case EVHTTP_REQ_GET:
		{
			//真正请求
			result_req = evhttp_make_request(evcon, req, EVHTTP_REQ_GET, uri);
			break;
		}
		case EVHTTP_REQ_POST:
		{
			struct evbuffer		*request_buffer = NULL;
			char				buffSize[16]    = {0};

			request_buffer = evhttp_request_get_output_buffer(req);
			
			if (request_buffer == NULL)
			{
				YDS_LOG(LOG_LEVEL_INFO, "[Libevent]get output buffer failed\n");
				ret = YDS_LIBEVENT_HTTP_CLIENT_GET_BUFF_ERR;
				goto cleanup;
			}

			switch (info->postType)
			{
				case POST_FORMAT:
				{
					char buff[1024]		= {0};
					char buff_end[64]	= {0};
		
					for (i = 0; i < info->formatSize; i++)
					{		
						memset(buff,0,sizeof(buff));
						sprintf(buff, 
							YDS_LIBEVENT_HTTP_POST_PORMAT_BODY,
							info->format[i].key, 
							info->format[i].value);
						evbuffer_add(request_buffer, buff, strlen(buff));
					}
					
					//尾部
					evbuffer_add(request_buffer, 
						YDS_LIBEVENT_HTTP_POST_PORMAT_TAIL,
						strlen(YDS_LIBEVENT_HTTP_POST_PORMAT_TAIL));
		
					//header
					evhttp_add_header(output_headers, 
						YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_TYPE, 
						YDS_LIBEVENT_HTTP_POST_PORMAT_CONTENT_TYPE_VALUE);

					size_t buff_size = evbuffer_get_length(request_buffer);
					evutil_snprintf(buffSize, sizeof(buffSize)-1, "%lu", (unsigned long)buff_size);
					evhttp_add_header(output_headers, 
						YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_LENGTH, buffSize);

					break;
				}
				case POST_JSON:
				{
					evbuffer_add(request_buffer, info->postData, strlen(info->postData));
					evutil_snprintf(buffSize, sizeof(buffSize)-1, "%lu", strlen(info->postData));

					evhttp_add_header(output_headers, 
						YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_TYPE, 
						YDS_LIBEVENT_HTTP_POST_PORMAT_CONTENT_LENGTH_VALUE);

					evhttp_add_header(output_headers, 
						YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_LENGTH, buffSize);
					break;
				}
				case POST_HEADER:
				{
					for (i = 0; i < info->formatSize; i++)
					{
						evhttp_add_header(output_headers, info->format[i].key, info->format[i].value);
					}
					break;
				}
				case POST_FILE:
				{
					int fd = open(info->filePath, O_RDONLY);
					
					if (fd == -1)
					{
						YDS_LOG(LOG_LEVEL_INFO,"[Libevent]open %s failed\n", info->filePath);
						ret = YDS_LIBEVENT_HTTP_CLIENT_FILE_OPEN_ERR;
						goto cleanup;
					}

					evbuffer_add_file(request_buffer, fd, 0, -1);

					size_t buff_size = evbuffer_get_length(request_buffer);

					evutil_snprintf(buffSize, sizeof(buffSize)-1, "%lu", (unsigned long)buff_size);

					evhttp_add_header(output_headers, 
						YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_LENGTH, buffSize);
				
					break;
				}
				case POST_BUFF:
				{
					evbuffer_add(request_buffer, info->postData, strlen(info->postData));
					evutil_snprintf(buffSize, sizeof(buffSize)-1, "%lu", strlen(info->postData));

					evhttp_add_header(output_headers, 
						YDS_LIBEVENT_HTTP_HEAD_KEY_CONTENT_LENGTH, buffSize);
					break;
				}
				default:
					break;
			}
			//真正请求
			result_req = evhttp_make_request(evcon, req, EVHTTP_REQ_POST, uri);	
			break;
		}
		default:
			break;
	}

	if (result_req != 0) {
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evhttp_make_request() failed\n");
		ret = YDS_LIBEVENT_HTTP_CLIENT_REQUEST_ERR;
		goto cleanup;
	}

	goto end;

cleanup:

	if (evcon){
		evhttp_connection_free(evcon);
	}

	if (http_uri){
		evhttp_uri_free(http_uri);
	}

	if (isHttps && ssl_ctx){
		SSL_CTX_free(ssl_ctx);
	}

	if (isHttps && ssl){
		SSL_free(ssl);
	}

	yds_libevent_deinit_openssl();

end:

	return ret;
}

//libevent get base
void yds_libevent_getBase(struct event_base **ev){
	*ev = base;
}

//libevent run
int yds_libevent_run(){

	INT8 ret = 0;

	if (base == NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event base not create\n");
		ret = YDS_LIBEVENT_EVENT_NULL_ERR;
	}else
	{
		ret = event_base_dispatch(base);
	}	

	return ret;
}


void yds_tcp_connlistener_err_deal(struct evconnlistener	*listener, 
								   void						*ctx)
{
	int err = EVUTIL_SOCKET_ERROR();
	YDS_LOG(LOG_LEVEL_INFO,"[Libevent]Got an error %d (%s) on the Tcplistener.Shutting down.\n", 
		err, evutil_socket_error_to_string(err));

	evconnlistener_free(listener);
	listener = NULL;
}

//close http server
void yds_libevent_closeHttpServer(){
	if (http != NULL)
	{
		evhttp_free(http);
		http = NULL;
	}else
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]http server not create\n");
	}
}

//close tcp server
void yds_libevent_closeTcpServer(){
	if (listenerTcp != NULL)
	{
		evconnlistener_disable(listenerTcp);
		evconnlistener_free(listenerTcp);
		listenerTcp = NULL;
	}else
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]tcp server not create\n");
	}
}

void yds_libevent_init_openssl(){
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x20700000L)
		// Initialize OpenSSL
	SSL_library_init();
	ERR_load_crypto_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	printf ("Using OpenSSL version \"%s\"\nand libevent version \"%s\"\n",
			SSLeay_version (SSLEAY_VERSION), event_get_version ());
#endif
}

void yds_libevent_deinit_openssl(){
	#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x20700000L)
	EVP_cleanup();
	ERR_free_strings();

#if OPENSSL_VERSION_NUMBER < 0x10000000L
	ERR_remove_state(0);
#else
	ERR_remove_thread_state(NULL);
#endif

	CRYPTO_cleanup_all_ex_data();

	sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
#endif /* (OPENSSL_VERSION_NUMBER < 0x10100000L) || \
	(defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x20700000L) */
}

//deinit
void yds_libevent_deinit(){

	if (http != NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evhttp_free\n");
		evhttp_free(http);
		http = NULL;
	}
	
	if (listenerTcp != NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]evconnlistener_free\n");
		evconnlistener_free(listenerTcp);
		listenerTcp = NULL;
	}
	
	if (conf!=NULL)
	{
		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event_config_free\n");
		event_config_free(conf);
		conf = NULL;
	}
	
	if (base!=NULL)
	{
		event_del(ev_term);

		YDS_LOG(LOG_LEVEL_INFO,"[Libevent]event_base_loopexit: %d\n",event_base_loopexit(base,NULL));
				
		base = NULL;
	}	

	libevent_global_shutdown();
}

