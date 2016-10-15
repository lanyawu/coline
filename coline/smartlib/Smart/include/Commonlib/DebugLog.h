#ifndef __COMMON_DEBUGLOG_H____

#include <Commonlib/types.h>

typedef enum{
	dtInfo = 1,
	dtLog,
	dtWarning,
	dtError
}DEBUGTYPE;


void PrintLogInfo(DEBUGTYPE dtType, const char *szFormat, ...);
void SetWriteLog(BOOL bWrite);
void SetWriteDebugType(int nType);

#ifdef _DEBUG
  #define PRINTDEBUGLOG(x, y, ...) PrintLogInfo(x, y, ##__VA_ARGS__)
  #define SETWRITEDEBUGLOG(x)      SetWriteLog(x)
  #define SETWRITEDEBUGTYPE(x)     SetWriteDebugType(x)
#else
  #define PRINTDEBUGLOG(x, y, ...)
  #define SETWRITEDEBUGLOG(x)
  #define SETWRITEDEBUGTYPE(x)
#endif
 
#endif
