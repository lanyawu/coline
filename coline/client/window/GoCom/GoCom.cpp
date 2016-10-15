#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <Core/CoreInterface.h>
#include <SmartSkin/smartskin.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/DebugLog.h>
#include <Core/common.h>
#include <xml/tinyxml.h>



#pragma warning(disable:4996)

#define CURRENT_CLIENT_VERSION "1.2.0.1"

static BOOL bUpdateVersion = FALSE;
const char *ExtactFilePath(const char *szFileName, char *szPath, int nMaxPath)
{
	if (szFileName)
	{
		int nPos = (int)strlen(szFileName);
        while(nPos > 0)
		{
			if (szFileName[nPos - 1] == '\\')
			{
				break;
			}
			nPos --;
		}
		if (nPos > 0)
		{
			if (nPos > nMaxPath)
				nPos = nMaxPath;
			strncpy(szPath, szFileName, nPos);
			return szPath;
		} else
			return NULL;
	} 
	return NULL;
}

/*typedef BOOL (WINAPI *_ChangeWindowMessageFilter)( UINT , DWORD);

BOOL AllowMeesageForVistaAbove(UINT uMessageID, BOOL bAllow)
{
    BOOL bResult = FALSE;
    HMODULE hUserMod = NULL;
    //vista and later
    hUserMod = LoadLibrary(_T("user32.dll"));
    if( NULL == hUserMod )
    {
        return FALSE;
    }
   
    _ChangeWindowMessageFilter pChangeWindowMessageFilter =
        (_ChangeWindowMessageFilter)GetProcAddress( hUserMod, "ChangeWindowMessageFilter");
    if( NULL == pChangeWindowMessageFilter )
    {
        return FALSE;
    }

    bResult = pChangeWindowMessageFilter( uMessageID, bAllow ? 1 : 2 );//MSGFLT_ADD: 1, MSGFLT_REMOVE: 2
   
    if( NULL != hUserMod )
    {
        FreeLibrary( hUserMod );
    }

    return bResult;
} */

void GetAppPath(std::string &strPath)
{
	WCHAR szTemp[MAX_PATH] = {0};
	char szTmp[MAX_PATH] = {0};
	::GetModuleFileNameW(NULL, szTemp, MAX_PATH);
	::WideCharToMultiByte(::GetACP(), 0, szTemp, -1, szTmp, MAX_PATH - 1, NULL, NULL);
	char szPath[MAX_PATH] = {0};
	ExtactFilePath(szTmp, szPath, MAX_PATH - 1);
    strPath = szPath;
	if (strPath.back() != '\\')
		strPath.push_back('\\'); 
}

void GetLocalAppPath(std::string &strPath)
{
	char szTmp[MAX_PATH] = {0};
	char szPrivatePath[MAX_PATH] = {0};
	CSystemUtils::GetLocalAppPath(szTmp, MAX_PATH - 1);
	CSystemUtils::IncludePathDelimiter(szTmp, szPrivatePath, MAX_PATH - 1);
	strPath = szPrivatePath;
	strPath += APPLICATION_PATH_NAME; 
	if (!CSystemUtils::DirectoryIsExists(strPath.c_str()))
		CSystemUtils::ForceDirectories(strPath.c_str());
}

BOOL CheckDefaultConfigure()
{
	CoInitialize(NULL);
	HRESULT hr; 
	IConfigure *pCfg = NULL;
	BOOL bSucc = FALSE;
	do
	{
		hr = CoCreateInstance(CLSID_CORE_FRAMEWORKCFG,	// CLSID of COM server
							  NULL,						//
							  CLSCTX_INPROC_SERVER,	   // it is a DLL 
							  __uuidof(IConfigure),	   // the interface IID
							  (void**)&pCfg			   // 
							  );
		
		if(FAILED(hr))
		{
			PRINTDEBUGLOG(dtInfo, "failed to initialize Configure server");
			break;
		}  
		char szCfgFileName[MAX_PATH] = {0};
		char szTmp[MAX_PATH] = {0};
		std::string szPrivatePath;
		GetLocalAppPath(szPrivatePath);
		strcpy(szCfgFileName, szPrivatePath.c_str());
		strcat(szCfgFileName, DEFAULT_CONFIGURE_FILE_NAME);
		std::string strValue;
		if (!CSystemUtils::FileIsExists(szCfgFileName))
		{
			pCfg->InitCfgFileName(szCfgFileName, "", TRUE); 
            TiXmlDocument xml;
			std::string strConfigXmlFile;
			GetAppPath(strConfigXmlFile);
			strConfigXmlFile += GLOBAL_CONFIG_FILE_NAME;
			if (xml.LoadFile(strConfigXmlFile.c_str()))
			{
				TiXmlElement *pNode = xml.FirstChildElement("config");
				if (pNode)
				{ 
					TiXmlElement *pChild = pNode->FirstChildElement("serverip");
					if (pChild && pChild->GetText() != NULL)
						pCfg->SetParamValue(TRUE, "Server", "Host", pChild->GetText());

					pChild = pNode->FirstChildElement("serverport");
				    if (pChild && pChild->GetText() != NULL)
				       pCfg->SetParamValue(TRUE, "Server", "Port", pChild->GetText());

					pChild = pNode->FirstChildElement("defaultdomain");
				    if (pChild && pChild->GetText() != NULL) 
						pCfg->SetParamValue(TRUE, "Server", "domain", pChild->GetText());

					pChild = pNode->FirstChildElement("orghost");
				    if (pChild && pChild->GetText() != NULL) 
						pCfg->SetParamValue(TRUE, "Server", "orghost", pChild->GetText());

					pChild = pNode->FirstChildElement("orghostport");
				    if (pChild && pChild->GetText() != NULL) 
						pCfg->SetParamValue(TRUE, "Server", "orghostport", pChild->GetText());

					pChild = pNode->FirstChildElement("clientversion");
				    if (pChild && pChild->GetText() != NULL) 
						pCfg->SetParamValue(TRUE, "client", "version", pChild->GetText()); 
					else
						pCfg->SetParamValue(TRUE, "client", "version", CURRENT_CLIENT_VERSION);
					 
				} // end if (pNode)				
			} // end if (xmldoc.
			PRINTDEBUGLOG(dtInfo, "初始化配置文件成功");
		} else
		{
			PRINTDEBUGLOG(dtInfo, "配置文件已经存在");
		}
		bSucc = TRUE;
	} while (FALSE);
	if (pCfg)
		pCfg->Release();
	pCfg = NULL;
	::CoUninitialize();
	return bSucc;
}

 
BOOL StartShellProcessor(const char *szAppName, const char *szParams, BOOL bWait)
{ 
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO); 
	TCHAR szwAppName[MAX_PATH]  = {0};
	CStringConversion::StringToWideChar(szAppName, szwAppName, MAX_PATH - 1);
	TCHAR *szwParams = NULL;
	BOOL bSucc = FALSE;
	if (szParams)
	{
		int nParamSize = ::strlen(szParams);
		szwParams = new TCHAR[nParamSize + 1];
		memset(szwParams, 0, sizeof(TCHAR) * (nParamSize + 1));
		CStringConversion::StringToWideChar(szParams, szwParams, nParamSize);
	}
    std::string strAppPath;
	GetAppPath(strAppPath);
	TCHAR szwWorkPath[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(strAppPath.c_str(), szwWorkPath, MAX_PATH - 1);
	SHELLEXECUTEINFO Info = {0};
	Info.cbSize = sizeof(Info);
	Info.lpFile = szwAppName;
	Info.fMask = SEE_MASK_NOCLOSEPROCESS;
	Info.lpParameters = szwParams;
	Info.lpDirectory = szwWorkPath;
	Info.nShow = SW_SHOW;
	Info.lpVerb =NULL;
	if (::ShellExecuteEx(&Info))
	{
		if (bWait)
			::WaitForSingleObject(Info.hProcess, INFINITE);
		bSucc = TRUE;
	} 
	 
	if (szwParams)
		delete []szwParams;
	return bSucc;
}
	 
#define UPDATER_APP_NAME "auclt.exe"
//检测是否需要更新复制文件
BOOL CALLBACK CheckUpdateLocalFiles(char *szRunFileName, int *nFileSize);

//检测是否需要升级文件
BOOL CheckUpdaterFiles()
{
	char szFile[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	if (CheckUpdateLocalFiles(szFile, &nSize))
	{
		std::string strAppPath;
		GetAppPath(strAppPath);
		std::string strAppName = strAppPath;
		strAppName += UPDATER_APP_NAME;
		std::string strParams = "update \""; 
		strParams += strAppPath;
		strParams += "config.xml\" \"";
		strParams += szFile;
		strParams += "\" \"";
		char szAppFileName[MAX_PATH] = {0};
		CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
		strParams += szAppFileName;
		strParams += "\"";
		return StartShellProcessor(strAppName.c_str(), strParams.c_str(), FALSE);
	}
	return FALSE;
}

BOOL CheckPlugins()
{ 
	std::string strAppPath;
	GetAppPath(strAppPath);
	strAppPath += "regplugins.exe";
	return StartShellProcessor(strAppPath.c_str(), "", TRUE); 
}

void LoadCoreFrameWork(HINSTANCE hInstance, const char *szInitUserName)
{
	HRESULT hr; 
	ICoreFrameWork *pCore = NULL;
	BOOL bSucc = FALSE;
	do
	{
		hr = CoCreateInstance(CLSID_CORE_FRAMEWORK,				// CLSID of COM server
							  NULL,						//
							  CLSCTX_INPROC_SERVER,		// it is a DLL 
							  __uuidof(ICoreFrameWork),	// the interface IID
							  (void**)&pCore			// 
							  );

		if(FAILED(hr))
		{
			if (CheckPlugins())
			{
				hr = CoCreateInstance(CLSID_CORE_FRAMEWORK,				// CLSID of COM server
							  NULL,						//
							  CLSCTX_INPROC_SERVER,		// it is a DLL 
							  __uuidof(ICoreFrameWork),	// the interface IID
							  (void**)&pCore			// 
							  );
			} else
			{
				PRINTDEBUGLOG(dtInfo, "CheckPlugin Failed");
			}
		}
		if (FAILED(hr))
		{
			PRINTDEBUGLOG(dtInfo, "failed to initialize COM server");
			break;
		}
		char szTmp[MAX_PATH] = {0};
		std::string strPath;
		GetLocalAppPath(strPath);
		strPath += DEFAULT_CONFIGURE_FILE_NAME;
		pCore->InitConfigure(strPath.c_str());
		hr = pCore->InitPlugins(hInstance);
		if (SUCCEEDED(hr))
		{ 
			ICoreLogin *pLogin = NULL;
			if (SUCCEEDED(pCore->QueryInterface(__uuidof(ICoreLogin), (void **)&pLogin)))
			{
				pLogin->ShowLogonWindow(szInitUserName, NULL, FALSE);
				pLogin->Release();
			} 
			IUIManager *pUI = NULL;
			hr = pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
			if (SUCCEEDED(hr) && pUI)
			{
				pUI->UIManagerRun();
				pUI->Release();
				pUI = NULL;
			}  
			bSucc = TRUE;
		} else 
		{
			PRINTDEBUGLOG(dtInfo, "Init Plugin Failed");
		}//end if (
	} while (FALSE);  
	if (pCore)
	{
		pCore->ClearPlugins();
		pCore->Release();
		pCore = NULL;
	}
	if (!bSucc)
	{
		::MessageBox(NULL, L"加载客户端失败，皮肤文件可能配置错误", L"错误", MB_OK);		
	} 
}

BOOL CheckAppStatus()
{
	BOOL bSucc = TRUE; 
	if (!CheckDefaultConfigure())
	{
		CheckPlugins();
		bSucc = CheckDefaultConfigure();
	} 
	return bSucc;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CoInitialize(NULL);
	if (::SkinCheckRunOption())
	{ 
		int nLen = ::strlen(lpCmdLine);
		TCHAR *szCmdLine = new TCHAR[nLen + 1];
		memset(szCmdLine, 0, sizeof(TCHAR) * (nLen + 1));
		CStringConversion::StringToWideChar(lpCmdLine, szCmdLine, nLen);
		int nArgc = 0;
		BOOL bSafeMode = FALSE;
		BOOL bUpdate = TRUE;
		char szAppName[MAX_PATH] = {0};
		char szAppPath[MAX_PATH] = {0};
		TCHAR szwAppPath[MAX_PATH] = {0};
		CSystemUtils::GetApplicationFileName(szAppName, MAX_PATH - 1);
		CSystemUtils::ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
		CStringConversion::StringToWideChar(szAppPath, szwAppPath, MAX_PATH - 1);
		::SetCurrentDirectory(szwAppPath);
		char szInitUserName[MAX_PATH] = {0};
		LPWSTR *szArgv = CommandLineToArgvW(szCmdLine, &nArgc);
		if (szArgv != NULL)
		{
			//debug
			char szTmp[MAX_PATH] = {0};
			for (int i = 0; i <nArgc; i ++)
			{ 
				if (::lstrcmpi(szArgv[i], L"updateversion") == 0)
					bUpdateVersion = TRUE; 
				else if (::lstrcmpi(szArgv[i], L"safemode") == 0)
					bSafeMode = TRUE;
				else if (::lstrcmpi(szArgv[i], L"noupdate") == 0)
					bUpdate = FALSE;
				else if (::wcsnicmp(szArgv[i], L"/user:", 6/*::lstrlen(L"user:")*/) == 0)
				{ 
					memset(szInitUserName, 0, MAX_PATH);
					CStringConversion::WideCharToString(szArgv[i] + 6, szInitUserName, MAX_PATH - 1); 
				} else if (::wcsnicmp(szArgv[i], L"coline:", 6/*::lstrlen(L"gocom:")*/) == 0)
				{
					//del gocom://
					memset(szInitUserName, 0, MAX_PATH);
					CStringConversion::WideCharToString(szArgv[i] + 8, szInitUserName, MAX_PATH - 1);
					//去除最后的/
					int len = ::strlen(szInitUserName);
					if (len > 0)
					{
						for (int i = len - 1; i >= 0; i ++)
						{
							if (szInitUserName[i] == '/')
								szInitUserName[i] = '\0';
							else
								break;
						}
					} //end if (len > 0)
				} //end else if (::wcsnicmp(szArgv[i].
			}
			LocalFree(szArgv); 
		} else
		{
			PRINTDEBUGLOG(dtError, "ARGV IS NULL");
		}
		if (bSafeMode)
		{
			CheckPlugins();
		}
		if (CheckAppStatus())
		{
			BOOL bSucc = TRUE;
			if (bUpdate)
				bSucc = !CheckUpdaterFiles();
			if (bSucc)
			{
				if (bUpdateVersion) //升级版本号
				{
					CheckDefaultConfigure();
				}
				//AllowMeesageForVistaAbove(WM_DROPFILES, TRUE);
				LoadCoreFrameWork(hInstance, szInitUserName);
			} else
			{
				//
			}
		} else
		{
			::MessageBox(NULL, L"客户端不完整，请重新安装能解决此问题", L"错误", MB_OK);
		}
	} else
	{
		::MessageBox(NULL, L"本系统需要Direct9以上版本支持", L"提示", MB_OK);
	}
	::CoUninitialize();
	return 0;
}

#pragma warning(default:4996)
