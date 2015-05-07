#include "main_frame.h"
#include "popmenu.h"
#include "resource.h"
#include "commonlist.h"

#define COMMON_BG _T("bg")

#define PC_MAIN_IMAGE _T("pctojen")
#define PC_MAIN_NAME  _T("pcname")

#define PC_MAIN_SEARCH_EDIT _T("search_edit")
#define PC_MAIN_SERACH_TIP  _T("search_tip")
#define PC_MAIN_SERACH_BTN	_T("searchbtn")

#define LIST_FRIEND  _T("friends")
#define LIST_GROUP   _T("groups")

#define BTN_IMAGE    _T("pctojen")
#define BTN_CLOSE    _T("closebtn")

#define FRIEND_LOGO         _T("logo")
#define FRIEND_NAME         _T("nickname")
#define FRIEND_DESCRIPTION  _T("description")


const TCHAR* const SEARCHTITLE=_T("搜索联系人，群组");

using namespace DuiLib;

main_frame::main_frame(void)
	:m_friendParentNode(NULL)
	,m_groupParentNode(NULL)
{
	m_mapChatDlg.clear();
	m_listFileInfo.clear();
}

main_frame::~main_frame(void)
{

}

LPCTSTR main_frame::GetWindowClassName() const
{
	return _T("EChatGUIClass");
}

CDuiString main_frame::GetSkinFile()
{
	return _T("main_frame.xml");
}

CDuiString main_frame::GetSkinFolder()
{
	return _T("");
}

void main_frame::InitWindow()
{
	//调整主窗口到合适位置，适用于各种分辨率
	RECT rect;
	int cx=::GetSystemMetrics( SM_CXSCREEN );
	int cy=::GetSystemMetrics( SM_CYSCREEN );
	::GetWindowRect(this->GetHWND(),&rect);
	::SetWindowPos(this->GetHWND(),NULL,cx-(rect.right-rect.left)-50,(cy/2-(rect.bottom-rect.top)/2)/2,rect.right-rect.left,rect.bottom-rect.top,0);
	SetIcon(IDI_ICONMAINFRAME);
	//启动核心通信模块
	bool bRet=false;
	m_IOCP.SetMainDlg(this);
	bRet=m_IOCP.Start();

	AddNotificationIcon(GetHWND());

	InitListFloder();

	FriendListItemInfo item;
	item.m_folder=false;
	item.m_empty=false;
	item.m_logo=_T("man_big.png");
	item.m_description=_T("123456789");
	item.m_nick_name=_T("123456789");
	AddNewFriend(item);

	
	item.m_logo=_T("default.png");
	item.m_nick_name=_T("duilib群组");
	item.m_description=_T("234.225.0.4");
	AddNewGroup(item);
}

void main_frame::Notify(TNotifyUI& msg)
{
	CDuiString sendName=msg.pSender->GetName();
	//tabs选项改变消息
	if(_tcsicmp(msg.sType,_T("selectchanged"))==0)
	{
		CTabLayoutUI* pControl=static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tabs")));
		if(_tcsicmp(sendName,_T("friendbtn"))==0)
		{
			pControl->SelectItem(0);
			return;
		}
		else if(_tcsicmp(sendName,_T("groupbtn"))==0)
		{
			pControl->SelectItem(1);
			return;
		}
		else 
		{
			return;
		}
	}
	//点击消息
	else if(_tcsicmp(msg.sType,_T("click"))==0)
	{
		if(_tcsicmp(sendName,PC_MAIN_SERACH_TIP)==0)
		{
			msg.pSender->SetVisible(false);
			CRichEditUI* pControl=static_cast<CRichEditUI*>(m_PaintManager.FindControl(PC_MAIN_SEARCH_EDIT));
			pControl->SetVisible();
			pControl->SetText(L"");
			pControl->SetFocus();
			return;
		}
		if(_tcsicmp(sendName,BTN_CLOSE)==0)
		{
			WndClosing();
			return;
		}
	}
	//killfocus消息
	else if(_tcsicmp(msg.sType,_T("killfocus"))==0)
	{
		if(_tcsicmp(sendName,PC_MAIN_SEARCH_EDIT)==0)
		{
			msg.pSender->SetVisible(false);
			CRichEditUI* pControl=static_cast<CRichEditUI*>(m_PaintManager.FindControl(PC_MAIN_SERACH_TIP));
			pControl->SetVisible();
			pControl->SetText(SEARCHTITLE);
			return;
		}
	}
	//ListUI左键消息
	else if(_tcsicmp(msg.sType,_T("itemlbclick"))==0)
	{
		commonlist* pControl=static_cast<commonlist*>(GetGoalCtrl(msg.pSender,_T("ListUI")));
		CListContainerElementUI* pElement=NULL;
		Node *node=NULL;
		if(pControl)
		{
			pElement=static_cast<CListContainerElementUI*>(GetGoalCtrl(msg.pSender,_T("ListContainerElementUI")));
			if(!pElement)
			{
				return;
			}
			int index=pControl->GetCurSel();
			node=(Node*)(pElement->GetTag());
			if(!node)
			{
				return;
			}
			if(node->data().m_folder)
			{
				pControl->SetChildVisible(node,!(node->data().m_child_visible));
			}
		}
	}
	//ListUI右键消息处理
	else if(_tcsicmp(msg.sType,_T("itemrbclick"))==0)
	{
		CListUI* pControl=static_cast<CListUI*>(GetGoalCtrl(msg.pSender,_T("ListUI")));
		CListContainerElementUI* pElement=NULL;
		Node* node=NULL;
		if(pControl&&_tcsicmp(pControl->GetName(),LIST_FRIEND)==0)
		{
			int index=pControl->GetCurSel();
			pElement=static_cast<CListContainerElementUI*>(pControl->GetItemAt(index));
			if(!pElement)
			{
				return;
			}
			node=(Node*)(pElement->GetTag());
			if(node->data().m_folder==false)
			{
				POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
				popmenu *pMenu = new popmenu(_T("menu.xml"));

				pMenu->SetMainDlg(this);
				pMenu->Init(*this, pt);
				pMenu->ShowWindow(TRUE);
				return;
			}
			else
			{
				return;
			}
		}
		else if(pControl&&_tcsicmp(pControl->GetName(),LIST_GROUP)==0)
		{
			int index=pControl->GetCurSel();
			pElement=static_cast<CListContainerElementUI*>(pControl->GetItemAt(index));
			if(!pElement)
			{
				return;
			}
			node=(Node*)(pElement->GetTag());
			if(node->data().m_folder==false)
			{
				POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
				popmenu *pMenu = new popmenu(_T("groupmenu.xml"));

				pMenu->SetMainDlg(this);
				pMenu->Init(*this, pt);
				pMenu->ShowWindow(TRUE);
				if(index<0)
				{
					pMenu->SetItemEnable(false);
				}
				else
				{
					pMenu->SetItemEnable(true);
				}
			}
			else
			{
				POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
				popmenu *pMenu = new popmenu(_T("groupmenu.xml"));

				pMenu->SetMainDlg(this);
				pMenu->Init(*this, pt);
				pMenu->ShowWindow(TRUE);
				pMenu->SetItemEnable(false);
			}
			return;

		}
	}
	//按钮右键消息
	else if(_tcsicmp(msg.sType,_T("rbclick"))==0)
	{
		CDuiString sendName=msg.pSender->GetName();
		if(_tcsicmp(sendName,BTN_IMAGE)==0)
		{
			POINT pt = {msg.ptMouse.x, msg.ptMouse.y};
			popmenu *pMenu = new popmenu(_T("changeinfomenu.xml"));

			pMenu->SetMainDlg(this);
			pMenu->Init(*this, pt);
			pMenu->ShowWindow(TRUE);
		}
	}
	//双击消息处理
	else if(_tcsicmp(msg.sType,_T("dbclick"))==0)
	{
		CListUI* parent=static_cast<CListUI*>(GetGoalCtrl(msg.pSender,_T("ListUI")));
		if(!parent)
		{
			return;
		}
		CListUI* pFriendControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_FRIEND));
		CListUI* pGroupControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_GROUP));
		int index=-1;
		CListContainerElementUI *pElement=NULL;
		index=parent->GetCurSel();
		pElement=static_cast<CListContainerElementUI*>(parent->GetItemAt(index));
		if(!pElement)
		{
			return;
		}
		Node* node=(Node*)(pElement->GetTag());
		if(!node)
		{
			return;
		}
		if(node->data().m_folder)
		{
			return;
		}
		if(pFriendControl==NULL||pGroupControl==NULL)
		{
			return;
		}
		if(parent==pFriendControl)
		{
			AddNewChatDlg();
			//AddNewChatDlg(WND_POINT,pFriendControl);
			return;
		}
		else if(parent==pGroupControl)
		{
			AddNewChatDlg(WND_MUTLICAST,pGroupControl);
			return;
		}
		else
		{
			return;
		}
	}


	/*if(_tcsicmp(sendName,L"audiomsg")==0)
	{
		OutputDebugString(msg.sType+L"\r\n");
		return;
	}*/
	__super::Notify(msg);
}

bool main_frame::AddNewFriend(FriendListItemInfo friendInfo)
{
	/*TCHAR str[MAX_PATH*2]={0};
	memset(str,0,sizeof(str));
	CListUI*  pControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_FRIEND));
	if(pControl==NULL)
	{
	return false;
	}
	CListContainerElementUI* pListElement=NULL;
	if( !m_dlgBuilder.GetMarkup()->IsValid())
	{
	pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("friend_list_item.xml"), (UINT)0, NULL, &m_PaintManager));
	}
	else 
	{
	pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &m_PaintManager));
	}
	if(pListElement==NULL)
	{
	return false;
	}
	pListElement->SetVisible();
	pControl->Add(pListElement);

	int size=MultiByteToWideChar(0,0,friendInfo.m_logo,-1,NULL,0);
	MultiByteToWideChar(0,0,friendInfo.m_logo,-1,str,size);
	CButtonUI* pBtn=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pListElement,FRIEND_LOGO));
	pBtn->SetNormalImage(str);

	memset(str,0,sizeof(str));
	size=MultiByteToWideChar(0,0,friendInfo.m_name,-1,NULL,0);
	MultiByteToWideChar(0,0,friendInfo.m_name,-1,str,size);
	CLabelUI* pLabel1=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pListElement,FRIEND_NAME));
	pLabel1->SetText(str);

	memset(str,0,sizeof(str));
	size=MultiByteToWideChar(0,0,friendInfo.m_ip,-1,NULL,0);
	MultiByteToWideChar(0,0,friendInfo.m_ip,-1,str,size);
	CLabelUI* pLabel2=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pListElement,FRIEND_DESCRIPTION));
	pLabel2->SetText(str);

	pListElement->SetFixedHeight(32);*/
	commonlist *pControl=static_cast<commonlist*>(m_PaintManager.FindControl(LIST_FRIEND));
	if(pControl->AddNode(friendInfo,m_friendParentNode))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool main_frame::AddNewGroup(FriendListItemInfo friendInfo)
{
	commonlist *pControl=static_cast<commonlist*>(m_PaintManager.FindControl(LIST_GROUP));
	if(pControl->AddNode(friendInfo,m_groupParentNode))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int main_frame::GetFriendIndex(FriendListItemInfo friendInfo)
{
	int index=-1;
	CListUI*  pControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_FRIEND));
	for(int i=0;i<pControl->GetCount();++i)
	{
		CListContainerElementUI* pChild=static_cast<CListContainerElementUI*>(pControl->GetItemAt(i));
		if(pChild==NULL)
		{
			continue;
		}
		CLabelUI* pLabel=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pChild,FRIEND_DESCRIPTION));
		if(pLabel==NULL)
		{
			continue;
		}
		CDuiString strIP=pLabel->GetText();
		if(_tcsicmp(strIP,friendInfo.m_description)==0)
		{
			index=i;
			return index;
		}
	}
	return index;
}

bool main_frame::RemoveFriend(FriendListItemInfo friendInfo)
{
	int index=GetFriendIndex(friendInfo);
	if(-1==index)
	{
		return true;
	}
	commonlist*  pControl=static_cast<commonlist*>(m_PaintManager.FindControl(LIST_FRIEND));
	if(pControl==NULL)
	{
		return false;
	}
	CListContainerElementUI* pElement=static_cast<CListContainerElementUI*>(pControl->GetItemAt(index));
	if(pElement==NULL)
	{
		return false;
	}
	Node* pNode=(Node*)pElement->GetTag();
	if(pNode==NULL)
	{
		return false;
	}

	return pControl->RemoveNode(pNode);
}

bool main_frame::RemoveGroup(FriendListItemInfo friendInfo)
{
	int index=GetFriendIndex(friendInfo);
	if(-1==index)
	{
		return true;
	}
	commonlist*  pControl=static_cast<commonlist*>(m_PaintManager.FindControl(LIST_GROUP));
	if(pControl==NULL)
	{
		return false;
	}
	CListContainerElementUI* pElement=static_cast<CListContainerElementUI*>(pControl->GetItemAt(index));
	if(pElement==NULL)
	{
		return false;
	}
	Node* pNode=(Node*)pElement->GetTag();
	if(pNode==NULL)
	{
		return false;
	}

	return pControl->RemoveNode(pNode);
}

bool main_frame::SetImage(const char *strImage)
{
	TCHAR str[MAX_PATH*2]={0};
	memset(str,0,sizeof(str));
	int size=MultiByteToWideChar(0,0,strImage,-1,NULL,0);
	MultiByteToWideChar(0,0,strImage,-1,str,size);
	CButtonUI* pControl=static_cast<CButtonUI*>(m_PaintManager.FindControl(PC_MAIN_IMAGE));
	if(pControl==NULL)
	{
		return false;
	}
	pControl->SetNormalImage(str);
	m_IOCP.SetPCImage(string(strImage));
	return true;
}

bool main_frame::SetName(const char *strName)
{
	TCHAR str[MAX_PATH*2]={0};
	memset(str,0,sizeof(str));
	int size=MultiByteToWideChar(0,0,strName,-1,NULL,0);
	MultiByteToWideChar(0,0,strName,-1,str,size);
	CLabelUI* pControl=static_cast<CLabelUI*>(m_PaintManager.FindControl(PC_MAIN_NAME));
	if(pControl==NULL)
	{
		return false;
	}
	pControl->SetText(str);
	m_IOCP.SetPCName(string(strName));
	return true;
}

bool main_frame::AddNewChatDlg()
{
	CTabLayoutUI* pControl=static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tabs")));
	if(pControl==NULL)
	{
		return false;
	}
	int index=pControl->GetCurSel();
	if(index<0||index>=2)
	{
		return false;
	}
	CListUI* pListUI=NULL;
	WNDTYPE type=WNDTYPE::WND_POINT;
	if(index==0)
	{
		pListUI=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_FRIEND));
		type=WNDTYPE::WND_POINT;
	}
	else
	{
		pListUI=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_GROUP));
		type=WNDTYPE::WND_MUTLICAST;
	}
	if(pListUI==NULL)
	{
		return false;
	}
	return AddNewChatDlg(type,pListUI);
}

bool main_frame::AddNewChatDlg(WNDTYPE type,DuiLib::CListUI* pControl)
{
	int index=pControl->GetCurSel();
	CListContainerElementUI* pElement=static_cast<CListContainerElementUI*>(pControl->GetItemAt(index));
	if(pElement==NULL)
	{
		return false;
	}

	CHorizontalLayoutUI* pBG=static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(COMMON_BG));
	if(pBG==NULL)
	{
		return false;
	}

	CButtonUI* pBtn=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_LOGO));
	if(pBtn==NULL)
	{
		return false;
	}
	CLabelUI* pLabelName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_NAME));
	if(pLabelName==NULL)
	{
		return false;
	}
	CLabelUI* pLabelDes=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_DESCRIPTION));
	if(pLabelDes==NULL)
	{
		return false;
	}
	CDuiString strBGI=pBG->GetBkImage();
	CDuiString strLogo=pBtn->GetNormalImage();
	CDuiString strName=pLabelName->GetText();
	CDuiString strDes=pLabelDes->GetText();

	chat_dialog* chatDlg=FindChatDlgByIP(strDes);
	if(chatDlg!=NULL)
	{
		chatDlg->SetWndEnableState();
		chatDlg->ShowWindow();
		return true;
	}

	chatDlg=new chat_dialog(type,strBGI,strLogo,strName,strDes);

	chatDlg->Create(NULL, _T("ChatDialog"),  UI_WNDSTYLE_FRAME,   WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,  NULL, 0, 0, 0, 0);
	chatDlg->CenterWindow();
	chatDlg->SetMainDlg(this);
	::ShowWindow(chatDlg->GetHWND(),SW_NORMAL);
	m_mapChatDlg.insert(make_pair(strDes,chatDlg));
	return true;
}

bool main_frame::AddNewChatDlg(const char* strIP)
{
	if(strIP==NULL)
	{
		return false;
	}
	WNDTYPE type=WNDTYPE::WND_POINT;
	int size=MultiByteToWideChar(0,0,strIP,-1,0,0);
	TCHAR *wstr=new TCHAR[size];
	MultiByteToWideChar(0,0,strIP,-1,wstr,size);
	CDuiString str(wstr);
	RELEASE(wstr);

	CListUI* pFriendControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_FRIEND));
	CListUI* pGroupControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_GROUP));
	if(pFriendControl==NULL||pGroupControl==NULL)
	{
		return false;
	}
	CListContainerElementUI* pElement=GetElementByDescription(str,pFriendControl);
	if(pElement==NULL)
	{
		type=WNDTYPE::WND_MUTLICAST;
		pElement=GetElementByDescription(str,pGroupControl);
		if(pElement==NULL)
		{
			return false;
		}
	}

	CHorizontalLayoutUI* pBG=static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(COMMON_BG));
	if(pBG==NULL)
	{
		return false;
	}

	CButtonUI* pBtn=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_LOGO));
	if(pBtn==NULL)
	{
		return false;
	}
	CLabelUI* pLabelName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_NAME));
	if(pLabelName==NULL)
	{
		return false;
	}
	CLabelUI* pLabelDes=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_DESCRIPTION));
	if(pLabelDes==NULL)
	{
		return false;
	}
	CDuiString strBGI=pBG->GetBkImage();
	CDuiString strLogo=pBtn->GetNormalImage();
	CDuiString strName=pLabelName->GetText();
	CDuiString strDes=pLabelDes->GetText();

	chat_dialog* chatDlg=FindChatDlgByIP(strDes);
	if(chatDlg!=NULL)
	{
		chatDlg->SetWndEnableState();
		chatDlg->ShowWindow();
		return true;
	}

	chatDlg=new chat_dialog(type,strBGI,strLogo,strName,strDes);

	chatDlg->Create(NULL, _T("ChatDialog"), UI_WNDSTYLE_FRAME,WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,  NULL, 0, 0, 0, 0);
	chatDlg->CenterWindow();
	chatDlg->SetMainDlg(this);
	::ShowWindow(chatDlg->GetHWND(),SW_NORMAL);

	m_mapChatDlg.insert(make_pair(strDes,chatDlg));
	return true;
}

bool main_frame::RemoveChatDlg(chat_dialog* chatDlg)
{
	for(map<CDuiString,chat_dialog*>::iterator ite=m_mapChatDlg.begin();ite!=m_mapChatDlg.end();)
	{
		if(ite->second==chatDlg)
		{
			m_mapChatDlg.erase(ite);
			ite=m_mapChatDlg.begin();
			continue;
		}
		++ite;
	}
	return true;
}

chat_dialog* main_frame::FindChatDlgByIP(DuiLib::CDuiString strIP)
{
	chat_dialog* chatDlg=NULL;
	if(m_mapChatDlg.end() == m_mapChatDlg.find(strIP))
	{
		return chatDlg;
	}
	chatDlg=m_mapChatDlg[strIP];
	return chatDlg;
}

CDuiString main_frame::GetName()
{
	string name=m_IOCP.GetPCName();
	int size=MultiByteToWideChar(0,0,name.c_str(),-1,0,0);
	TCHAR *cstr=new TCHAR[size];
	MultiByteToWideChar(0,0,name.c_str(),-1,cstr,size);
	CDuiString str(cstr);
	RELEASE(cstr);
	return str;
}

void main_frame::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	//delete this;
	return;
}

bool main_frame::RecvUDPMessage(const char* strIP,UDPDATA data)
{
	//strIP指的是发送端IP地址结构，data中的m_addr则是目的地址
	//对于点对点的通信二者是相同的，对于多播，则不同,信息应当传送到m_addr对应的聊天窗口
	int size=MultiByteToWideChar(0,0,strIP,-1,0,0);
	TCHAR *cstrIP=new TCHAR[size];
	MultiByteToWideChar(0,0,strIP,-1,cstrIP,size);
	CDuiString str(cstrIP);
	RELEASE(cstrIP);
	chat_dialog* chatDlg=m_mapChatDlg.find(str)->second;
	if(chatDlg==NULL)
	{
		return false;
	}
	size=MultiByteToWideChar(0,0,data.m_message,-1,0,0);
	TCHAR *cMsg=new TCHAR[size];
	MultiByteToWideChar(0,0,data.m_message,-1,cMsg,size);
	CDuiString msg(cMsg);
	RELEASE(cMsg);

	size=MultiByteToWideChar(0,0,strIP,-1,0,0);
	TCHAR *cstrIP2=new TCHAR[size];
	MultiByteToWideChar(0,0,strIP,-1,cstrIP2,size);
	CDuiString str2(cstrIP2);
	RELEASE(cstrIP2);

	chatDlg->RecvMessage(GetNameByDescription(str2),msg);
	return true;
}

bool main_frame::SendUDPMessage(WNDTYPE type,CDuiString description,CDuiString message)
{
	int size=WideCharToMultiByte(0,0,description.GetData(),-1,0,0,NULL,NULL);
	char* strIP=new char[size];
	WideCharToMultiByte(0,0,description.GetData(),-1,strIP,size,NULL,NULL);

	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr(strIP);
	addr.sin_port=htons(USER_PORT);

	UDPDATA data;
	memset(&data,0,sizeof(data));
	data.m_addr=addr;
	memcpy(data.m_hostName,m_IOCP.GetPCHostName().c_str(),(m_IOCP.GetPCHostName().size()+1)*sizeof(char));
	memcpy(data.m_name,m_IOCP.GetPCName().c_str(),(m_IOCP.GetPCName().size()+1)*sizeof(char));
	memcpy(data.m_image,m_IOCP.GetPCImage().c_str(),(m_IOCP.GetPCImage().size()+1)*sizeof(char));

	size=WideCharToMultiByte(0,0,message.GetData(),-1,0,0,NULL,NULL);
	char *strMsg=new char[size];
	WideCharToMultiByte(0,0,message.GetData(),-1,strMsg,size,NULL,NULL);
	memcpy(data.m_message,strMsg,size);

	RELEASE(strIP);
	RELEASE(strMsg);

	switch (type)
	{ 
	case WNDTYPE::WND_POINT:
		data.m_msgType=UDPMSGTYPE::COMMONPOINT;
		break;
	case WNDTYPE::WND_MUTLICAST:
		data.m_msgType=UDPMSGTYPE::COMMONMULTICAST;
		break;
	default:
		return false;
	}
	return m_IOCP.SendUDPMessage(addr,data,m_IOCP.GetUDPSendIOContext());
}

bool main_frame::RecvTCPMessage(PER_IO_CONTEXT* pIoContext)
{
	int size=MultiByteToWideChar(0,0,inet_ntoa(pIoContext->m_senderAddr.sin_addr),-1,0,0);
	TCHAR *cstrIP=new TCHAR[size];
	MultiByteToWideChar(0,0,inet_ntoa(pIoContext->m_senderAddr.sin_addr),-1,cstrIP,size);
	CDuiString str(cstrIP);
	RELEASE( cstrIP);
	if(m_mapChatDlg.end()==m_mapChatDlg.find(str))
	{
		AddNewChatDlg(inet_ntoa(pIoContext->m_senderAddr.sin_addr));
		RecvTCPMessage(pIoContext);
		return true;
	}
	chat_dialog* chatDlg=m_mapChatDlg.find(str)->second;
	if(chatDlg==NULL)
	{
		return false;
	}

	FILEINFO *fileInfo=NULL;
	PACKAGE_HEAD head;
	memset(&head,0,sizeof(head));
	memcpy(&head,pIoContext->m_szBuffer,PACKAGE_HEAD_LEN);
	TCPDATA data;
	memset(&data,0,sizeof(data));
	switch(head.m_command)
	{
	case TCP_HEAD_TYPE::TCP_MSG:
		memcpy(&data,pIoContext->m_szBuffer+PACKAGE_HEAD_LEN,pIoContext->m_wsaBuf.len);
		switch(data.m_type)
		{
		case TCPMSGTYPE::FILE_SEND:
			fileInfo=new FILEINFO();
			memcpy(fileInfo->m_realName,data.m_fileName,sizeof(data.m_fileName));
			fileInfo->m_pIoContext=pIoContext;
			fileInfo->m_type=FILEOPERATIONTYPE::FILE_OPE_ACCEPT;
			fileInfo->m_fileLen=data.m_fileLen;
			fileInfo->m_finishLen=0;

			//对方请求发送文件
			if(chatDlg->AddNewFileItem(data,FILEOPERATIONTYPE::FILE_OPE_ACCEPT,fileInfo))
			{
				m_listFileInfo.push_back(fileInfo);
			}
			else
			{
				RELEASE(fileInfo);
				fileInfo=NULL;
			}
			break;
		case TCPMSGTYPE::FILE_ACCEPT:
			//对方同意发送文件
			if(!SendFile(m_IOCP.GetIoContextBySocket(pIoContext->m_socket,OPERATION_TYPE::SEND_POSTED),data.m_fileName))
			{
				return false;
			}
			break;
		case TCPMSGTYPE::FILE_FINISH:
			//文件发送完成
			fileInfo=GetFileInfoByIoContext(pIoContext);
			RemoveFileInfo(fileInfo);
			break;
		case TCPMSGTYPE::FILE_STOP:
			//文件取消发送
			fileInfo=GetFileInfoByIoContext(pIoContext);
			RemoveFileInfo(fileInfo);
			break;
		default:
			return false;
		}
		break;
	case TCP_HEAD_TYPE::TCP_FILE:
		//文件流
		WriteDataInFile(pIoContext);
		break;
	default:
		return false;
	}
	return true;
}

CDuiString main_frame::GetNameByDescription(CDuiString description)
{
	CDuiString str;
	memset(&str,0,sizeof(str));
	CListUI* pFriendContrl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_FRIEND));
	CListUI* pGroupControl=static_cast<CListUI*>(m_PaintManager.FindControl(LIST_GROUP));
	CListContainerElementUI* pElement=NULL;
	CLabelUI* pLabelName=NULL;
	CLabelUI* pLabelDes=NULL;
	if(pFriendContrl==NULL||pGroupControl==NULL)
	{
		return str;
	}
	for(int i=0;i<pFriendContrl->GetCount();++i)
	{
		pElement=static_cast<CListContainerElementUI*>(pFriendContrl->GetItemAt(i));
		if(pElement==NULL)
		{
			continue;
		}
		pLabelDes=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_DESCRIPTION));
		if(pLabelDes==NULL)
		{
			continue;
		}
		if(_tcsicmp(description,pLabelDes->GetText())==0)
		{
			pLabelName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_NAME));
			if(pLabelName==NULL)
			{
				continue;
			}
			str=pLabelName->GetText();
			return str;
		}
	}
	for(int i=0;i<pGroupControl->GetCount();++i)
	{
		pElement=static_cast<CListContainerElementUI*>(pGroupControl->GetItemAt(i));
		if(pElement==NULL)
		{
			continue;
		}
		pLabelDes=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_DESCRIPTION));
		if(pLabelDes==NULL)
		{
			continue;
		}
		if(_tcsicmp(description,pLabelDes->GetText())==0)
		{
			pLabelName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FRIEND_NAME));
			if(pLabelName==NULL)
			{
				continue;
			}
			str=pLabelName->GetText();
			return str;
		}
	}
	return str;
}

bool main_frame::SetChatDlgEnabledState(const char* strIP,bool enable)
{
	int size=MultiByteToWideChar(0,0,strIP,-1,0,0);
	TCHAR *str=new TCHAR[size];
	MultiByteToWideChar(0,0,strIP,-1,str,size);
	CDuiString duiStr(str);
	RELEASE(str);
	chat_dialog* chatDlg=FindChatDlgByIP(str);
	if(chatDlg==NULL)
	{
		return false;
	}
	chatDlg->SetWndEnableState(enable);
	return true;
}

CListContainerElementUI* main_frame::GetElementByDescription(CDuiString description,CListUI* pControl)
{

	CListContainerElementUI *pElement=NULL;
	for(int i=0;i<pControl->GetCount();++i)
	{
		CListContainerElementUI* pTemp=static_cast<CListContainerElementUI*>(pControl->GetItemAt(i));
		if(pTemp==NULL)
		{
			continue;
		}
		CLabelUI* pLabel=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pTemp,FRIEND_DESCRIPTION));
		if(pLabel==NULL)
		{
			continue;
		}
		CDuiString ip=pLabel->GetText();
		if(_tcsicmp(pLabel->GetText(),description)==0)
		{
			pElement=pTemp;
			break;
		}
	}
	return pElement;
}

FILEINFO* main_frame::GetFileInfoByRealName(const char* realName)
{
	FILEINFO* fileInfo=NULL;
	memset(&fileInfo,0,sizeof(fileInfo));
	for(list<FILEINFO*>::iterator ite=m_listFileInfo.begin();ite!=m_listFileInfo.end();++ite)
	{
		if(strcmp((*ite)->m_realName,realName)==0)
		{
			fileInfo=*ite;
			break;
		}
	}
	return fileInfo;
}

FILEINFO* main_frame::GetFileInfoByIoContext(const PER_IO_CONTEXT* pIoContext)
{
	FILEINFO* fileInfo=NULL;
	memset(&fileInfo,0,sizeof(fileInfo));
	for(list<FILEINFO*>::iterator ite=m_listFileInfo.begin();ite!=m_listFileInfo.end();++ite)
	{
		if((*ite)->m_pIoContext==pIoContext)
		{
			fileInfo=*ite;
			break;
		}
	}
	return fileInfo;
}

bool main_frame::SendFile(PER_IO_CONTEXT* pIoContext,const char* fileName)
{
	FILEINFO *fileInfo=GetFileInfoByRealName(fileName);
	if(fileInfo==NULL)
	{
		return false;
	}

	PACKAGE_HEAD head;
	TCPSENDDATA sendData;

	FILE *fileHandle = NULL;
	fopen_s(&fileHandle,fileName,"rb");
	if(NULL==fileHandle)
	{
		return false;
	}
	int nBuff = PACKAGE_DATA_LEN;
	if(setvbuf(fileHandle, (char*)&nBuff, _IOFBF, sizeof(nBuff)))
	{
		return false;
	}

	u_long ulFlagCount = 0;            //记录读了多少数据
	u_long FaceReadByte = 0;    //一次实际读取的字节数
	char sBuff[PACKAGE_DATA_LEN];    

	//移动读取位置到文件开始的位置
	fseek(fileHandle, 0, SEEK_SET);

	while(!feof(fileHandle))
	{
		fileInfo=GetFileInfoByRealName(fileName);
		if(fileInfo==NULL)
		{
			break;
		}
		if(pIoContext!=NULL&&fileInfo!=NULL)
		{
			memset(&head,0,sizeof(head));
			memset(&sendData,0,sizeof(sendData));
			sendData.m_socket=pIoContext->m_socket;
			head.m_command=TCP_HEAD_TYPE::TCP_FILE;

			memset(sBuff,0,sizeof(sBuff));
			FaceReadByte = fread(sBuff, 1, PACKAGE_DATA_LEN, fileHandle);
			ulFlagCount+=FaceReadByte;

			head.m_dataLen=FaceReadByte;

			memcpy(&sendData,&head,sizeof(head));
			memcpy(&sendData+sizeof(head),&sBuff,sizeof(sBuff));
			if(!m_IOCP.SendTCPMessage(sendData,pIoContext))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	if(pIoContext!=NULL&&fileInfo!=NULL)
	{
		//附带发送一个完成消息
		memset(&head,0,sizeof(head));
		memset(&sendData,0,sizeof(sendData));
		sendData.m_socket=pIoContext->m_socket;
		head.m_command=TCP_HEAD_TYPE::TCP_MSG;

		TCPDATA msgData;
		memset(&msgData,0,sizeof(msgData));
		memcpy(msgData.m_fileName,fileName,sizeof(fileName));
		msgData.m_type=TCPMSGTYPE::FILE_FINISH;
		msgData.m_fileLen=ulFlagCount;

		head.m_dataLen=sizeof(msgData);

		memcpy(&sendData,&head,sizeof(head));
		memcpy(&sendData+sizeof(head),&msgData,sizeof(msgData));
		if(!m_IOCP.SendTCPMessage(sendData,pIoContext))
		{
			fclose(fileHandle);
			return false;
		}

		//关闭文件
		fclose(fileHandle);
		return true;
	}
	fclose(fileHandle);
	return false;
}

bool main_frame::ApproveAccpetRequest(LPCTSTR fileName)
{
	int size=WideCharToMultiByte(0,0,fileName,-1,NULL,0,NULL,NULL);
	char *str=new char[size];
	WideCharToMultiByte(0,0,fileName,-1,str,size,NULL,NULL);
	FILEINFO* pFile=GetFileInfoByRealName(str);
	RELEASE(str);

	if(pFile==NULL||pFile->m_pIoContext==NULL)
	{
		m_listFileInfo.remove(pFile);
		return false;
	}
	TCPSENDDATA sendData;
	TCPDATA    data;
	PACKAGE_HEAD head;
	memset(&sendData,0,sizeof(sendData));
	memset(&data,0,sizeof(data));
	memset(&head,0,sizeof(head));

	sendData.m_socket=pFile->m_pIoContext->m_socket;
	head.m_command=TCP_HEAD_TYPE::TCP_MSG;

	memcpy(data.m_fileName,pFile->m_realName,sizeof(pFile->m_realName));
	data.m_fileLen=pFile->m_fileLen;
	data.m_type=TCPMSGTYPE::FILE_ACCEPT;

	head.m_dataLen=sizeof(data);

	memcpy(&sendData,&head,sizeof(head));
	memcpy(&sendData+sizeof(head),&data,sizeof(data));

	if(!m_IOCP.SendTCPMessage(sendData,m_IOCP.GetIoContextBySocket(pFile->m_pIoContext->m_socket,OPERATION_TYPE::SEND_POSTED)))
	{
		RemoveFileInfo(pFile);
		//m_listFileInfo.remove(pFile);
		return false;
	}
	return true;
}

bool main_frame::WriteDataInFile(PER_IO_CONTEXT* pIoContext)
{
	FILEINFO*  pFileInfo=GetFileInfoByIoContext(pIoContext);
	if(pFileInfo==NULL)
	{
		return false;
	}
	TCPSENDDATA sendData;
	PACKAGE_HEAD head;
	memset(&sendData,0,sizeof(&sendData));
	memset(&head,0,sizeof(&head));
	memcpy(&sendData,pIoContext->m_szBuffer,pIoContext->m_wsaBuf.len+sizeof(PACKAGE_DATA_LEN));
	memcpy(&head,&sendData,sizeof(PACKAGE_HEAD));

	FILE *fileHandle = NULL;
	fopen_s(&fileHandle,pFileInfo->m_saveName, "ab");
	if(NULL == pFileInfo)
	{
		return false;
	}
	int nBuff = head.m_dataLen;
	if(setvbuf(fileHandle, (char*)&nBuff, _IOFBF, sizeof(nBuff)))
	{
		return false;
	}

	fseek(fileHandle,0,SEEK_END);
	u_long nNumberOfBytesWritten = fwrite(&sendData+sizeof(PACKAGE_HEAD), 1,head.m_dataLen, fileHandle);

	if(head.m_dataLen != nNumberOfBytesWritten)
	{
		return false;
	}
	else
	{
		pFileInfo->m_finishLen += nNumberOfBytesWritten;
	}
	fflush(fileHandle);                                //清除文件缓冲区
	//关闭文件
	fclose(fileHandle);
	return true;
}

CControlUI* main_frame::GetGoalCtrl(CControlUI* srcCtrl,LPCTSTR className)
{
	CControlUI* parent=srcCtrl;
	while(parent!=NULL)
	{
		CDuiString name=parent->GetClass();
		if(_tcsicmp(parent->GetClass(),className)==0)
		{
			break;
		}
		parent=parent->GetParent();
	}
	return parent;
}

bool main_frame::FreshFriendsList()
{
	return m_IOCP.SendUDPLogOnMessage();
}

bool main_frame::RemoveFileInfo(FILEINFO* fileInfo)
{
	if(fileInfo==NULL)
	{
		return false;
	}
	m_listFileInfo.remove(fileInfo);
	m_IOCP.ClientClose(fileInfo->m_pIoContext);
	RELEASE(fileInfo);
	return true;
}

bool main_frame::RemoveFileInfo(DuiLib::CDuiString fileName)
{
	FILEINFO* fileInfo=NULL;
	for(list<FILEINFO*>::iterator ite=m_listFileInfo.begin();ite!=m_listFileInfo.end();++ite)
	{
		int size=MultiByteToWideChar(0,0,(*ite)->m_realName,-1,NULL,0);
		TCHAR *str=new TCHAR[size];
		MultiByteToWideChar(0,0,(*ite)->m_realName,-1,str,size);
		if(_tcsicmp(str,fileName)==0)
		{
			fileInfo=*ite;
			RELEASE(str);
			break;
		}
		RELEASE(str);
	}
	if(fileInfo==NULL)
	{
		return false;
	}
	return RemoveFileInfo(fileInfo);
}

bool main_frame::ConnectServer(LPCTSTR fileName,LPCTSTR ip)
{
	return m_IOCP.ConnectServer(fileName,ip);
}

void main_frame::AddFileInfo(FILEINFO* fileInfo)
{
	if(fileInfo==NULL)
	{
		return;
	}
	m_listFileInfo.push_back(fileInfo);
}

LRESULT main_frame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled=TRUE;
	switch(uMsg)
	{
	case WM_SIZE:
		if(wParam==SIZE_MINIMIZED)
		{
			::ShowWindow(GetHWND(),SW_HIDE);
			return 0;
		}
		else
		{
			break;
		}
	case WMAPP_NOTIFYCALLBACK:
		switch(LOWORD(lParam))
		{
		case NIN_SELECT:          //左键单机消息                       
			::ShowWindow(GetHWND(),SW_NORMAL);         
			break;
		case WM_CONTEXTMENU:    //右键单击消息
			MessageBox(NULL,L"",L"",0);
			break;
		default:break;	
		}
		return 0;
	case WM_CLOSE:
		DeleteNotificationIcon(GetHWND());
		bHandled=FALSE;
		return 0;
	case WM_CREATECHATDLG:
		AddNewChatDlg((char*)wParam);
		return 0;
	default:break;
	}
	bHandled=FALSE;
	return 0;
}

BOOL main_frame::AddNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = {sizeof(nid)};
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	nid.uID=IDI_ICONMAINFRAME;
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	nid.hIcon=LoadIcon(m_PaintManager.GetInstance(),MAKEINTRESOURCE(IDI_ICONMAINFRAME));
	wcscpy_s(nid.szTip, L"EChat");
	Shell_NotifyIcon(NIM_ADD, &nid);

	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL main_frame::DeleteNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = {sizeof(nid)};
	nid.hWnd =hwnd ;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	nid.uID=IDI_ICONMAINFRAME;
	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

CControlUI* main_frame::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("commonlist")) == 0)
	{
		return new commonlist(m_PaintManager);
	}
	else if (_tcsicmp(pstrClass,_T("commonlist")) == 0)
	{
		return new commonlist(m_PaintManager);
	}
	return NULL;
}

bool main_frame::InitListFloder()
{
	commonlist *pListFriend=static_cast<commonlist*>(m_PaintManager.FindControl(LIST_FRIEND));
	commonlist *pListGroup=static_cast<commonlist*>(m_PaintManager.FindControl(LIST_GROUP));
	if(pListFriend==NULL||pListGroup==NULL)
	{
		return false;
	}
	FriendListItemInfo item;
	item.m_folder = true;
	item.m_empty = false;
	item.m_nick_name = _T("我的好友");
	m_friendParentNode = pListFriend->AddNode(item, NULL);

	item.m_nick_name=_T("我的群组");
	m_groupParentNode = pListGroup->AddNode(item, NULL);
	return true;
}

void main_frame::WndClosing()
{
	m_IOCP.SendUDPLogOffMessage();
}

bool main_frame::isExsitedWnd(const char* strIP)
{
	bool retbool=false;
	int size=MultiByteToWideChar(0,0,strIP,-1,0,0);
	TCHAR *cstrIP=new TCHAR[size];
	MultiByteToWideChar(0,0,strIP,-1,cstrIP,size);
	CDuiString str(cstrIP);
	RELEASE(cstrIP);
	if(m_mapChatDlg.end()==m_mapChatDlg.find(str))
	{
		return retbool;
	}
	retbool=true;
	return retbool;
}