#ifndef _POPMENU_H_
#define _POPMENU_H_

#include <UIlib.h>

class main_frame;

class popmenu :
	public DuiLib::WindowImplBase
{
public:
	explicit popmenu(LPCTSTR pszXMLPath): m_strXMLPath(pszXMLPath){ }
	virtual LPCTSTR    GetWindowClassName()const{ return _T("CDuiMenu "); }
	virtual DuiLib::CDuiString GetSkinFolder()          { return _T("");  }
	virtual DuiLib::CDuiString GetSkinFile()            { return m_strXMLPath; }
	virtual void OnFinalMessage(HWND hWnd){ delete this; }
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void Init(HWND hWndParent, POINT ptPos);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Notify(DuiLib::TNotifyUI& msg);
	DuiLib::CControlUI* GetGoalCtrl(DuiLib::CControlUI* srcCtrl,LPCTSTR className);
	void SetMainDlg(main_frame* mainDlg)  { m_mainDlg=mainDlg; }
	bool SetItemEnable(bool enable=true);
protected:
	~popmenu() {} ;
private:
	DuiLib::CDuiString  m_strXMLPath;
	main_frame* m_mainDlg;

};
#endif

