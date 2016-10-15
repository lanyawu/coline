// SmartSync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Commonlib/DebugLog.h>
#include "PathSync.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:  
			 break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			 break;
    }
    return TRUE;
}

BOOL CALLBACK  SmartSyncPath(const char *szSrcPath, const char *szDestPath,
	                         LPSMARTSYNCCALLBACK pCallBack)
{
	CPathSync Sync;
	BOOL b = Sync.SyncPath(szSrcPath, szDestPath, pCallBack);
	if (pCallBack)
		pCallBack(SMARTSYNC_TYPE_COMPLETE, NULL, NULL, NULL, NULL, NULL,
		           Sync.GetDirCount(), Sync.GetFileCount());
	return b;
}

BOOL CALLBACK  EstablishSyncPath(const char *szPath,  LPSMARTSYNCCALLBACK pCallBack)
{
	//
	CPathSync Sync;
	BOOL b = Sync.InitFileList(szPath, pCallBack);
	if (pCallBack)
		pCallBack(SMARTSYNC_TYPE_COMPLETE, NULL, NULL, NULL, NULL, NULL,
		          Sync.GetDirCount(), Sync.GetFileCount());
	return b;
}

BOOL CALLBACK  AnalyseSyncPath(const char *szSrcPath, const char *szDestPath,
	                            LPSMARTSYNCCALLBACK pCallBack)
{
	CPathSync Sync;
	BOOL b = Sync.AnalyseSync(szSrcPath, szDestPath, pCallBack);
	if (pCallBack)
		pCallBack(SMARTSYNC_TYPE_COMPLETE, NULL, NULL, NULL, NULL, NULL,
		           Sync.GetDirCount(), Sync.GetFileCount());
	return b;
}
