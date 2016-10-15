#define WIN32_LEAN_AND_MEAN	
#include <combase.h>
#include <classfactory.h>
#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include "GPlusImpl.h"
#include "../IMCommonLib/IMDllRegistar.h"

 
// 
_declspec(selectany) GUID CLSID_CORE_GPLUS  =  { 0xcd9645d, 0x2ae1, 0x4fc5, 
                                               { 0xb8, 0x28, 0xa6, 0x79, 0xda, 0x9e, 0x2e, 0xb3 } };
 
 
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
    if (IsEqualIID(rclsid, CLSID_CORE_GPLUS))
    {
       // 为 CmyInterface 声明类工厂
       CClassFactory<CGPlusImpl> *pcf = new  CClassFactory<CGPlusImpl>;
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
	return CPluginRegistar::RegisterPlugin(CLSID_CORE_GPLUS, "GPlusLib", "GPlusObj", szPath,
		                                  "CoLine Plus插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IGPlus)) ? S_OK:S_FALSE;
}

STDAPI DllUnregisterServer(void)
{
	return CPluginRegistar::UnRegisterPlugin(CLSID_CORE_GPLUS, "GPlusLib", "GPlusObj",
		                    PLUGIN_TYPE_EXTERNAL) ? S_OK:S_FALSE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
