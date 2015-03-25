#include "UDPModel.h"
#include <ws2ipdef.h>
#include <string>
//224.0.2.0��238.255.255.255  ��ʱ�ಥ��ַ
using namespace std;

CUDPModel::CUDPModel(void):
	m_UDPSocket(INVALID_SOCKET)
{
	m_listIPSocket.clear();
}

CUDPModel::~CUDPModel(void)
{
}

//��ʼ��UDP�����sokcet�½����󶨵�ַ
bool CUDPModel::Initialize(SOCKADDR_IN addr)
{
	//�׽�����֧�ֶಥ,���ڹ㲥���������ɽϴ󸺵�����ˣ������ߺ�����ʱ���Ƿ��Ͷಥ���ݰ����ǹ㲥���ݰ�
	m_UDPSocket=WSASocket(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,WSA_FLAG_OVERLAPPED|WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF);
	if(INVALID_SOCKET==m_UDPSocket)
	{
		return false;
	}
	//�����׽���֧�ֹ㲥
	bool fBroadcast=true;
	int ret=setsockopt(m_UDPSocket,SOL_SOCKET,SO_BROADCAST,(CHAR *)&fBroadcast,sizeof(bool));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//���ö˿ڿ�����
	bool bFlag=true;
	ret=setsockopt(m_UDPSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//�����׽�����Ϣ������Χ
	int nIPTTL=16;
	DWORD backByte=0;
	ret=WSAIoctl(m_UDPSocket,SIO_MULTICAST_SCOPE,&nIPTTL,sizeof(nIPTTL),NULL,0,&backByte,NULL,NULL);
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//�رնಥ�ػ�
	bool val=false;
	ret=WSAIoctl(m_UDPSocket,SIO_MULTIPOINT_LOOPBACK,&val,sizeof(val),NULL,0,&backByte,NULL,NULL);
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//�󶨵�ַ
	ret=bind(m_UDPSocket,(SOCKADDR*)&addr,sizeof(addr));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	return true;
}
//����һ���ಥ��
bool CUDPModel::JoinMutliCast(SOCKADDR_IN addr)
{
	SOCKET logSocket=WSAJoinLeaf(m_UDPSocket,(SOCKADDR*)&addr,sizeof(addr),NULL,NULL,NULL,NULL,JL_BOTH);
	if(INVALID_SOCKET==logSocket)
	{
		return false;
	}
	IPSOCKET data;
	memset(&data,0,sizeof(data));
	memcpy(data.m_ip,inet_ntoa(addr.sin_addr),sizeof(inet_ntoa(addr.sin_addr)));
	data.m_socket=logSocket;
	m_listIPSocket.push_back(data);
	return true;
}
//�뿪һ���ಥ�飬�ر���Ӧ�ಥ��socket
bool CUDPModel::LeaveMutliCast(SOCKADDR_IN addr)
{
	IP_MREQ ipMerq;
	memset(&ipMerq,0,sizeof(ipMerq));
	ipMerq.imr_multiaddr.S_un.S_addr=addr.sin_addr.S_un.S_addr;
	ipMerq.imr_multiaddr.S_un.S_addr=inet_addr(INADDR_ANY);
	int ret=setsockopt(m_UDPSocket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char*)&ipMerq,sizeof(ipMerq));
	if(SOCKET_ERROR==ret)
	{
		return false;
	}
	list<IPSOCKET>::iterator ite=m_listIPSocket.begin();
	for(;ite!=m_listIPSocket.end();++ite)
	{
		if(string(ite->m_ip)==string(inet_ntoa(addr.sin_addr)))
		{
			break;
		}
	}
	if(ite==m_listIPSocket.end())
	{
		return false;
	}
	shutdown(ite->m_socket,SD_BOTH);
	closesocket(ite->m_socket);
	m_listIPSocket.erase(ite);
	return true;
}
//����UDP��Ϣ��ָ��ip��ַ
bool CUDPModel::SendUDPMessage(PER_IO_CONTEXT* pIoContext)
{
	DWORD dwBytes=0;
	DWORD flags=0;
	OVERLAPPED* p_ol=&pIoContext->m_Overlapped;
	pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
	pIoContext->m_bIsPackageHead=false;
	pIoContext->m_socket=m_UDPSocket;
	int ret=WSASendTo(m_UDPSocket,&pIoContext->m_wsaBuf,1,&dwBytes,flags,(SOCKADDR*)&pIoContext->m_senderAddr,sizeof(pIoContext->m_senderAddr),p_ol,NULL);
	if(SOCKET_ERROR==ret)
	{
		ret=WSAGetLastError();
		if(WSA_IO_PENDING==ret)
		{
			OutputDebugString(L"SendUDPMessage\r\n");
			return true;
		}
		else
		{
			return false;
		}
	}
	OutputDebugString(L"SendUDPMessage\r\n");
	return true;
}
//����UDP��Ϣ
bool CUDPModel::RecvUDPMessage(PER_IO_CONTEXT* pIoContext)
{
	int size=sizeof(SOCKADDR);
	DWORD dwBytes=0;
	DWORD flags=0;
	OVERLAPPED* p_ol=&pIoContext->m_Overlapped;
	pIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
	pIoContext->m_bIsPackageHead=false;
	pIoContext->m_socket=m_UDPSocket;
	pIoContext->ResetBuffer();
	int ret=WSARecvFrom(m_UDPSocket,&pIoContext->m_wsaBuf,1,&dwBytes,&flags,(SOCKADDR*)&pIoContext->m_senderAddr,&size,p_ol,NULL);
	if(SOCKET_ERROR==ret)
	{
		if(WSA_IO_PENDING==WSAGetLastError())
		{
			OutputDebugString(L"RecvUDPMsg\r\n");
			return true;
		}
		else
		{
			return false;
		}
	}
	OutputDebugString(L"RecvUDPMsg\r\n");
	return true;
}
//ж��UDPģ�飬���UDP�����Դ
bool CUDPModel::Uninstall()
{
	IP_MREQ ipMerq;
	memset(&ipMerq,0,sizeof(ipMerq));
	for(list<IPSOCKET>::iterator ite=m_listIPSocket.begin();ite!=m_listIPSocket.end();)
	{
		ipMerq.imr_multiaddr.S_un.S_addr=inet_addr(ite->m_ip);
		ipMerq.imr_multiaddr.S_un.S_addr=inet_addr(INADDR_ANY);
		int ret=setsockopt(m_UDPSocket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char*)&ipMerq,sizeof(ipMerq));
		if(SOCKET_ERROR==ret)
		{
			return false;
		}
		shutdown(ite->m_socket,SD_BOTH);
		closesocket(ite->m_socket);
		m_listIPSocket.erase(ite);
		ite=m_listIPSocket.begin();
	}
	shutdown(m_UDPSocket,SD_BOTH);
	closesocket(m_UDPSocket);
	return true;
}
//���׽��ְ󶨵���ɶ˿�
bool CUDPModel::AssociateWithIOCP(HANDLE completionPort)
{
	if(INVALID_HANDLE_VALUE==completionPort)
	{
		return false;
	}
	if(NULL==CreateIoCompletionPort((HANDLE)m_UDPSocket,completionPort,(ULONG_PTR)m_UDPSocket,0))
	{
		return false;
	}
	return true;
}