#ifndef __PATHSYNC_H_____
#define __PATHSYNC_H_____

#include "FileListMap.h"

class CPathSync
{
public:
	CPathSync(void);
	~CPathSync(void);
public:
	BOOL SyncPath(const char *szSrcPath, const char *szDestPath,  LPSMARTSYNCCALLBACK pCallBack);
	BOOL InitFileList(const char *szPath,  LPSMARTSYNCCALLBACK pCallBack);
	BOOL AnalyseSync(const char *szSrcPath, const char *szDestPath,  LPSMARTSYNCCALLBACK pCallBack);

	int  GetDirCount();
	int  GetFileCount();
private:
	BOOL Sync(const TCHAR *szwSubDir, LPSMARTSYNCCALLBACK pCallBack, int nType);
	BOOL CopyFileToDest(const TCHAR *szwFileName, LPSMARTSYNCCALLBACK pCallBack, int nType,
		                const char *szNewFileSize, const char *szLastModiTime, const char *szOldModiTime);
private:
	CFileListMap m_FileMap;
	TCHAR m_szSrcPath[MAX_PATH];
	TCHAR m_szDestPath[MAX_PATH];
	int m_nDirCount;
	int m_nFileCount;
	int m_nRefreshCount;
};

#endif
