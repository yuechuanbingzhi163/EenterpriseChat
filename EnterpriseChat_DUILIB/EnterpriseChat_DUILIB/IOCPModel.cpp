#include <WS2tcpip.h>
#include <io.h>
#include "IOCPModel.h"
#include "main_frame.h"
#include <UIlib.h>
#include <time.h>

using namespace std;
using namespace DuiLib;

CIOCPModel::CIOCPModel(void):
	m_IOCompletionPort(INVALID_HANDLE_VALUE),
	m_nThreads(0),
	m_UDPSendIOContext(NULL),
	m_UDPRecvIOContext(NULL),
	m_mainDlg(NULL),
	m_image("")
{
	m_listUDPMSG.clear();
	m_listFriens.clear();
	m_iniIPName.clear();
	m_listGroup.clear();
}

CIOCPModel::~CIOCPModel(void)
{
}

//����
bool CIOCPModel::Start()
{
	if(!LoadSocketLib())
	{
		return false;
	}
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr(GetIP().c_str());
	addr.sin_port=htons(USER_PORT);
	if(!Initialize(addr))
	{
		return false;
	}
	return true;
}
//ֹͣ
bool CIOCPModel::Stop()
{
	SendUDPLogOnMessage(m_UDPSendIOContext);
	Sleep(500);
	if(!Uninstall())
	{
		return false;
	}
	UnloadSocketLib();
	return true;
}
//�����׽��ֿ⣬ʹ��windows socket�ı�Ҫ����
bool CIOCPModel::LoadSocketLib()
{
	WSADATA wsaData;
	if(0!=WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		return false;
	}
	return true;
}
//ж���׽��ֿ⣬�����˳�ǰ���һ��
void CIOCPModel::UnloadSocketLib()
{

	WSACleanup();
}
//��ʼ��
bool CIOCPModel::Initialize(SOCKADDR_IN addr)
{
	//�����ʼ��������
	//��ʼ����ɶ˿ڡ���ʼ��UDP(�󶨵���ɶ˿�)����ʼ��TCP(�󶨵���ɶ˿ڡ�Ͷ��TCP����)�����뵽��ʼ�ಥ�顢����������Ϣ��Ͷ��UDP���ղ���
	//��ʼ����ɶ˿�
	m_UDPSendIOContext=new PER_IO_CONTEXT();
	m_UDPRecvIOContext=new PER_IO_CONTEXT();
	InitializeCriticalSection(&m_UDPCritical);
	InitializeCriticalSection(&m_TCPCritical);
	InitializePC(addr);
	if(!InitializeCompletionPort())
	{
		return false;
	}
	//��ʼ��UDP
	if(!InitializeUDP(addr))
	{
		return false;
	}
	//��ʼ��TCP
	if(!InitializeTCP(addr))
	{
		return false;
	}
	//���뵽��ʼ�ಥ��
	SOCKADDR_IN mutlicastAddr;
	memset(&mutlicastAddr,0,sizeof(mutlicastAddr));
	mutlicastAddr.sin_family=AF_INET;
	mutlicastAddr.sin_addr.S_un.S_addr=inet_addr(LOGONOFF_MUTLICAST_ADDR);
	mutlicastAddr.sin_port=htons(USER_PORT);
	if(!m_UDPModel.JoinMutliCast(mutlicastAddr))
	{
		return false;
	}
	//����������Ϣ
	SendUDPLogOnMessage(m_UDPSendIOContext);
	//Ͷ�ݽ��ܲ���
	if(!RecvUDPMessage(m_UDPRecvIOContext))
	{
		return false;
	}
	return true;
}
//��ʼ����ɶ˿�
bool CIOCPModel::InitializeCompletionPort()
{
	m_IOCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);

	if(NULL==m_IOCompletionPort)
	{
		return false;
	}

	m_nThreads=WORKER_THREADS_PER_PROCESSOR*GetNumberOfProcessors()+2;

	m_workerThreads= new HANDLE[m_nThreads];

	DWORD nThreadID;

	for(int i=0;i<m_nThreads;++i)
	{
		THREADPARAMS_WORKER* pThreadParams = new THREADPARAMS_WORKER;
		pThreadParams->pIOCPModel = this;
		pThreadParams->nThreadNo  = i+1;
		m_workerThreads[i]=CreateThread(0,0,WorkerThread,(void*)pThreadParams,0,&nThreadID);
	}
	return true;
}
//��ʼ��UDPģ��
bool CIOCPModel::InitializeUDP(SOCKADDR_IN addr)
{
	if(!m_UDPModel.Initialize(addr))
	{
		return false;
	}
	if(!m_UDPModel.AssociateWithIOCP(m_IOCompletionPort))
	{
		return false;
	}
	return true;
}
//��ʼ��TCPģ��
bool CIOCPModel::InitializeTCP(SOCKADDR_IN addr)
{
	if(!m_TCPModel.Initialize(addr))
	{
		return false;
	}
	if(!m_TCPModel.AssociateWithIOCP(m_IOCompletionPort))
	{
		return false;
	}
	if(m_nThreads)
	{
		for(int i=0;i<m_nThreads;++i)
		{
			PER_IO_CONTEXT* pIoContext= new PER_IO_CONTEXT();
			if(false==this->AcceptTCPConnect(pIoContext))
			{
				RELEASE(pIoContext);
				return false;
			}
			m_listListenData.push_back(pIoContext);
		}
	}
	return true;
}
//ж�س���ģ��
bool CIOCPModel::Uninstall()
{
	if(!UninstallUDP())
	{
		return false;
	}
	if(!UninstallTCP())
	{
		return false;
	}
	if(!UninstallCompletionPort())
	{
		return false;
	}
	return true;
}
//ж����ɶ˿�
bool CIOCPModel::UninstallCompletionPort()
{
	for(int i=0;i<m_nThreads*2;++i)
	{
		PostQueuedCompletionStatus(m_IOCompletionPort,0,(DWORD)EXIT_CODE,NULL);
	}
	WaitForMultipleObjects(m_nThreads,m_workerThreads,TRUE,INFINITE);
	for(int i=0;i<=m_nThreads;++i)
	{
		RELEASE_HANDLE(m_workerThreads[i]);
	}
	delete [] m_workerThreads;
	RELEASE_HANDLE(m_IOCompletionPort);
	return true;
}
//ж��UDP���
bool CIOCPModel::UninstallUDP()
{
	if(!m_UDPModel.Uninstall())
	{
		return false;
	}
	RELEASE(m_UDPRecvIOContext);
	RELEASE(m_UDPSendIOContext);
	return true;
}
//ж��TCP���
bool CIOCPModel::UninstallTCP()
{
	if(!m_TCPModel.Uninstall())
	{
		return false;
	}
	PER_IO_CONTEXT* pIoContext=NULL;
	for(list<PER_IO_CONTEXT*>::iterator ite=m_listListenData.begin();ite!=m_listListenData.end();)
	{

		pIoContext=*ite;
		shutdown(pIoContext->m_socket,SD_BOTH);
		closesocket(pIoContext->m_socket);
		m_listListenData.erase(ite);
		RELEASE(pIoContext);
		ite=m_listListenData.begin();
	}
	for(list<PER_IO_CONTEXT*>::iterator ite=m_listClientData.begin();ite!=m_listClientData.end();)
	{

		pIoContext=*ite;
		shutdown(pIoContext->m_socket,SD_BOTH);
		closesocket(pIoContext->m_socket);
		m_listClientData.erase(ite);
		RELEASE(pIoContext);
		ite=m_listClientData.begin();
	}
	return true;
}
//��ȡϵͳ������������
int  CIOCPModel::GetNumberOfProcessors()
{
	SYSTEM_INFO systemInfo;
	ZeroMemory(&systemInfo,sizeof(SYSTEM_INFO));
	GetSystemInfo(&systemInfo);
	return systemInfo.dwNumberOfProcessors;
}
//��IP�Ƿ��Ѿ��ں����б��д���
bool CIOCPModel::IsExsitedInFriendsList(SOCKADDR_IN addr)
{
	if(m_listFriens.size()==0)
	{
		return false;
	}
	string strIP(inet_ntoa(addr.sin_addr));
	for(list<string>::iterator ite=m_listFriens.begin();ite!=m_listFriens.end();++ite)
	{
		if(*ite==strIP)
		{
			return true;
		}
	}
	return false;
}
//�׽�������ɶ˿ڰ�
bool CIOCPModel::SocketAssociateWithIOCP(SOCKET bindSocket)
{
	if(INVALID_SOCKET==bindSocket)
	{
		return false;
	}
	if(NULL==CreateIoCompletionPort((HANDLE)bindSocket,m_IOCompletionPort,(ULONG_PTR)bindSocket,0))
	{
		return false;
	}
	return true;
}
//�����׽����Ƿ���δ������ɵ�����
list<TCPSENDDATA>::iterator CIOCPModel::isExsitTCPSendData(SOCKET sendSocket)
{
	list<TCPSENDDATA>::iterator ite=m_listTCPSendData.begin();
	if(0==m_listTCPSendData.size())
	{
		ite=m_listTCPSendData.end();
		return ite;
	}
	for(;ite!=m_listTCPSendData.end();++ite)
	{
		if(ite->m_socket==sendSocket)
		{
			return ite;
		}
	}
	return ite;
}
//��ȡ����IP
string CIOCPModel::GetIP()
{
	char hostName[50]={0};
	char *cAddr;
	addrinfo hint;
	addrinfo *res;
	SOCKADDR_IN *addr;
	memset(&hint,0,sizeof(hint));
	hint.ai_family=AF_INET;
	hint.ai_flags = AI_PASSIVE; /* For wildcard IP address */
	hint.ai_protocol = 0; /* Any protocol */
	hint.ai_socktype = SOCK_DGRAM;
	if(0!=gethostname(hostName,sizeof(hostName)))
	{
		return NULL;
	}
	if(0!=getaddrinfo(hostName,NULL,&hint,&res))
	{
		return NULL;
	}
	addr=(SOCKADDR_IN*)res->ai_addr;
	cAddr=inet_ntoa(addr->sin_addr);
	string strIP(cAddr);
	return strIP;
}
//��ȡ����������
string CIOCPModel::GetHost()
{
	char name[MAX_PATH];
	memset(name,0,sizeof(name));
	gethostname(name,sizeof(name));
	return string(name);
}
//��ʼ��������Ϣ
void CIOCPModel::InitializePC(SOCKADDR_IN addr)
{
	CreateDirectory(L"D:\\EChatAcceptEx",NULL);
	m_strIP=string(inet_ntoa(addr.sin_addr));
	m_hostName=GetHost();
	SetPCImage("default.png");
	SetPCName(m_hostName);

	TCHAR strFileName[MAX_PATH]={0};
	memset(strFileName,0,sizeof(strFileName));
	int size=MultiByteToWideChar(0,0,OWNINI_PATH,-1,NULL,0);
	MultiByteToWideChar(0,0,OWNINI_PATH,-1,strFileName,size);

	LoadPCInfo(strFileName);

	//��ȡ���������������ļ���ϣ����½�����ʾ
	m_mainDlg->SetImage(GetPCImage().c_str());
	m_mainDlg->SetName(GetPCName().c_str());

	//���ػ�ȡ����IP����ע����
	memset(strFileName,0,sizeof(strFileName));
	size=MultiByteToWideChar(0,0,FRIINI_PATH,-1,NULL,0);
	MultiByteToWideChar(0,0,FRIINI_PATH,-1,strFileName,size);

	LoadFriendInfo(strFileName);

	//���ػ�ȡӵ��Ⱥ����Ϣ
	LoadGroupInfo(strFileName);
}
//ɾ������
bool CIOCPModel::RemoveFriendFromList(std::string ip)
{
	for(list<string>::iterator ite=m_listFriens.begin();ite!=m_listFriens.end();++ite)
	{
		if(*ite==ip)
		{
			m_listFriens.remove(ip);
		}
	}
	return true;
}
//���ر�����Ϣ
bool CIOCPModel::LoadPCInfo(LPCTSTR fileName)
{
	char strFileName[MAX_PATH]={0};
	memset(strFileName,0,sizeof(strFileName));
	int size=WideCharToMultiByte(0,0,fileName,-1,NULL,0,NULL,NULL);
	WideCharToMultiByte(0,0,fileName,-1,strFileName,size,NULL,NULL);

	TCHAR cStrIP[MAX_PATH];
	memset(cStrIP,0,sizeof(cStrIP));
	TCHAR cStrHostName[MAX_PATH];
	memset(cStrHostName,0,sizeof(cStrHostName));
	TCHAR cStrName[MAX_PATH];
	memset(cStrName,0,sizeof(cStrName));
	TCHAR cStrImage[MAX_PATH];
	memset(cStrImage,0,sizeof(cStrImage));

	if(-1==_access((strFileName),0))
	{
		//�����ļ�
		//AfxMessageBox(L"�����ļ������ڣ�����");
		size=MultiByteToWideChar(0,0,m_strIP.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_strIP.c_str(),-1,cStrIP,size);

		size=MultiByteToWideChar(0,0,m_hostName.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_hostName.c_str(),-1,cStrHostName,size);

		size=MultiByteToWideChar(0,0,m_name.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_name.c_str(),-1,cStrName,size);

		size=MultiByteToWideChar(0,0,m_image.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_image.c_str(),-1,cStrImage,size);


		WritePrivateProfileString(cStrIP,PCHOSTNAME,cStrHostName,fileName);
		WritePrivateProfileString(cStrIP,PCNAME,cStrName,fileName);
		WritePrivateProfileString(cStrIP,PCIMAGE,cStrImage,fileName);
	}
	else
	{
		TCHAR str1[MAX_PATH]={0};
		memset(str1,0,sizeof(str1));
		TCHAR str2[MAX_PATH]={0};
		memset(str2,0,sizeof(str2));
		TCHAR str3[MAX_PATH]={0};
		memset(str3,0,sizeof(str3));

		size=MultiByteToWideChar(0,0,m_strIP.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_strIP.c_str(),-1,cStrIP,size);

		size=MultiByteToWideChar(0,0,m_hostName.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_hostName.c_str(),-1,cStrHostName,size);

		size=MultiByteToWideChar(0,0,m_name.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_name.c_str(),-1,cStrName,size);

		m_image=string("default.png");
		size=MultiByteToWideChar(0,0,m_image.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,m_image.c_str(),-1,cStrImage,size);


		//��ȡ�����ļ���PC�����������ǳơ�ͷ�񣬲����θ�ֵ�ǳƣ�ͷ��������ж��������Ƿ���ͬ����ͬ������д��
		GetPrivateProfileString(cStrIP,PCHOSTNAME,cStrHostName,str1,MAX_PATH,fileName);
		GetPrivateProfileString(cStrIP,PCNAME,cStrName,str2,MAX_PATH,fileName);
		GetPrivateProfileString(cStrIP,PCIMAGE,cStrImage,str3,MAX_PATH,fileName);

		int number=WideCharToMultiByte(0,0,str1,-1,0,0,NULL,NULL);
		char* cstr1=new char[number];
		WideCharToMultiByte(0,0,str1,-1,cstr1,number,NULL,NULL);
		if(string(cstr1)!=m_hostName)
		{
			WritePrivateProfileString(cStrIP,PCHOSTNAME,cStrHostName,fileName);
		}
		RELEASE(cstr1);

		number=WideCharToMultiByte(0,0,str2,-1,0,0,NULL,NULL);
		char* cstr2=new char[number];
		WideCharToMultiByte(0,0,str2,-1,cstr2,number,NULL,NULL);
		SetPCName(string(cstr2));
		RELEASE( cstr2);

		number=WideCharToMultiByte(0,0,str3,-1,0,0,NULL,NULL);
		char* cstr3=new char[number];
		WideCharToMultiByte(0,0,str3,-1,cstr3,number,NULL,NULL);
		SetPCImage(string(cstr3));
		RELEASE( cstr3);
	}
	return true;
}
//���غ�����Ϣ
bool CIOCPModel::LoadFriendInfo(LPCTSTR fileName)
{
	char strFileName[MAX_PATH]={0};
	memset(strFileName,0,sizeof(strFileName));
	int size=WideCharToMultiByte(0,0,fileName,-1,NULL,0,NULL,NULL);
	WideCharToMultiByte(0,0,fileName,-1,strFileName,size,NULL,NULL);

	if(0==_access((strFileName),0))
	{
		int i; 
		int iPos=0; 
		int iMaxCount;
		TCHAR chSectionNames[MAX_ALLSECTIONS]={0}; //�ܵ���������ַ���
		TCHAR chSection[MAX_SECTION]={0}; //���һ��������
		GetPrivateProfileSectionNames(chSectionNames,MAX_ALLSECTIONS,fileName);
		for(i=0;i<MAX_ALLSECTIONS;i++)
		{
			if (chSectionNames[i]==0)
			{
				if (chSectionNames[i]==chSectionNames[i+1])
				{
					break;
				}
			}
		}
		iMaxCount=i+1; //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ������֡�
		//arrSection.RemoveAll();//���ԭ����
		for(i=0;i<iMaxCount;i++)
		{
			chSection[iPos++]=chSectionNames[i];
			if(chSectionNames[i]==0)
			{ 
				TCHAR cstrName[MAX_SECTION]={0};
				GetPrivateProfileString(chSection,FRIENDNAME,L"",cstrName,sizeof(cstrName),fileName);
				m_iniIPName.insert(make_pair(chSection,cstrName));
				memset(chSection,0,i);
				iPos=0;
			}
		}
	}
	return true;
}
//����Ⱥ����Ϣ
bool CIOCPModel::LoadGroupInfo(LPCTSTR fileName)
{
	char strFileName[MAX_PATH]={0};
	memset(strFileName,0,sizeof(strFileName));
	int size=WideCharToMultiByte(0,0,fileName,-1,NULL,0,NULL,NULL);
	WideCharToMultiByte(0,0,fileName,-1,strFileName,size,NULL,NULL);

	if(0==_access((strFileName),0))
	{
		int i; 
		int iPos=0; 
		int iMaxCount;
		TCHAR chSectionNames[MAX_ALLSECTIONS]={0}; //�ܵ���������ַ���
		TCHAR chSection[MAX_SECTION]={0}; //���һ��������
		GetPrivateProfileSectionNames(chSectionNames,MAX_ALLSECTIONS,fileName);
		for(i=0;i<MAX_ALLSECTIONS;i++)
		{
			if (chSectionNames[i]==0)
			{
				if (chSectionNames[i]==chSectionNames[i+1])
				{
					break;
				}
			}
		}
		iMaxCount=i+1; //Ҫ��һ��0��Ԫ�ء����ҳ�ȫ���ַ����Ľ������֡�
		GROUPINFO *groupInfo=NULL;
		//arrSection.RemoveAll();//���ԭ����
		for(i=0;i<iMaxCount;i++)
		{
			chSection[iPos++]=chSectionNames[i];
			if(chSectionNames[i]==0)
			{ 
				groupInfo=new GROUPINFO;
				memcpy(groupInfo->m_groupIP,chSection,sizeof(chSection));
				TCHAR cstrName[MAX_SECTION]={0};
				memset(cstrName,0,sizeof(cstrName));
				GetPrivateProfileString(chSection,GROUPBUILDER,L"",cstrName,sizeof(cstrName),fileName);
				memcpy(groupInfo->m_builder,cstrName,sizeof(cstrName));
				memset(cstrName,0,sizeof(cstrName));
				GetPrivateProfileString(chSection,GROUPNAME,L"",cstrName,sizeof(cstrName),fileName);
				memcpy(groupInfo->m_groupName,cstrName,sizeof(cstrName));
				memset(cstrName,0,sizeof(cstrName));
				GetPrivateProfileString(chSection,GROUPIMAGE,L"",cstrName,sizeof(cstrName),fileName);
				memcpy(groupInfo->m_groupImage,cstrName,sizeof(cstrName));
				groupInfo->m_isManager=GetPrivateProfileInt(chSection,GROUPMANAGER,0,fileName);
				groupInfo->m_isMember=GetPrivateProfileInt(chSection,GROUPMEMBER,0,fileName);
				m_listGroup.push_back(groupInfo);
				memset(chSection,0,i);
				iPos=0;
			}
		}
	}
	return true;
}
//�Ƿ���Ⱥ�鴴����
bool CIOCPModel::IsGroupBuilder(LPCTSTR groupIP)
{
	TCHAR cstrIP[MAX_PATH]={0};
	memset(cstrIP,0,sizeof(cstrIP));
	int size=MultiByteToWideChar(0,0,m_strIP.c_str(),-1,NULL,0);
	MultiByteToWideChar(0,0,m_strIP.c_str(),-1,cstrIP,size);
	GROUPINFO *groupInfo=GetGroupInfo(groupIP);
	if(groupInfo==NULL)
	{
		return false;
	}
	if(_tcsicmp(groupInfo->m_builder,cstrIP)==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//�Ƿ���Ⱥ�������
bool CIOCPModel::IsGroupManager(LPCTSTR groupIP)
{
	GROUPINFO *groupInfo=GetGroupInfo(groupIP);
	if(groupInfo==NULL)
	{
		return false;
	}
	if(groupInfo->m_isManager==0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
//��ȡȺ����Ϣ
GROUPINFO* CIOCPModel::GetGroupInfo(LPCTSTR groupIP)
{
	GROUPINFO* groupInfo=NULL;
	for(list<GROUPINFO*>::iterator ite=m_listGroup.begin();ite!=m_listGroup.end();++ite)
	{
		if(_tcsicmp((*ite)->m_groupIP,groupIP)==0)
		{
			groupInfo=*ite;
			break;
		}
	}
	return groupInfo;
}
//�����Ƿ�ΪȺ�����Ա
void CIOCPModel::SetManager(LPCTSTR groupIP,bool isManager,LPCTSTR fileName)
{
	TCHAR cstrIP[MAX_PATH]={0};
	memset(cstrIP,0,sizeof(cstrIP));
	int size=MultiByteToWideChar(0,0,m_strIP.c_str(),-1,NULL,0);
	MultiByteToWideChar(0,0,m_strIP.c_str(),-1,cstrIP,size);
	GROUPINFO *groupInfo=GetGroupInfo(groupIP);
	if(groupInfo==NULL)
	{
		return ;
	}
	groupInfo->m_isManager=isManager;
	if(isManager)
	{
		WritePrivateProfileString(groupIP,GROUPMANAGER,_T("1"),fileName);
	}
	else
	{
		WritePrivateProfileString(groupIP,GROUPMANAGER,_T("0"),fileName);
	}
}
//�����Ƿ�ΪȺ��Ա
void CIOCPModel::SetMember(LPCTSTR groupIP,bool isMember,LPCTSTR fileName)
{
	TCHAR cstrIP[MAX_PATH]={0};
	memset(cstrIP,0,sizeof(cstrIP));
	int size=MultiByteToWideChar(0,0,m_strIP.c_str(),-1,NULL,0);
	MultiByteToWideChar(0,0,m_strIP.c_str(),-1,cstrIP,size);
	GROUPINFO *groupInfo=GetGroupInfo(groupIP);
	if(groupInfo==NULL)
	{
		return ;
	}
	groupInfo->m_isMember=isMember;
	if(isMember)
	{
		WritePrivateProfileString(groupIP,GROUPMEMBER,_T("1"),fileName);
	}
	else
	{
		WritePrivateProfileString(groupIP,GROUPMEMBER,_T("0"),fileName);
	}
}
//ȺIP�Ƿ��Ѿ�����
bool CIOCPModel::isExsitGroup(LPCTSTR groupIP)
{
	for(list<GROUPINFO*>::iterator ite=m_listGroup.begin();ite!=m_listGroup.end();++ite)
	{
		if(_tcsicmp((*ite)->m_groupIP,groupIP)==0)
		{
			return true;
		}
	}
	return false;
}
//����Ⱥ���鲥IP
std::string CIOCPModel::AllocationGroupIP()
{
	TCHAR citem[MAX_PATH]={0};
	char item[MAX_PATH]={0};
	memset(citem,0,sizeof(citem));
	memset(item,0,sizeof(item));
	string ip;
	srand((unsigned)time(0));

	int ret=rand()%16+224;
	sprintf_s(item,"%d",ret);
	ip+=item;

	memset(item,0,sizeof(item));
	ret=rand()%256;
	sprintf_s(item,"%d",ret);
	ip+=item;

	memset(item,0,sizeof(item));
	ret=rand()%254+2;
	ip+=item;

	memset(item,0,sizeof(item));
	ret=rand()%255+1;
	ip+=item;

	int size=MultiByteToWideChar(0,0,ip.c_str(),-1,NULL,0);
	MultiByteToWideChar(0,0,ip.c_str(),-1,citem,size);
	while(isExsitGroup(citem))
	{
		ip=AllocationGroupIP();
		memset(citem,0,sizeof(citem));
		size=MultiByteToWideChar(0,0,ip.c_str(),-1,NULL,0);
		MultiByteToWideChar(0,0,ip.c_str(),-1,citem,size);
	}
	return ip;
}
//����Ⱥ��
string CIOCPModel::BuilderGroup(LPCTSTR groupName,LPCTSTR groupImage)
{
	if(_tcsicmp(groupName,_T(""))==0||_tcsicmp(groupImage,_T(""))==0)
	{
		return string("");
	}
	GROUPINFO *info=new GROUPINFO;
	string ip=AllocationGroupIP();
	TCHAR strIP[MAX_PATH]={0};
	memset(strIP,0,sizeof(strIP));
	int size=MultiByteToWideChar(0,0,ip.c_str(),-1,NULL,0);
	MultiByteToWideChar(0,0,ip.c_str(),-1,strIP,size);
	memcpy(info->m_groupIP,strIP,sizeof(strIP));
	memcpy(info->m_groupName,groupName,sizeof(groupName));
	memcpy(info->m_groupImage,groupImage,sizeof(groupImage));
	TCHAR strBuilder[MAX_PATH]={0};
	memset(strBuilder,0,sizeof(strBuilder));
	size=MultiByteToWideChar(0,0,m_strIP.c_str(),-1,NULL,0);
	MultiByteToWideChar(0,0,m_strIP.c_str(),-1,strBuilder,size);
	memcpy(info->m_builder,strBuilder,sizeof(strBuilder));
	info->m_isManager=1;
	info->m_isMember=1;

	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr(ip.c_str());
	addr.sin_port=htons(USER_PORT);

	TCHAR filePath[MAX_PATH]={0};
	memset(filePath,0,sizeof(filePath));
	size=MultiByteToWideChar(0,0,GROUPINI_PATH,-1,NULL,0);
	MultiByteToWideChar(0,0,GROUPINI_PATH,-1,filePath,size);

	if(m_UDPModel.JoinMutliCast(addr))
	{
		m_listGroup.push_back(info);
		WritePrivateProfileString(strIP,GROUPNAME,groupName,filePath);
		WritePrivateProfileString(strIP,GROUPIMAGE,groupImage,filePath);
		WritePrivateProfileString(strIP,GROUPBUILDER,strBuilder,filePath);
		WritePrivateProfileString(strIP,GROUPMANAGER,_T("1"),filePath);
		WritePrivateProfileString(strIP,GROUPMEMBER,_T("1"),filePath);
	}
	else
	{
		RELEASE(info);
	}
	return ip;
}
//����Ⱥ��
bool CIOCPModel::JoinGroup(LPCTSTR groupBuilder,LPCTSTR groupIP,LPCTSTR groupName,LPCTSTR groupImage)
{
	if(_tcsicmp(groupBuilder,_T(""))==0||_tcsicmp(groupIP,_T(""))==0||_tcsicmp(groupName,_T(""))==0||_tcsicmp(groupImage,_T(""))==0)
	{
		return false;
	}
	GROUPINFO *info=new GROUPINFO;
	memcpy(info->m_groupIP,groupIP,sizeof(groupIP));
	memcpy(info->m_groupName,groupName,sizeof(groupName));
	memcpy(info->m_groupImage,groupImage,sizeof(groupImage));
	memcpy(info->m_builder,groupBuilder,sizeof(groupBuilder));
	info->m_isManager=0;
	info->m_isMember=1;

	char ip[MAX_PATH]={0};
	memset(ip,0,sizeof(ip));
	int size=WideCharToMultiByte(0,0,groupIP,-1,NULL,0,NULL,NULL);
	WideCharToMultiByte(0,0,groupIP,-1,ip,size,NULL,NULL);

	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr(ip);
	addr.sin_port=htons(USER_PORT);

	TCHAR filePath[MAX_PATH]={0};
	memset(filePath,0,sizeof(filePath));
	size=MultiByteToWideChar(0,0,GROUPINI_PATH,-1,NULL,0);
	MultiByteToWideChar(0,0,GROUPINI_PATH,-1,filePath,size);

	if(m_UDPModel.JoinMutliCast(addr))
	{
		m_listGroup.push_back(info);
		WritePrivateProfileString(groupIP,GROUPNAME,groupName,filePath);
		WritePrivateProfileString(groupIP,GROUPIMAGE,groupImage,filePath);
		WritePrivateProfileString(groupIP,GROUPBUILDER,groupBuilder,filePath);
		WritePrivateProfileString(groupIP,GROUPMANAGER,_T("0"),filePath);
		WritePrivateProfileString(groupIP,GROUPMEMBER,_T("0"),filePath);
	}
	else
	{
		RELEASE(info);
	}
	return true;
}
//�˳�Ⱥ��
bool CIOCPModel::QuitGroup(LPCTSTR groupIP)
{
	TCHAR strPath[MAX_PATH]={0};
	memset(strPath,0,sizeof(strPath));
	int size=MultiByteToWideChar(0,0,GROUPINI_PATH,-1,NULL,0);
	MultiByteToWideChar(0,0,GROUPINI_PATH,-1,strPath,size);

	WritePrivateProfileString(groupIP,NULL,NULL,strPath);
	GROUPINFO *info=GetGroupInfo(groupIP);
	if(info==NULL)
	{
		return false;
	}
	m_listGroup.remove(info);
	RELEASE(info);
	return true;
}
/*****************************************/
//ͨ�Ź����߳�
DWORD WINAPI CIOCPModel::WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParam = (THREADPARAMS_WORKER*)lpParam;
	CIOCPModel* pIOCPModel = (CIOCPModel*)pParam->pIOCPModel;
	int nThreadNo = (int)pParam->nThreadNo;

	OVERLAPPED           *pOverlapped = NULL;
	SOCKET	              *pSocket=NULL;
	DWORD                dwBytesTransfered = 0;
	while(true)
	{
		BOOL bReturn=GetQueuedCompletionStatus(pIOCPModel->m_IOCompletionPort,&dwBytesTransfered,(PULONG_PTR)&pSocket,
			&pOverlapped,INFINITE);

		//AfxMessageBox(L"ȡ����Ϣ����");
		if(EXIT_CODE==pSocket)
		{
			//AfxMessageBox(L"EXIT_CODE");
			break;
		}
		if(!bReturn)
		{
			//DWORD dwErr = WSAGetLastError();
			////AfxMessageBox(L"GetQueuedCompletionStatus���󣡣�:"+strError);
			//PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped); 
			//map<string,CDialog*>::iterator ite=pIOCPModel->m_chatDlg.find(string(inet_ntoa(pIoContext->m_senderIP.sin_addr)));

			//((CEnterpriseChatDlg*)pIOCPModel->m_mainDlg)->removeFriend(pIoContext);
			//if(ite!=pIOCPModel->m_chatDlg.end())
			//{
			//	CChatDlg* chatDlg=(CChatDlg*)(ite->second);
			//	chatDlg->setIsOnline(false);
			//}
			//break;
			//DWORD dwErr = GetLastError();

			//// ��ʾһ����ʾ��Ϣ
			//if( !pIOCPModel->HandleError( pSocketContext,dwErr ) )
			//{
			//	break;
			//}

			//������
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped); 
			if(!pIoContext)
			{
				continue;
			}
			if(pIoContext->m_socket==pIOCPModel->m_UDPModel.GetUDPSocket())
			{
				TCHAR strTemp[MAX_PATH];
				memset(strTemp,0,sizeof(strTemp));
				FriendListItemInfo item;
				int size=MultiByteToWideChar(0,0,inet_ntoa(pIoContext->m_senderAddr.sin_addr),-1,NULL,0);
				MultiByteToWideChar(0,0,inet_ntoa(pIoContext->m_senderAddr.sin_addr),-1,strTemp,size);
				item.m_description=CDuiString(strTemp);
				pIOCPModel->RemoveFriendFromList(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
				pIOCPModel->GetMainDlg()->RemoveFriend(item);
				(pIOCPModel->m_mainDlg)->SetChatDlgEnabledState(inet_ntoa(pIoContext->m_senderAddr.sin_addr),false);

			}
			if(pIoContext->m_socketType==SOCKET_TYPE::TYPE_TCP)
			{
				pIOCPModel->ClientClose(pIoContext);
			}
			continue;
		}
		else
		{
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_Overlapped); 
			if(dwBytesTransfered==0)
			{
				pIOCPModel->ClientClose(pIoContext);
				continue;
			}
			//AfxMessageBox(L"GetQueuedCompletionStatus��������");
			switch (pIoContext->m_OpType)
			{
			case ACCEPT_POSTED:
				pIOCPModel->AcceptTCPConnectCallback(pIoContext,dwBytesTransfered);
				break;
			case RECV_POSTED:
				//UDP����
				if(pIoContext->m_socketType==SOCKET_TYPE::TYPE_UDP)
				{
					pIOCPModel->RecvUDPMessageCallback(pIoContext);
				}
				//TCP����
				else if(pIoContext->m_socketType==SOCKET_TYPE::TYPE_TCP)
				{
					pIOCPModel->RecvTCPMessageCallback(pIoContext,dwBytesTransfered);
				}
				break;
			case SEND_POSTED:
				//UDP����
				//AfxMessageBox(L"������Ϣ��ϣ���");	
				if(pIoContext->m_socketType==SOCKET_TYPE::TYPE_UDP)
				{
					pIOCPModel->SendUDPMessageCallback(pIoContext);
				}
				//TCP����
				else if(pIoContext->m_socketType==SOCKET_TYPE::TYPE_UDP)
				{
					pIOCPModel->SendTCPMessageCallback(pIoContext);
				}
				break;
			case BROADCASE_POST:	
				break;
			default:
				break;
			}
		}
	}
	RELEASE(pParam);
	return 0;
}
/***************************************/


/*************************************************/
/*>>>>>>>>>UDPģ�鴦��<<<<<<<<<<<*/
//���з���UDP��Ϣ
bool CIOCPModel::SendUDPMessage(SOCKADDR_IN addr,UDPDATA data,PER_IO_CONTEXT* pIoContext,bool isBlocking,bool isAdd)
{
	list<UDPSENDDATA>::iterator ite=m_listUDPMSG.begin();
	//UDPDATA�е�m_addr�ṹ���ڴ��������û�Ŀ�ĵ�ַ���ڶಥ�У����ܵ��ĵ�ַһ�������Ƕಥ��ַ�������Ҫ����������������һ���ಥ��
	data.m_addr=addr;
	UDPSENDDATA sendData;
	//������û��Ϣ��������������Ϣ
	if(isBlocking)
	{
		if(ite==m_listUDPMSG.end())
		{
			pIoContext->m_bIsPackageHead=false;
			pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
			pIoContext->m_recvPos=0;
			pIoContext->m_senderAddr=addr;
			pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
			pIoContext->ResetBuffer();
			memcpy(pIoContext->m_szBuffer,&data,sizeof(data));
			pIoContext->m_wsaBuf.len=sizeof(data);
			EnterCriticalSection(&m_UDPCritical);//�����ٽ�����
			if(isAdd)
			{
				sendData.m_addr=addr;
				sendData.m_UDPData=data;
				m_listUDPMSG.push_back(sendData);
			}
			if(!m_UDPModel.SendUDPMessage(pIoContext))
			{
				m_listClientData.pop_back();
				LeaveCriticalSection(&m_UDPCritical);
				return false;
			}
			LeaveCriticalSection(&m_UDPCritical);
			return true;
		}
		//�����л�����Ϣ������뵽��Ϣ�����У��ȴ�����
		else
		{
			if(isAdd)
			{
				sendData.m_addr=addr;
				sendData.m_UDPData=data;
				m_listUDPMSG.push_back(sendData);
			}
			return true;
		}
	}
	else
	{
		pIoContext->m_bIsPackageHead=false;
		pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
		pIoContext->m_recvPos=0;
		pIoContext->m_senderAddr=addr;
		pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
		pIoContext->ResetBuffer();
		memcpy(pIoContext->m_szBuffer,&data,sizeof(data));
		pIoContext->m_wsaBuf.len=sizeof(data);
		if(!m_UDPModel.SendUDPMessage(pIoContext))
		{
			return false;
		}
		return true;
	}
}
//UDP SEND_POSTED�ص�����
bool CIOCPModel::SendUDPMessageCallback(PER_IO_CONTEXT* pIoContext)
{
	EnterCriticalSection(&m_UDPCritical);
	list<UDPSENDDATA>::iterator ite=m_listUDPMSG.begin();
	if(ite==m_listUDPMSG.end())
	{
		LeaveCriticalSection(&m_UDPCritical);
		return true;
	}
	m_listUDPMSG.erase(ite);
	ite=m_listUDPMSG.begin();
	if(ite==m_listUDPMSG.end())
	{
		LeaveCriticalSection(&m_UDPCritical);
		return true;
	}
	LeaveCriticalSection(&m_UDPCritical);
	UDPSENDDATA data=*ite;
	if(!SendUDPMessage(data.m_addr,data.m_UDPData,pIoContext,false,false))
	{
		return false;
	}
	return true;
}
//UDP������Ϣ
bool CIOCPModel::RecvUDPMessage(PER_IO_CONTEXT* pIoContext)
{
	pIoContext->m_bIsPackageHead=false;
	pIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
	pIoContext->m_recvPos=0;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
	pIoContext->ResetBuffer();
	if(!m_UDPModel.RecvUDPMessage(pIoContext))
	{
		return false;
	}
	return true;
}
//UDP������Ϣ�ص�����
bool CIOCPModel::RecvUDPMessageCallback(PER_IO_CONTEXT* pIoContext)
{
	TCHAR strTemp[MAX_PATH]={0};
	char  cTemp[MAX_PATH]={0};
	int size=-1;
	memset(strTemp,0,sizeof(strTemp));
	memset(cTemp,0,sizeof(cTemp));
	UDPDATA *data=(UDPDATA*)pIoContext->m_szBuffer;
	SOCKADDR_IN addr=pIoContext->m_senderAddr;
	UDPDATA sendData;
	GROUPINFO* groupInfo=NULL;
	FriendListItemInfo friendInfo; //������Ϣ����Ҫ������������Ϣ
	string strIP; //Զ�˷��ͷ�IP
	string aimIP; //Զ�˷��ͷ�����Ŀ��IP����Ҫ���ڶಥ������������ͬ�Ķಥ��
	switch (data->m_msgType)
	{
		//������Ϣ
	case UDPMSGTYPE::LOGON:
		if(false==IsExsitedInFriendsList(pIoContext->m_senderAddr))
		{
			//���û���Ӻ����б��ط���Ϣ
			strIP=string(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
			m_listFriens.push_back(strIP);

			friendInfo.m_folder=false;
			friendInfo.m_empty=false;
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_image,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_image,-1,strTemp,size);
			friendInfo.m_logo=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_name,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_name,-1,strTemp,size);
			friendInfo.m_nick_name=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,strIP.c_str(),-1,NULL,0);
			MultiByteToWideChar(0,0,strIP.c_str(),-1,strTemp,size);
			friendInfo.m_description=CDuiString(strTemp);

			m_mainDlg->AddNewFriend(friendInfo);

			sendData.m_msgType=UDPMSGTYPE::LOGON;
			memcpy(sendData.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
			memcpy(sendData.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
			memcpy(sendData.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));
			SendUDPMessage(addr,sendData,pIoContext);
		}
		break;
	case UDPMSGTYPE::LOGOFF:
		//������Ϣ
		if(true==IsExsitedInFriendsList(pIoContext->m_senderAddr))
		{
			strIP=string(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
			for(list<string>::iterator ite=m_listFriens.begin();ite!=m_listFriens.end();++ite)
			{
				if(*ite==strIP)
				{
					m_listFriens.remove(strIP);
				}
			}

			friendInfo.m_folder=false;
			friendInfo.m_empty=false;
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_image,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_image,-1,strTemp,size);
			friendInfo.m_logo=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_name,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_name,-1,strTemp,size);
			friendInfo.m_nick_name=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,strIP.c_str(),-1,NULL,0);
			MultiByteToWideChar(0,0,strIP.c_str(),-1,strTemp,size);
			friendInfo.m_description=CDuiString(strTemp);

			m_mainDlg->RemoveFriend(friendInfo);
		}
		break;
	case UDPMSGTYPE::COMMONPOINT:
		if(false==IsExsitedInFriendsList(pIoContext->m_senderAddr))
		{
			strIP=string(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
			m_listFriens.push_back(strIP);

			friendInfo.m_folder=false;
			friendInfo.m_empty=false;
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_image,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_image,-1,strTemp,size);
			friendInfo.m_logo=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_name,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_name,-1,strTemp,size);
			friendInfo.m_nick_name=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,strIP.c_str(),-1,NULL,0);
			MultiByteToWideChar(0,0,strIP.c_str(),-1,strTemp,size);
			friendInfo.m_description=CDuiString(strTemp);

			m_mainDlg->AddNewFriend(friendInfo);

			sendData.m_msgType=UDPMSGTYPE::LOGON;
			memcpy(sendData.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
			memcpy(sendData.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
			memcpy(sendData.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));
			SendUDPMessage(addr,sendData,pIoContext);
		}
		m_mainDlg->RecvUDPMessage(inet_ntoa(addr.sin_addr),*data);
		//��ͨ��Ϣ��Ե���Ϣ
		break;
	case UDPMSGTYPE::COMMONMULTICAST:
		if(false==IsExsitedInFriendsList(pIoContext->m_senderAddr))
		{
			strIP=string(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
			m_listFriens.push_back(strIP);

			friendInfo.m_folder=false;
			friendInfo.m_empty=false;
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_image,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_image,-1,strTemp,size);
			friendInfo.m_logo=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,data->m_name,-1,NULL,0);
			MultiByteToWideChar(0,0,data->m_name,-1,strTemp,size);
			friendInfo.m_nick_name=CDuiString(strTemp);
			memset(strTemp,0,sizeof(strTemp));
			size=MultiByteToWideChar(0,0,strIP.c_str(),-1,NULL,0);
			MultiByteToWideChar(0,0,strIP.c_str(),-1,strTemp,size);
			friendInfo.m_description=CDuiString(strTemp);

			m_mainDlg->AddNewFriend(friendInfo);

			sendData.m_msgType=UDPMSGTYPE::LOGON;
			memcpy(sendData.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
			memcpy(sendData.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
			memcpy(sendData.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));
			SendUDPMessage(addr,sendData,pIoContext);
		}
		m_mainDlg->RecvUDPMessage(inet_ntoa(data->m_addr.sin_addr),*data);
		//��ͨ�ಥ��Ϣ
		break;
	case UDPMSGTYPE::NAT:
		//��͸��Ϣ
		break;
	case UDPMSGTYPE::MULTICAST_CREATE:
		//Ⱥ������Ϣ
		groupInfo=(GROUPINFO*)(data->m_message);
		if(groupInfo==NULL)
		{
			break;
		}
		JoinGroup(groupInfo->m_builder,groupInfo->m_groupIP,groupInfo->m_groupName,groupInfo->m_groupImage);
		break;
	case UDPMSGTYPE::MULTICAST_INVITE:
		memset(strTemp,0,sizeof(strTemp));
		size=MultiByteToWideChar(0,0,GROUPINI_PATH,-1,NULL,0);
		MultiByteToWideChar(0,0,GROUPINI_PATH,-1,strTemp,size);
		groupInfo=(GROUPINFO*)(data->m_message);
		if(groupInfo==NULL)
		{
			break;
		}
		if(GetGroupInfo(groupInfo->m_groupIP)==0)
		{
			JoinGroup(groupInfo->m_builder,groupInfo->m_groupIP,groupInfo->m_groupName,groupInfo->m_groupImage);
		}
		SetMember(groupInfo->m_groupIP,true,strTemp);
		//������Ⱥ��Ϣ
		break;
	case UDPMSGTYPE::MULTICAST_BANISH:
		groupInfo=(GROUPINFO*)(data->m_message);
		if(groupInfo==NULL)
		{
			break;
		}
		memset(cTemp,0,sizeof(cTemp));
		size=WideCharToMultiByte(0,0,groupInfo->m_groupIP,-1,NULL,0,NULL,NULL);
		WideCharToMultiByte(0,0,groupInfo->m_groupIP,-1,cTemp,size,NULL,NULL);
		sendData.m_addr.sin_family=AF_INET;
		sendData.m_addr.sin_addr.S_un.S_addr=inet_addr(cTemp);
		sendData.m_addr.sin_port=htons(USER_PORT);
		addr=sendData.m_addr;
		memcpy(sendData.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
		memcpy(sendData.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
		memcpy(sendData.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));
		memcpy(sendData.m_message,groupInfo,sizeof(*groupInfo));
		sendData.m_msgType=UDPMSGTYPE::MULTICAST_QUIT;
		SendUDPMessage(addr,sendData,pIoContext);
		QuitGroup(groupInfo->m_groupIP);
		//���߳�Ⱥ��Ϣ
		break;
	case UDPMSGTYPE::MULTICAST_JOIN:
		//����Ⱥ��Ϣ
		break;
	case UDPMSGTYPE::MULTICAST_QUIT:
		//�˳�Ⱥ��Ϣ
		break;
	case UDPMSGTYPE::MULTICAST_SETMANAGER:
		//����ΪȺ����Ա
		break;
	case UDPMSGTYPE::MULTICAST_CACELMANAGER:
		//ȡ��Ⱥ����Ա���
		break;
	default:
		break;
	}
	RecvUDPMessage(pIoContext);
	return true;
}
//UDP����������Ϣ
bool CIOCPModel::SendUDPLogOnMessage(PER_IO_CONTEXT* pIoContext)
{
	UDPDATA data;
	memset(&data,0,sizeof(data));
	data.m_msgType=UDPMSGTYPE::LOGON;
	memcpy(data.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
	memcpy(data.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
	memcpy(data.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));

	SOCKADDR_IN mutlicastAddr;
	memset(&mutlicastAddr,0,sizeof(mutlicastAddr));
	mutlicastAddr.sin_family=AF_INET;
	mutlicastAddr.sin_addr.S_un.S_addr=inet_addr(LOGONOFF_MUTLICAST_ADDR);
	mutlicastAddr.sin_port=htons(USER_PORT);

	OVERLAPPED* p_ol=&pIoContext->m_Overlapped;
	pIoContext->m_bIsPackageHead=false;
	pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
	pIoContext->m_recvPos=0;
	pIoContext->m_senderAddr=mutlicastAddr;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
	pIoContext->ResetBuffer();
	memcpy(pIoContext->m_szBuffer,&data,sizeof(data));
	if(!SendUDPMessage(mutlicastAddr,data,pIoContext))
	{
		return false;
	}
	return true;
}
//UDP����������Ϣ
bool CIOCPModel::SendUDPLogOffMessage(PER_IO_CONTEXT* pIoContext)
{

	UDPDATA data;
	memset(&data,0,sizeof(data));
	data.m_msgType=UDPMSGTYPE::LOGOFF;
	memcpy(data.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
	memcpy(data.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
	memcpy(data.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));

	SOCKADDR_IN mutlicastAddr;
	memset(&mutlicastAddr,0,sizeof(mutlicastAddr));
	mutlicastAddr.sin_family=AF_INET;
	mutlicastAddr.sin_addr.S_un.S_addr=inet_addr(LOGONOFF_MUTLICAST_ADDR);
	mutlicastAddr.sin_port=htons(USER_PORT);

	OVERLAPPED* p_ol=&pIoContext->m_Overlapped;
	pIoContext->m_bIsPackageHead=false;
	pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
	pIoContext->m_recvPos=0;
	pIoContext->m_senderAddr=mutlicastAddr;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_UDP;
	pIoContext->ResetBuffer();
	memcpy(pIoContext->m_szBuffer,&data,sizeof(data));
	if(!SendUDPMessage(mutlicastAddr,data,pIoContext))
	{
		return false;
	}
	return true;
}


/***********************************************/
/*>>>>>>>>TCPģ�鴦��<<<<<<<<<<<<<*/
//Ͷ��TCP AcceptEx����
bool CIOCPModel::AcceptTCPConnect(PER_IO_CONTEXT* pIoContext)
{
	if(!m_TCPModel.AcceptTCPConnect(pIoContext))
	{
		m_listListenData.remove(pIoContext);
		RELEASE(pIoContext);
		return false;
	}
	return true;
}
//��������
bool CIOCPModel::AcceptTCPConnectCallback(PER_IO_CONTEXT* pIoContext,ULONG dwIoSize)
{
	//���ܿͻ������ӣ�����ȡ��һ����Ϣ
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	m_TCPModel.m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2),  
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&ClientAddr, &remoteLen);  

	//AfxMessageBox( _T("�ͻ�������"));
	char *info=pIoContext->m_wsaBuf.buf;
	PACKAGE_HEAD* data=(PACKAGE_HEAD*)info;

	PER_IO_CONTEXT* pNewIoContext = new PER_IO_CONTEXT;
	pNewIoContext->m_socket           = pIoContext->m_socket;
	memcpy(&(pNewIoContext->m_senderAddr), ClientAddr, sizeof(SOCKADDR_IN));
	memcpy(pNewIoContext->m_szBuffer,pIoContext->m_wsaBuf.buf,sizeof(pIoContext->m_wsaBuf.buf));

	//Ϊÿ���Ǽ���tcp�׽����䱸���ͺͽ���ͨ�����ݽṹ
	PER_IO_CONTEXT* pSendIoContext = new PER_IO_CONTEXT;
	pSendIoContext->m_socket=pIoContext->m_socket;
	memcpy(&(pSendIoContext->m_senderAddr), ClientAddr, sizeof(SOCKADDR_IN));
	//memcpy(pIoContext->m_wsaBuf.buf,pNewIoContext->m_szBuffer,sizeof(pIoContext->m_wsaBuf.buf));

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	if( false==this->SocketAssociateWithIOCP( pNewIoContext->m_socket ) )
	{
		RELEASE( pNewIoContext );
		pIoContext->ResetBuffer();
		this->AcceptTCPConnect( pIoContext ); 	
		return false;
	} 

	m_listClientData.push_back(pNewIoContext);
	m_listClientData.push_back(pSendIoContext);

	//�жϽ��յ��İ�ͷ�����Ƿ��㹻
	if(dwIoSize>=pIoContext->m_wsaBuf.len)
	{
		pNewIoContext->m_bIsPackageHead=false;
		pNewIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
		pNewIoContext->m_recvPos+=dwIoSize;
		pNewIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
		pNewIoContext->m_wsaBuf.buf=pNewIoContext->m_szBuffer+pNewIoContext->m_recvPos;
		pNewIoContext->m_wsaBuf.len=data->m_dataLen;
	}
	else
	{
		pNewIoContext->m_bIsPackageHead=true;
		pNewIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
		pNewIoContext->m_recvPos+=dwIoSize;
		pNewIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
		pNewIoContext->m_wsaBuf.buf=pNewIoContext->m_szBuffer+pNewIoContext->m_recvPos;
		pNewIoContext->m_wsaBuf.len-=dwIoSize;
	}
	if(false==RecvTCPMessage(pNewIoContext))
	{
		m_listClientData.remove(pNewIoContext);
		m_listClientData.remove(pSendIoContext);
		RELEASE(pSendIoContext);
		RELEASE(pNewIoContext);
		return false;
	}
	if(!this->AcceptTCPConnect( pIoContext ))
	{
		return false;
	}
	return true;
}
//Ͷ��TCP WSARecv����
bool CIOCPModel::RecvTCPMessage(PER_IO_CONTEXT* pIoContext)
{
	DWORD dwBytes=0;
	DWORD flags=0;
	WSABUF *p_wbuf=&pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol=&pIoContext->m_Overlapped;
	int nBytesRecv =WSARecv(pIoContext->m_socket,p_wbuf,1,&dwBytes,&flags,p_ol,NULL);
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		shutdown(pIoContext->m_socket,SD_BOTH);
		closesocket(pIoContext->m_socket);
		m_listClientData.remove(pIoContext);
		RELEASE(pIoContext);
		return false;
	}
	return true;
}
//���յ��ͻ�����Ϣ
bool CIOCPModel::RecvTCPMessageCallback(PER_IO_CONTEXT* pIoContext,ULONG dwIoSize)
{
	//�ѶϿ�����
	if(0==dwIoSize)
	{
		return ClientClose(pIoContext);
	}
	//���յ��㹻�ĳ���
	else if(dwIoSize==pIoContext->m_wsaBuf.len)
	{
		if(true==pIoContext->m_bIsPackageHead)//����ǰ�ͷ����Ͷ�ݽ��ܰ������
		{
			PACKAGE_HEAD *pPackageHead = (PACKAGE_HEAD *)(pIoContext->m_szBuffer);
			pIoContext->m_bIsPackageHead=false;
			pIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
			pIoContext->m_recvPos+=dwIoSize;
			pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
			pIoContext->m_wsaBuf.buf=pIoContext->m_szBuffer+pIoContext->m_recvPos;
			pIoContext->m_wsaBuf.len=pPackageHead->m_dataLen;
		}
		else//����ǰ��壬���ʾ���յ������İ���������壬Ͷ����һ����ͷ����
		{
			if(false==IsExsitedInFriendsList(pIoContext->m_senderAddr))
			{
				FriendListItemInfo friendInfo;
				memset(&friendInfo,0,sizeof(friendInfo));
				string strIP=string(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
				m_listFriens.push_back(strIP);

				TCHAR strTemp[MAX_PATH]={0};
				memset(strTemp,0,sizeof(strTemp));
				int size=-1;
				friendInfo.m_folder=false;
				friendInfo.m_empty=false;
				friendInfo.m_logo=CDuiString(_T("default.png"));
				friendInfo.m_nick_name=CDuiString(_T("Default"));
				memset(strTemp,0,sizeof(strTemp));
				size=MultiByteToWideChar(0,0,strIP.c_str(),-1,NULL,0);
				MultiByteToWideChar(0,0,strIP.c_str(),-1,strTemp,size);
				friendInfo.m_description=CDuiString(strTemp);

				m_mainDlg->AddNewFriend(friendInfo);

				SOCKADDR_IN addr;
				memset(&addr,0,sizeof(addr));
				addr.sin_family=AF_INET;
				addr.sin_addr.S_un.S_addr=inet_addr(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
				addr.sin_port=htons(USER_PORT);
				UDPDATA sendData;
				memset(&sendData,0,sizeof(sendData));
				sendData.m_msgType=UDPMSGTYPE::LOGON;
				memcpy(sendData.m_hostName,m_hostName.c_str(),(m_hostName.size()+1)*sizeof(char));
				memcpy(sendData.m_name,m_name.c_str(),(m_name.size()+1)*sizeof(char));
				memcpy(sendData.m_image,m_image.c_str(),(m_image.size()+1)*sizeof(char));
				SendUDPMessage(addr,sendData,pIoContext);
			}
			m_mainDlg->RecvTCPMessage(pIoContext);
			if(pIoContext==NULL)
			{
				return false;
			}
			pIoContext->ResetBuffer();
			pIoContext->m_bIsPackageHead=true;
			pIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
			pIoContext->m_recvPos=0;
			pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
			pIoContext->m_wsaBuf.buf=pIoContext->m_szBuffer+pIoContext->m_recvPos;
			pIoContext->m_wsaBuf.len=PACKAGE_HEAD_LEN;
		}
	}
	else
	{
		pIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
		pIoContext->m_recvPos+=dwIoSize;
		pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
		pIoContext->m_wsaBuf.buf=pIoContext->m_szBuffer+pIoContext->m_recvPos;
		pIoContext->m_wsaBuf.len-=dwIoSize;
	}
	if(!RecvTCPMessage(pIoContext))
	{
		return false;
	}
	return true;
}
//Ͷ��TCP WSASend����
bool CIOCPModel::SendTCPMessage(TCPSENDDATA sendData,PER_IO_CONTEXT* pIoContext,bool isBlocking,bool isAdd)
{
	list<TCPSENDDATA>::iterator ite=isExsitTCPSendData(pIoContext->m_socket);
	DWORD sendBytes=0;
	OVERLAPPED* p_ol=&pIoContext->m_Overlapped;
	if(isBlocking)
	{
		if(ite==m_listTCPSendData.end())
		{
			pIoContext->ResetBuffer();
			memcpy(pIoContext->m_szBuffer,sendData.m_msgBuff,sizeof(sendData.m_msgBuff));
			pIoContext->m_bIsPackageHead=false;
			pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
			pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
			pIoContext->m_wsaBuf.buf=pIoContext->m_szBuffer;
			pIoContext->m_wsaBuf.len=sizeof(sendData.m_msgBuff);
			int ret=WSASend(pIoContext->m_socket,&pIoContext->m_wsaBuf,1,&sendBytes,0,p_ol,NULL);
			if(SOCKET_ERROR==ret&&WSA_IO_PENDING!=WSAGetLastError())
			{
				return false;
			}
			if(isAdd)
			{
				m_listTCPSendData.push_back(sendData);
			}
			return true;
		}
		else
		{
			if(isAdd)
			{
				m_listTCPSendData.push_back(sendData);
			}
			return true;
		}
	}
	else
	{
		pIoContext->ResetBuffer();
		memcpy(pIoContext->m_szBuffer,sendData.m_msgBuff,sizeof(sendData.m_msgBuff));
		pIoContext->m_bIsPackageHead=false;
		pIoContext->m_OpType=OPERATION_TYPE::SEND_POSTED;
		pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
		pIoContext->m_wsaBuf.buf=pIoContext->m_szBuffer;
		pIoContext->m_wsaBuf.len=sizeof(sendData.m_msgBuff);
		int ret=WSASend(pIoContext->m_socket,&pIoContext->m_wsaBuf,1,&sendBytes,0,p_ol,NULL);
		if(SOCKET_ERROR==ret&&WSA_IO_PENDING!=WSAGetLastError())
		{
			return false;
		}
		return true;
	}
}
//TCP������Ϣ�ص�����
bool CIOCPModel::SendTCPMessageCallback(PER_IO_CONTEXT* pIoContext)
{
	list<TCPSENDDATA>::iterator ite=isExsitTCPSendData(pIoContext->m_socket);
	if(ite==m_listTCPSendData.end())
	{
		return true;
	}
	m_listTCPSendData.erase(ite);
	ite=isExsitTCPSendData(pIoContext->m_socket);
	if(ite==m_listTCPSendData.end())
	{
		return true;
	}
	TCPSENDDATA sendData=*ite;
	if(!SendTCPMessage(sendData,pIoContext,false,false))
	{
		return false;
	}
	return true;
}
//connect�ڲ�����
bool CIOCPModel::ConnectServer(PER_IO_CONTEXT* pIoContext)
{
	if(pIoContext==NULL)
	{
		return false;
	}
	if(0!=connect(pIoContext->m_socket,(SOCKADDR*)(&(pIoContext->m_senderAddr)),sizeof(pIoContext->m_senderAddr)))
	{
		return false;
	}
	if(NULL==CreateIoCompletionPort((HANDLE)(pIoContext->m_socket),m_IOCompletionPort,(ULONG_PTR)(pIoContext->m_socket),0))
	{
		return false;
	}
	return true;
}
//connect�ⲿ�ӿ�
bool CIOCPModel::ConnectServer(LPCTSTR fileName,LPCTSTR ip)
{
	int realLen=0;
	FILE* fileHandle=NULL;
	int size=WideCharToMultiByte(0,0,fileName,-1,NULL,0,NULL,NULL);
	char *strName=new char[size];
	WideCharToMultiByte(0,0,fileName,-1,strName,size,NULL,NULL);
	fopen_s(&fileHandle,strName,"rb");
	if(fileHandle==NULL)
	{
		RELEASE(strName);
		return false;
	}

	FILEINFO* fileInfo=new FILEINFO;
	fileInfo->m_fileLen=realLen;
	fileInfo->m_finishLen=0;
	memcpy(fileInfo->m_realName,strName,size);
	fileInfo->m_type=FILEOPERATIONTYPE::FILE_OPE_SEND;

	RELEASE(strName);
	fseek(fileHandle,0,SEEK_END);
	realLen=ftell(fileHandle);
	fclose(fileHandle);

	size=WideCharToMultiByte(0,0,ip,-1,NULL,0,NULL,NULL);
	char *strIP=new char[size];
	WideCharToMultiByte(0,0,ip,-1,strIP,size,NULL,NULL);

	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr(strIP);
	addr.sin_port=htons(USER_PORT);
	RELEASE(strIP);

	SOCKET sendSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
	if(sendSocket==INVALID_SOCKET)
	{
		RELEASE(fileInfo);
		return false;
	}
	PER_IO_CONTEXT* pIoContext=new PER_IO_CONTEXT;
	pIoContext->m_bIsPackageHead=false;
	pIoContext->m_OpType=OPERATION_TYPE::NULL_POSTED;
	pIoContext->m_recvPos=0;
	pIoContext->m_senderAddr=addr;
	pIoContext->m_socket=sendSocket;
	pIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;

	if(false==ConnectServer(pIoContext))
	{
		ClientClose(pIoContext);
		RELEASE(pIoContext);
		RELEASE(fileInfo);
		return false;
	}

	TCPSENDDATA sendData;
	sendData.m_socket=sendSocket;

	TCPDATA data;
	memset(&data,0,sizeof(data));
	data.m_fileLen=realLen;
	memcpy(data.m_fileName,fileInfo->m_realName,sizeof(fileInfo->m_realName));
	data.m_type=TCPMSGTYPE::FILE_SEND;

	PACKAGE_HEAD head;
	memset(&head,0,sizeof(head));
	head.m_command=TCP_HEAD_TYPE::TCP_MSG;
	head.m_dataLen=sizeof(data);

	memcpy(sendData.m_msgBuff,&head,sizeof(head));
	memcpy(sendData.m_msgBuff+sizeof(head),&data,sizeof(data));

	PER_IO_CONTEXT *pRecvIoContext=new PER_IO_CONTEXT;
	pRecvIoContext->m_bIsPackageHead=true;
	pRecvIoContext->m_OpType=OPERATION_TYPE::RECV_POSTED;
	pRecvIoContext->m_recvPos=0;
	pRecvIoContext->m_senderAddr=addr;
	pRecvIoContext->m_socket=sendSocket;
	pRecvIoContext->m_socketType=SOCKET_TYPE::TYPE_TCP;
	pRecvIoContext->m_wsaBuf.len=sizeof(PACKAGE_HEAD);

	if(SendTCPMessage(sendData,pIoContext)==false)
	{
		ClientClose(pIoContext);
		RELEASE(fileInfo);
		RELEASE(pIoContext);
		RELEASE(pRecvIoContext);
		return false;
	}

	if(RecvTCPMessage(pRecvIoContext)==false)
	{
		ClientClose(pIoContext);
		ClientClose(pIoContext);
		RELEASE(fileInfo);
		RELEASE(pIoContext);
		RELEASE(pRecvIoContext);
	}

	fileInfo->m_pIoContext=pRecvIoContext;

	m_listClientData.push_back(pIoContext);
	m_listClientData.push_back(pRecvIoContext);
	m_mainDlg->AddFileInfo(fileInfo);
	return true;

}
//�ͻ��˶Ͽ�TCP����ʱ���ر�ͨ���׽���
bool CIOCPModel::ClientClose(PER_IO_CONTEXT* pIoContext)
{
	if(pIoContext==NULL)
	{
		return false;
	}
	SOCKET tempSocket=pIoContext->m_socket;
	//ɾ���仹δ���͵�����
	for(list<TCPSENDDATA>::iterator ite=m_listTCPSendData.begin();ite!=m_listTCPSendData.end();)
	{
		if(ite->m_socket==tempSocket)
		{
			m_listTCPSendData.erase(ite);
			ite=m_listTCPSendData.begin();
			continue;
		}
		++ite;
	}
	//�ر��׽���
	shutdown(pIoContext->m_socket,SD_BOTH);
	closesocket(pIoContext->m_socket);
	//ɾ�����ݽṹ
	for(list<PER_IO_CONTEXT*>::iterator ite=m_listClientData.begin();ite!=m_listClientData.end();)
	{
		if((*ite)->m_socket==tempSocket)
		{
			m_listClientData.erase(ite);
			RELEASE(*ite);
			ite=m_listClientData.begin();
			continue;
		}
		++ite;
	}
	return true;
}
//
PER_IO_CONTEXT* CIOCPModel::GetIoContextBySocket(SOCKET tempSocket,OPERATION_TYPE type)
{
	PER_IO_CONTEXT* pIoContext=NULL;
	for(list<PER_IO_CONTEXT*>::iterator ite=m_listClientData.begin();ite!=m_listClientData.end();++ite)
	{
		if((*ite)->m_socket==tempSocket&&(*ite)->m_OpType==type)
		{
			pIoContext=*ite;
			break;
		}
	}
	return pIoContext;
}

