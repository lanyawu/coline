#ifndef __ITMSG_DEBUGLOG_H__
#define __ITMSG_DEBUGLOG_H__

#include <list>
#include <Commonlib/types.h>
#include <Commonlib/GuardLock.h>

typedef struct __tagLogItem
{
	char *szLog;
	WORD wLen;
}LOGITEM, *LPLOGITEM;

using namespace std;

typedef enum{
	dtInfo = 1,
	dtLog,
	dtWarning,
	dtError
}DEBUGTYPE;

void  ___Trace(LPCTSTR pstrFormat, ...);

#ifdef _DEBUG
   #define TRACE ___Trace
#else
   #define TRACE
#endif


class CDebugLog
{
public:
	CDebugLog(DEBUGTYPE DebugType);
	~CDebugLog();
	friend void PrintDebugLogInfo(DEBUGTYPE dtType, const char *szFormat, ...);
public:
	void AddLog(DEBUGTYPE dtType, const char * lpszLog, BOOL bNeedTimeInfo = TRUE);
	void AddErrorLog(const char *lpszLog, BOOL bNeedTimeInfo = TRUE);
	void PrintLogInfo(DEBUGTYPE dtType, const char *szFormat, ...);
	void SetWriteLog(BOOL bWriteLog);
	void SetWriteDebugType(int nType); //设置显示的最低等级
	void Terminate();
private:
	static DWORD WINAPI __SaveLogThread(LPVOID lpParam);
	static void GetCurrLogFileName(char *szLogFileName, bool IsError);
	void FinishWriteLog();
	LPLOGITEM GetLogItem(WORD wSize);
	HANDLE m_hBreak;
	HANDLE m_hSaveLogThd;
	std::list<LPLOGITEM> m_LogList;
	CGuardLock m_Lock;
	CGuardLock m_ErrorLock;
	BOOL m_bWriteLog;
	BOOL m_bTerminated;
	DEBUGTYPE m_DebugType; //当前要显示的最低级别类型
public:
	static CDebugLog m_Log;

};


#endif