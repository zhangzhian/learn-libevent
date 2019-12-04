#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
 
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
 
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
 
#define MYHTTPD_SIGNATURE   "TBoxHttpd v0.1"

unsigned short serverPort = 8888;

void die_most_horribly_from_openssl_error (const char *func)
{ 
	fprintf (stderr, "%s failed:\n", func);

	/* This is the OpenSSL function that prints the contents of the
	* error stack to the specified file handle. */
	ERR_print_errors_fp (stderr);

	exit (EXIT_FAILURE);
}

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
static void
login_cb (struct evhttp_request *req, void *arg)
{ 
    struct evbuffer *evb = NULL;
    const char *uri = evhttp_request_get_uri (req);
    struct evhttp_uri *decoded = NULL;
 
    /* 判断 req 是否是GET 请求 */
    if (evhttp_request_get_command (req) == EVHTTP_REQ_GET)
    {
        struct evbuffer *buf = evbuffer_new();
        if (buf == NULL) return;
        evbuffer_add_printf(buf, "Requested: %s\n", uri);
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        return;
    }
 
    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }
 
    printf ("Got a POST request for <%s>\n", uri);
 
    //判断此URI是否合法
    decoded = evhttp_uri_parse (uri);
    if (! decoded)
    { 
        printf ("It's not a good URI. Sending BADREQUEST\n");
        evhttp_send_error (req, HTTP_BADREQUEST, 0);
        return;
    }
 
    /* Decode the payload */
    struct evbuffer *buf = evhttp_request_get_input_buffer (req);
    evbuffer_add (buf, "", 1);    /* NUL-terminate the buffer */
    char *payload = (char *) evbuffer_pullup (buf, -1);
    int post_data_len = evbuffer_get_length(buf);
    char request_data_buf[4096] = {0};
    memcpy(request_data_buf, payload, post_data_len);
    printf("[post_data][%d]=\n %s\n", post_data_len, payload);
 
    /* This holds the content we're sending. */

    //HTTP header

    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");
 
    evb = evbuffer_new ();
	evbuffer_add_printf(evb, "%s", "123123");
    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", evb);
 
    if (decoded)
        evhttp_uri_free (decoded);
    if (evb)
        evbuffer_free (evb);
 
    printf("[response]:\n");

}
 
/**
 * This callback is responsible for creating a new SSL connection
 * and wrapping it in an OpenSSL bufferevent.  This is the way
 * we implement an https server instead of a plain old http server.
 */
static struct bufferevent* bevcb(struct event_base *base, void *arg)
{ 
    struct bufferevent* r;
    SSL_CTX *ctx = (SSL_CTX *) arg;
 
    r = bufferevent_openssl_socket_new (base,
            -1,
            SSL_new (ctx),
            BUFFEREVENT_SSL_ACCEPTING,
            BEV_OPT_CLOSE_ON_FREE);
    return r;
}
 
static void server_setup_certs (SSL_CTX *ctx,
        const char *certificate_chain,
        const char *private_key)
{ 
    printf ("Loading certificate chain from '%s'\n"
            "and private key from '%s'\n",
            certificate_chain, private_key);
 
    if (1 != SSL_CTX_use_certificate_chain_file (ctx, certificate_chain))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_certificate_chain_file");
 
    if (1 != SSL_CTX_use_PrivateKey_file (ctx, private_key, SSL_FILETYPE_PEM))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_PrivateKey_file");
 
    if (1 != SSL_CTX_check_private_key (ctx))
        die_most_horribly_from_openssl_error ("SSL_CTX_check_private_key");
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
	} else {
		fprintf(stderr, "evutil_inet_ntop failed\n");
		return 1;
	}

	return 0;
}


static int serve_some_http (void)
{ 
    struct event_base *base;
    struct evhttp *http;
    struct evhttp_bound_socket *handle;
 
	//创建event_base
    base = event_base_new ();
    if (! base)
    { 
        fprintf (stderr, "Couldn't create an event_base: exiting\n");
        return 1;
    }
 
    /* 创建一个 evhttp 句柄，去处理用户端的requests请求 */
    http = evhttp_new (base);
    if (! http)
    {
		fprintf (stderr, "couldn't create evhttp. Exiting.\n");
        return 1;
    }
	/******************************************/
    /* 创建SSL上下文环境 ，可以理解为 SSL句柄 */
    SSL_CTX *ctx = SSL_CTX_new (SSLv23_server_method ());
    SSL_CTX_set_options (ctx,
            SSL_OP_SINGLE_DH_USE |
            SSL_OP_SINGLE_ECDH_USE |
            SSL_OP_NO_SSLv2);
 
    /* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
     * We just hardcode a single curve which is reasonably decent.
     * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
    EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
    if (! ecdh)
        die_most_horribly_from_openssl_error ("EC_KEY_new_by_curve_name");
    if (1 != SSL_CTX_set_tmp_ecdh (ctx, ecdh))
        die_most_horribly_from_openssl_error ("SSL_CTX_set_tmp_ecdh");
 
    /* 选择服务器证书 和 服务器私钥. */
    const char *certificate_chain = "server-certificate-chain.pem";
    const char *private_key = "server-private-key.pem";
    /* 设置服务器证书 和 服务器私钥 到 
     OPENSSL ctx上下文句柄中 */
    server_setup_certs (ctx, certificate_chain, private_key);
	
    /* 
        使我们创建好的evhttp句柄 支持 SSL加密
        实际上，加密的动作和解密的动作都已经帮
        我们自动完成，我们拿到的数据就已经解密之后的

		设置用于为与给定evhttp对象的连接创建新的bufferevent的回调。
		您可以使用它来覆盖默认的bufferevent类型，
		例如，使此evhttp对象使用SSL缓冲区事件而不是未加密的事件。
		新的缓冲区事件必须在未设置fd的情况下进行分配。
	*/
    evhttp_set_bevcb (http, bevcb, ctx);
	/******************************************/
    /* 设置http回调函数 */
    //默认回调
    //evhttp_set_gencb (http, send_document_cb, NULL);
    //专属uri路径回调
    evhttp_set_cb(http, "/login", login_cb, NULL);
 
    /* 设置监听IP和端口 */
    handle = evhttp_bind_socket_with_handle (http, "0.0.0.0", serverPort);
    if (! handle)
    { 
        fprintf (stderr, "couldn't bind to port %d. Exiting.\n",(int) serverPort);
        return 1;
    }

 	//监听socket
	if (display_listen_sock(handle)) {
		return 1;
	}
 
    /* 开始阻塞监听 (永久执行) */
    event_base_dispatch (base);
 
 
    return 0;
}
 
int main (int argc, char **argv)
{ 
    /*OpenSSL 初始化 */
	signal (SIGPIPE, SIG_IGN);

	SSL_library_init ();
	SSL_load_error_strings ();
	OpenSSL_add_all_algorithms ();

	printf ("Using OpenSSL version \"%s\"\nand libevent version \"%s\"\n",
			SSLeay_version (SSLEAY_VERSION),
			event_get_version ());
    /* now run http server (never returns) */
    return serve_some_http ();
}
