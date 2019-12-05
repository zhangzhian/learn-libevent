/** ###################################################################
**     Filename  : yds_libevent_decode.h
**     Project   : 
**     Processor : ARM7
**     Component : 
**     Version   : 
**     Compiler  : 
**     Date/Time : 2019/12/03, 20:10
**     Abstract  :
**         This is libevent decode utils 
**     Settings  :
**
**     Contents  :
**
**     Copyright : 1997 - 2010 Freescale Semiconductor, Inc. All Rights Reserved.
**
** ###################################################################*/

/** ===========================include=============================== **/
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h> 
#include "yds_rte.h"
/** ===========================define================================ **/

typedef enum
{
    TCP_DATA_NORAML,				//normal
	TCP_DATA_DOUBLING,				//Doubling
}YDS_TCP_DATA_TYPE_E;

typedef enum
{
    TCP_SIZE_BIGBYTE,				//BigByte
	TCP_SIZE_LITTLEBYTE,			//LittleByte
}YDS_TCP_SIZE_MODULE_E;

typedef enum
{
    TCP_SIZE_ALL,				
	TCP_SIZE_ONLY_DATA,	
}YDS_TCP_SIZE_TYPE_E;

typedef struct
{
	const char				*header;
	const char				*end;
	UINT8					sizeLength;	//data size length 1,2,...
	UINT16					sizeIndex;	//data size index ,form protocol start
	YDS_TCP_SIZE_MODULE_E	sizeModule;	//big-endian or little-endian
	YDS_TCP_SIZE_TYPE_E		sizeType;
	UINT16					otherLength;//not include sizeLength, headerLength, endLength and dataLength 
	YDS_TCP_DATA_TYPE_E		type;
	
} evUniversalForamt;	

//TEST
//int tcp_decode_hz_protobuf(struct evbuffer *buff, UINT8 **data);
/** ==========================func================================ **/

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
int tcp_decode_universal(struct evbuffer *buff, UINT8 **data, evUniversalForamt format);

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
int tcp_decode_all(struct evbuffer *buff, UINT8 **data);
