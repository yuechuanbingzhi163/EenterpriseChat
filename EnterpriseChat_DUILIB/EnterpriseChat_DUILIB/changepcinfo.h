#ifndef _CHANGEPCINFO_H_
#define _CHANGEPCINOF_H_
#include <UIlib.h>
class changepcinfo :
	public DuiLib::WindowImplBase
{
public:
	changepcinfo(void);
	~changepcinfo(void);
	virtual DuiLib::CDuiString GetSkinFile();
	virtual DuiLib::CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void OnFinalMessage(HWND hWnd);
	virtual void Notify(DuiLib::TNotifyUI& msg);
	virtual LPCTSTR GetWindowClassName() const;
};
#endif _CHANGEPCINFO_H_

