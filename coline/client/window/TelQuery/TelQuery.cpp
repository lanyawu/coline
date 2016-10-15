#define WIN32_LEAN_AND_MEAN	
#include <combase.h>
#include <classfactory.h>
#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include "TelQueryImpl.h"
#include "../IMCommonLib/IMDllRegistar.h"

 // {49A9CDB4-6C36-405C-ADB1-FA8ABF1B24FE}
_declspec(selectany) GUID CLSID_CORE_TELQUERY  =  { 0x49a9cdb4, 0x6c36, 0x405c, 
                                                  { 0xad, 0xb1, 0xfa, 0x8a, 0xbf, 0x1b, 0x24, 0xfe } };
 




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
    if (IsEqualIID(rclsid, CLSID_CORE_TELQUERY))
    {
       // 为 CmyInterface 声明类工厂
       CClassFactory<CTelQueryImpl> *pcf = new  CClassFactory<CTelQueryImpl>;
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
	return CPluginRegistar::RegisterPlugin(CLSID_CORE_TELQUERY, "TelQueryLib", "TelQueryObj", szPath,
		                                  "电话查询插件", PLUGIN_TYPE_EXTERNAL, __uuidof(ITelQuery)) ? S_OK:S_FALSE;
}

STDAPI DllUnregisterServer(void)
{
	return CPluginRegistar::UnRegisterPlugin(CLSID_CORE_TELQUERY, "TelQueryLib", "TelQueryObj",
		                    PLUGIN_TYPE_EXTERNAL) ? S_OK:S_FALSE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
