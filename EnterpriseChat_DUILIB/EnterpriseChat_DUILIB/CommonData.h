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

#pragma pack(push,1) //��ʼ�������ݰ�, �����ֽڶ��뷽ʽ
/*----------------------��ͷ---------------------*/
typedef struct tagPACKAGEHEAD
{
	TCP_HEAD_TYPE  m_command;
	DWORD          m_dataLen;//����ĳ���
	tagPACKAGEHEAD()
	{
		m_command=TCP_HEAD_TYPE::TCP_NONE;
		m_dataLen=0;
	}
}PACKAGE_HEAD;
#pragma pack(pop) //�����������ݰ�, �ָ�ԭ�����뷽ʽ

#define PACKAGE_HEAD_LEN sizeof(PACKAGE_HEAD)
#define PACKAGE_DATA_LEN 8000

#define OWNINI_PATH  "D:\\EChatAcceptEx\\own.ini"
#define FRIINI_PATH	 "D:\\EChatAcceptEx\\friends.ini"
#define GROUPINI_PATH "D:\\EchatAcceptEx\\groups.ini"
#define MAX_ALLSECTIONS 2048  //ȫ���Ķ���
#define MAX_SECTION 260  //һ����������

#define UDPMSG_LEN 500
#define EXIT_CODE NULL

//�ͷ�ָ��
#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}
// �ͷž����
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}

//UDP��Ϣ��𣬷�Ϊ���ߣ����ߣ���ͨ��Ϣ��UDP��͸��Ϣ
typedef enum _UDPMSGTYPE
{
	LOGON, //����
	LOGOFF, //����
	COMMONPOINT, //��ͨ��Ե���Ϣ
	COMMONMULTICAST, //��ͨȺ��Ϣ
	SHAREFILE,   //�����ļ���Ϣ
	NAT,    //UDP��͸��Ϣ
	MULTICAST_CREATE, //Ⱥ���������У�
	MULTICAST_INVITE,	//�������Ⱥ��Ϣ��һ��һ��
	MULTICAST_BANISH,	//���߳�Ⱥ��Ϣ��һ��һ��
	MULTICAST_JOIN,		//����Ⱥ��Ϣ��Ⱥ�飩
	MULTICAST_QUIT		//�˳�Ⱥ��Ϣ��Ⱥ�飩
}UDPMSGTYPE,*PUDPMSGTYPE;
//UDP���ݰ�
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
//�ಥ��ַ��socket�ṹ��
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
//����WSABUF��������С
#define MAX_BUFFER_LEN 1024*8
#define TCP_DATA_LEN   PACKAGE_HEAD_LEN+PACKAGE_DATA_LEN
//�첽������ʶ
typedef enum _OPERATION_TYPE  
{  
	BROADCASE_POST ,                   //��־Ͷ�ݵ��ǹ㲥����
	ACCEPT_POSTED,                     // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,                       // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,                       // ��־Ͷ�ݵ��ǽ��ղ���
	NULL_POSTED                        // ���ڳ�ʼ����������
}OPERATION_TYPE,*POPERATION_TYPE;
//�׽�������
typedef enum _SOCKET_TYPE
{
	TYPE_UDP,
	TYPE_TCP,
	NONE_TYPE
}SOCKET_TYPE,*PSOCKET_TYPE;
//�첽ͨ�Žṹ���ڷ��ͻ��ǽ���ö��
typedef enum _PERIOCONTEXT_TYPE
{
	PER_IOCONTEXT_NULL,
	PER_IOCONTEXT_SEND,
	PER_IOCONTEXT_ACCEPT,
	PER_IOCONTEXT_LISTEN
}PERIOCONTEXT_TYPE;
//�첽ͨ�Žṹ
typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)    
	SOCKET         m_socket;									// ������������ʹ�õ�Socket
	SOCKET_TYPE	   m_socketType;							   //�׽�������UDP/TCP
	OPERATION_TYPE m_OpType;									// ��ʶ�������������(ACCEPT/RECV/SEND)
	WSABUF         m_wsaBuf;                                   // WSA���͵Ļ����������ڸ��ص�������������
	char           m_szBuffer[MAX_BUFFER_LEN];                 // �����WSABUF�������ַ��Ļ�����
	int            m_recvPos;
	bool		   m_bIsPackageHead;
	SOCKADDR_IN    m_senderAddr;

	// ��ʼ��
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
	// �ͷŵ�Socket
	~_PER_IO_CONTEXT()
	{
		if( m_socket!=INVALID_SOCKET )
		{
			shutdown(m_socket,SD_BOTH);
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}
	// ���û���������
	void ResetBuffer()
	{
		memset( m_szBuffer,0,sizeof(m_szBuffer));
		m_recvPos=0;
	}
}PER_IO_CONTEXT,*PPER_IO_CONTEXT;
//�����׽���ʹ�õ����ݽṹ
//typedef struct _PER_SOCKET_CONTEXT
//{  
//	SOCKET      m_socket;                                  // ÿһ���ͻ������ӵ�Socket
//	SOCKADDR_IN m_clientAddr;                              // �ͻ��˵ĵ�ַ
//	std::list<PER_IO_CONTEXT*> m_listIoContext;             // �ͻ���������������������ݣ�
//	// Ҳ����˵����ÿһ���ͻ���Socket���ǿ���������ͬʱͶ�ݶ��IO�����
//
//	// ��ʼ��
//	_PER_SOCKET_CONTEXT()
//	{
//		m_socket = INVALID_SOCKET;
//		memset(&m_clientAddr, 0, sizeof(m_clientAddr)); 
//		m_listIoContext.clear();
//	}
//
//	// �ͷ���Դ
//	~_PER_SOCKET_CONTEXT()
//	{
//		if( m_socket!=INVALID_SOCKET )
//		{
//			shutdown(m_socket,SD_BOTH);
//			closesocket( m_socket );
//			m_socket = INVALID_SOCKET;
//		}
//		// �ͷŵ����е�IO����������
//		for(std::list<PER_IO_CONTEXT*>::iterator ite=m_listIoContext.begin();ite!=m_listIoContext.end();++ite)
//		{
//			delete *ite;
//		}
//		m_listIoContext.clear();
//	}
//	// ��ȡһ���µ�IoContext
//	PER_IO_CONTEXT* GetNewIoContext()
//	{
//		PER_IO_CONTEXT* p = new PER_IO_CONTEXT;
//		m_listIoContext.push_back( p );
//		return p;
//	}
//	// ���������Ƴ�һ��ָ����IoContext
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
//udp������Ϣ�ṹ
typedef struct _UDPSENDDATA
{
	SOCKADDR_IN m_addr;
	UDPDATA     m_UDPData;
}UDPSENDDATA,*PUDPSENDDATA;
//tcp������Ϣ�ṹ
typedef struct _TCPSENDDATA
{
	SOCKET m_socket;
	char   m_msgBuff[TCP_DATA_LEN];
}TCPSENDDATA,*PTCPSENDDATA;
//������Ϣ�ṹ
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
//�������ͣ��ಥ���촰��/��Ե����촰��
typedef enum _WNDTYPE
{
	WND_POINT,
	WND_MUTLICAST
}WNDTYPE;
//�ļ���������
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
