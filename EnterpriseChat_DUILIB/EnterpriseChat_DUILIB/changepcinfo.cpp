#include "changepcinfo.h"

using namespace DuiLib;

changepcinfo::changepcinfo(void)
{
}


changepcinfo::~changepcinfo(void)
{
}

LPCTSTR changepcinfo::GetWindowClassName() const
{
	return _T("changepcinfoClass");
}

CDuiString changepcinfo::GetSkinFile()
{
	return _T("changepcinfo.xml");
}

CDuiString changepcinfo::GetSkinFolder()
{
	return _T("");
}

void changepcinfo::InitWindow()
{

}

void changepcinfo::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
	return;
}

void changepcinfo::Notify(DuiLib::TNotifyUI& msg)
{
	__super::Notify(msg);
}