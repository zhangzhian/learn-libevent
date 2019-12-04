#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h> 
#include "yds_rte.h"

int tcp_decode_hz_protobuf(struct evbuffer *input, UINT8 **data);