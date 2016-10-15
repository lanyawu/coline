// TestSvrCom.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SvrInterfaceImpl.h"
#include <Registrar.h>
#include <commonlib/systemutils.h>
#include <combase.h>
#include <classfactory.h>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

long g_cRefThisDll;
// ����Ψһ��һ��ȫ�̱������Ҳ�����������ʹ���κ�ȫ�̱���
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
    if (IsEqualIID(rclsid, CLSID_TEST_SVR))
    {
       // Ϊ CmyInterface �����๤��
       CClassFactory<CTestSvrInterfaceImpl> *pcf = new  CClassFactory<CTestSvrInterfaceImpl>;
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
	// �����Ӧ�ô�����׼��ע������ 
	char szPath[MAX_PATH] = {0};
	CSystemUtils::GetModuleAppFileName(g_hModule, szPath, MAX_PATH - 1); 
	return CDllRegistrar::RegisterObject(CLSID_TEST_SVR,"TestSvrLib", "TestSvrObj", szPath) ? S_OK:S_FALSE;
}

STDAPI DllUnregisterServer(void)
{
	return CDllRegistrar::UnRegisterObject(CLSID_TEST_SVR,"TestSvrLib", "TestSvrObj") ? S_OK:S_FALSE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

