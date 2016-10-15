#include <time.h>
#include <stdio.h>
#include <string.h>
#include <Commonlib/StringUtils.h>
#include "DebugLog.h"

#pragma warning(disable:4996)

#ifdef _DEBUG
   #include <shlwapi.h>
   #pragma comment(lib, "shlwapi.lib")
#endif


#define lengthof(x) (sizeof(x)/sizeof(*x))
void  ___Trace(LPCTSTR pstrFormat, ...)
{
#ifdef _DEBUG
   TCHAR szBuffer[300] = { 0 };
   va_list args;
   va_start(args, pstrFormat);
   ::wvnsprintf(szBuffer, lengthof(szBuffer) - 2, pstrFormat, args);
   _tcscat(szBuffer, _T("\n"));
   va_end(args);
   ::OutputDebugString(szBuffer);
#endif
}

/*void CreateDebugLogInstance(DEBUGTYPE DebugType)
{
	if (!g_DebugLog)
		g_DebugLog = new CDebugLog(DebugType);
}

void DestroyDebugLogInstance()
{
	if (g_DebugLog)
		delete g_DebugLog;
	g_DebugLog = NULL;
}
*/
CDebugLog::CDebugLog(DEBUGTYPE DebugType):
           m_bWriteLog(TRUE),
		   m_bTerminated(TRUE),
		   m_hBreak(NULL),
		   m_hSaveLogThd(NULL),
		   m_DebugType(DebugType)
{
	m_hBreak = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSaveLogThd = CreateThread(NULL, 0, __SaveLogThread, this, 0, NULL);
}

CDebugLog::~CDebugLog()
{
	Terminate();
}

void CDebugLog::SetWriteLog(BOOL bWriteLog) 
{
	m_bWriteLog = bWriteLog; 
}

void CDebugLog::GetCurrLogFileName(char *szLogFileName, bool IsError)
{
#ifdef _UNICODE
	TCHAR cBuffer[_MAX_PATH] = {0};
	TCHAR cDrive[_MAX_DRIVE] = {0};
	TCHAR cDir[_MAX_DIR] = {0};
	TCHAR cFileName[_MAX_FNAME] = {0};
	TCHAR cExt[_MAX_EXT] = {0};

	GetModuleFileName(NULL, cBuffer, _MAX_PATH);

	_wsplitpath_s(cBuffer, cDrive, _MAX_DRIVE, cDir, _MAX_DIR, cFileName, _MAX_FNAME, cExt, _MAX_EXT); 	

    time_t now = time(NULL);
	tm *t = localtime(&now);
	TCHAR szFileName[1024] = {0};
	// 写错误日志
	if (IsError)
		wsprintf(szFileName, _T("%s%slog\\%serrorr-%02d-%02d-%02d.log"), cDrive, cDir,cFileName, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	else
		wsprintf(szFileName, _T("%s%slog\\%slog-%02d-%02d-%02d.log"), cDrive, cDir,cFileName, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday); 
	CStringConversion::WideCharToString(szFileName, szLogFileName, 1024);
#else
	char cBuffer[_MAX_PATH];
	char cDrive[_MAX_DRIVE];
	char cDir[_MAX_DIR];
	char cFileName[_MAX_FNAME];
	char cExt[_MAX_EXT];

	GetModuleFileName(NULL, cBuffer, _MAX_PATH);

	_splitpath_s(cBuffer, cDrive, cDir, cFileName, cExt); 	

    time_t now = time(NULL);
	tm *t = localtime(&now);
	// 写错误日志
	if (IsError)
		sprintf_s(szLogFileName, _MAX_PATH, "%s%slog\\%serrorr-%02d-%02d-%02d.log", cDrive, cDir,cFileName, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	else
		sprintf_s(szLogFileName,_MAX_PATH, "%s%slog\\%slog-%02d-%02d-%02d.log", cDrive, cDir,cFileName, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
#endif
}

//立即写入文件
void CDebugLog::AddErrorLog(const char *lpszLog, BOOL bNeedTimeInfo)
{

    time_t now = time(NULL);
	tm *t = localtime(&now);
	char szLogFile[_MAX_PATH];
	GetCurrLogFileName(szLogFile, true);
	CGuardLock::COwnerLock Lock(m_ErrorLock);
	FILE *fp;
	fp = fopen(szLogFile, "a+b");
	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		char szLog[8192] = {0};
		try
		{
			if(bNeedTimeInfo)
			{
				 _snprintf(szLog, 8191, "%02d-%02d-%02d %02d:%02d:%02d  错误: %s \n",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, lpszLog);
			}
			else
			{
				 _snprintf(szLog, 8191, "%s \n", lpszLog);
			}
			fwrite(szLog, 1, strlen(szLog) * sizeof(char), fp);
        	printf("%s", szLog);
		}
		catch(...)
		{
			//
		}
		fclose(fp);
	}
}

static char DEBUGTYPE_TIP[5][8] = {"信息", "信息", "日志", "警告", "错误"};

char * GetTipFromDebugType(DEBUGTYPE dtType)
{
	return DEBUGTYPE_TIP[dtType];
}

void CDebugLog::AddLog(DEBUGTYPE dtType, const char * lpszLog, BOOL bNeedTimeInfo)
{
	if ((!m_bWriteLog) || m_bTerminated || (dtType < m_DebugType))
		return;

	char szLog[8192] = {0};

	if (bNeedTimeInfo)
	{	
		time_t now = time(NULL);
	    tm *t = localtime(&now);
		_snprintf(szLog, 8191, "%02d-%02d-%02d %02d:%02d:%02d  %s: %s\n",
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, GetTipFromDebugType(dtType), lpszLog);
	} else
        _snprintf(szLog, 8191, "%s\n", lpszLog);
	WORD wSize = (WORD)strlen(szLog);
	LPLOGITEM pItem = GetLogItem(wSize + 1);
    if (pItem)
	{
		memcpy(pItem->szLog, szLog, wSize);
        pItem->wLen = wSize;

		m_Lock.Lock();
		m_LogList.push_back(pItem);
		m_Lock.UnLock();
	}

	printf("%s", szLog);
}

LPLOGITEM CDebugLog::GetLogItem(WORD wSize)
{
	LPLOGITEM pItem = NULL;
	try
	{
		pItem = new LOGITEM;
	}catch(...)
	{
		//PRINTDEBUGLOG(dtError, "请求日志内存出错, size: %d", sizeof(LOGITEM));
		pItem = NULL;
	}
	if (pItem)
	{
		try
		{
			pItem->szLog = new char[wSize];
			memset(pItem->szLog, 0, wSize);
		}catch(...)
		{
			//PRINTDEBUGLOG(dtError, "请求日志内存出错, size: %d", wSize);
			delete pItem;
			pItem = NULL;
		}
	}
	return pItem;
}

void CDebugLog::Terminate()
{
	m_bTerminated = TRUE;
	if (m_hBreak)
	{
		SetEvent(m_hBreak);
	}
	if (m_hSaveLogThd)
	{
		if (::WaitForSingleObject(m_hSaveLogThd, 10000) == WAIT_TIMEOUT)
		{
			::OutputDebugString(L"Free Debuglog Log Thread TimeOut");
		}
	}
	FinishWriteLog();	
	if (m_hSaveLogThd)
		CloseHandle(m_hSaveLogThd);
	if (m_hBreak)
		CloseHandle(m_hBreak);
	m_hSaveLogThd = NULL;
	m_hBreak = NULL;
}

DWORD WINAPI CDebugLog::__SaveLogThread(LPVOID lpParam)
{	
	CDebugLog *pThis = (CDebugLog *)lpParam;
	pThis->m_bTerminated = FALSE;
	 while(!pThis->m_bTerminated)
	{
       if (!pThis->m_LogList.empty())
		{
			char szLogFile[_MAX_PATH];
			pThis->GetCurrLogFileName(szLogFile, false);
			FILE *fp;
			
			fp = fopen(szLogFile, "a+b");

			if (fp)
			{
				fseek(fp, 0, SEEK_END);
			}
			LPLOGITEM pItem;
			while(!pThis->m_bTerminated)
			{
				pItem = NULL;
				
				pThis->m_Lock.Lock();
				if (!pThis->m_LogList.empty())
				{
					pItem= pThis->m_LogList.front();
					pThis->m_LogList.pop_front();
				}
				pThis->m_Lock.UnLock();

				if (pItem)
				{
					try
					{
						if (fp)
							fwrite(pItem->szLog, 1, pItem->wLen, fp);
						delete []pItem->szLog;
						delete pItem;
					} catch(...)
					{
						//
					}
				} else
					break;
			} //end while(true)
			if (fp)
			{
				fclose(fp);
			}
		}
		if (pThis->m_bTerminated)
			break;
		
		if (WaitForSingleObject(pThis->m_hBreak, 10000) != WAIT_TIMEOUT)
			break;
	}
	pThis = NULL;
	return 0;
}

void CDebugLog::FinishWriteLog()
{
	char szLogFile[_MAX_PATH];
	GetCurrLogFileName(szLogFile, false);
	FILE *fp;
	m_Lock.Lock();
	fp = fopen(szLogFile, "a+b");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
	}
	try
	{
		while(!m_LogList.empty())
		{
			LPLOGITEM pItem= m_LogList.front();
			if (fp)
				fwrite(pItem->szLog, 1, pItem->wLen, fp);
			delete []pItem->szLog;
			delete pItem;
			m_LogList.pop_front();
		}
	}catch(...)
	{
		//
	}
	if (fp)
	{
		fclose(fp);
	}		
	m_Lock.UnLock();
}

void CDebugLog::PrintLogInfo(DEBUGTYPE dtType, const char *szFormat, ...)
{
	if (dtError == dtType)
	{
		char szLog[8192] = {0};
		va_list   arg;  
		va_start(arg, szFormat);  
		vsnprintf(szLog, 8191, szFormat, arg);  
		va_end(arg);
		AddErrorLog(szLog);
	} else
	{
		if ((dtType < m_DebugType) || (!m_bWriteLog))
			return;
		char szLog[8192] = {0};
		va_list   arg;  
		va_start(arg, szFormat);  
		vsnprintf(szLog, 8191, szFormat, arg);  
		va_end(arg);
		AddLog(dtType, szLog);
	}
}
 
void CDebugLog::SetWriteDebugType(int nType)
{
	m_DebugType = (DEBUGTYPE)(nType);
}

#pragma warning(default:4996)