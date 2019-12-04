#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <signal.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

char uri_root[512];

static const struct table_entry {
	const char *extension;
	const char *content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" },
	{ NULL, NULL },
};

/* 尝试猜测“path”的内容类型 */
static const char *
guess_content_type(const char *path)
{
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

/* 用于/test URI请求的回调 */
static void
dump_request_cb(struct evhttp_request *req, void *arg)
{
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf_in;
	struct evbuffer *buf_out;
	struct evhttp_uri *decoded = NULL;
    struct evkeyvalq params;
	char *decoded_path;
	const char *path;
	char cbuf[1024] = {0};
	const char *uri = evhttp_request_get_uri(req);

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

	printf("Received a %s request for %s\nHeaders:\n",
	    cmdtype, uri);

	headers = evhttp_request_get_input_headers(req);

	for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}

	/*********************************/
	/* 解析 URI */
	decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	/* 获取path */
	path = evhttp_uri_get_path(decoded);
	if (!path){
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
	}
	printf("path: %s\n", path);

	
    //解析URI的参数
    //将URL数据封装成key-value格式,q=value1, s=value2
    evhttp_parse_query(uri, &params);
	//得到a所对应的value
	const char *a_data = evhttp_find_header(&params, "a");

    printf("a=%s\n",a_data);
	/*********************************/
	if (strcmp(cmdtype,"POST") == 0)
	{
		//获取POST方法的数据
		buf_in = evhttp_request_get_input_buffer(req);

		if (buf_in==NULL)
		{
			printf("evBuf null, err\n");
			goto err;
		}
		//获取长度
		int buf_in_len = evbuffer_get_length(buf_in);
	
		printf("evBuf len:%d\n",buf_in_len);
    
		if(buf_in_len <= 0)
		{
			goto err;
		}
		//将数据从evbuff中移动到char *
		int str_len = evbuffer_remove(buf_in,cbuf,sizeof(cbuf));

		if (str_len <= 0)
		{
			printf("post parameter null err\n");
			goto err;
		}
		printf("str_len:%d cbuf:%s\n",str_len,cbuf);
	}
	/*********************************/
	buf_out = evbuffer_new();
	if(!buf_out)
    {
        puts("failed to create response buffer \n");
        return;
    }
	evbuffer_add_printf(buf_out,"%s","success");
	evhttp_send_reply(req, 200, "OK", buf_out);
	return;
err:
    evhttp_send_error(req, HTTP_INTERNAL, 0);
}

static void
send_document_cb(struct evhttp_request *req, void *arg)
{
	evhttp_send_error(req, 404, "url was not found");
}

static void
do_term(int sig, short events, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	event_base_loopbreak(base);
	fprintf(stderr, "Got %i, Terminating\n", sig);
}

static int
display_listen_sock(struct evhttp_bound_socket *handle)
{
	struct sockaddr_storage ss;
	evutil_socket_t fd;
	ev_socklen_t socklen = sizeof(ss);
	char addrbuf[128];
	void *inaddr;
	const char *addr;
	int got_port = -1;

	fd = evhttp_bound_socket_get_fd(handle);
	memset(&ss, 0, sizeof(ss));
	if (getsockname(fd, (struct sockaddr *)&ss, &socklen)) {
		perror("getsockname() failed");
		return 1;
	}

	if (ss.ss_family == AF_INET) {
		got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
		inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
	} else if (ss.ss_family == AF_INET6) {
		got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
		inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
	}
	else {
		fprintf(stderr, "Weird address family %d\n",
		    ss.ss_family);
		return 1;
	}

	addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
	    sizeof(addrbuf));
	if (addr) {
		printf("Listening on %s:%d\n", addr, got_port);
		evutil_snprintf(uri_root, sizeof(uri_root),
		    "http://%s:%d",addr,got_port);
	} else {
		fprintf(stderr, "evutil_inet_ntop failed\n");
		return 1;
	}

	return 0;
}

int
main(int argc, char **argv)
{

	struct event_base *base = NULL;
	struct evhttp *http = NULL;
	struct evhttp_bound_socket *handle = NULL;
	struct evconnlistener *lev = NULL;
	struct event *term = NULL;
	int ret = 0;

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ret = 1;
		goto err;
	}

	//event_base
	base = event_base_new();

	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		ret = 1;
	}

	/* 创建一个新的evhttp对象来处理请求。 */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		ret = 1;
	}

	/* / test URI将所有请求转储到stdout并说200 OK。 */
	evhttp_set_cb(http, "/test", dump_request_cb, NULL);

	/*要接受任意请求，需要设置一个“通用”cb。 还可以为特定路径添加回调。 */
	evhttp_set_gencb(http, send_document_cb, NULL);

	//绑定socket
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", 8888);

	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n", 8888);
		ret = 1;
		goto err;
	}
	
	//监听socket
	if (display_listen_sock(handle)) {
		ret = 1;
		goto err;
	}

	//终止信号
	term = evsignal_new(base, SIGINT, do_term, base);
	if (!term)
		goto err;
	if (event_add(term, NULL))
		goto err;

	//事件分发
	event_base_dispatch(base);

err:

	if (http)
		evhttp_free(http);
	if (term)
		event_free(term);
	if (base)
		event_base_free(base);

	return ret;
}
