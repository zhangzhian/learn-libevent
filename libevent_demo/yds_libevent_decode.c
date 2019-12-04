#include "yds_libevent_decode.h"

int tcp_decode_hz_protobuf(struct evbuffer *buff, UINT8 **data){
	
	int size = -1;
	const char *header = "#START*";
	const char *end = "#END*";
	struct evbuffer_ptr p;
			
	p = evbuffer_search(buff, header, strlen(header), NULL);
	//printf("[Libevent]p.pos1:%d\n", p.pos);	

	if (p.pos < 0)
		return size;
		//break;

	//清楚无效数据
	evbuffer_drain(buff,p.pos);
	//读取长度
	UINT8 u8size[2] = {0};
	evbuffer_ptr_set(buff, &p, strlen(header), EVBUFFER_PTR_SET);
	evbuffer_copyout_from(buff, &p, u8size, sizeof(u8size));
	UINT16 u16size = (UINT16)((UINT16)u8size[0] << 8) + (UINT16)u8size[1];
			
	//printf(" u16size:%d \n",u16size);
	p = evbuffer_search(buff, end, strlen(end), NULL);

	if (p.pos < 0)
	{
		evbuffer_drain(buff,evbuffer_get_length(buff));
	}else if( p.pos != u16size - strlen(end)){
		//不完整数据
		evbuffer_drain(buff,sizeof(header));
		size = 0;	
	}else
	{
		//完整数据
		//取数据
		*data = (UINT8 *)malloc(u16size);
		evbuffer_remove(buff,*data,u16size);		
		size = u16size;
	}

	return size;

}