#include "main_frame.h"
#include "chat_dialog.h"
#include <Shlobj.h>
#include "audiovideolib.h"




#define CHATDLG_BG _T("bg")
#define CHATDLG_LOGO _T("logo")
#define CHATDLG_NAME _T("nickname")
#define CHATDLG_DES  _T("description")
#define CHATDLG_RECVEDIT _T("view_richedit")
#define CHATDLG_SENDEDIT _T("input_richedit")
#define CHATDLG_CANCELBTN _T("closebtn")
#define CHATDLG_SENDBTN   _T("sendbtn")
#define CHATDLG_FILECONTAINER _T("right_part")
#define CHATDLG_FILECONTAINEREMPTY _T("right_part_empty")
#define CHATDLG_FILELIST _T("file_list")
#define CHATDLG_MEMBERLIST _T("member_list")


#define FILE_IMAGE _T("logo")
#define FILE_NAME  _T("filename")
#define FILE_SAVENAME _T("savefilename")
#define FILE_REALNAME _T("realfilename")
#define FILE_PROGRESS _T("download")
#define FILE_LENGTH   _T("filelength")
#define FILE_OPENBTN  _T("open")
#define FILE_OPERATIONBTN _T("state")
#define FILE_TYPE       _T("type")

#define ITEM_HEIGHT 40
#define LISTPART_NORMAL_WIDTH 100
#define LISTPART_EXTRA_WIDTH  200
#define LISTPART_GROUPEXTRA_WIDTH 150

#define WND_FILE_SEND _T("send")
#define WND_FILE_ACCEPT _T("accept")
#define STATE_FILE_CANCEL _T("取消")
#define STATE_FILE_ACCEPT _T("接受")
#define STATE_FILE_FINISH _T("完成")

#define AUDIO_CONTROL _T("audiobtn")
#define VIDEO_CONTROL _T("videobtn")


#define SENDAUDIOMSG_TIMER 1001
using namespace DuiLib;
using namespace std;
using namespace avl;
using namespace cv;

chat_dialog::chat_dialog(WNDTYPE type,CDuiString BGI,CDuiString logo,CDuiString name,CDuiString description):
	m_mainDlg(NULL),
	m_type(type),
	m_BGI(BGI),
	m_logo(logo),
	m_name(name),
	m_description(description),
	m_isEnabled(true),
	m_isHaveFile(false)
{
}

chat_dialog::~chat_dialog(void)
{
}

LPCTSTR chat_dialog::GetWindowClassName() const
{
	return _T("ChatDialog");
}

CDuiString chat_dialog::GetSkinFile()
{
	return _T("chat_dialog.xml");
}

CDuiString chat_dialog::GetSkinFolder()
{
	return _T("");
}

void chat_dialog::InitWindow()
{
	SetBGI(m_BGI);
	SetLogo(m_logo);
	SetName(m_name);
	SetDescription(m_description);

	AllowMeesageForVistaAbove(SPI_SETANIMATION, MSGFLT_ADD);
	AllowMeesageForVistaAbove(WM_DROPFILES, MSGFLT_ADD);
	DragAcceptFiles(GetHWND(),TRUE);

	SetIsHaveFile(false);
	SetIsGroupChatDlg(m_type);

	//RecvMessage(_T("一语成谶"),_T("耶耶耶我i万i万i万i万i万i"));
	//RecvMessage(_T("一语成谶"),_T("哈哈哈哈哈哈哈哈哈哈哈哈"));
	//RecvMessage(_T("c:\\desktop\\wini\\create\\init\\dddddddd.xml"),_T("send"),TCPMSGTYPE::FILE_STOP);
	//RecvMessage(_T("一语成谶"),_T("滴答滴答滴答滴答滴答"));
	//RecvMessage(_T("一语成谶"),_T("哈哈哈哈哈哈哈哈哈哈哈哈"));
	//TCPDATA data;
	//data.m_fileLen=5000;
	//memcpy(data.m_fileName,"c:\\desktop\\wini.xml",sizeof("c:\\desktop\\wini.xml"));
	//AddNewFileItem(data,FILEOPERATIONTYPE::FILE_OPE_SEND,NULL);
	//AddNewFileItem(data,FILEOPERATIONTYPE::FILE_OPE_ACCEPT,NULL);
	/*for(int i=0;i<10;++i)
	{
	if( !m_dlgBuilder.GetMarkup()->IsValid())
	{
	pElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("file_list_item.xml"), (UINT)0, NULL, &m_PaintManager));
	}
	else 
	{
	pElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &m_PaintManager));
	}
	pList->Add(pElement);
	pElement->SetFixedHeight(40);
	}*/
}

void chat_dialog::Notify(DuiLib::TNotifyUI& msg)
{
	CDuiString sendName=msg.pSender->GetName();
	if(_tcsicmp(msg.sType,_T("click"))==0)
	{
		if(_tcsicmp(sendName,_T("sendbtn"))==0)
		{
			CRichEditUI* pRichEdit=static_cast<CRichEditUI*>(m_PaintManager.FindControl(CHATDLG_SENDEDIT));
			if(pRichEdit==NULL)
			{
				return;
			}
			CDuiString str=pRichEdit->GetTextRange(0,pRichEdit->GetTextLength());
			if(_tcsicmp(str,_T(""))==0)
			{
				return;
			}
			pRichEdit->SetText(_T(""));
			RecvMessage(m_mainDlg->GetName(),str);
			m_mainDlg->SendUDPMessage(m_type,m_description,str);
			pRichEdit->SetFocus();
			return;
		}
		if(_tcsicmp(sendName,_T("open"))==0)
		{
			return;
		}
		if(_tcsicmp(sendName,_T("state"))==0)
		{
			CListUI* pControl=static_cast<CListUI*>(m_PaintManager.FindControl(CHATDLG_FILELIST));
			CListContainerElementUI* pElement=static_cast<CListContainerElementUI*>(GetGoalCtrl(msg.pSender,_T("ListContainerElementUI")));
			CButtonUI* pBtn=static_cast<CButtonUI*>(msg.pSender);
			if(pElement==NULL||pBtn==NULL)
			{
				return;
			}
			CLabelUI* pLabelName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_NAME));
			CLabelUI*  pLabel=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_REALNAME));
			CButtonUI*  pOpenBtn=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_OPENBTN));
			if(pLabel==NULL||pOpenBtn==NULL)
			{
				return;
			}
			CDuiString name=pLabelName->GetText();
			CDuiString stateStr=pBtn->GetText();
			CDuiString fileName=pLabel->GetText();
			CDuiString saveName(L"");
			char cFileName[MAX_PATH]={0};
			memset(cFileName,0,sizeof(cFileName));
			int size=WideCharToMultiByte(0,0,fileName,-1,NULL,0,NULL,NULL);
			WideCharToMultiByte(0,0,fileName,-1,cFileName,size,NULL,NULL);
			//比较字符串
			if(_tcsicmp(stateStr,STATE_FILE_ACCEPT)==0)
			{
				LPITEMIDLIST lpDlist; 
				BROWSEINFO broInfo;
				TCHAR chPath[MAX_PATH];
				memset(&chPath,0,sizeof(chPath));
				memset(&broInfo,0,sizeof(broInfo));
				memset(&lpDlist,0,sizeof(lpDlist));
				broInfo.hwndOwner =GetHWND();
				broInfo.lpszTitle = _T("文件保存路径: ");
				broInfo.ulFlags = BIF_RETURNONLYFSDIRS;
				lpDlist=SHBrowseForFolder(&broInfo);
				if(lpDlist!=NULL)
				{
					SHGetPathFromIDList(lpDlist, chPath);//把项目标识列表转化成字符串
				}
				else
				{
					wsprintf(chPath,L"%s",L"D:\\EChatAcceptEx");
				}
				saveName+=chPath;
				TCHAR ch=saveName.GetAt(saveName.GetLength()-1);
				if(ch!=L'\\')
				{
					saveName+=L"\\";
				}
				saveName+=name;
				FILEINFO* fileInfo=m_mainDlg->GetFileInfoByRealName(cFileName);
				size=WideCharToMultiByte(0,0,saveName,-1,NULL,0,NULL,NULL);
				memset(cFileName,0,sizeof(cFileName));
				WideCharToMultiByte(0,0,saveName,-1,cFileName,size,NULL,NULL);
				memcpy(fileInfo->m_saveName,cFileName,size);

				SECURITY_ATTRIBUTES attr;
				attr.nLength =fileInfo->m_fileLen;
				attr.lpSecurityDescriptor = NULL;
				HANDLE hRe = CreateFile(saveName.GetData(),GENERIC_WRITE, FILE_SHARE_WRITE, &attr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if(INVALID_HANDLE_VALUE == hRe)
				{
					pControl->Remove(pElement);
					m_mainDlg->RemoveFileInfo(fileInfo);
					return;
				}

				if(m_mainDlg->ApproveAccpetRequest(fileName.GetData()))
				{
					pOpenBtn->SetVisible();
					pBtn->SetText(STATE_FILE_CANCEL);
				}
				else
				{
					pControl->Remove(pElement);
					m_mainDlg->RemoveFileInfo(fileInfo);
					DeleteFile(saveName.GetData());
				}
				return;
			}
			if(_tcsicmp(stateStr,STATE_FILE_CANCEL)==0)
			{
				m_mainDlg->RemoveFileInfo(fileName);
				return;
			}
			if(_tcsicmp(stateStr,STATE_FILE_FINISH)==0)
			{
				return;
			}
			return;
		}
	}

	__super::Notify(msg);
}

bool chat_dialog::SetBGI(CDuiString image)
{
	CHorizontalLayoutUI* pBG=static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(CHATDLG_BG));
	if(pBG==NULL)
	{
		return false;
	}
	pBG->SetBkImage(image);
	m_BGI=image;
	return true;
}

bool chat_dialog::SetName(CDuiString name)
{
	CLabelUI* pControl=static_cast<CLabelUI*>(m_PaintManager.FindControl(CHATDLG_NAME));
	if(pControl==NULL)
	{
		return false;
	}
	pControl->SetText(name);
	m_name=name;
	return true;
}

bool chat_dialog::SetLogo(DuiLib::CDuiString logo)
{
	CButtonUI* pControl=static_cast<CButtonUI*>(m_PaintManager.FindControl(CHATDLG_LOGO));
	if(pControl==NULL)
	{
		return false;
	}
	pControl->SetNormalImage(logo);
	m_logo=logo;
	return true;
}

bool chat_dialog::SetDescription(DuiLib::CDuiString description)
{
	CLabelUI* pControl=static_cast<CLabelUI*>(m_PaintManager.FindControl(CHATDLG_DES));
	if(pControl==NULL)
	{
		return false;
	}
	pControl->SetText(description);
	m_description=description;
	return true;
}

bool chat_dialog::RecvMessage(DuiLib::CDuiString name,CDuiString message)
{
	CRichEditUI* pRichEdit=static_cast<CRichEditUI*>(m_PaintManager.FindControl(CHATDLG_RECVEDIT));
	if(pRichEdit==NULL)
	{
		return false;
	}
	CHARFORMAT2 cf;
	memset(&cf,0,sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwReserved = 0;
	cf.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE | CFM_UNDERLINETYPE;
	cf.dwEffects = CFE_LINK;
	cf.bUnderlineType = CFU_UNDERLINE;
	cf.crTextColor = RGB(220, 0, 0);

	long lSelBegin = 0, lSelEnd = 0;
	lSelEnd = lSelBegin = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelEnd, lSelEnd);
	pRichEdit->ReplaceSel(name.GetData(), false);

	lSelEnd = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelBegin, lSelEnd);
	pRichEdit->SetSelectionCharFormat(cf);

	TCHAR strTime[MAX_PATH]={0};
	memset(&strTime,0,sizeof(strTime));
	SYSTEMTIME time;
	memset(&time,0,sizeof(time));
	GetLocalTime(&time);
	wsprintf(strTime,_T(": %d-%d-%d %d:%d:%d\n"),time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);

	lSelEnd = lSelBegin = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelEnd, lSelEnd);
	pRichEdit->ReplaceSel(strTime, false);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = RGB(0, 0, 0);
	cf.dwEffects = 0;
	lSelEnd = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelBegin, lSelEnd);
	pRichEdit->SetSelectionCharFormat(cf);

	PARAFORMAT2 pf;
	memset(&pf,0,sizeof(pf));
	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_STARTINDENT;
	pf.dxStartIndent = 0;
	pRichEdit->SetParaFormat(pf);

	lSelEnd = lSelBegin = pRichEdit->GetTextLength();

	pRichEdit->SetSel(-1, -1);
	pRichEdit->ReplaceSel(message.GetData(), false);

	pRichEdit->SetSel(-1, -1);
	pRichEdit->ReplaceSel(_T("\n"), false);

	cf.crTextColor = RGB(0, 0, 0);
	lSelEnd = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelBegin, lSelEnd);
	pRichEdit->SetSelectionCharFormat(cf);

	memset(&pf,0,sizeof(pf));
	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_STARTINDENT;
	pf.dxStartIndent = 220;
	pRichEdit->SetParaFormat(pf);

	pRichEdit->EndDown();
	return true;
}

bool chat_dialog::RecvMessage(DuiLib::CDuiString fileName,DuiLib::CDuiString type,TCPMSGTYPE msgType)
{
	CRichEditUI* pRichEdit=static_cast<CRichEditUI*>(m_PaintManager.FindControl(CHATDLG_RECVEDIT));
	if(pRichEdit==NULL)
	{
		return false;
	}
	CHARFORMAT2 cf;
	memset(&cf,0,sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwReserved = 0;
	cf.dwMask = CFM_COLOR  | CFM_BOLD |CFM_ITALIC|CFM_UNDERLINE;
	cf.dwEffects=CFE_ITALIC;
	cf.crBackColor=RGB(255,255,255);
	cf.bUnderlineType = CFU_UNDERLINE;
	cf.crTextColor = RGB(0, 0, 150);

	PARAFORMAT2 pf;
	memset(&pf,0,sizeof(pf));
	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_STARTINDENT;
	pf.dxStartIndent=500;



	TCHAR strTime[MAX_PATH]={0};
	memset(&strTime,0,sizeof(strTime));
	SYSTEMTIME time;
	memset(&time,0,sizeof(time));
	GetLocalTime(&time);
	wsprintf(strTime,_T("%d-%d-%d %d:%d:%d\n"),time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond);

	CDuiString str;
	str+=fileName;
	str+=L"  ";

	switch(msgType)
	{
	case TCPMSGTYPE::FILE_STOP:
		if(_tcsicmp(type,WND_FILE_SEND)==0)
		{
			str+=_T("发送失败！\t");
			break;
		}
		else
		{
			str+=_T("接收失败！\t");
		}
		break;
	case TCPMSGTYPE::FILE_FINISH:
		if(_tcsicmp(type,WND_FILE_SEND)==0)
		{
			str+=_T("发送完成！\t");
			break;
		}
		else
		{
			str+=_T("接收完成！\t");
			break;
		}
		break;
	}

	str+=strTime;


	long lSelBegin = 0, lSelEnd = 0;
	lSelEnd = lSelBegin = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelEnd, lSelEnd);
	pRichEdit->ReplaceSel(str, false);

	lSelEnd = pRichEdit->GetTextLength();
	pRichEdit->SetSel(lSelBegin, lSelEnd);
	pRichEdit->SetSelectionCharFormat(cf);
	pRichEdit->SetParaFormat(pf);
	return true;
}

void chat_dialog::OnFinalMessage(HWND hWnd)
{
	m_mainDlg->RemoveChatDlg(this);
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
	return;
}

bool chat_dialog::SetWndEnableState(bool enable)
{
	if(m_isEnabled==enable)
	{
		return true;
	}
	CButtonUI* pBtn=static_cast<CButtonUI*>(m_PaintManager.FindControl(CHATDLG_SENDBTN));
	if(pBtn==NULL)
	{
		return false;
	}
	m_isEnabled=enable;
	pBtn->SetEnabled(m_isEnabled);
	return true;
}

bool chat_dialog::AddNewFileItem(TCPDATA data,FILEOPERATIONTYPE type,FILEINFO* fileInfo)
{
	SetIsHaveFile(true);
	CListUI* pControl=static_cast<CListUI*>(m_PaintManager.FindControl(CHATDLG_FILELIST));
	if(pControl==NULL)
	{
		return false;
	}
	CListContainerElementUI* pElement=NULL;
	if( !m_dlgBuilder.GetMarkup()->IsValid())
	{
		pElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("file_list_item.xml"), (UINT)0, NULL, &m_PaintManager));
	}
	else 
	{
		pElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &m_PaintManager));
	}
	if(pElement==NULL)
	{
		return false;
	}

	CButtonUI* pBtn=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_IMAGE));
	CLabelUI* pLabelFileName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_NAME));
	CLabelUI* pLabelFileSaveName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_SAVENAME));
	CLabelUI* pLabelFileRealName=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_REALNAME));
	CLabelUI* pLabelFileType=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_TYPE));
	CProgressUI* pProgress=static_cast<CProgressUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_PROGRESS));
	CLabelUI* pLabelFileLen=static_cast<CLabelUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_LENGTH));
	CButtonUI* pBtnState=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_OPERATIONBTN));
	CButtonUI* pBtnOpen=static_cast<CButtonUI*>(m_PaintManager.FindSubControlByName(pElement,FILE_OPENBTN));


	if(pBtn==NULL||pLabelFileName==NULL||pLabelFileSaveName==NULL||pLabelFileRealName==NULL||pProgress==NULL||pLabelFileLen==NULL||pBtnState==NULL||pLabelFileType==NULL||pBtnOpen==NULL)
	{
		return false;
	}

	pControl->Add(pElement);
	pElement->SetFixedHeight(ITEM_HEIGHT);

	string tempStr=string(data.m_fileName);
	int size=MultiByteToWideChar(0,0,data.m_fileName,-1,NULL,0);
	TCHAR *strRealName=new TCHAR[size];
	MultiByteToWideChar(0,0,data.m_fileName,-1,strRealName,size);

	//赋值实际名称
	pLabelFileRealName->SetText(strRealName);
	RELEASE( strRealName);

	//赋值短名称
	int index=tempStr.find_last_of("\\");
	tempStr=tempStr.substr(index+1);
	size=MultiByteToWideChar(0,0,tempStr.c_str(),-1,NULL,0);
	TCHAR *strName=new TCHAR[size];
	MultiByteToWideChar(0,0,tempStr.c_str(),-1,strName,size);
	pLabelFileName->SetText(strName);

	//赋值类型
	switch (type)
	{
	case FILEOPERATIONTYPE::FILE_OPE_SEND:
		pLabelFileType->SetText(WND_FILE_SEND);
		pBtnState->NeedUpdate();
		pBtnState->SetText(STATE_FILE_CANCEL);
		pBtnOpen->SetVisible();
		break;
	case FILEOPERATIONTYPE::FILE_OPE_ACCEPT:
		pLabelFileType->SetText(WND_FILE_ACCEPT);
		pBtnState->SetText(STATE_FILE_ACCEPT);
		pBtnOpen->SetVisible(false);
		break;
	default:
		break;
	}

	//进度条置0
	pProgress->SetValue(0);

	//赋值大小
	if(data.m_fileLen>=1024*1024)
	{
		tempStr="0M/";
		double len=(double)(data.m_fileLen)/(1024*1024);
		char strLen[20]={0};
		memset(strLen,0,sizeof(strLen));
		sprintf_s(strLen,"%.2fM",len);
		tempStr+=strLen;
	}
	else
	{
		tempStr="0KB/";
		double len=(double)(data.m_fileLen)/(1024);
		char strLen[20]={0};
		memset(strLen,0,sizeof(strLen));
		sprintf_s(strLen,"%.2fKB",len);
		tempStr+=strLen;
	}
	size=MultiByteToWideChar(0,0,tempStr.c_str(),-1,NULL,0);
	TCHAR *strLen=new TCHAR[size];
	MultiByteToWideChar(0,0,tempStr.c_str(),-1,strLen,size);
	pLabelFileLen->SetText(strLen);
	return true;
}

bool chat_dialog::SetIsHaveFile(bool haveFile)
{
	if(m_isHaveFile==haveFile)
	{
		return true;
	}
	if(haveFile)
	{
		RECT rect;
		GetWindowRect(this->GetHWND(),&rect);
		SetWindowPos(this->GetHWND(),NULL,rect.left,rect.top,rect.right-rect.left+LISTPART_EXTRA_WIDTH,rect.bottom-rect.top,0);
		CVerticalLayoutUI* pVertical=static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(CHATDLG_FILECONTAINER));
		CListUI*  pList=static_cast<CListUI*>(m_PaintManager.FindControl(CHATDLG_FILELIST));
		if(pVertical==NULL||pList==NULL)
		{
			return false;
		}
		pVertical->SetFixedWidth(LISTPART_NORMAL_WIDTH+LISTPART_EXTRA_WIDTH);
		pList->SetVisible();
	}
	else
	{
		RECT rect;
		GetWindowRect(this->GetHWND(),&rect);
		SetWindowPos(this->GetHWND(),NULL,rect.left,rect.top,rect.right-rect.left-LISTPART_EXTRA_WIDTH,rect.bottom-rect.top,0);
		CVerticalLayoutUI* pVertical=static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(CHATDLG_FILECONTAINER));
		CListUI*  pList=static_cast<CListUI*>(m_PaintManager.FindControl(CHATDLG_FILELIST));
		if(pVertical==NULL||pList==NULL)
		{
			return false;
		}
		pVertical->SetFixedWidth(LISTPART_NORMAL_WIDTH);
		pList->SetVisible(false);
	}
	m_isHaveFile=haveFile;
	return true;
}

void chat_dialog::SetIsGroupChatDlg(WNDTYPE type)
{
	if(type==WNDTYPE::WND_MUTLICAST)
	{
		CHorizontalLayoutUI* pHorizontal=static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(CHATDLG_FILECONTAINEREMPTY));
		CVerticalLayoutUI* pVertical=static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(CHATDLG_FILECONTAINER));
		CListUI*  pList=static_cast<CListUI*>(m_PaintManager.FindControl(CHATDLG_MEMBERLIST));
		if(pVertical==NULL||pList==NULL||pHorizontal==NULL)
		{
			return;
		}
		pVertical->SetFixedWidth(LISTPART_GROUPEXTRA_WIDTH);
		pHorizontal->SetVisible();
		pList->SetVisible();
	}
}

CControlUI* chat_dialog::GetGoalCtrl(CControlUI* srcCtrl,LPCTSTR className)
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

LRESULT chat_dialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)  
{  
	switch (uMsg)  
	{  
	case WM_DROPFILES:  
		{  
			DropFileOnDlg((HDROP)wParam);  
		}  
		break;  
	default:  
		break;  
	}  
	return __super::HandleMessage(uMsg, wParam, lParam);  
}  

void chat_dialog::DropFileOnDlg(HDROP hDrop)  
{  
	if(m_type==WNDTYPE::WND_MUTLICAST)
	{
		return;
	}
	WORD                wNumFilesDropped = DragQueryFile(hDrop, -1, NULL, 0);  
	WORD                wPathnameSize = 0;  
	WCHAR *             pFilePathName = NULL;  
	std::wstring        strFirstFile = L"";  
	string              strFileName;
	TCPDATA  data;
	CLabelUI* pLabel=static_cast<CLabelUI*>(m_PaintManager.FindControl(CHATDLG_DES));
	if(pLabel==NULL)
	{
		return;
	}
	CDuiString wstrIP=pLabel->GetText();
	// there may be many, but we'll only use the first  
	for (int index=0;index<wNumFilesDropped ;++index)  
	{  
		wPathnameSize = DragQueryFile(hDrop, index, NULL, 0);  
		wPathnameSize++;  
		pFilePathName = new WCHAR[wPathnameSize];  
		if (NULL == pFilePathName)  
		{  
			DragFinish(hDrop);  
			return;  
		}  
		memset(pFilePathName, 0,sizeof(pFilePathName));  
		DragQueryFile(hDrop, index, pFilePathName, wPathnameSize);  
		strFirstFile = pFilePathName;
		int size=WideCharToMultiByte(0,0,pFilePathName,-1,NULL,0,NULL,NULL);
		char *cstrFile=new char[size];
		WideCharToMultiByte(0,0,pFilePathName,-1,cstrFile,size,NULL,NULL);
		strFileName=cstrFile;
		RELEASE(pFilePathName); 
		RELEASE(cstrFile);

		if(m_mainDlg->GetFileInfoByRealName(strFileName.c_str()))
		{
			continue;
		}

		int realLen=0;
		FILE* fileHandle=NULL;

		fopen_s(&fileHandle,strFileName.c_str(),"rb");
		if(fileHandle==NULL)
		{
			continue;
		}

		fseek(fileHandle,0,SEEK_END);
		realLen=ftell(fileHandle);
		fclose(fileHandle);

		data.m_fileLen=realLen;
		memcpy(data.m_fileName,strFileName.c_str(),(strFileName.size()+1)*sizeof(char));
		data.m_type=TCPMSGTYPE::FILE_SEND;

		m_mainDlg->ConnectServer(strFirstFile.c_str(),wstrIP.GetData());
		FILEINFO* fileInfo=m_mainDlg->GetFileInfoByRealName(strFileName.c_str());
		if(fileInfo)
		{
			AddNewFileItem(data,FILEOPERATIONTYPE::FILE_OPE_SEND,fileInfo);
		}
	}  
	DragFinish(hDrop);   
}  

BOOL chat_dialog::AllowMeesageForVistaAbove(UINT uMessageID, BOOL bAllow)
{
	BOOL bResult = FALSE;
	HMODULE hUserMod = NULL;

	hUserMod = LoadLibrary(_T("user32.dll"));
	if( NULL == hUserMod )
	{
		return FALSE;
	}

	bResult = ChangeWindowMessageFilter( uMessageID, bAllow ? 1 : 2 );//MSGFLT_ADD: 1, MSGFLT_REMOVE: 2

	if( NULL != hUserMod )
	{
		FreeLibrary( hUserMod );
	}

	return bResult;
}


LRESULT chat_dialog::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled=FALSE;
	CControlUI* pAudioCtrl=NULL;

	CPoint point;
	RECT rect;

	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		//OutputDebugString(L"wm_lbuttondown");
		point.x = LOWORD(lParam);
		point.y = HIWORD(lParam);
		pAudioCtrl=m_PaintManager.FindControl(AUDIO_CONTROL);
		if(pAudioCtrl==NULL)
		{
			break;
		}
		rect = pAudioCtrl->GetPos();
		if(IsPointInRect(point,rect))
		{
			OutputDebugString(L"audiobtn\r\n");
			caudio::GetInstance()->WaveInWork();
			SetTimer(GetHWND(),SENDAUDIOMSG_TIMER,50,NULL);
			pAudioCtrl->SetText(L"语音...");

		}
		break;
	case WM_LBUTTONUP:
		OutputDebugString(L"wm_lbuttonup\r\n");
		caudio::GetInstance()->WaveInStopWorking();
		pAudioCtrl=m_PaintManager.FindControl(AUDIO_CONTROL);
		if(pAudioCtrl!=NULL)
		{
			pAudioCtrl->SetText(L"发送录音消息");
		}
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case SENDAUDIOMSG_TIMER:
			{
				if(!caudio::GetInstance()->IsWaveInWorking())
				{
					caudio::GetInstance()->WaveOutWork();
					//cvideo::GetInstance()->OpenVideoCapture();
					KillTimer(GetHWND(),SENDAUDIOMSG_TIMER);
				}
			}
		default:
			break;
		}
		bHandled=TRUE;
	default: break;
	}
	return 0;
}


const bool chat_dialog::IsPointInRect(const DuiLib::CPoint point,const RECT rect)
{
	if(rect.left<=point.x&&rect.right>=point.x&&rect.top<=point.y&&rect.bottom>=point.y)
	{
		return true;
	}
	return false;
}