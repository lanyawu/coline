#include "StdAfx.h"
#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <Crypto/crypto.h>
#include "PathSync.h"

const char FILEINFO_DB_KEY[] = "smartsynckey";
const char FILEINFO_DB_NAME[] = "smartsync.db";

#pragma warning(disable:4996)

CPathSync::CPathSync(void):
           m_nDirCount(0),
		   m_nFileCount(0),
		   m_nRefreshCount(0)
{
	memset(m_szSrcPath, 0, sizeof(TCHAR) * MAX_PATH);
	memset(m_szDestPath, 0, sizeof(TCHAR) * MAX_PATH);
}


CPathSync::~CPathSync(void)
{
}

BOOL CPathSync::SyncPath(const char *szSrcPath, const char *szDestPath, LPSMARTSYNCCALLBACK pCallBack)
{
	memset(m_szSrcPath, 0, sizeof(TCHAR) * MAX_PATH);
	memset(m_szDestPath, 0, sizeof(TCHAR) * MAX_PATH);
	m_nDirCount = 0;
	m_nRefreshCount = 0;
	m_nFileCount = 0;
	char szDbName[MAX_PATH];
	CSystemUtils::IncludePathDelimiter(szDestPath, szDbName, MAX_PATH - 1);
	CStringConversion::StringToWideChar(szDbName, m_szDestPath, MAX_PATH - 1);
	strcat(szDbName, FILEINFO_DB_NAME);
	PRINTDEBUGLOG(dtInfo, "start establish directory list");
	m_FileMap.InitFileList(szDestPath, szDbName, FILEINFO_DB_KEY, pCallBack, FALSE);
	PRINTDEBUGLOG(dtInfo, "establish directory list complete");
	memset(szDbName, 0, MAX_PATH);
	CSystemUtils::IncludePathDelimiter(szSrcPath, szDbName, MAX_PATH - 1);
	CStringConversion::StringToWideChar(szDbName, m_szSrcPath, MAX_PATH - 1);
	Sync(L"", pCallBack, SMARTSYNC_TYPE_SYNC);
	PRINTDEBUGLOG(dtInfo, "Directory:%d File: %d Refresh:%d", m_nDirCount, m_nFileCount, m_nRefreshCount);
	return TRUE;
}

BOOL CPathSync::AnalyseSync(const char *szSrcPath, const char *szDestPath,  LPSMARTSYNCCALLBACK pCallBack)
{
	memset(m_szSrcPath, 0, sizeof(TCHAR) * MAX_PATH);
	memset(m_szDestPath, 0, sizeof(TCHAR) * MAX_PATH);
	m_nDirCount = 0;
	m_nRefreshCount = 0;
	m_nFileCount = 0;
	char szDbName[MAX_PATH];
	CSystemUtils::IncludePathDelimiter(szDestPath, szDbName, MAX_PATH - 1);
	CStringConversion::StringToWideChar(szDbName, m_szDestPath, MAX_PATH - 1);
	strcat(szDbName, FILEINFO_DB_NAME);
	PRINTDEBUGLOG(dtInfo, "start establish directory list");
	m_FileMap.InitFileList(szDestPath, szDbName, FILEINFO_DB_KEY, pCallBack, FALSE);
	PRINTDEBUGLOG(dtInfo, "establish directory list complete");
	memset(szDbName, 0, MAX_PATH);
	CSystemUtils::IncludePathDelimiter(szSrcPath, szDbName, MAX_PATH - 1);
	CStringConversion::StringToWideChar(szDbName, m_szSrcPath, MAX_PATH - 1);
	Sync(L"", pCallBack, SMARTSYNC_TYPE_ANALYSE);
	return TRUE;
}

int  CPathSync::GetDirCount()
{
	return m_nDirCount;
}

int  CPathSync::GetFileCount()
{
	return m_nFileCount;
}

BOOL CPathSync::InitFileList(const char *szPath, LPSMARTSYNCCALLBACK pCallBack)
{
	char szDbName[MAX_PATH];
	CSystemUtils::IncludePathDelimiter(szPath, szDbName, MAX_PATH - 1);
	strcat(szDbName, FILEINFO_DB_NAME);
	BOOL b = m_FileMap.InitFileList(szPath, szDbName, FILEINFO_DB_KEY, pCallBack, TRUE);
	m_nDirCount = m_FileMap.GetDirCount();
	m_nFileCount = m_FileMap.GetFileCount();
	return b;
}

BOOL CPathSync::Sync(const TCHAR *szwSubDir, LPSMARTSYNCCALLBACK pCallBack, int nType)
{
	WIN32_FIND_DATA data = {0};
	int nSize = ::lstrlen(m_szSrcPath) + ::lstrlen(szwSubDir);
	TCHAR *szTemp = new TCHAR[nSize + 8];
	memset(szTemp, 0, (nSize + 8) * sizeof(TCHAR));
	::lstrcpy(szTemp, m_szSrcPath);
	::lstrcat(szTemp, szwSubDir); 
	::lstrcat(szTemp, L"*.*");
	HANDLE hFind = ::FindFirstFile(szTemp, &data);
    if (hFind)
	{
		do
		{
			if ((::lstrcmp(data.cFileName, L".") != 0)
				&& (::lstrcmp(data.cFileName, L"..") != 0))
			{
				int nSubDirSize = ::lstrlen(data.cFileName);
				nSubDirSize += ::lstrlen(szwSubDir);
				nSubDirSize += 8;
				TCHAR *szSubDir = new TCHAR[nSubDirSize];
				memset(szSubDir, 0, nSubDirSize * sizeof(TCHAR));
				::lstrcpy(szSubDir, szwSubDir);
				::lstrcat(szSubDir, data.cFileName);
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
				{					
					::lstrcat(szSubDir, L"\\");
					m_nDirCount ++;
					char szForcePath[MAX_PATH] = {0};
					CStringConversion::WideCharToString(m_szDestPath, szForcePath, MAX_PATH - 1);
					char szForceSubDir[MAX_PATH] = {0};
					CStringConversion::WideCharToString(szSubDir, szForceSubDir, MAX_PATH - 1);
					strcat(szForcePath, szForceSubDir);
					CSystemUtils::ForceDirectories(szForcePath);
					Sync(szSubDir, pCallBack, nType);					
				} else
				{
					char szMD5[36] = {0};
					char *szFileName = new char[nSubDirSize * 2];
					memset(szFileName, 0, nSubDirSize * 2);
					CStringConversion::WideCharToString(szSubDir, szFileName, nSubDirSize * 2 - 1);
					SYSTEMTIME LastTime = {0};
					if (FileTimeToSystemTime(&(data.ftLastWriteTime), &LastTime))
					{
						char szTime[64] = {0};
						sprintf(szTime, "%02d-%02d-%02d %02d:%02d:%02d", LastTime.wYear, LastTime.wMonth,
							LastTime.wDay, LastTime.wHour, LastTime.wMinute, LastTime.wSecond);
						md5_encode(szFileName, ::strlen(szFileName), szMD5);
						char szFileSize[32] = {0};
						::itoa(data.nFileSizeLow, szFileSize, 10);
						std::string strOldTime;
						if (!m_FileMap.IsNewestFile(szMD5, szTime, strOldTime))
						{
							if (CopyFileToDest(szSubDir, pCallBack, nType, szFileSize, szTime, strOldTime.c_str()))
							{
								if (nType == SMARTSYNC_TYPE_SYNC) 
									m_FileMap.InsertNewFile(szFileName, szMD5, szTime, szFileSize);
								m_nRefreshCount ++;
							} else
							{
								PRINTDEBUGLOG(dtInfo, "Copy File Failed");
							} //end else if (CopyFileToDest(..
						} //end if (!m_FileMap..
						m_nFileCount ++;
					} //end if (FileTimeToSystemTime(...
				} //end else if ((data.dwFileAttributes
				delete []szSubDir;
			} //end else if ((::lstrcmp(data.cFileName...
		} while (::FindNextFile(hFind, &data));		
		::FindClose(hFind);
	}
	delete []szTemp;
	return TRUE;
}

BOOL CPathSync::CopyFileToDest(const TCHAR *szwFileName, LPSMARTSYNCCALLBACK pCallBack, int nType,
	              const char *szNewFileSize, const char *szLastModiTime, const char *szOldModiTime)
{
	char szFileName[MAX_PATH] = {0};
	CStringConversion::WideCharToString(szwFileName, szFileName, MAX_PATH - 1);
	if (pCallBack)
		pCallBack(nType, szFileName, szNewFileSize, szLastModiTime, 0,
			szOldModiTime, m_nDirCount, m_nFileCount); 
	if (nType == SMARTSYNC_TYPE_SYNC)
	{
		TCHAR szSrcFile[MAX_PATH] = {0};
		TCHAR szDestFile[MAX_PATH] = {0};
		::lstrcpy(szSrcFile, m_szSrcPath);
		::lstrcat(szSrcFile, szwFileName);
		::lstrcpy(szDestFile, m_szDestPath);
		::lstrcat(szDestFile, szwFileName);
		return ::CopyFile(szSrcFile, szDestFile, FALSE);
	}  
	return TRUE;
}

#pragma warning(default:4996)
