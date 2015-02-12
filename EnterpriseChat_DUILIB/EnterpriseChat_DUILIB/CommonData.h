#ifndef _COMMONDATA_H_
#define _COMMONDATA_H_

#include <WinSock2.h>
#include <list>
#include <UIlib.h>

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 100;

const TCHAR* const PCNAME=_T("NAME");
const TCHAR* const PCIMAGE=_T("IMAGE");
const TCHAR* const PCHOSTNAME=_T("HOSTNAME");

const TCHAR* const FRIENDNAME=_T("NAME");

const TCHAR* const GROUPNAME= _T("GROUPNAME");
const TCHAR* const GROUPIMAGE=_T("GROUPIMAGE");
const TCHAR* const GROUPBUILDER=_T("GROUPBUILDER");
const TCHAR* const GROUPMANAGER=_T("GROUPMANAGER");
const TCHAR* const GROUPMEMBER=_T("GROUPMEMBER");

typedef enum _TCP_HEAD_TYPE
{
	TCP_NONE,
	TCP_MSG,
	TCP_FILE
}TCP_HEAD_TYPE;

typedef enum _TCPMSGTYPE
{
	FILE_NONE,
	FILE_SEND,
	FILE_ACCEPT,
	FILE_STOP,
	FILE_FINISH
}TCPMSGTYPE;

typedef struct  _TCPDATA
{
	TCPMSGTYPE  m_type;
	int  m_fileLen;
	char m_fileName[MAX_PATH];
	_TCPDATA()
	{
		m_type=FILE_NONE;
		m_fileLen=0;
		memset(m_fileName,0,sizeof(m_fileName));
	}
}TCPDATA;

#pragma pack(push,1) //开始定义数据包, 采用字节对齐方式
/*----------------------包头---------------------*/
typedef struct tagPACKAGEHEAD
{
	TCP_HEAD_TYPE  m_command;
	DWORD          m_dataLen;//包体的长度
	tagPACKAGEHEAD()
	{
		m_command=TCP_HEAD_TYPE::TCP_NONE;
		m_dataLen=0;
	}
}PACKAGE_HEAD;
#pragma pack(pop) //结束定义数据包, 恢复原来对齐方式

#define PACKAGE_HEAD_LEN sizeof(PACKAGE_HEAD)
#define PACKAGE_DATA_LEN 8000

#define OWNINI_PATH  "D:\\EChatAcceptEx\\own.ini"
#define FRIINI_PATH	 "D:\\EChatAcceptEx\\friends.ini"
#define GROUPINI_PATH "D:\\EchatAcceptEx\\groups.ini"
#define MAX_ALLSECTIONS 2048  //全部的段名
#define MAX_SECTION 260  //一个段名长度

#define UDPMSG_LEN 500
#define EXIT_CODE NULL

//释放指针
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
// 释放句柄宏
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}

//UDP信息类别，分为上线，下线，普通信息，UDP穿透信息
typedef enum _UDPMSGTYPE
{
	LOGON, //上线
	LOGOFF, //下线
	COMMONPOINT, //普通点对点消息
	COMMONMULTICAST, //普通群消息
	SHAREFILE,   //共享文件消息
	NAT,    //UDP穿透消息
	MULTICAST_CREATE, //群创建（所有）
	MULTICAST_INVITE,	//邀请加入群消息（一对一）
	MULTICAST_BANISH,	//被踢出群消息（一对一）
	MULTICAST_JOIN,		//加入群消息（群组）
	MULTICAST_QUIT		//退出群消息（群组）
}UDPMSGTYPE,*PUDPMSGTYPE;
//UDP数据包
typedef struct _UDPDATA
{
	UDPMSGTYPE m_msgType;
	SOCKADDR_IN m_addr;
	char   m_image[MAX_PATH];
	char   m_hostName[MAX_PATH];
	char   m_name[MAX_PATH];
	char   m_fileName[MAX_PATH];
	char   m_message[UDPMSG_LEN];
	_UDPDATA()
	{
		m_msgType=UDPMSGTYPE::COMMONPOINT;
		memset(m_image,0,sizeof(m_image));
		memset(m_message,0,sizeof(m_message));
		memset(m_hostName,0,sizeof(m_hostName));
		memset(m_name,0,sizeof(m_name));
		memset(m_fileName,0,sizeof(m_fileName));
		memset(&m_addr,0,sizeof(m_addr));
	}
}UDPDATA,*PUDPDATA;
//多播地址与socket结构体
typedef struct _IPSOCKET
{
	char m_ip[20];
	SOCKET m_socket;
	_IPSOCKET()
	{
		memset(m_ip,0,sizeof(m_ip));
		m_socket=INVALID_SOCKET;
	}
}IPSOCKET,*PIPSOCKET;
//定义WSABUF缓冲区大小
#define MAX_BUFFER_LEN 1024*8
#define TCP_DATA_LEN   PACKAGE_HEAD_LEN+PACKAGE_DATA_LEN
//异步操作标识
typedef enum _OPERATION_TYPE  
{  
	BROADCASE_POST ,                   //标志投递的是广播操作
	ACCEPT_POSTED,                     // 标志投递的Accept操作
	SEND_POSTED,                       // 标志投递的是发送操作
	RECV_POSTED,                       // 标志投递的是接收操作
	NULL_POSTED                        // 用于初始化，无意义
}OPERATION_TYPE,*POPERATION_TYPE;
//套接字类型
typedef enum _SOCKET_TYPE
{
	TYPE_UDP,
	TYPE_TCP,
	NONE_TYPE
}SOCKET_TYPE,*PSOCKET_TYPE;
//异步通信结构用于发送还是接受枚举
typedef enum _PERIOCONTEXT_TYPE
{
	PER_IOCONTEXT_NULL,
	PER_IOCONTEXT_SEND,
	PER_IOCONTEXT_ACCEPT,
	PER_IOCONTEXT_LISTEN
}PERIOCONTEXT_TYPE;
//异步通信结构
typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)    
	SOCKET         m_socket;									// 这个网络操作所使用的Socket
	SOCKET_TYPE	   m_socketType;							   //套接字类型UDP/TCP
	OPERATION_TYPE m_OpType;									// 标识网络操作的类型(ACCEPT/RECV/SEND)
	WSABUF         m_wsaBuf;                                   // WSA类型的缓冲区，用于给重叠操作传参数的
	char           m_szBuffer[MAX_BUFFER_LEN];                 // 这个是WSABUF里具体存字符的缓冲区
	int            m_recvPos;
	bool		   m_bIsPackageHead;
	SOCKADDR_IN    m_senderAddr;

	// 初始化
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));  
		ZeroMemory( m_szBuffer,MAX_BUFFER_LEN );
		memset(&m_senderAddr,0,sizeof(m_senderAddr));
		m_socket = INVALID_SOCKET;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType     = NULL_POSTED;
		m_socketType=NONE_TYPE;
		m_bIsPackageHead=false;
		m_recvPos=0;
	}
	// 释放掉Socket
	~_PER_IO_CONTEXT()
	{
		if( m_socket!=INVALID_SOCKET )
		{
			shutdown(m_socket,SD_BOTH);
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}
	// 重置缓冲区内容
	void ResetBuffer()
	{
		memset( m_szBuffer,0,sizeof(m_szBuffer));
		m_recvPos=0;
	}
}PER_IO_CONTEXT,*PPER_IO_CONTEXT;
//监听套接字使用的数据结构
//typedef struct _PER_SOCKET_CONTEXT
//{  
//	SOCKET      m_socket;                                  // 每一个客户端连接的Socket
//	SOCKADDR_IN m_clientAddr;                              // 客户端的地址
//	std::list<PER_IO_CONTEXT*> m_listIoContext;             // 客户端网络操作的上下文数据，
//	// 也就是说对于每一个客户端Socket，是可以在上面同时投递多个IO请求的
//
//	// 初始化
//	_PER_SOCKET_CONTEXT()
//	{
//		m_socket = INVALID_SOCKET;
//		memset(&m_clientAddr, 0, sizeof(m_clientAddr)); 
//		m_listIoContext.clear();
//	}
//
//	// 释放资源
//	~_PER_SOCKET_CONTEXT()
//	{
//		if( m_socket!=INVALID_SOCKET )
//		{
//			shutdown(m_socket,SD_BOTH);
//			closesocket( m_socket );
//			m_socket = INVALID_SOCKET;
//		}
//		// 释放掉所有的IO上下文数据
//		for(std::list<PER_IO_CONTEXT*>::iterator ite=m_listIoContext.begin();ite!=m_listIoContext.end();++ite)
//		{
//			delete *ite;
//		}
//		m_listIoContext.clear();
//	}
//	// 获取一个新的IoContext
//	PER_IO_CONTEXT* GetNewIoContext()
//	{
//		PER_IO_CONTEXT* p = new PER_IO_CONTEXT;
//		m_listIoContext.push_back( p );
//		return p;
//	}
//	// 从数组中移除一个指定的IoContext
//	void RemoveContext( PER_IO_CONTEXT* pContext )
//	{
//		//assert( pContext!=NULL );
//
//		for(std::list<PER_IO_CONTEXT*>::iterator ite=m_listIoContext.begin();ite!=m_listIoContext.end();++ite)
//		{
//			if( pContext==*ite )
//			{
//				delete pContext;
//				pContext = NULL;
//				m_listIoContext.erase(ite);				
//				break;
//			}
//		}
//	}
//}PER_SOCKET_CONTEXT,*PPER_SOCKET_CONTEXT;
//udp队列消息结构
typedef struct _UDPSENDDATA
{
	SOCKADDR_IN m_addr;
	UDPDATA     m_UDPData;
}UDPSENDDATA,*PUDPSENDDATA;
//tcp队列消息结构
typedef struct _TCPSENDDATA
{
	SOCKET m_socket;
	char   m_msgBuff[TCP_DATA_LEN];
}TCPSENDDATA,*PTCPSENDDATA;
//好友信息结构
//typedef struct _FRIENDINFO
//{
//	char m_logo[MAX_PATH];
//	char m_name[MAX_PATH];
//	char m_ip[MAX_PATH];
//	_FRIENDINFO()
//	{
//		memset(m_logo,0,sizeof(m_logo));
//		memset(m_name,0,sizeof(m_name));
//		memset(m_ip,0,sizeof(m_ip));
//	}
//}FRIENDINFO;
//窗口类型，多播聊天窗口/点对点聊天窗口
typedef enum _WNDTYPE
{
	WND_POINT,
	WND_MUTLICAST
}WNDTYPE;
//文件操作类型
typedef enum _FILEOPERATIONTYPE
{
	FILE_OPE_SEND,
	FILE_OPE_ACCEPT
}FILEOPERATIONTYPE;
typedef struct _FILEINFO
{
	PER_IO_CONTEXT* m_pIoContext;
	char m_saveName[MAX_PATH];
	char m_realName[MAX_PATH];
	FILEOPERATIONTYPE m_type;
	int m_fileLen;
	int m_finishLen;
	_FILEINFO()
	{
		m_pIoContext=NULL;
		memset(m_realName,0,sizeof(m_realName));
		memset(m_saveName,0,sizeof(m_saveName));
		m_fileLen=0;
		m_finishLen=0;
	}
}FILEINFO;

typedef struct _FriendListItemInfo
{
	bool m_folder;
	bool m_empty;
	DuiLib::CDuiString m_id;
	DuiLib::CDuiString m_logo;
	DuiLib::CDuiString m_nick_name;
	DuiLib::CDuiString m_description;
}FriendListItemInfo;

typedef struct _GROUPINFO
{
	TCHAR m_groupIP[MAX_PATH];
	TCHAR m_groupName[MAX_PATH];
	TCHAR m_groupImage[MAX_PATH];
	TCHAR m_builder[MAX_PATH];
	int     m_isManager;
	int     m_isMember;
	_GROUPINFO()
	{
		memset(m_groupIP,0,sizeof(m_groupIP));
		memset(m_groupName,0,sizeof(m_groupName));
		memset(m_builder,0,sizeof(m_builder));
		memset(m_groupImage,0,sizeof(m_groupImage));
		m_isManager=0;
		m_isMember=0;
	}
}GROUPINFO;
#endif
