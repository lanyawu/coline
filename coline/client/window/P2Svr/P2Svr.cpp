#include "P2SvrManager.h"

CP2SvrManager m_P2SvrManager;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: 
			 break;
		case DLL_THREAD_ATTACH:
			 break;
		case DLL_THREAD_DETACH:
			 break;
		case DLL_PROCESS_DETACH:			 
			 break;
    }
    return TRUE;
}
 
BOOL CALLBACK P2SvrInit()
{
	return  (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK); 
}

BOOL CALLBACK P2SvrDestroy()
{
	curl_global_cleanup();
	return TRUE;
}

BOOL CALLBACK P2SvrCancelTask(HANDLE hTask)
{
	return m_P2SvrManager.CancelTask(hTask);
}

HANDLE CALLBACK P2SvrAddDlTask(const char *szUrl, const char *szLocalFileName, int nType,
	                         void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait)
{
	return m_P2SvrManager.AddTask(szUrl, szLocalFileName, nType, pOverlapped, pCallBack, bWait, FALSE);
}

HANDLE CALLBACK P2SvrPostFile(const char *szUrl, const char *szLocalFileName, const char *szParams, int nType,
	                        void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait)
{
	return m_P2SvrManager.PostHttpFile_(szUrl, szLocalFileName, szParams, nType,
		                    pOverlapped, pCallBack, bWait);
}

HANDLE CALLBACK P2SvrAddUpTask(const char *szUrl, const char *szLocalFileName, int nType,
	                          void *pOverlapped, LPP2SVRCALLBACK pCallBack, BOOL bWait)
{
	return m_P2SvrManager.AddTask(szUrl, szLocalFileName, nType, pOverlapped, pCallBack, bWait, TRUE);
}
