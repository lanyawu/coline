// CutImage.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Commonlib/DebugLog.h>
#include "ScreenDialog.h"
#include "CutImage.h"

#ifdef _DEBUG
CDebugLog CDebugLog::m_Log(dtInfo);
#endif

 
HINSTANCE g_hModule = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			 g_hModule = (HINSTANCE)hModule;
			 //COM³õÊ¼»¯
	         ::CoInitialize(NULL);
    	     //DirectX9¼ì²â
			 ::LoadLibrary(_T("d3d9.dll"));		 
			 break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			 break;
	}
	return TRUE;
}

int CALLBACK CutImage(HWND hParent, BOOL bHideParent)
{
	CScreenDialog *Dlg = new CScreenDialog();
	return Dlg->CaptureScreen(g_hModule, hParent, bHideParent);
}
