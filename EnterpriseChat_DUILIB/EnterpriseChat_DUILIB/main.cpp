#ifndef _MAIN_H__
#define _MAIN_H__

#include "IOCPModel.h"
#include "stdafx.h"
#include "main_frame.h"
#include <WinInet.h>

#include <atlbase.h>
CComModule _Module;
#include <atlwin.h>

#include <UIlib.h>

using namespace DuiLib;

#pragma comment(lib,"WS2_32.LIB")
#pragma comment(lib,"Shell32.lib")
#pragma comment(lib,"Wininet.lib")

#ifdef _DEBUG
#   ifdef _UNICODE
#       pragma comment(lib, "DuiLib_ud.lib")
#   else
#       pragma comment(lib, "DuiLib_d.lib")
#   endif
#else
#   ifdef _UNICODE
#       pragma comment(lib, "DuiLib_u.lib")
#   else
#       pragma comment(lib, "DuiLib.lib")
#   endif
#endif

#if defined(WIN32) && !defined(UNDER_CE)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
#endif
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

	HINSTANCE hInstRich = ::LoadLibrary(_T("Riched20.dll"));

	::CoInitialize(NULL);
	::OleInitialize(NULL);

	_Module.Init( 0, hInstance );

#if defined(WIN32) && !defined(UNDER_CE)
	HRESULT Hr = ::CoInitialize(NULL);
#else
	HRESULT Hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
	if( FAILED(Hr) ) return 0;

	DWORD flag=0;
	if(!InternetGetConnectedState(&flag,0))
	{
		MessageBox(NULL,_T("未连接到互联网或局域网,请在确保有外部连接的情况下使用本软件！"),NULL,0);
		return 0;
	}

	main_frame* pFrame = new main_frame(); 
	if( pFrame == NULL ) return 0;
#if defined(WIN32) && !defined(UNDER_CE)
	pFrame->Create(NULL, _T("EChat"), UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 600, 800);
#else
	pFrame->Create(NULL, _T("EChat"), UI_WNDSTYLE_FRAME, WS_EX_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
#endif

	pFrame->ShowModal();
	/*::ShowWindow(*pFrame, SW_SHOW);
	CPaintManagerUI::MessageLoop();
	CPaintManagerUI::Term();*/

	_Module.Term();

	::OleUninitialize();
	::CoUninitialize();

	::FreeLibrary(hInstRich);

	return 0;
}




#endif