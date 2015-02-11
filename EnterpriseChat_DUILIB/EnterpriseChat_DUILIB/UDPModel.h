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
	//��ʼ��UDP�׽���
	bool Initialize(SOCKADDR_IN addr);
	//�ر�UDP�׽���
	bool Uninstall();
	//���뵽һ���ಥ��
	bool JoinMutliCast(SOCKADDR_IN addr);
	//�뿪һ���ಥ��
	bool LeaveMutliCast(SOCKADDR_IN addr);
	//����UDP��Ϣ
	bool SendUDPMessage(PER_IO_CONTEXT* pIoContext);
	//����UDP��Ϣ
	bool RecvUDPMessage(PER_IO_CONTEXT* pIoContext);
	//���׽��ֵ���ɶ˿�
	bool AssociateWithIOCP(HANDLE completionPort);
	//����UDP�׽���
	SOCKET GetUDPSocket() { return m_UDPSocket; }
};
#endif
