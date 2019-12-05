#include "yds_libevent_decode.h"

UINT8 hex_to_char(const UINT8 ch);

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

/*
** ===================================================================
**     Method      :  tcp_decode_all
**
**     Description :
**         tcp decode all data func
**     Parameters  : 
**                 : 
**                 : 
**                 : 
**     Returns     : None
** ===================================================================
*/
int tcp_decode_all(struct evbuffer *buff, UINT8 **data){
	
	int msize = evbuffer_get_length(buff);
	//完整数据
	//取数据
	*data = (UINT8 *)malloc(msize);
	evbuffer_remove(buff,*data,msize);		
	
	return msize;
	
}

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
int tcp_decode_universal(struct evbuffer *buff, UINT8 **data, evUniversalForamt format){
	int size = -1;
	struct evbuffer *preBuff;
	const char *header = NULL;
	const char *end = NULL;
	UINT8 header_size = 0;
	UINT8 end_size = 0;
	struct evbuffer_ptr p;
	UINT32 msize = 0;

	//相等，返回零		
	if (strcmp(format.header, "") != 0)
	{
		header = format.header;
		header_size = strlen(header);
	}else
	{
		evbuffer_drain(buff,evbuffer_get_length(buff));
		return size;
	}
	
	if (strcmp(format.end, "") != 0)
	{
		end = format.end;
		end_size = strlen(end);
	}

	if (format.sizeLength == 0 || format.sizeLength > 4 || format.sizeIndex - header_size < 0)
	{
		evbuffer_drain(buff,evbuffer_get_length(buff));
		return size;
	}

	if(format.type == TCP_DATA_DOUBLING)
	{
		//倍增， 处理数据
		int i;
		int buff_size = evbuffer_get_length(buff);
	
		UINT8* data_double = (UINT8 *)malloc(buff_size);

		evbuffer_copyout(buff,data_double,buff_size);	

		struct evbuffer *data_input = evbuffer_new();

		for (i = 0; i < buff_size; i = i+2)
		{
			UINT8 temp;

			temp = hex_to_char(data_double[i]) * 16 + hex_to_char(data_double[i+1]);
				
			evbuffer_add(data_input,&temp,1);
				
		}

		free(data_double);

		preBuff = buff;

		buff = data_input;
	}

	//printf("%d\n", evbuffer_get_length(buff));	
	p = evbuffer_search(buff, header, header_size, NULL);
	//printf("p.pos1:%d\n", p.pos);	

	if (p.pos < 0){
		if (format.type == TCP_DATA_DOUBLING)
		{
			evbuffer_drain(preBuff,evbuffer_get_length(preBuff));
		}else
		{
			evbuffer_drain(buff,evbuffer_get_length(buff));
		}
		return size;
	}		

	//清楚无效数据
	evbuffer_drain(buff,p.pos);
	if (format.type == TCP_DATA_DOUBLING)
	{
		evbuffer_drain(preBuff,p.pos*2);
	}

	//读取长度
	UINT8* u8size = (UINT8 *)malloc(format.sizeLength);
	evbuffer_ptr_set(buff, &p, format.sizeIndex, EVBUFFER_PTR_SET);

	evbuffer_copyout_from(buff, &p, u8size, format.sizeLength);
	int i ;
	for (i = 0; i < format.sizeLength; i++)
	{
		if (format.sizeModule == TCP_SIZE_BIGBYTE)
		{
			msize += (UINT32)((UINT32)u8size[i] << (8 * (format.sizeLength -1 - i)));
		}else{
			msize += (UINT32)((UINT32)u8size[i] << (8 * (i)));
		}
	}

	free(u8size);
	//处理msize，获取真实的整包数据大小
	if (format.sizeType == TCP_SIZE_ONLY_DATA)
	{
		msize = msize + header_size + end_size + format.sizeLength +format.otherLength;
	}

	//printf("msize:%d \n",msize);
	//无end
	if (end == NULL)
	{
		//完整数据
		//取数据
		*data = (UINT8 *)malloc(msize);
		evbuffer_remove(buff,*data,msize);
		if (format.type == TCP_DATA_DOUBLING)
		{
			evbuffer_drain(preBuff,msize*2);
		}
		size = msize;
	}else
	{
		p = evbuffer_search(buff, end, end_size, NULL);

		if (p.pos < 0)
		{
			evbuffer_drain(buff,evbuffer_get_length(buff));
			if (format.type == TCP_DATA_DOUBLING)
			{
				evbuffer_drain(preBuff,evbuffer_get_length(preBuff));
			}
		}else if( p.pos != msize - end_size){
			//不完整数据
			evbuffer_drain(buff,header_size);			
			if (format.type == TCP_DATA_DOUBLING)
			{
				evbuffer_drain(preBuff,header_size*2);
			}
			size = 0;
		}else
		{
			//完整数据
			//取数据
			*data = (UINT8 *)malloc(msize);
			evbuffer_remove(buff,*data,msize);		
			size = msize;
			if (format.type == TCP_DATA_DOUBLING)
			{
				evbuffer_drain(preBuff,msize*2);
			}
		}
	}
	
	return size;
}

/*把16进制转换为ASCII字符 */
UINT8 hex_to_char(const UINT8 ch)
{
	UINT8 value = 0;

	if(ch >= 0x30 && ch <=0x39)
	{
		value = ch - 0x30;
	}
	else if(ch >= 0x41 && ch <= 0x46)//大写字母
	{
		value = ch - 0x37;
	}
	else if(ch >= 0x61 && ch <= 0x66)//小写字母
	{
		value = ch - 0x57;
	}
	return value;
}