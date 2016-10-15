// FileVer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <xml/tinyxml.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <time.h>
#include <crypto/crypto.h>

#pragma warning(disable:4996)

int m_nModiFileCount = 0;
int m_nNewAppendCount = 0;
BOOL IsSameFile(TiXmlElement *pOldFileNode, const char *szFileName, const char *szMd5, std::string &strNewVer)
{
	TiXmlElement *pChild = pOldFileNode->FirstChildElement();
	strNewVer = "1001";
	while (pChild)
	{
		if (stricmp(pChild->Attribute("id"), szFileName) == 0)
		{
			if (::stricmp(pChild->Attribute("md5"), szMd5) == 0)
				return TRUE;
			else
			{
				const char *szVer = pChild->Attribute("vernumber");
				if (szVer)
				{
					int nVer = ::atoi(szVer);
					nVer ++;
					char szTmp[32] = {0};
					::itoa(nVer, szTmp, 10);
					strNewVer = szTmp;
				}
			} //end else
			break;
		} //end if (
		pChild = pChild->NextSiblingElement();
	}
	return FALSE;
}

void PaserFileName(TiXmlElement *pOldFileNode, TiXmlElement *pNewFileNode, std::string &strAbsPath,
	               std::string &strPath, const char *szFileName, const char *szFileModiDate)
{
	printf(".");
	std::string strFileName = strAbsPath;
	strFileName += strPath;
	strFileName += szFileName;
	char szMd5[64] = {0};
	if (md5_encodefile(strFileName.c_str(), szMd5))
	{
		std::string strVer;
		int nFileSize = CSystemUtils::GetFileSize(strFileName.c_str());
		char szFileSize[32] = {0}; 
		::itoa(nFileSize, szFileSize, 10);
		char szFileId[64] = {0};
		std::string strFileAbstract = strPath;
		strFileAbstract += szFileName;
		md5_encode(strFileAbstract.c_str(), strFileAbstract.size(), szFileId);
		if (!IsSameFile(pOldFileNode, szFileId, szMd5, strVer))
		{
			TiXmlElement *pChild = pNewFileNode->FirstChildElement();
			BOOL bNotFound = TRUE;
			while (pChild)
			{
				if (::stricmp(pChild->Attribute("id"), szFileId) == 0)
				{
					pChild->SetAttribute("md5", szMd5);
					pChild->SetAttribute("vernumber", strVer.c_str());
					pChild->SetAttribute("size", szFileSize);
					pChild->SetAttribute("lastdate", szFileModiDate);
					m_nModiFileCount ++;
					bNotFound = FALSE;
					break;
				}
				pChild = pChild->NextSiblingElement();
			} //end while
			if (bNotFound)
			{
				TiXmlElement pNew("file");
				pNew.SetAttribute("id", szFileId);
				pNew.SetAttribute("name", szFileName);
				pNew.SetAttribute("size", szFileSize);
				pNew.SetAttribute("md5", szMd5);
				pNew.SetAttribute("vernumber", strVer.c_str());
				pNew.SetAttribute("version", strVer.c_str());
				std::string strRemoteName = strPath;
				strRemoteName += szFileName;
				pNew.SetAttribute("remotename", strRemoteName.c_str());
				std::string strLocalName = ".\\";
				strLocalName += strRemoteName;
				pNew.SetAttribute("localname", strLocalName.c_str());
				pNew.SetAttribute("comment", szFileName);
				pNew.SetAttribute("lastdate", szFileModiDate);
				pNewFileNode->InsertEndChild(pNew);
				m_nNewAppendCount ++;
			}
		}
	}
}

void CreateXmlByPath(TiXmlElement *pOldFileNode, TiXmlElement *pNewFileNode, std::string &strAbsPath,
	                 std::string &strPath)
{
	if (strPath.empty())
		printf("\npath:.");
	else
		printf("\npath:%s", strPath.c_str());
	std::string strCurrPath = strAbsPath;
	strCurrPath += strPath;
	strCurrPath += "*.*";
	TCHAR szwTmp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(strCurrPath.c_str(), szwTmp, MAX_PATH - 1);
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(szwTmp, &fd);
	if (hFind != NULL)
	{
		do
		{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				if ((::lstrcmp(fd.cFileName, L".") != 0)
					&& (::lstrcmp(fd.cFileName, L"..") != 0))
				{
					char szDir[MAX_PATH] = {0};
					CStringConversion::WideCharToString(fd.cFileName, szDir, MAX_PATH - 1);
					std::string strTmp = strPath;
					strTmp += szDir;
					strTmp += "\\";
					CreateXmlByPath(pOldFileNode, pNewFileNode, strAbsPath, strTmp);
				}
			} else
			{
				char szFileName[MAX_PATH] = {0};
				CStringConversion::WideCharToString(fd.cFileName, szFileName, MAX_PATH - 1);
				SYSTEMTIME systime;
				::FileTimeToSystemTime(&fd.ftLastWriteTime, &systime); 
				char szTime[64] = {0}; 
				sprintf(szTime, "%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d", systime.wYear, systime.wMonth, systime.wDay,
					systime.wHour, systime.wMinute, systime.wSecond);
				PaserFileName(pOldFileNode, pNewFileNode, strAbsPath, strPath, szFileName, szTime);
			}
		} while (::FindNextFile(hFind, &fd)); 
		::FindClose(hFind); 
	}
}

void CreateNewXml(const char *szOldXmlFile, const char *szPath, const char *szNewFile)
{
	
    TiXmlDocument xmlOld, xmlNew;
	std::string strAbsPath = szPath; 
	if (xmlOld.LoadFile(szOldXmlFile, TIXML_DEFAULT_ENCODING) 
		&& xmlNew.LoadFile(szOldXmlFile, TIXML_DEFAULT_ENCODING))
	{
		TiXmlElement *pFirst;
		TiXmlElement *pOldNode = NULL, *pNewNode = NULL;
		//get new ver file
		pFirst = xmlOld.FirstChildElement();
		if (pFirst)
		{
			TiXmlElement *pChild = pFirst->FirstChildElement();
			while (pChild)
			{
				if (::stricmp(pChild->Value(), "files") == 0)
				{
					pOldNode = pChild;
					break;
				}
				pChild = pChild->NextSiblingElement();
			} //end while (pChild)
		} //end if (pFirst)

		//get new ver file
		pFirst = xmlNew.FirstChildElement();
		if (pFirst)
		{
			TiXmlElement *pChild = pFirst->FirstChildElement();
			while (pChild)
			{
				if (::stricmp(pChild->Value(), "files") == 0)
				{
					pNewNode = pChild;
					break;
				}
				pChild = pChild->NextSiblingElement();
			} //end while (pChild)
		} //end if (pFirst)
		if (pOldNode && pNewNode)
		{
			std::string strPath;
			char szTime[64] = {0};
			CSystemUtils::DateTimeToStr((DWORD)time(NULL), szTime);
			pNewNode->SetAttribute("lastDate", szTime); 
			CreateXmlByPath(pOldNode, pNewNode, strAbsPath, strPath);
			xmlNew.SaveFile(szNewFile);
		}
	}	
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 4)
	{
		char szXmlFile[MAX_PATH] = {0};
		char szPath[MAX_PATH] = {0};
		char szNewXmlFile[MAX_PATH] = {0};
		CStringConversion::WideCharToString(argv[1], szXmlFile, MAX_PATH - 1);
		CStringConversion::WideCharToString(argv[2], szPath, MAX_PATH - 1);
		CStringConversion::WideCharToString(argv[3], szNewXmlFile, MAX_PATH - 1);
		CreateNewXml(szXmlFile, szPath, szNewXmlFile);
		printf("\n\nModify File Count:%d", m_nModiFileCount);
		printf("\nNew Append File Count:%d", m_nNewAppendCount);
	} else
		printf("\n \t \t FileVer OldVerFileName Path NewVerFileName\n");
	return 0;
}

#pragma warning(default:4996)
