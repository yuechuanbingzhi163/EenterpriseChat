#ifndef _UDPMODEL_H_
#define _UDPMODEL_H_

#include <WinSock2.h>
#include <list>
#include "CommonData.h"

class CUDPModel
{
public:
	CUDPModel(void);
	~CUDPModel(void);
public:
	
private:
	std::list<IPSOCKET> m_listIPSocket;
	SOCKET m_UDPSocket;
public:
	//初始化UDP套接字
	bool Initialize(SOCKADDR_IN addr);
	//关闭UDP套接字
	bool Uninstall();
	//加入到一个多播组
	bool JoinMutliCast(SOCKADDR_IN addr);
	//离开一个多播组
	bool LeaveMutliCast(SOCKADDR_IN addr);
	//发送UDP信息
	bool SendUDPMessage(PER_IO_CONTEXT* pIoContext);
	//接受UDP信息
	bool RecvUDPMessage(PER_IO_CONTEXT* pIoContext);
	//绑定套接字到完成端口
	bool AssociateWithIOCP(HANDLE completionPort);
	//返回UDP套接字
	SOCKET GetUDPSocket() { return m_UDPSocket; }
};
#endif
