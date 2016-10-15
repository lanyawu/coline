// TestLoadCom.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../TestSvrCom/SvrInterface.h"
#include "../include/combase.h"
#include <time.h>
LPDLL_GET_CLASS_OBJECT m_pGetObject;
int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(0);
	HRESULT hr;
	/*HMODULE hModule = LoadLibrary(L"TestSvrCom.dll");
	if (hModule)
		m_pGetObject = (LPDLL_GET_CLASS_OBJECT) ::GetProcAddress(hModule, "DllGetClassObject");
	else
	{
		printf("GetModule Failed\n");
		return 0;
	}
	ITestSvrInterface *pTest = NULL;
	hr = m_pGetObject(CLSID_TEST_SVR, __uuidof(ITestSvrInterface), (void **)&pTest);	*/
	ITestSvrInterface *pTest = NULL;
	hr = CoCreateInstance(CLSID_TEST_SVR,				// CLSID of COM server
						  NULL,						//
						  CLSCTX_INPROC_SERVER,		// it is a DLL 
						  __uuidof(ITestSvrInterface),	// the interface IID
						  (void**)&pTest			// 
						  );

	if(FAILED(hr))
	{
		printf("failed to initialize COM server");
		return 0;
	}

	long value = 10;
	printf("Enter a number : ") , scanf("%ld",&value);

	hr = pTest->Cube(&value);  // Call interface method 
	if(FAILED(hr))
		printf("failed square of ImyInterface.\n");
	else
		printf("Cube of the number is %ld. \n",value);

	ITestSubSvrInterface *pSubSvr = NULL;
	hr = pTest->QueryInterface(__uuidof(ITestSubSvrInterface), (void **)&pSubSvr);
	if (FAILED(hr))
	{
		printf("get sub svr failed\n");
	} else
	{
		pSubSvr->SetNameById(120, "120_Name");
		pSubSvr->SetNameById(1, "1_Name");
		int nId = time(NULL) % 97;
		char szTmp[16] = {0};
		::itoa(nId, szTmp, 10);
		printf("set %d Name:%s\n", nId, szTmp);
		pSubSvr->SetNameById(nId, szTmp);
		char szName[MAX_PATH] = {0};
		if (pSubSvr->GetNameById(120, szName) == S_OK)
		{
			printf("120 Name:%s\n", szName);
		} else
			printf("Get 120 Name Failed\n");
		if (pSubSvr->GetNameById(12, szName) == S_OK)
		{
			printf("12 Name:%s\n", szName);
		} else
		{
			printf("get 12 Name Failed\n");
		}
		scanf("%d", &nId);
		if (pSubSvr->GetNameById(nId, szName) == S_OK)
		{
			printf("%d Name:%s\n", nId, szName);
		} else
		{
			printf("get %d Name Failed\n", nId);
		}
		pSubSvr->Release();
	}
	pTest->Release();	// dont forget to do this ....
	//::FreeLibrary(hModule);
}

