#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <xml/tinyxml.h>
#include <ShellAPI.h>

BOOL StartShellProcessor(const char *szAppName, const char *szParams, const char *szWorkPath, BOOL bWait)
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
	TCHAR szwWorkPath[MAX_PATH] = {0};
    if (szWorkPath)
		CStringConversion::StringToWideChar(szWorkPath, szwWorkPath, MAX_PATH - 1);
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

void CheckPlugins()
{
	typedef BOOL (CALLBACK *CHECK_PLUGINS)(const char *szAppPath);
	std::string strAppPath;
	char szFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szFileName, szAppPath, MAX_PATH - 1);
	strAppPath = szAppPath;
	strAppPath += "checkVersion.dll";
	TCHAR szwFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(strAppPath.c_str(), szwFileName, MAX_PATH - 1);
	HMODULE h = ::LoadLibrary(szwFileName);
	if (h != NULL)
	{
		CHECK_PLUGINS pProc = (CHECK_PLUGINS)::GetProcAddress(h, "CheckMainPlugins");
		if (pProc)
			pProc(NULL);
		::FreeLibrary(h);
	}
}

void AutoUpdate(std::vector<std::string> &ParamList)
{
	if (ParamList.size() == 3)
	{
		std::string strRunAppName = ParamList.back();
		ParamList.pop_back();
		std::string strNewVerFileName = ParamList.back();
		ParamList.pop_back();
		std::string strOldVerFileName = ParamList.back();
		TiXmlDocument xmldoc;
		if (xmldoc.LoadFile(strNewVerFileName.c_str()))
		{
			TiXmlElement *pNode = xmldoc.FirstChildElement();
			if (!pNode)
				return; 
			//run
			TiXmlElement *pRun = pNode->FirstChildElement("run");
			if (pRun)
			{
				TiXmlElement *pChild = pRun->FirstChildElement();
				char szAppPath[MAX_PATH] = {0}; 
				CSystemUtils::ExtractFilePath(strNewVerFileName.c_str(), szAppPath, MAX_PATH - 1);
				char szWorkPath[MAX_PATH] = {0};
				CSystemUtils::ExtractFilePath(strOldVerFileName.c_str(), szWorkPath, MAX_PATH - 1);
				while (pChild)
				{
					const char *szName = pChild->Attribute("name");
					if (szName)
					{
						std::string strFileName = szAppPath;
						strFileName += szName;
						if (CSystemUtils::FileIsExists(strFileName.c_str()))
						{
							StartShellProcessor(strFileName.c_str(), NULL, szWorkPath, TRUE);
							CSystemUtils::DeleteFilePlus(strFileName.c_str());
						}
					}
					pChild = pChild->NextSiblingElement();
				}
			}
			//CheckPlugins();
		}
		if (!CSystemUtils::MoveFilePlus(strNewVerFileName.c_str(), strOldVerFileName.c_str(), TRUE))
		{
			CSystemUtils::CopyFilePlus(strNewVerFileName.c_str(), strOldVerFileName.c_str(), TRUE);
			CSystemUtils::DeleteFilePlus(strNewVerFileName.c_str());
		}
		StartShellProcessor(strRunAppName.c_str(), "updateversion", NULL, FALSE);
	}
}

void ParserCmdLine(std::vector<std::string> &ParamList)
{
	if (ParamList.size() > 0)
	{
		std::vector<std::string>::iterator it = ParamList.begin();
		std::string strTmp = (*it);
		ParamList.erase(it);
		if (::stricmp(strTmp.c_str(), "checkplugin") == 0)
		{
			CheckPlugins();
		} else if (::stricmp(strTmp.c_str(), "update") == 0)
			AutoUpdate(ParamList);
	} //end if (ParamList.size()	
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
 	int nLen = ::strlen(lpCmdLine);
	TCHAR *szCmdLine = new TCHAR[nLen + 1];
	memset(szCmdLine, 0, sizeof(TCHAR) * (nLen + 1));
	CStringConversion::StringToWideChar(lpCmdLine, szCmdLine, nLen);
	int nArgc = 0;
	LPWSTR *szArgv = CommandLineToArgvW(szCmdLine, &nArgc);
	if (szArgv != NULL)
	{	
		std::vector<std::string> ParamList;

		for (int i = 0; i < nArgc; i ++)
		{
			int nLen = ::lstrlen(szArgv[i]) * 2;
			char *szTmp = new char[nLen + 1];
			memset(szTmp, 0, nLen + 1);
			CStringConversion::WideCharToString(szArgv[i], szTmp, nLen);
			ParamList.push_back(szTmp);
			delete []szTmp;
		}
		LocalFree(szArgv);
		ParserCmdLine(ParamList);
	}
	return 0;
}
