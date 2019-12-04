#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http.h>

static int ignore_cert = 0;

static void
http_request_done(struct evhttp_request *req, void *ctx)
{
	char buffer[256];
	int nread;
	struct evbuffer *buf_in;
	char cbuf[64] = {0};

	//错误处理，打印
	if (!req || !evhttp_request_get_response_code(req)) {
		
		struct bufferevent *bev = (struct bufferevent *) ctx;
		unsigned long oslerr;
		int printed_err = 0;
		int errcode = EVUTIL_SOCKET_ERROR();
		fprintf(stderr, "some request failed - no idea which one though!\n");

		/* 如果OpenSSL错误队列为空，则可能是套接字错误。 让我们尝试打印 */
		if (! printed_err)
			fprintf(stderr, "socket error = %s (%d)\n",
				evutil_socket_error_to_string(errcode),
				errcode);
		return;
	}

	fprintf(stderr, "Response line: %d %s\n",
	    evhttp_request_get_response_code(req),
	    evhttp_request_get_response_code_line(req));

	//获取POST方法的数据
	buf_in = evhttp_request_get_input_buffer(req);

	if (buf_in==NULL)
	{
		printf("evBuf null, err\n");
	}
	//获取长度
	int buf_in_len = evbuffer_get_length(buf_in);
	
	printf("evBuf len:%d\n",buf_in_len);
    
	if(buf_in_len <= 0)
	{
	}
	//将数据从evbuff中移动到char *
	int str_len = evbuffer_remove(buf_in,cbuf,sizeof(cbuf));

	if (str_len <= 0)
	{
		printf("post parameter null err\n");
	}
	printf("str_len:%d cbuf:%s\n",str_len,cbuf);
}

static void
err(const char *msg)
{
	fputs(msg, stderr);
}

int
main(int argc, char **argv)
{
	int r;
	struct event_base *base = NULL;
	struct evhttp_uri *http_uri = NULL;
	const char *url = NULL;
	const char *scheme, *host, *path, *query;
	char uri[256];
	int port;
	int retries = 0;
	int timeout = -1;

	struct bufferevent *bev;
	struct evhttp_connection *evcon = NULL;
	struct evhttp_request *req;
	struct evkeyvalq *output_headers;

	int i;
	int ret = 0;
	
	//初始化url
	url = "http://127.0.0.1:8888/test?a=123";
	if (!url) {
		goto error;
	}

	http_uri = evhttp_uri_parse(url);
	if (http_uri == NULL) {
		err("malformed url");
		goto error;
	}

	scheme = evhttp_uri_get_scheme(http_uri);
	//忽略大小写比较字符串
	if (scheme == NULL || strcasecmp(scheme, "http") != 0) {
		err("url must be http");
		goto error;
	}

	host = evhttp_uri_get_host(http_uri);
	if (host == NULL) {
		err("url must have a host");
		goto error;
	}

	port = evhttp_uri_get_port(http_uri);

	if (port == -1) {
		port = 80;
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

	// 创建 event base
	base = event_base_new();
	if (!base) {
		perror("event_base_new()");
		goto error;
	}

	if (strcasecmp(scheme, "http") == 0) {
		bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	} 

	if (bev == NULL) {
		fprintf(stderr, "bufferevent_socket_new() failed\n");
		goto error;
	}

	evcon = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);
	if (evcon == NULL) {
		fprintf(stderr, "evhttp_connection_base_bufferevent_new() failed\n");
		goto error;
	}

	//重试
	if (retries > 0) {
		evhttp_connection_set_retries(evcon, retries);
	}
	//超时
	if (timeout >= 0) {
		evhttp_connection_set_timeout(evcon, timeout);
	}

	//回调
	req = evhttp_request_new(http_request_done, bev);

	if (req == NULL) {
		fprintf(stderr, "evhttp_request_new() failed\n");
		goto error;
	}

	output_headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_headers, "Host", host);
	evhttp_add_header(output_headers, "Connection", "close");

	//真正请求
	r = evhttp_make_request(evcon, req, EVHTTP_REQ_GET, uri);

	if (r != 0) {
		fprintf(stderr, "evhttp_make_request() failed\n");
		goto error;
	}

	event_base_dispatch(base);

	goto cleanup;

error:
	printf("error stop");
	ret = 1;
cleanup:
	if (evcon)
		evhttp_connection_free(evcon);
	if (http_uri)
		evhttp_uri_free(http_uri);
	if (base)
		event_base_free(base);

	return ret;
}
