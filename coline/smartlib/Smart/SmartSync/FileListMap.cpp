#include "StdAfx.h"
#include <time.h>
#include <Commonlib/DebugLog.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <Crypto/Crypto.h>
#include "FileListMap.h"

const char FILEINFO_TABLE_NAME[] = "FileInfo";
const char CREATE_FILEINFO_TABLE_SQL[] = "create table FileInfo(id INTEGER PRIMARY KEY, \
										  FileName VARCHAR(512), FileMD5 VARCHAR(36),\
										  ModifyTime VARCHAR(128), FileSize VARCHAR(24),\
										  InceptTime VARCHAR(128));";
#pragma warning(disable:4996)

CFileListMap::CFileListMap(void):
              m_DBOP(NULL),
			  m_nFileCount(0),
			  m_nDirCount(0)
{
}


CFileListMap::~CFileListMap(void)
{
	if (m_DBOP)
		delete m_DBOP;
	m_DBOP = NULL;
}

BOOL CFileListMap::InitFileList(const char *szPath, const char *szDBName, const char *szKey,
	                            LPSMARTSYNCCALLBACK pCallBack, BOOL bForce)
{
	//
	m_nDirCount = 0;
	m_nFileCount = 0;
	if (m_DBOP)
		delete m_DBOP;
	m_DBOP = new CSqliteDBOP(szDBName, szKey);
	if (!m_DBOP->TableIsExists(FILEINFO_TABLE_NAME))
	{
		if (m_DBOP->Execute(CREATE_FILEINFO_TABLE_SQL))
		{
			char szFullPath[256] = {0};
			TCHAR szwFullPath[256] = {0};
			CSystemUtils::IncludePathDelimiter(szPath, szFullPath, 256);
			CStringConversion::StringToWideChar(szFullPath, szwFullPath, 255);
			EstablishFileDB(szwFullPath, L"", pCallBack);
		} else
		{
			PRINTDEBUGLOG(dtInfo, "Create Table Failed");
		}
	} else
	{
		if (bForce)
		{
			char strSql[] = "delete from FileInfo";
			if (m_DBOP->Execute(strSql))
			{
				char szFullPath[256] = {0};
				TCHAR szwFullPath[256] = {0};
				CSystemUtils::IncludePathDelimiter(szPath, szFullPath, 256);
				CStringConversion::StringToWideChar(szFullPath, szwFullPath, 255);
				EstablishFileDB(szwFullPath, L"", pCallBack);
			}
		} else
		{
			char szSql[] = "select FileMD5, ModifyTime from FileInfo";
			char **szResult;
			int nRow, nCol;
			if (m_DBOP->Open(szSql, &szResult, nRow, nCol))
			{
				for (int i = 1; i <= nRow; i ++)
				{
					m_FileList.insert(std::pair<std::string, std::string>(szResult[2 * i], szResult[2 * i + 1]));
				}
			}
		}
	}
	return TRUE;
}

BOOL CFileListMap::IsNewestFile(const char *szFileMD5, const char *szModifyTime,
	                            std::string &strOldModiTime)
{
	std::map<std::string, std::string>::iterator it = m_FileList.find(szFileMD5);
	if (it != m_FileList.end())
	{
		if (::strcmp(it->second.c_str(), szModifyTime) == 0)
			return TRUE;
		else
			strOldModiTime = it->second;
	}
	return FALSE;
}

BOOL CFileListMap::InsertNewFile(const char *szFileName, const char *szPathMD5, const char *szModifyTime,
		                const char *szFileSize)
{
	std::map<std::string, std::string>::iterator it = m_FileList.find(szPathMD5);
	if (it != m_FileList.end())
	{
		it->second = szModifyTime;
		std::string strSql = "update ";
	    strSql += FILEINFO_TABLE_NAME;
		strSql += " set ModifyTime='";
		strSql += szModifyTime;
		strSql += "',FileSize='";
		strSql += szFileSize;
		char szNow[64] = {0};
	    CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szNow);
		strSql += "',InceptTime='";
		strSql += szNow;
		strSql += "' where FileMD5='";
		strSql += szPathMD5;
		strSql += "'";
		return m_DBOP->Execute(strSql.c_str());
	} else
	{
		if (InsertFileInfo(szFileName, szPathMD5, szModifyTime, szFileSize))
		{
			m_FileList[szPathMD5] = szModifyTime;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CFileListMap::InsertFileInfo(const char *szFileName, const char *szPathMD5, const char *szModifyTime,
		                const char *szFileSize)
{
	std::string strSql = "insert into ";
	strSql += FILEINFO_TABLE_NAME;
	char szNow[64] = {0};
	CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szNow);
	strSql += "(FileName,FileMD5,ModifyTime,FileSize,InceptTime) values('";
	strSql += szFileName;
	strSql += "','";
	strSql += szPathMD5;
	strSql += "','";
	strSql += szModifyTime;
	strSql += "','";
	strSql += szFileSize;
	strSql += "','";
	strSql += szNow;
	strSql += "')";
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CFileListMap::EstablishFileDB(const TCHAR *szRootPath, const TCHAR *szwSubDir, 
	                               LPSMARTSYNCCALLBACK pCallBack)
{
	WIN32_FIND_DATA data = {0};
	int nSize = ::lstrlen(szRootPath) + ::lstrlen(szwSubDir);
	TCHAR *szTemp = new TCHAR[nSize + 8];
	memset(szTemp, 0, (nSize + 8) * sizeof(TCHAR));
	::lstrcpy(szTemp, szRootPath);
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
					EstablishFileDB(szRootPath, szSubDir, pCallBack);					
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
						m_FileList.insert(std::pair<std::string, std::string>(szMD5, szTime));
						InsertFileInfo(szFileName, szMD5, szTime, szFileSize);
						m_nFileCount ++;
					}
				}
				if (pCallBack)
					pCallBack(SMARTSYNC_TYPE_ESTABLISH, NULL, NULL, NULL, NULL, NULL,
					          m_nDirCount, m_nFileCount);
				delete []szSubDir;
			}
		} while (::FindNextFile(hFind, &data));		
		::FindClose(hFind);
	}
	delete []szTemp;
	return TRUE;
}

int  CFileListMap::GetDirCount()
{
	return m_nDirCount;
}

int  CFileListMap::GetFileCount()
{
	return m_nFileCount;
}

#pragma warning(default:4996)
