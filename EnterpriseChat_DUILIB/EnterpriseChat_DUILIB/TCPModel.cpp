#include "TCPModel.h"



CTCPModel::CTCPModel(void):
	m_TCPSocket(INVALID_SOCKET),
	m_lpfnAcceptEx(NULL),
	m_lpfnGetAcceptExSockAddrs(NULL)
{
}


CTCPModel::~CTCPModel(void)
{
}

//初始化TCP套接字
bool CTCPModel::Initialize(SOCKADDR_IN addr)
{
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	DWORD dwBytes = 0;  

	m_TCPSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET==m_TCPSocket)
	{
		return false;
	}
	int ret=bind(m_TCPSocket,(SOCKADDR*)&addr,sizeof(addr));
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_TCPSocket);
		return false;
	}
	ret=listen(m_TCPSocket,SOMAXCONN);
	if(SOCKET_ERROR==ret)
	{
		closesocket(m_TCPSocket);
		return false;
	}

	if(SOCKET_ERROR == WSAIoctl(m_TCPSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, 
		sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL))  
	{  
		closesocket(m_TCPSocket);
		return false;  
	}  

	if(SOCKET_ERROR == WSAIoctl(m_TCPSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), &m_lpfnGetAcceptExSockAddrs, sizeof(m_lpfnGetAcceptExSockAddrs),
		&dwBytes,NULL,NULL))  
	{  
		closesocket(m_TCPSocket);
		return false; 
	}
	return true;
}
//关闭TCP套接字
bool CTCPModel::Uninstall()
{
	if(SOCKET_ERROR == shutdown(m_TCPSocket,SD_BOTH)&&10057 != WSAGetLastError())//10057错误为套接字无连接，监听套接字应为连接
	{
		return false;
	}
	if(SOCKET_ERROR == closesocket(m_TCPSocket))
	{
		return false;
	}
	m_TCPSocket=INVALID_SOCKET;
	return true;
}
//将套接字绑定到完成端口
bool CTCPModel::AssociateWithIOCP(HANDLE completionPort)
{
	if(INVALID_HANDLE_VALUE==completionPort)
	{
		return false;
	}
	if(NULL==CreateIoCompletionPort((HANDLE)m_TCPSocket,completionPort,(ULONG_PTR)m_TCPSocket,0))
	{
		return false;
	}
	return true;
}
//投递AcceptEx
bool CTCPModel::AcceptTCPConnect(PER_IO_CONTEXT* pIoContext)
{
	DWORD dwBytes=0;
	pIoContext->m_OpType=ACCEPT_POSTED;
	pIoContext->m_bIsPackageHead=true;
	pIoContext->m_recvPos=0;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
	pIoContext->ResetBuffer();
	pIoContext->m_wsaBuf.len=PACKAGE_HEAD_LEN+(sizeof(SOCKADDR_IN)+16)*2;
	WSABUF *p_wbuf=&pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol=&pIoContext->m_Overlapped;

	SOCKET acceptSocket =WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
	pIoContext->m_socket=acceptSocket;
	if(INVALID_SOCKET==pIoContext->m_socket)
	{
		return false;
	}
	if(false==m_lpfnAcceptEx(m_TCPSocket,pIoContext->m_socket,
		(void*)p_wbuf->buf,p_wbuf->len-(sizeof(SOCKADDR_IN)+16)*2,
		sizeof(SOCKADDR_IN)+16,sizeof(SOCKADDR_IN)+16,&dwBytes,p_ol))
	{
		if(WSA_IO_PENDING!=WSAGetLastError())
		{
			return false;
		}
	}
	return true;
}