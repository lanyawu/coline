#include <Commonlib/types.h>

#define SMARTSYNC_TYPE_ESTABLISH 1
#define SMARTSYNC_TYPE_ANALYSE   2
#define SMARTSYNC_TYPE_SYNC      3
#define SMARTSYNC_TYPE_COMPLETE  99

typedef void (CALLBACK *LPSMARTSYNCCALLBACK)(int nType, const char *szSubFile, const char *szNewFileSize,
	         const char *szLastModiTime, const char *szOldFileSize, const char *szOldModiTime,
			 int nDirCount, int nFileCount);

BOOL CALLBACK  SmartSyncPath(const char *szSrcPath, const char *szDestPath, 
	                     LPSMARTSYNCCALLBACK pCallBack);
BOOL CALLBACK  EstablishSyncPath(const char *szPath, LPSMARTSYNCCALLBACK pCallBack);
BOOL CALLBACK  AnalyseSyncPath(const char *szSrcPath, const char *szDestPath, LPSMARTSYNCCALLBACK *pCallBack);
