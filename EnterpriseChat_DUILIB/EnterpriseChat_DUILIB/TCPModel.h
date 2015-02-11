#ifndef _TCPMODEL_H_
#define _TCPMODEL_H_

#include <WinSock2.h>
#include "CommonData.h"
#include <MSWSock.h>

class CTCPModel
{
public:
	CTCPModel(void);
	~CTCPModel(void);
private:
	SOCKET m_TCPSocket;
	LPFN_ACCEPTEX					m_lpfnAcceptEx;					//AcceptEx 函数指针
public:
	LPFN_GETACCEPTEXSOCKADDRS		m_lpfnGetAcceptExSockAddrs;		//GetAcceptExSockaddrs 的函数指针
public:
	//初始化TCP套接字
	bool Initialize(SOCKADDR_IN addr);
	//关闭UDP套接字
	bool Uninstall();
	//绑定套接字到完成端口
	bool AssociateWithIOCP(HANDLE completionPort);
	//投递AcceptEx
	bool AcceptTCPConnect(PER_IO_CONTEXT* pIoContext);
};
#endif
