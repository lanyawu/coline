#ifndef __FILELISTMAP_H____
#define __FILELISTMAP_H____

#include <Commonlib/SqliteDBOP.h>
#include <map>
#include <string>
#include "SmartSync.h"

class CFileListMap  
{
public:
	CFileListMap(void);
	~CFileListMap(void);
public:
	BOOL InitFileList(const char *szPath, const char *szDBName, const char *szKey,
		              LPSMARTSYNCCALLBACK pCallBack, BOOL bForce);
	BOOL InsertFileInfo(const char *szFileName, const char *szPathMD5, const char *szModifyTime,
		                const char *szFileSize);
	BOOL IsNewestFile(const char *szFileMD5, const char *szModifyTime, std::string &strOldModiTime);
	BOOL InsertNewFile(const char *szFileName, const char *szPathMD5, const char *szModifyTime,
		                const char *szFileSize);
	int  GetDirCount();
	int  GetFileCount();
private:
	BOOL EstablishFileDB(const TCHAR *szRootPath, const TCHAR *szwSubDir, LPSMARTSYNCCALLBACK pCallBack);
private:
	CSqliteDBOP *m_DBOP;
	std::map<std::string, std::string> m_FileList;
	int m_nDirCount;
	int m_nFileCount;
};

#endif
