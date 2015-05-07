#ifndef _CHAT_DIALOG_H_
#define _CHAT_DIALOG_H_

#include <UIlib.h>

class main_frame;

class chat_dialog :
	public DuiLib::WindowImplBase
{
public:
	chat_dialog(WNDTYPE type,DuiLib::CDuiString BGI,DuiLib::CDuiString logo,DuiLib::CDuiString name,DuiLib::CDuiString description);
	~chat_dialog(void);
public:
	virtual LPCTSTR GetWindowClassName() const;
	virtual DuiLib::CDuiString GetSkinFile();
	virtual DuiLib::CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void Notify(DuiLib::TNotifyUI& msg);
	virtual void OnFinalMessage(HWND hWnd);
	virtual void DropFileOnDlg(HDROP hDrop);  
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BOOL AllowMeesageForVistaAbove(UINT uMessageID, BOOL bAllow);
	void SetMainDlg(main_frame* mainDlg) { m_mainDlg=mainDlg; }
	bool SetBGI(DuiLib::CDuiString image);
	bool SetName(DuiLib::CDuiString name);
	bool SetLogo(DuiLib::CDuiString logo);
	bool SetDescription(DuiLib::CDuiString description);
	DuiLib::CDuiString GetBGI() { return m_BGI; }
	DuiLib::CDuiString GetLogo() { return m_logo; }
	DuiLib::CDuiString GetName() { return m_name; }
	DuiLib::CDuiString GetDescription() { return m_description; }
	bool RecvMessage(DuiLib::CDuiString name,DuiLib::CDuiString message);
	bool RecvMessage(DuiLib::CDuiString fileName,DuiLib::CDuiString type,TCPMSGTYPE msgType);
	bool SetWndEnableState(bool enable=true);
	bool AddNewFileItem(TCPDATA data,FILEOPERATIONTYPE type,FILEINFO* fileInfo);
	bool SetIsHaveFile(bool haveFile);
	void SetIsGroupChatDlg(WNDTYPE type);
	DuiLib::CControlUI* GetGoalCtrl(DuiLib::CControlUI* srcCtrl,LPCTSTR className);
	const bool IsPointInRect(const DuiLib::CPoint,const RECT rect); 
private:
	main_frame* m_mainDlg;
	DuiLib::CDuiString m_BGI;
	DuiLib::CDuiString m_logo;
	DuiLib::CDuiString m_name;
	DuiLib::CDuiString m_description;
	bool m_isEnabled;
	WNDTYPE m_type;
	DuiLib::CDialogBuilder m_dlgBuilder;
	bool m_isHaveFile;
};

#endif

