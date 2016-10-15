#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/DebugLog.h>
#include <Core/CoreInterface.h>
#include <curl/curl.h>
#include <string>
#include <map>
#include <fstream>
#include <xml/tinyxml.h>
#include "CheckVersion.h" 

const char APPLICATION_PLUGIN_PATH[] = "plugins\\";
const char UPDATER_TEMP_PATH[]       = "updater\\";
const char UPDATER_CONFIG_NAME[]     = "config.xml";
#pragma warning(disable:4996)

static HANDLE g_hUpdateThread = NULL;
static BOOL   g_bUpdateTerminated = FALSE;
std::string g_strCurrentVerFileName;
std::string g_strUpdateUrl;
std::string g_strUpdateTempPath;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: 
			  
			 break;

		case DLL_THREAD_ATTACH:
			 break;
		case DLL_THREAD_DETACH:
			 break;
		case DLL_PROCESS_DETACH:
			 g_bUpdateTerminated = TRUE;
			 if (g_hUpdateThread)
			 {
				 ::WaitForSingleObject(g_hUpdateThread, 5000);
				 ::CloseHandle(g_hUpdateThread);
				 g_hUpdateThread = NULL;
			 }
			 break;
    }
    return TRUE;
}
 
BOOL RegisterCom(const TCHAR *szFileName)
{
	::CoInitialize(NULL);
	typedef HRESULT (CALLBACK *DLL_REGISTER)();
	HMODULE h = ::LoadLibrary(szFileName);
	BOOL bSucc = FALSE;
	if (h != NULL)
	{
		DLL_REGISTER pReg = (DLL_REGISTER)::GetProcAddress(h, "DllRegisterServer");
		bSucc = pReg && (SUCCEEDED(pReg()));
		if (!bSucc)
		{
			char szName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szFileName, szName, MAX_PATH - 1);
			PRINTDEBUGLOG(dtError, "register plugin failed:%s", szName);
		} else
		{
			char szName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szFileName, szName, MAX_PATH - 1);
			PRINTDEBUGLOG(dtError, "register plugin succ:%s", szName);
		}
		::FreeLibrary(h); 
	} else
	{
		char szName[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szFileName, szName, MAX_PATH - 1);
		PRINTDEBUGLOG(dtError, "load plugin failed:%s", szName);
	}
	::CoUninitialize();
	return bSucc;
}

void GetApplicationPath(std::string &strAppPath)
{
	char szTmp[MAX_PATH] = {0};
	char szAppFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	HANDLE h = ::GetModuleHandle(L"CheckVersion.dll");
	TCHAR szwCheck[MAX_PATH] = {0};
	::GetModuleFileName((HMODULE)h, szwCheck, MAX_PATH - 1);
	CStringConversion::WideCharToString(szwCheck, szAppFileName, MAX_PATH - 1); 
	memset(szTmp, 0, MAX_PATH);
	CSystemUtils::ExtractFilePath(szAppFileName, szTmp, MAX_PATH - 1);
	TCHAR szwTmp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szTmp, szwTmp, MAX_PATH - 1);
	::SetCurrentDirectory(szwTmp);
	CSystemUtils::IncludePathDelimiter(szTmp, szAppPath, MAX_PATH - 1);
	strAppPath = szAppPath;
}

BOOL CALLBACK CheckMainPlugins(const char *szAppPath)
{ 
	BOOL bSucc = FALSE;
	WIN32_FIND_DATA fd;
	std::string strPluginPath;
	if (szAppPath)
		strPluginPath = szAppPath;
	else
		GetApplicationPath(strPluginPath);
	strPluginPath += APPLICATION_PLUGIN_PATH;
	TCHAR szPath[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(strPluginPath.c_str(), szPath, MAX_PATH - 1);
	strPluginPath += "*.plg";
	TCHAR szTmp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(strPluginPath.c_str(), szTmp, MAX_PATH - 1);
	HANDLE hFind = ::FindFirstFile(szTmp, &fd);
	if (hFind != NULL)
	{
		do
		{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				CStdString_ strFileName = szPath;
				strFileName += fd.cFileName;
				RegisterCom(strFileName.GetData());
			} else
			{
				CStdString_ strFileName = szPath;
				strFileName += fd.cFileName;
				char szTmp[MAX_PATH] = {0};
				CStringConversion::WideCharToString(strFileName.GetData(), szTmp, MAX_PATH - 1);
				PRINTDEBUGLOG(dtError, "PATH:%s", szTmp);
			}
		} while (::FindNextFile(hFind, &fd)); 
		::FindClose(hFind);
		bSucc = TRUE;
	}
	return bSucc;
}

size_t WriteLocalFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
	fstream *fs = (fstream *)(stream);
	if (g_bUpdateTerminated)
		return 0; //terminate
 
	if (fs && fs->is_open())
	{
		fs->write((char *)buffer, (std::streamsize)(size * nmemb));
		return nmemb;
	}
	return 0;
} 

BOOL DownloadFileFromUrl(const char *szUrl, const char *szLocalFileName)
{
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szLocalFileName, szTemp, MAX_PATH - 1);
	fstream fs(szTemp, std::ios::out | std::ios::binary); 
	BOOL bSucc = FALSE;
	if (fs.is_open())
	{
		PRINTDEBUGLOG(dtInfo, "dl Url:%s local:%s", szUrl, szLocalFileName);
		CURL *pUrl = curl_easy_init();  
	    if (pUrl)
	    { 
			curl_easy_setopt(pUrl, CURLOPT_URL, szUrl);
			//设置下载回调函数
			curl_easy_setopt(pUrl, CURLOPT_WRITEFUNCTION, WriteLocalFile);
			//设置参数
			curl_easy_setopt(pUrl, CURLOPT_WRITEDATA, &fs);
 
			//
			if (curl_easy_perform(pUrl) == CURLE_OK)
				bSucc = TRUE;
	        //清除
			curl_easy_cleanup(pUrl);
			pUrl = NULL; 
		}  
		fs.close(); 
	} 
	return bSucc;
}

BOOL PaserVersionFiles(TiXmlElement *pNode, std::map<CAnsiString_, std::string> &verFiles)
{
	TiXmlElement *pChild = pNode->FirstChildElement();
	while (pChild)
	{
		if (pChild->Attribute("name") && pChild->Attribute("version"))
			verFiles.insert(std::pair<CAnsiString_, std::string>(pChild->Attribute("name"), pChild->Attribute("version")));
		pChild = pChild->NextSiblingElement();
	}
	return TRUE;
}

BOOL CompareVersion(const char *szCurrVerFileName, const char *szNewVerFileName, 
	                std::map<CAnsiString_, std::string> &dlFiles)
{
	std::map<CAnsiString_, std::string> CurrFiles;
	std::map<CAnsiString_, std::string> NewFiles;
	TiXmlDocument xmlCurr, xmlNew;
	BOOL bUpdate = FALSE;
	if (xmlCurr.LoadFile(szCurrVerFileName) && xmlNew.LoadFile(szNewVerFileName))
	{
		TiXmlElement *pCurrRoot = xmlCurr.RootElement();
		TiXmlElement *pNewRoot = xmlNew.RootElement();
		TiXmlElement *pCurrFileNode, *pNewFileNode;
		if (pCurrRoot && pNewRoot && (pNewRoot->Attribute("url") != NULL))
		{ 
			pCurrFileNode = pCurrRoot->FirstChildElement(); 
			while (pCurrFileNode)
			{
				if (::stricmp(pCurrFileNode->Value(), "files") == 0)
					break;
				pCurrFileNode = pCurrFileNode->NextSiblingElement();
			}
			pNewFileNode = pNewRoot->FirstChildElement();
			while (pNewFileNode)
			{
				if (::stricmp(pNewFileNode->Value(), "files") == 0)
					break;
				pNewFileNode = pNewFileNode->NextSiblingElement();
			}
			if (pCurrFileNode && pNewFileNode)
			{
				if ((::stricmp(pCurrFileNode->Attribute("version"), pNewFileNode->Attribute("version")) != 0)
					&& PaserVersionFiles(pCurrFileNode, CurrFiles)
					&& PaserVersionFiles(pNewFileNode, NewFiles))
				{
					std::map<CAnsiString_, std::string>::iterator it, it2;
					for (it = NewFiles.begin(); it != NewFiles.end(); it ++)
					{
						it2 = CurrFiles.find(it->first);
						if (it2 != CurrFiles.end())
						{
							if (::stricmp(it->second.c_str(), it2->second.c_str()) != 0)
								dlFiles.insert(std::pair<CAnsiString_, std::string>(it->first, it->second));
						} else
						{
							dlFiles.insert(std::pair<CAnsiString_, std::string>(it->first, it->second));
						} //end if (it2 != ..
					} //end for(..
					bUpdate = TRUE;
				} //end if (PaserVersionFiles(...
			} //end if (pCurrFileNode && 
		} //end if (pCurrRoot && 
	} //end if (xmlCurr.LoadFile(...
	return bUpdate;
}

void GetFullPath(const char *szPath, std::string &strLocalFileName)
{
	if (strlen(szPath) > 2)
	{
		if ((szPath[0] == '.') && (szPath[1] == '\\'))
		{
			strLocalFileName = g_strUpdateTempPath;
			strLocalFileName += &szPath[2];
			char szPath[MAX_PATH] = {0};
			CSystemUtils::ExtractFilePath(strLocalFileName.c_str(), szPath, MAX_PATH - 1);
			CSystemUtils::ForceDirectories(szPath);
		} //end if (szPath[0]
	} //end if (strlen(szPath)	
}

BOOL GetRemoteNameAndLocalName(TiXmlElement *pFileNode, const char *szFileName, std::string &strRemoteName, std::string &strLocalName)
{
	TiXmlElement *pChild = pFileNode->FirstChildElement();
	while (pChild)
	{
		if (::stricmp(pChild->Attribute("name"), szFileName) == 0)
		{
			if (pChild->Attribute("localname") && pChild->Attribute("remotename"))
			{
				strRemoteName = pChild->Attribute("remotename");
				strLocalName = pChild->Attribute("localname");
				return TRUE;
			} //end if (pChild-> 
		} //end if (::stricmp(pChild->			 
		pChild = pChild->NextSiblingElement();
	} //end while (pChild)
	return FALSE;
}

void DownloadUpdateFiles(const char *szNewVerFile, std::map<CAnsiString_, std::string> &dlFiles)
{
	TiXmlDocument xml;
	BOOL bUpdate = FALSE;
	if (xml.LoadFile(szNewVerFile))
	{ 
		TiXmlElement *pRoot = xml.RootElement();
		TiXmlElement *pFileNode;
		if (pRoot && (pRoot->Attribute("url") != NULL))
		{ 
			std::string strSvr = pRoot->Attribute("url");
			pFileNode = pRoot->FirstChildElement(); 
			while (pFileNode)
			{
				if (::stricmp(pFileNode->Value(), "files") == 0)
					break;
				pFileNode = pFileNode->NextSiblingElement();
			}
			std::map<CAnsiString_, std::string> urlMap;
			if (pFileNode)
			{
	
			} //end if (pFileNode)
			std::map<CAnsiString_, std::string>::iterator it;
			std::string strUrl, strLocalFileName, strRemoteName, strLocalName;
			bUpdate = TRUE;
			for (it = dlFiles.begin(); it != dlFiles.end(); it ++)
			{
				if (GetRemoteNameAndLocalName(pFileNode, it->first.c_str(), strRemoteName, strLocalName))
				{
					strUrl = strSvr;
					strUrl += strRemoteName;
					GetFullPath(strLocalName.c_str(), strLocalFileName);
					if (!DownloadFileFromUrl(strUrl.c_str(), strLocalFileName.c_str()))
					{
						bUpdate = FALSE;
						break;
					} //end if (!DownloadFileFromUrl(..
				} //end if (it2 ...
			} //end for(..
			if (bUpdate)
			{
				pRoot->SetAttribute("complete", "true");
				xml.SaveFile();
			}
		} //end if (pRoot && 
	} //end if (xml.LoadFile(..
}

DWORD WINAPI CheckUpdateThread(LPVOID lpParam)
{
	std::string strNewCurrFileName = g_strUpdateTempPath;
	strNewCurrFileName += UPDATER_CONFIG_NAME; 
	if (DownloadFileFromUrl(g_strUpdateUrl.c_str(), strNewCurrFileName.c_str()))
	{
		std::map<CAnsiString_, std::string> dlFiles; 
		if (CompareVersion(g_strCurrentVerFileName.c_str(), strNewCurrFileName.c_str(), dlFiles))
		{
			DownloadUpdateFiles(strNewCurrFileName.c_str(), dlFiles);
		}
	} //end if (DownloadFileFromUrl(..
	//
	return 0;
}

BOOL CALLBACK UpdateFilesFromSvr(const char *szVerFile, const char *szUrl, const char *szTempPath)
{
	if ((g_hUpdateThread == NULL) && (szVerFile != NULL) && (szUrl != NULL)
		&& (szTempPath != NULL))
	{
		g_strCurrentVerFileName = szVerFile;
		g_strUpdateUrl = szUrl;
		g_strUpdateTempPath = szTempPath;
		g_hUpdateThread = ::CreateThread(NULL, 0, CheckUpdateThread, NULL, 0, NULL);
		return TRUE;
	}
	return FALSE;
}

//检测是否需要更新复制文件
BOOL CALLBACK CheckUpdateLocalFiles(char *szRunFileName, int *nFileSize)
{
	char szPath[MAX_PATH] = {0};
	CSystemUtils::GetLocalAppPath(szPath, MAX_PATH - 1); 
	std::string strVerFile = szPath;
	strVerFile += "\\CoLine\\Updater\\";
	strVerFile += UPDATER_CONFIG_NAME;
	if (CSystemUtils::FileIsExists(strVerFile.c_str()))
	{
		TiXmlDocument xmldoc;
		if (xmldoc.LoadFile(strVerFile.c_str()))
		{
			TiXmlElement *pNode = xmldoc.FirstChildElement();
			if (pNode && (pNode->Attribute("complete") != NULL))
			{
				if (::stricmp(pNode->Attribute("complete"), "true") == 0)
				{
					::strcpy(szRunFileName, strVerFile.c_str());
					return TRUE;
				} //end if (::stricmp(
			} //end if (pNode && 
		} //end if (xmldoc.LoadFile(..
	} //end if (CSystemUtils::
	return FALSE;
}

BOOL MovePathFiles(const TCHAR *szSrcPath, const TCHAR *szDstPath)
{ 
	CStdString_ strTmp = szSrcPath;
	strTmp += L"*.*"; 
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(strTmp, &fd);
	if (hFind != NULL)
	{
		do
		{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				CStdString_ strSrcFile = szSrcPath;
				strSrcFile += fd.cFileName;
				CStdString_ strDstFile = szDstPath;
				strDstFile += fd.cFileName;
				::MoveFileEx(strSrcFile, strDstFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
			} else if ((::lstrcmp(fd.cFileName, L".") != 0)
				&& (::lstrcmp(fd.cFileName, L"..") != 0))
			{
				CStdString_ strSrc = szSrcPath;
				CStdString_ strDst = szDstPath;
				strSrc += fd.cFileName;
				strSrc += L"\\";
				strDst += fd.cFileName;
				strDst += L"\\";
				MovePathFiles(strSrc, strDst);
			}
		} while (::FindNextFile(hFind, &fd)); 
		::FindClose(hFind);
		return TRUE;
	} //end
	return FALSE;
}

//复制文件
BOOL CALLBACK CopyUpdateFiles(const char *szAppFileName, const char *szTempPath)
{
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
	TCHAR szSrc[MAX_PATH] = {0};
	TCHAR szDst[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szTempPath, szSrc, MAX_PATH - 1);
	CStringConversion::StringToWideChar(szAppPath, szDst, MAX_PATH - 1);
	return MovePathFiles(szSrc, szDst); 
}

#pragma warning(default:4996)
