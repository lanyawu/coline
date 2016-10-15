#define WIN32_LEAN_AND_MEAN	
#include <combase.h>
#include <classfactory.h>
#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include "widgetboximpl.h"
#include "../IMCommonLib/IMDllRegistar.h"

// {92A2FE50-8379-403E-9995-AE80AF267D75}
_declspec(selectany) GUID CLSID_CORE_WIDGETBOX  =  { 0x92a2fe50, 0x8379, 0x403e, 
                                                   { 0x99, 0x95, 0xae, 0x80, 0xaf, 0x26, 0x7d, 0x75 } };

 




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
    if (IsEqualIID(rclsid, CLSID_CORE_WIDGETBOX))
    {
       // 为 CmyInterface 声明类工厂
       CClassFactory<CWidgetBoxImpl> *pcf = new  CClassFactory<CWidgetBoxImpl>;
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
	return CPluginRegistar::RegisterPlugin(CLSID_CORE_WIDGETBOX, "WidgetBoxLib", "WidgetBoxObj", szPath,
		                                  "WidgetBox工具盒插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IWidgetBox)) ? S_OK:S_FALSE;
}

STDAPI DllUnregisterServer(void)
{
	return CPluginRegistar::UnRegisterPlugin(CLSID_CORE_WIDGETBOX, "WidgetBoxLib", "WidgetBoxObj",
		                    PLUGIN_TYPE_EXTERNAL) ? S_OK:S_FALSE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
