#define WIN32_LEAN_AND_MEAN	
#include <combase.h>
#include <classfactory.h>
#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include "BMCFrameImpl.h"
#include "../IMCommonLib/IMDllRegistar.h"

 // {8EBF3B16-CA3B-44C5-9532-FE71903AEF0D}
_declspec(selectany) GUID CLSID_CORE_BMCFRAME    =  { 0x8ebf3b16, 0xca3b, 0x44c5, 
                                                    { 0x95, 0x32, 0xfe, 0x71, 0x90, 0x3a, 0xef, 0xd } };



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
    if (IsEqualIID(rclsid, CLSID_CORE_BMCFRAME))
    {
       // 为 CmyInterface 声明类工厂
       CClassFactory<CBMCFrameImpl> *pcf = new  CClassFactory<CBMCFrameImpl>;
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
	return CPluginRegistar::RegisterPlugin(CLSID_CORE_BMCFRAME, "BMCFrameLib", "BMCFrameObj", szPath,
		                                  "业务消息中心", PLUGIN_TYPE_EXTERNAL, __uuidof(IBMCFrame)) ? S_OK:S_FALSE;
}

STDAPI DllUnregisterServer(void)
{
	return CPluginRegistar::UnRegisterPlugin(CLSID_CORE_BMCFRAME, "BMCFrameLib", "BMCFrameObj",
		                    PLUGIN_TYPE_EXTERNAL) ? S_OK:S_FALSE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
