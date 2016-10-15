#include <stdio.h>
#include <SmartLog/SmartLog.h>
#include <Commonlib/DebugLog.h>

void PrintLogInfo(DEBUGTYPE dtType, const char *szFormat, ...)
{
	char szLog[8192] = {0};
	va_list   arg;  
	va_start(arg, szFormat);  
	vsnprintf(szLog, 8191, szFormat, arg);  
	va_end(arg);
	::PrintSmartLog((int) dtType, szLog);
}

void SetWriteLog(BOOL bWrite)
{
	::SetSmartLogIsWrite(bWrite);
}

void SetWriteDebugType(int nType)
{
	::SetSmartLogType(nType);
}
