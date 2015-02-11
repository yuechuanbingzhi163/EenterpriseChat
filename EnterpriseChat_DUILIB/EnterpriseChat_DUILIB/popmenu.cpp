#include "main_frame.h"
#include "popmenu.h"
using namespace DuiLib;

#define ITEM_CHAT _T("menu_chat")
#define ITEM_FILE _T("menu_file")
#define ITEM_CHANGENAME _T("menu_changename")
#define ITEM_FRESH  _T("menu_freshlist")
#define ITEM_CHNAGEINFO _T("menu_changeinfo")

#define SINGLE_ITEM_HEIGHT 38
#define ITEM_HEIGHT 25

LRESULT popmenu::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Close();
	bHandled = FALSE;
	return 0;
}

void popmenu::Init(HWND hWndParent, POINT ptPos)
{
	Create(hWndParent, _T("MenuWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	::ClientToScreen(hWndParent, &ptPos);
	::SetWindowPos(*this, NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

LRESULT popmenu::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL    bHandled = TRUE;

	switch( uMsg )
	{
	case WM_KILLFOCUS:    
		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); 
		break; 
	default:
		bHandled = FALSE;
	}

	if(bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes)) 
	{
		return lRes;
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void popmenu::Notify(DuiLib::TNotifyUI& msg)
{
	bool ret=false;
	CDuiString sendName=msg.pSender->GetName();
	if(_tcsicmp(msg.sType,_T("itemlbclick"))==0)
	{
		CListContainerElementUI* pControl=static_cast<CListContainerElementUI*>(GetGoalCtrl(msg.pSender,_T("ListContainerElementUI")));
		if(pControl==NULL)
		{
			return;
		}
		CDuiString ctrlName=pControl->GetName();
		if(_tcsicmp(ctrlName,ITEM_CHAT)==0||_tcsicmp(ctrlName,ITEM_FILE)==0)
		{
			ret=m_mainDlg->AddNewChatDlg();
		}
		else if(_tcsicmp(ctrlName,ITEM_CHANGENAME)==0)
		{

		}
		else if(_tcsicmp(ctrlName,ITEM_FRESH)==0)
		{
			ret=m_mainDlg->FreshFriendsList();
		}
		else if(_tcsicmp(ctrlName,ITEM_CHNAGEINFO)==0)
		{

		}
		::SendMessage(GetHWND(),WM_KILLFOCUS,NULL,NULL);
		return;
	}
	__super::Notify(msg);
}

CControlUI* popmenu::GetGoalCtrl(CControlUI* srcCtrl,LPCTSTR className)
{
	CControlUI* parent=srcCtrl;
	while(parent!=NULL)
	{
		if(_tcsicmp(parent->GetClass(),className)==0)
		{
			break;
		}
		parent=parent->GetParent();
	}
	return parent;
}

bool popmenu::SetItemEnable(bool enable)
{
	RECT rect;
	::GetWindowRect(GetHWND(),&rect);
	CListContainerElementUI* pControl=static_cast<CListContainerElementUI*>(m_PaintManager.FindControl(ITEM_CHAT));
	if(pControl==NULL)
	{
		return false;
	}
	pControl->SetVisible(enable);
	if(enable)
	{
		::SetWindowPos(GetHWND(),NULL,rect.left,rect.top,rect.right-rect.left,ITEM_HEIGHT*5,0);
	}
	else
	{
		::SetWindowPos(GetHWND(),NULL,rect.left,rect.top,rect.right-rect.left,SINGLE_ITEM_HEIGHT,0);
	}
	return true;
}