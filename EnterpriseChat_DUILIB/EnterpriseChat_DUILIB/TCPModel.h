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
	LPFN_ACCEPTEX					m_lpfnAcceptEx;					//AcceptEx ����ָ��
public:
	LPFN_GETACCEPTEXSOCKADDRS		m_lpfnGetAcceptExSockAddrs;		//GetAcceptExSockaddrs �ĺ���ָ��
public:
	//��ʼ��TCP�׽���
	bool Initialize(SOCKADDR_IN addr);
	//�ر�UDP�׽���
	bool Uninstall();
	//���׽��ֵ���ɶ˿�
	bool AssociateWithIOCP(HANDLE completionPort);
	//Ͷ��AcceptEx
	bool AcceptTCPConnect(PER_IO_CONTEXT* pIoContext);
};
#endif
