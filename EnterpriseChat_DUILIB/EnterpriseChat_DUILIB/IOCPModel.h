#ifndef _IOCPMODEL_H_H
#define _IOCPMODEL_H_H
#include "UDPModel.h"
#include "TCPModel.h"
#include <string>
#include <map>


//设置临时多播首地址为上下线多播地址，每个socket初始化时都要加入该地址
#define LOGONOFF_MUTLICAST_ADDR  "224.0.2.0"
//UDP使用端口
#define USER_PORT 5050
//线程数
#define WORKER_THREADS_PER_PROCESSOR 2

class CIOCPModel;
class main_frame;

//线程参数数据结构
typedef struct _THREADPARAMS_WORKER
{
	CIOCPModel* pIOCPModel;
	int         nThreadNo; 
}THREADPARAMS_WORKER,*PTHREADPARAMS_WORKER;

class CIOCPModel
{
public:
	CIOCPModel(void);
	~CIOCPModel(void);
private:
	//udp相关变量
	CUDPModel m_UDPModel;
	PER_IO_CONTEXT* m_UDPSendIOContext;
	PER_IO_CONTEXT* m_UDPRecvIOContext;
	std::list<UDPSENDDATA>  m_listUDPMSG;
	CRITICAL_SECTION				m_UDPCritical;					//UDP相关临界区
	//tcp相关变量
	CTCPModel m_TCPModel;
	std::list<PER_IO_CONTEXT*> m_listListenData;
	std::list<PER_IO_CONTEXT*> m_listClientData;
	std::list<TCPSENDDATA>     m_listTCPSendData;
	CRITICAL_SECTION		   m_TCPCritical;                     //TCP相关临界区
	//完成端口相关变量
	HANDLE	  m_IOCompletionPort;
	int       m_nThreads;
	HANDLE*   m_workerThreads;
	std::list<std::string>  m_listFriens;
	std::list<GROUPINFO*>    m_listGroup;
	std::string m_strIP;
	std::string m_hostName;
	std::string m_name;
	//窗体相关变量
	std::string m_image;
	std::map<LPCTSTR,LPCTSTR> m_iniIPName;					//配置文件中储存的IP、昵称对
	main_frame* m_mainDlg;
public:
	//启动
	bool Start();
	//停止
	bool Stop();
	//返回本机主机名
	inline std::string GetPCHostName() {return m_hostName;}
	//设置本机昵称
	inline void SetPCName(std::string name) {m_name=name;}
	//返回本机昵称
	inline std::string GetPCName()  {return m_name;}
	//返回本机头像
	inline std::string GetPCImage() {return m_image;}
	//设置本机头像
	inline void SetPCImage(std::string image) {m_image=image;}
	//设置主聊天窗口指针
	inline void SetMainDlg(main_frame* pMainDlg) {m_mainDlg=pMainDlg;}
	//获取主窗口指针
	inline main_frame* GetMainDlg() {return m_mainDlg;}
	//获取UDP通信结构指针
	inline PER_IO_CONTEXT* GetUDPSendIOContext() { return m_UDPSendIOContext; }
private:
	// 加载Socket库
	bool LoadSocketLib();
	// 卸载Socket库
	void UnloadSocketLib();
	//初始化操作
	bool Initialize(SOCKADDR_IN addr);
	//初始化完成端口
	bool InitializeCompletionPort();
	//初始化UDP相关
	bool InitializeUDP(SOCKADDR_IN addr);
	//初始化TCP相关
	bool InitializeTCP(SOCKADDR_IN addr);
	//卸载程序模块
	bool Uninstall();
	//卸载完成端口
	bool UninstallCompletionPort();
	//卸载UDP相关
	bool UninstallUDP();
	//卸载TCP相关
	bool UninstallTCP();
	//获取系统CPU数量信息
	int  GetNumberOfProcessors();
	//该IP是否已经在好友列表中存在
	bool IsExsitedInFriendsList(SOCKADDR_IN addr);
	//套接字与完成端口绑定
	bool SocketAssociateWithIOCP(SOCKET socket);
	//发送套接字是否有未发送完成的数据
	std::list<TCPSENDDATA>::iterator isExsitTCPSendData(SOCKET sendSocket);
public:
	//客户端断开TCP连接时，关闭通信套接字
	bool ClientClose(PER_IO_CONTEXT* pIoContext);
private:
	//获取本机IP
	std::string GetIP();
	//获取本机主机名
	std::string GetHost();
	//初始化本机信息
	void InitializePC(SOCKADDR_IN addr);
	//加载本机信息
	bool LoadPCInfo(LPCTSTR fileName);
	//加载好友信息
	bool LoadFriendInfo(LPCTSTR fileName);
	//加载群组信息
	bool LoadGroupInfo(LPCTSTR fileName);
	//工作线程
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
public:
	//队列发送UDP消息
	bool SendUDPMessage(SOCKADDR_IN addr,UDPDATA data,PER_IO_CONTEXT* pIoContext,bool isBlocking=true,bool isAdd=true);
	//供外部调用的上线信息发送函数
	bool SendUDPLogOnMessage() { return SendUDPLogOnMessage(m_UDPSendIOContext); }
private:
	//UDP SEND_POST回调函数
	bool SendUDPMessageCallback(PER_IO_CONTEXT* pIoContext);
	//UDP接受消息
	bool RecvUDPMessage(PER_IO_CONTEXT* pIoContext);
	//UDP接受消息回调函数
	bool RecvUDPMessageCallback(PER_IO_CONTEXT* pIoContext);
	//UDP发送上线信息
	bool SendUDPLogOnMessage(PER_IO_CONTEXT* pIoContext);
	//UDP发送下线消息
	bool SendUDPLogOffMessage(PER_IO_CONTEXT* pIoContext);
	//投递TCP AcceptEx操作
	bool AcceptTCPConnect(PER_IO_CONTEXT* pIoContext);
	//接收到客户端连接
	bool AcceptTCPConnectCallback(PER_IO_CONTEXT* pIoContext,ULONG dwIoSize);
	//投递TCP WSARecv操作
	bool RecvTCPMessage(PER_IO_CONTEXT* pIoContext);
	//接收到客户端信息
	bool RecvTCPMessageCallback(PER_IO_CONTEXT* pIoContext,ULONG dwIoSize);
public:
	//投递TCP WSASend操作
	bool SendTCPMessage(TCPSENDDATA sendData,PER_IO_CONTEXT* pIoContext,bool isBlocking=true,bool isAdd=true);
	//connect外部接口
	bool ConnectServer(LPCTSTR fileName,LPCTSTR ip);
private:
	//TCP发送信息回调函数
	bool SendTCPMessageCallback(PER_IO_CONTEXT* pIoContext);
	//connect操作
	bool ConnectServer(PER_IO_CONTEXT* pIoContext);
public:
	//
	PER_IO_CONTEXT* GetIoContextBySocket(SOCKET,OPERATION_TYPE);
	//删除下线好友
	bool RemoveFriendFromList(std::string ip);
	//是否是群组创建者
	bool IsGroupBuilder(LPCTSTR groupIP);
	//是否是群组管理者
	bool IsGroupManager(LPCTSTR groupIP);
	//获取群组信息
	GROUPINFO* GetGroupInfo(LPCTSTR groupIP);
	//设置是否为群组管理员
	void SetManager(LPCTSTR groupIP,bool isManager,LPCTSTR fileName);
	//设置是否为群组成员
	void SetMember(LPCTSTR groupIP,bool isMember,LPCTSTR fileName);
	//群IP是否已经存在
	bool isExsitGroup(LPCTSTR groupIP);
	//分配群组组播IP
	std::string AllocationGroupIP();
	//创建群组
	std::string BuilderGroup(LPCTSTR groupName,LPCTSTR groupImage);
	//加入群组
	bool JoinGroup(LPCTSTR groupBuilder,LPCTSTR groupIP,LPCTSTR groupName,LPCTSTR groupImage);
	//退出群组
	bool QuitGroup(LPCTSTR groupIP);
};
#endif
