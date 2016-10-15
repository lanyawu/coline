#include "DebugLog.h"
#include <SmartLog/SmartLog.h>

CDebugLog CDebugLog::m_Log(dtInfo);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
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

BOOL CALLBACK SetSmartLogType(int nType)
{
	CDebugLog::m_Log.SetWriteDebugType(nType);
	return TRUE;
}

BOOL CALLBACK SetSmartLogIsWrite(BOOL bWrite)
{
	CDebugLog::m_Log.SetWriteLog(bWrite);
	return TRUE;
}

BOOL CALLBACK PrintSmartLog(int nType, const char *szLog)
{
	DEBUGTYPE dtType = (DEBUGTYPE)nType;
	if (dtError == dtType)
	{
		CDebugLog::m_Log.AddErrorLog(szLog);
	} else
	{
		CDebugLog::m_Log.AddLog(dtType, szLog);
	}
	return TRUE;
}
