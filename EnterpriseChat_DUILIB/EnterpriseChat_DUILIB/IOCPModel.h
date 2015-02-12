#ifndef _IOCPMODEL_H_H
#define _IOCPMODEL_H_H
#include "UDPModel.h"
#include "TCPModel.h"
#include <string>
#include <map>


//������ʱ�ಥ�׵�ַΪ�����߶ಥ��ַ��ÿ��socket��ʼ��ʱ��Ҫ����õ�ַ
#define LOGONOFF_MUTLICAST_ADDR  "224.0.2.0"
//UDPʹ�ö˿�
#define USER_PORT 5050
//�߳���
#define WORKER_THREADS_PER_PROCESSOR 2

class CIOCPModel;
class main_frame;

//�̲߳������ݽṹ
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
	//udp��ر���
	CUDPModel m_UDPModel;
	PER_IO_CONTEXT* m_UDPSendIOContext;
	PER_IO_CONTEXT* m_UDPRecvIOContext;
	std::list<UDPSENDDATA>  m_listUDPMSG;
	CRITICAL_SECTION				m_UDPCritical;					//UDP����ٽ���
	//tcp��ر���
	CTCPModel m_TCPModel;
	std::list<PER_IO_CONTEXT*> m_listListenData;
	std::list<PER_IO_CONTEXT*> m_listClientData;
	std::list<TCPSENDDATA>     m_listTCPSendData;
	CRITICAL_SECTION		   m_TCPCritical;                     //TCP����ٽ���
	//��ɶ˿���ر���
	HANDLE	  m_IOCompletionPort;
	int       m_nThreads;
	HANDLE*   m_workerThreads;
	std::list<std::string>  m_listFriens;
	std::list<GROUPINFO*>    m_listGroup;
	std::string m_strIP;
	std::string m_hostName;
	std::string m_name;
	//������ر���
	std::string m_image;
	std::map<LPCTSTR,LPCTSTR> m_iniIPName;					//�����ļ��д����IP���ǳƶ�
	main_frame* m_mainDlg;
public:
	//����
	bool Start();
	//ֹͣ
	bool Stop();
	//���ر���������
	inline std::string GetPCHostName() {return m_hostName;}
	//���ñ����ǳ�
	inline void SetPCName(std::string name) {m_name=name;}
	//���ر����ǳ�
	inline std::string GetPCName()  {return m_name;}
	//���ر���ͷ��
	inline std::string GetPCImage() {return m_image;}
	//���ñ���ͷ��
	inline void SetPCImage(std::string image) {m_image=image;}
	//���������촰��ָ��
	inline void SetMainDlg(main_frame* pMainDlg) {m_mainDlg=pMainDlg;}
	//��ȡ������ָ��
	inline main_frame* GetMainDlg() {return m_mainDlg;}
	//��ȡUDPͨ�Žṹָ��
	inline PER_IO_CONTEXT* GetUDPSendIOContext() { return m_UDPSendIOContext; }
private:
	// ����Socket��
	bool LoadSocketLib();
	// ж��Socket��
	void UnloadSocketLib();
	//��ʼ������
	bool Initialize(SOCKADDR_IN addr);
	//��ʼ����ɶ˿�
	bool InitializeCompletionPort();
	//��ʼ��UDP���
	bool InitializeUDP(SOCKADDR_IN addr);
	//��ʼ��TCP���
	bool InitializeTCP(SOCKADDR_IN addr);
	//ж�س���ģ��
	bool Uninstall();
	//ж����ɶ˿�
	bool UninstallCompletionPort();
	//ж��UDP���
	bool UninstallUDP();
	//ж��TCP���
	bool UninstallTCP();
	//��ȡϵͳCPU������Ϣ
	int  GetNumberOfProcessors();
	//��IP�Ƿ��Ѿ��ں����б��д���
	bool IsExsitedInFriendsList(SOCKADDR_IN addr);
	//�׽�������ɶ˿ڰ�
	bool SocketAssociateWithIOCP(SOCKET socket);
	//�����׽����Ƿ���δ������ɵ�����
	std::list<TCPSENDDATA>::iterator isExsitTCPSendData(SOCKET sendSocket);
public:
	//�ͻ��˶Ͽ�TCP����ʱ���ر�ͨ���׽���
	bool ClientClose(PER_IO_CONTEXT* pIoContext);
private:
	//��ȡ����IP
	std::string GetIP();
	//��ȡ����������
	std::string GetHost();
	//��ʼ��������Ϣ
	void InitializePC(SOCKADDR_IN addr);
	//���ر�����Ϣ
	bool LoadPCInfo(LPCTSTR fileName);
	//���غ�����Ϣ
	bool LoadFriendInfo(LPCTSTR fileName);
	//����Ⱥ����Ϣ
	bool LoadGroupInfo(LPCTSTR fileName);
	//�����߳�
	static DWORD WINAPI WorkerThread(LPVOID lpParam);
public:
	//���з���UDP��Ϣ
	bool SendUDPMessage(SOCKADDR_IN addr,UDPDATA data,PER_IO_CONTEXT* pIoContext,bool isBlocking=true,bool isAdd=true);
	//���ⲿ���õ�������Ϣ���ͺ���
	bool SendUDPLogOnMessage() { return SendUDPLogOnMessage(m_UDPSendIOContext); }
private:
	//UDP SEND_POST�ص�����
	bool SendUDPMessageCallback(PER_IO_CONTEXT* pIoContext);
	//UDP������Ϣ
	bool RecvUDPMessage(PER_IO_CONTEXT* pIoContext);
	//UDP������Ϣ�ص�����
	bool RecvUDPMessageCallback(PER_IO_CONTEXT* pIoContext);
	//UDP����������Ϣ
	bool SendUDPLogOnMessage(PER_IO_CONTEXT* pIoContext);
	//UDP����������Ϣ
	bool SendUDPLogOffMessage(PER_IO_CONTEXT* pIoContext);
	//Ͷ��TCP AcceptEx����
	bool AcceptTCPConnect(PER_IO_CONTEXT* pIoContext);
	//���յ��ͻ�������
	bool AcceptTCPConnectCallback(PER_IO_CONTEXT* pIoContext,ULONG dwIoSize);
	//Ͷ��TCP WSARecv����
	bool RecvTCPMessage(PER_IO_CONTEXT* pIoContext);
	//���յ��ͻ�����Ϣ
	bool RecvTCPMessageCallback(PER_IO_CONTEXT* pIoContext,ULONG dwIoSize);
public:
	//Ͷ��TCP WSASend����
	bool SendTCPMessage(TCPSENDDATA sendData,PER_IO_CONTEXT* pIoContext,bool isBlocking=true,bool isAdd=true);
	//connect�ⲿ�ӿ�
	bool ConnectServer(LPCTSTR fileName,LPCTSTR ip);
private:
	//TCP������Ϣ�ص�����
	bool SendTCPMessageCallback(PER_IO_CONTEXT* pIoContext);
	//connect����
	bool ConnectServer(PER_IO_CONTEXT* pIoContext);
public:
	//
	PER_IO_CONTEXT* GetIoContextBySocket(SOCKET,OPERATION_TYPE);
	//ɾ�����ߺ���
	bool RemoveFriendFromList(std::string ip);
	//�Ƿ���Ⱥ�鴴����
	bool IsGroupBuilder(LPCTSTR groupIP);
	//�Ƿ���Ⱥ�������
	bool IsGroupManager(LPCTSTR groupIP);
	//��ȡȺ����Ϣ
	GROUPINFO* GetGroupInfo(LPCTSTR groupIP);
	//�����Ƿ�ΪȺ�����Ա
	void SetManager(LPCTSTR groupIP,bool isManager,LPCTSTR fileName);
	//�����Ƿ�ΪȺ���Ա
	void SetMember(LPCTSTR groupIP,bool isMember,LPCTSTR fileName);
	//ȺIP�Ƿ��Ѿ�����
	bool isExsitGroup(LPCTSTR groupIP);
	//����Ⱥ���鲥IP
	std::string AllocationGroupIP();
	//����Ⱥ��
	std::string BuilderGroup(LPCTSTR groupName,LPCTSTR groupImage);
	//����Ⱥ��
	bool JoinGroup(LPCTSTR groupBuilder,LPCTSTR groupIP,LPCTSTR groupName,LPCTSTR groupImage);
	//�˳�Ⱥ��
	bool QuitGroup(LPCTSTR groupIP);
};
#endif
