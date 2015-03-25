#include "UDPModel.h"
#include <ws2ipdef.h>
#include <string>
//224.0.2.0～238.255.255.255  临时多播地址
using namespace std;

CUDPModel::CUDPModel(void):
	m_UDPSocket(INVALID_SOCKET)
{
	m_listIPSocket.clear();
}

CUDPModel::~CUDPModel(void)
{
}

//初始化UDP，完成sokcet新建，绑定地址
bool CUDPModel::Initialize(SOCKADDR_IN addr)
{
	//套接字须支持多播,由于广播会对网络造成较大负担，因此，在上线和下线时考虑发送多播数据包而非广播数据包
	m_UDPSocket=WSASocket(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,WSA_FLAG_OVERLAPPED|WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF);
	if(INVALID_SOCKET==m_UDPSocket)
	{
		return false;
	}
	//设置套接字支持广播
	bool fBroadcast=true;
	int ret=setsockopt(m_UDPSocket,SOL_SOCKET,SO_BROADCAST,(CHAR *)&fBroadcast,sizeof(bool));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//设置端口可重用
	bool bFlag=true;
	ret=setsockopt(m_UDPSocket,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//设置套接字信息传播范围
	int nIPTTL=16;
	DWORD backByte=0;
	ret=WSAIoctl(m_UDPSocket,SIO_MULTICAST_SCOPE,&nIPTTL,sizeof(nIPTTL),NULL,0,&backByte,NULL,NULL);
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//关闭多播回环
	bool val=false;
	ret=WSAIoctl(m_UDPSocket,SIO_MULTIPOINT_LOOPBACK,&val,sizeof(val),NULL,0,&backByte,NULL,NULL);
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	//绑定地址
	ret=bind(m_UDPSocket,(SOCKADDR*)&addr,sizeof(addr));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_UDPSocket);
		return false;
	}
	return true;
}
//加入一个多播组
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
//离开一个多播组，关闭相应多播组socket
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
//发送UDP信息到指定ip地址
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
//接受UDP信息
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
//卸载UDP模块，清除UDP相关资源
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
//将套接字绑定到完成端口
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