// SmartUtils.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <Registrar.h>
#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include <combase.h>
#include <classfactory.h>
#include <SmartUtils/SmartUtilsInterface.h>

#include "SmartStreamImpl.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

long g_cRefThisDll;
 
long * CObjRoot::p_ObjCount = NULL; 

HMODULE g_hModule = NULL;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: 
			 g_hModule = hModule;
			 CObjRoot::p_ObjCount = &g_cRefThisDll; 
			 break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			 break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
	*ppvOut = NULL;
    if (IsEqualIID(rclsid, __uuidof(ISmartStream)))
    {
		CClassFactory<CSmartStreamImpl> *pcf = new CClassFactory<CSmartStreamImpl>;
		return pcf->QueryInterface(riid, ppvOut);
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI  DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllRegisterServer(void)
{
	// 这个类应该创建标准的注册表入口 
	char szPath[MAX_PATH] = {0};
	CSystemUtils::GetModuleAppFileName(g_hModule, szPath, MAX_PATH - 1); 
	BOOL bSucc = CDllRegistrar::RegisterObject(CLSID_SMART_UTILS, "SmartUtilsLib", "SmartUtilsObj", szPath) ? S_OK:S_FALSE;
	bSucc = bSucc && CDllRegistrar::RegisterObject(__uuidof(ISmartStream), "SmartUtilsLib", "SmartStream", szPath) ? S_OK : S_FALSE;
	return bSucc;
}

STDAPI DllUnregisterServer(void)
{
	BOOL bSucc =  CDllRegistrar::UnRegisterObject(CLSID_SMART_UTILS, "SmartUtilsLib", "SmartUtilsObj") ? S_OK : S_FALSE;
	bSucc = bSucc && CDllRegistrar::UnRegisterObject(__uuidof(ISmartStream), "SmartUtilsLib", "SmartStream") ? S_OK : S_FALSE;
	return bSucc;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

