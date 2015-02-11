#ifndef _MAIN_FRAME_H__
#define _MAIN_FRAME_H__

#include "IOCPModel.h"
#include "chat_dialog.h"
#include <UIlib.h>
#include <map>
#include "commonlist.h"

class main_frame :
	public DuiLib::WindowImplBase
{
public:
	main_frame(void);
	~main_frame(void);
public:
	virtual LPCTSTR GetWindowClassName() const;
	virtual DuiLib::CDuiString GetSkinFile();
	virtual DuiLib::CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void Notify(DuiLib::TNotifyUI& msg);
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
public:
	bool AddNewChatDlg(WNDTYPE type,DuiLib::CListUI* pControl);
	bool AddNewChatDlg();
	bool AddNewChatDlg(const char* strIP);
	bool RemoveChatDlg(chat_dialog* chatDlg);
	bool AddNewFriend(FriendListItemInfo friendInfo);
	bool AddNewGroup(FriendListItemInfo friendInfo);
	int  GetFriendIndex(FriendListItemInfo friendInfo);
	bool RemoveFriend(FriendListItemInfo friendInfo);
	bool RemoveGroup(FriendListItemInfo friendInfo);
	bool SetImage(const char *strImage);
	bool SetName(const char *strName);
	chat_dialog* FindChatDlgByIP(DuiLib::CDuiString strIP);
	DuiLib::CDuiString GetName();
	bool RecvUDPMessage(const char* strIP,UDPDATA data);
	bool SendUDPMessage(WNDTYPE type,DuiLib::CDuiString description,DuiLib::CDuiString message);
	bool RecvTCPMessage(PER_IO_CONTEXT* pIoContext);
	DuiLib::CDuiString GetNameByDescription(DuiLib::CDuiString description);
	bool SetChatDlgEnabledState(const char* strIP,bool enable=true);
	DuiLib::CListContainerElementUI* GetElementByDescription(DuiLib::CDuiString description,DuiLib::CListUI* pControl);
	FILEINFO* GetFileInfoByRealName(const char* realName);
	FILEINFO* GetFileInfoByIoContext(const PER_IO_CONTEXT* pIoContext);
	bool SendFile(PER_IO_CONTEXT* pIoContext,const char* fileName);
	bool ApproveAccpetRequest(LPCTSTR fileName);
	bool WriteDataInFile(PER_IO_CONTEXT* pIoContext);
	DuiLib::CControlUI* GetGoalCtrl(DuiLib::CControlUI* srcCtrl,LPCTSTR className);
	bool FreshFriendsList();
	bool RemoveFileInfo(FILEINFO* fileInfo);
	bool RemoveFileInfo(DuiLib::CDuiString fileName);
	bool ConnectServer(LPCTSTR fileName,LPCTSTR ip);
	void AddFileInfo(FILEINFO* fileInfo);
	BOOL AddNotificationIcon(HWND hwnd);
	BOOL DeleteNotificationIcon(HWND hwnd);
	bool InitListFloder();
private:
	std::map<DuiLib::CDuiString,chat_dialog*> m_mapChatDlg;
	DuiLib::CDialogBuilder m_dlgBuilder;
	CIOCPModel m_IOCP;
	std::list<FILEINFO*> m_listFileInfo;
	Node* m_friendParentNode;
	Node* m_groupParentNode;
};
#endif
