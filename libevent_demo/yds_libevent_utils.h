/** ###################################################################
**     Filename  : yds_libevent_utils.h
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
#include <event2/bufferevent.h>
#include <event2/event.h>
#include "yds_rte.h"

/** ===========================include=============================== **/

/** ===========================define================================ **/

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


typedef enum
{
    TIMER_TIMEOUT,				//wait ... exec ... wait
    TIMER_NOW_TIMEOUT,			//exec ... wait ... exec
}YDS_TIMER_TYPE_E;

typedef enum
{
    CONN_SUCCESS,				//connection succeeded
    CONN_CLOSE,					//connection closed
	CONN_ERROR,					//connection error
}YDS_CONN_RESULT_E;

//typedef enum
//{
//	FORMAT_NANO,				//nano
//    FORMAT_NORMAL,				//normal
//	FORMAT_HZ_PROTOBUF,			//hz protobuf 
//}YDS_TCP_DATA_FORMAT_E;

typedef enum
{
    TCP_CLIENT,				//clent
	TCP_SERVER,				//server
}YDS_TCP_TYPE_E;

typedef void (*tcp_recv_data_cb)(struct bufferevent *bev, UINT8 *data, int length);

typedef struct
{
	tcp_recv_data_cb		readcb;
	bufferevent_data_cb		writecb;
	bufferevent_event_cb	eventcb; //YDS_CONN_RESULT_E

} bufferEvCB;

typedef int (*tcp_decode)(struct evbuffer *buff, UINT8 **data);

//typedef struct
//{
//	YDS_TCP_DATA_FORMAT_E type;	
//	int allLength;
//	const char* header;	
//
//} evTcpDataFormat;

typedef struct
{
	YDS_TCP_TYPE_E	tcp;
	//evTcpDataFormat* format;
	tcp_decode		decode;
	bufferEvCB		*cb;

} evTcpContext;

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
int yds_libevent_addTimerEvent( YDS_TIMER_TYPE_E eFlag, 
								struct timeval delayTime, 
								void (*cb)(evutil_socket_t, short, void *));

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
int yds_libevent_addTcpServerEvent( int port,
									//evTcpDataFormat format,
									tcp_decode decode,
									bufferEvCB cb);

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
struct bufferevent* yds_libevent_addTcpClientEvent( const char* ip,
													int port,
													//evTcpDataFormat format,
													tcp_decode decode,
													bufferEvCB cb);


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
struct event* yds_libevent_addManualActiveEvent(void (*cb)(evutil_socket_t, short, void *));

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

int yds_libevent_removeEvent(struct event *ev);
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
//void yds_libevent_getBase(struct event_base **ev);

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