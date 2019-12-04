/*********************************************************************

**********************************************************************/
#ifndef _RTE_H_
#define _RTE_H_

/***********************************************************
                   ������������ѡ��                     
***********************************************************/

/***********************************************************
 *                   ��׼���Ǳ�׼ͷ�ļ�                    *
***********************************************************/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <semaphore.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
 *                        ��������                         *
***********************************************************/

/***********************************************************
 *                       ȫ�ֺ�                            *
***********************************************************/

#if	(!defined(TRUE) || (TRUE!=1))
#undef TRUE
#define TRUE    1
#endif

#if	(!defined(FALSE) || (FALSE!=0))
#undef FALSE
#define FALSE   0
#endif

#if	(!defined(YES) || (YES!=1))
#undef YES
#define YES     (BYTE)1
#endif

#if	(!defined(NO) || (NO!=0))
#undef NO
#define NO      (BYTE)0
#endif

#ifndef WAIT_FOREVER
# define WAIT_FOREVER ((WORD32)0xFFFFFFFF)
#endif  /* WAIT_FOREVER */

#ifndef NO_WAIT
# define NO_WAIT ((WORD32)0)
#endif  /* NO_WAIT */

#ifndef NULL
# define NULL ((void *)0)
#endif  /* NULL */

#ifndef LEOF
#define LEOF ((WORD32)(0xFFFFFFFF))
#endif

#ifndef WEOF
#define WEOF ((WORD16)(0xFFFF))
#endif

#ifndef NONE
#define NONE	(-1)
#endif

#ifndef EOS
#define EOS		'\0'
#endif

#define FAST	register
#define IMPORT	extern
#define LOCAL	static
/***********************************************************
 *                     ȫ����������                        *
***********************************************************/
typedef unsigned short      WORD16;
typedef unsigned short      WORD;
typedef signed short        SWORD16;
typedef unsigned int        WORD32;
typedef signed int          SWORD32;
typedef unsigned long long  WORD64;
typedef long long           SWORD64;
typedef unsigned int        DWORD;
typedef signed   long int   SWORDPTR;  /* ��ָ���С��ͬ��WORD����, 32λ����64λ */
typedef unsigned long int   WORDPTR;   /* ��ָ���С��ͬ��WORD����, 32λ����64λ */
typedef unsigned long int   WORDTIME;     /* ��ʾʱ�� */
typedef WORD32              YDS_TIMER_ID;
typedef WORDPTR             TIMER_T;
typedef size_t              SIZE_T;
typedef long int            SSIZE_T;

typedef int              INT;
typedef char             CHAR;
typedef unsigned char    BYTE;
typedef signed   char    SBYTE;
typedef unsigned char    BOOLEAN;
typedef unsigned char    T_PAD;

/* ����cplusplus��VOID����ֱ��������ʹ��void */
#ifndef __cplusplus
typedef void             VOID;
#else
#define VOID             void
#endif

typedef void*            LPVOID;
typedef int              SOCKET;
typedef fd_set           SOCKET_SET;
typedef WORD32          (*FUNCPTR)();
typedef int              STATUS;
typedef VOID (*TaskEntryProto)(VOID *);
typedef	signed char		    INT8;
typedef	signed short		    INT16;
typedef	signed int		        INT32;
typedef	long long	    INT64;
typedef	unsigned char	UINT8;
typedef	unsigned short	UINT16;
typedef	unsigned int	UINT32;
typedef	unsigned long long UINT64;
typedef	unsigned char	UCHAR;
typedef unsigned short	USHORT;
typedef	unsigned int	UINT;
typedef unsigned long	ULONG;

/* ֧�ָ����ӡ���� */
typedef double      DOUBLE;
typedef float       FLOAT; 
typedef long double LDOUBLE;

typedef	int		BOOL;
#define OK		0
#define ERROR		(-1)

typedef pid_t   PID_T;
typedef WORD32  YDS_STATUS;    /**< �������״̬����,ʾ����YDS_SUCCESS */
typedef sem_t              YDS_SEM_T;  /**< �ź����������� */
typedef pthread_t          YDS_TASK_T; /**< �����TaskID�������� */

typedef struct sockaddr_in      SOCKADDR_IN;  /**< inet�׽��ֵ�ַ�ṹ */
typedef struct sockaddr         SOCKADDR;     /**< sockaddr�׽��ֵ�ַ�ṹ */
typedef struct msghdr           T_SockMsgHdr; /**< msghdr�ṹ */
typedef socklen_t               SOCKLEN_T;    /**< ֧�Ŷ����socklen_t�ṹ */
typedef int                     SOCK_STATUS;  /**< socket�������״̬ */

#define INVALID_SYS_TASKID          (YDS_TASK_T)0       /**< ��Ч��ϵͳ����ID */



#define ydsErrCode_t  INT32

#define   LOG_LEVEL_DEBUG    1
#define   LOG_LEVEL_INFO     2
#define   LOG_LEVEL_WARN   3
#define   LOG_LEVEL_ERROR   4


/***********************************************************
 *                     ȫ�ֺ���ԭ��                        *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of _RTE_H_ */

