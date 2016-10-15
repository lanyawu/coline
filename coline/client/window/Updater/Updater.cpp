#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include <SmartSkin/smartskin.h>
#include <Commonlib/stringutils.h>
#include <xml/tinyxml.h>
#include <NetLib/asock.h>
#include <vector>
#include <map>
#include <string>
#include <ShellAPI.h>

#pragma warning(disable:4996)

#define WM_COPYCOMPLETE (WM_USER + 0x300)

typedef struct FontStyle
{
	int nFontSize;
	int nFontStyle;
	int cfColor;
	TCHAR szFaceName[32];
}CFontStyle;
 
void GetApplicationPath(std::string &strAppPath)
{
	char szTmp[MAX_PATH] = {0};
	char szAppFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
	memset(szTmp, 0, MAX_PATH);
	CSystemUtils::DeletePathDelimiter(szAppPath, szTmp, MAX_PATH - 1);
	memset(szAppPath, 0, MAX_PATH);
	CSystemUtils::ExtractFilePath(szTmp, szAppPath, MAX_PATH - 1);
	memset(szTmp, 0, MAX_PATH - 1);
	CSystemUtils::IncludePathDelimiter(szAppPath, szTmp, MAX_PATH - 1);
	strAppPath = szTmp;
}

 

//回调函数
BOOL CALLBACK SkinEventWindow(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName, POINT *ptMouse, 
					WPARAM wParam, LPARAM lParam, void *pOverlapped);
 
//windows消息回调
BOOL CALLBACK SkinWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes, 
	                                             void *pOverlapped);
 
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
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
	TCHAR szwWorkPath[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szAppPath, szwWorkPath, MAX_PATH - 1);
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

class CUpdaterWindow
{
public:
	CUpdaterWindow(const char *szWinName, const char *szTitle, HWND hParent, DWORD dwStyle, DWORD dwExStyle, RECT *prc, 
		       BOOL bForceCreate)
	{
		m_hWnd = SkinCreateWindowByName(szWinName, NULL, hParent, dwStyle, dwExStyle, prc, 
			           bForceCreate, SkinEventWindow, SkinWindowProc, this);
		::SkinOrderWindowMessage(m_hWnd, WM_COPYCOMPLETE);
	}
	~CUpdaterWindow()
	{
		::SkinCloseWindow(m_hWnd);
	}
 
	void Show()
	{
		::ShowWindow(m_hWnd, SW_SHOW);
	}
    BOOL IsRunFile(const TCHAR *szSrcName)
	{
		char szFileName[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szSrcName, szFileName, MAX_PATH - 1);
		std::vector<std::string>::iterator it;
		for (it = m_vcRunList.begin(); it != m_vcRunList.end(); it ++)
		{
			if (::stricmp(it->c_str(), szFileName) == 0)
			{
				return TRUE;
			}
		} //end for
		return FALSE;
	}

	BOOL MovePathFiles(const TCHAR *szSrcPath, const TCHAR *szDstPath)
	{ 
		char szDstTmp[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szDstPath, szDstTmp, MAX_PATH - 1);
		if (!CSystemUtils::DirectoryIsExists(szDstTmp))
			CSystemUtils::ForceDirectories(szDstTmp);
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
					if (!IsRunFile(strSrcFile))
					{ 
					    CStdString_ strDstFile = szDstPath;
					    strDstFile += fd.cFileName;
						if (!::MoveFileEx(strSrcFile, strDstFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
						{
							if (!::MoveFileEx(strSrcFile, strDstFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT))
							{
								PRINTDEBUGLOG(dtInfo, "movefile failed");
							} //end
						} //end
					}
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

 
	static DWORD WINAPI UpdateThread(LPVOID lpParam)
	{
		CUpdaterWindow *pThis = (CUpdaterWindow *) lpParam;
     
		TiXmlDocument xmlOld, xmlNew;
		if (xmlOld.LoadFile(pThis->m_strOldVerFileName.c_str())
			&& xmlNew.LoadFile(pThis->m_strNewVerFileName.c_str()))
		{
			char szAppPath[MAX_PATH] = {0};
			char szTempPath[MAX_PATH] = {0};
			CSystemUtils::ExtractFilePath(pThis->m_strOldVerFileName.c_str(), szAppPath, MAX_PATH - 1);
			CSystemUtils::ExtractFilePath(pThis->m_strNewVerFileName.c_str(), szTempPath, MAX_PATH - 1);
			TCHAR szSrc[MAX_PATH] = {0};
			TCHAR szDst[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(szTempPath, szSrc, MAX_PATH - 1);
			CStringConversion::StringToWideChar(szAppPath, szDst, MAX_PATH - 1);
			pThis->MovePathFiles(szSrc, szDst); 
		} //end if (
		::PostMessage(pThis->m_hWnd, WM_COPYCOMPLETE, 0, 0);
		return 0;
	}

	void ShowComment()
	{
		TiXmlDocument xmldoc;
		if (xmldoc.LoadFile(m_strNewVerFileName.c_str()))
		{
			TiXmlElement *pNode = xmldoc.FirstChildElement();
			if (!pNode)
				return;
			TiXmlElement *pComment = pNode->FirstChildElement("comment");
			if (pComment)
			{
				TiXmlElement *pItem = pComment->FirstChildElement();
				CCharFontStyle fs = {0};
				fs.cfColor = 0;
				fs.nFontSize = 10;
				lstrcpy(fs.szFaceName, L"tohoma"); 
				while (pItem)
				{
					const char *p = pItem->GetText();
					if (p)
					{
						int nSize = ::strlen(p);
						TCHAR *szTmp = new TCHAR[nSize + 1];
						memset(szTmp, 0, sizeof(TCHAR) * (nSize + 1));
						CStringConversion::UTF8ToWideChar(p, szTmp, nSize);
						SkinRichEditInsertTip(m_hWnd, L"updatecomment", &fs, 0, szTmp);
						delete []szTmp;
					}
					pItem = pItem->NextSiblingElement();
				}				
			}
			//run
			TiXmlElement *pRun = pNode->FirstChildElement("run");
			if (pRun)
			{
				TiXmlElement *pChild = pRun->FirstChildElement();
				char szAppPath[MAX_PATH] = {0}; 
				CSystemUtils::ExtractFilePath(m_strOldVerFileName.c_str(), szAppPath, MAX_PATH - 1);
				while (pChild)
				{
					const char *szName = pChild->Attribute("name");
					if (szName)
					{
						std::string strFileName = szAppPath;
						strFileName += szName;
					}
					pChild = pChild->NextSiblingElement();
				}
			}
		}
		//
	}

	BOOL WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
	{
		if (uMsg == WM_COPYCOMPLETE)
		{
			std::vector<std::string>::iterator it;
			for (it = m_vcRunList.begin(); it != m_vcRunList.end(); it ++)
			{
				StartShellProcessor(it->c_str(), NULL, TRUE);
			}
			StartShellProcessor(m_strRunAppName.c_str(), "updateversion", FALSE);
			::SkinCloseWindow(m_hWnd);
		}
		return FALSE;
	}

	BOOL SkinEvent(LPCTSTR pstrEvent, LPCTSTR pstrControlName, POINT *ptMouse, 
					          WPARAM wParam, LPARAM lParam)
	{
		if (::lstrcmp(pstrEvent, L"click") == 0)
		{
			if (::lstrcmp(pstrControlName, L"ok") == 0)
			{
				::SkinSetControlEnable(m_hWnd, L"ok", FALSE);
				::SkinSetControlEnable(m_hWnd, L"cancel", FALSE);
				HANDLE hThread = ::CreateThread(NULL, 0, UpdateThread, this, 0, NULL);
				::CloseHandle(hThread);
			} else if (::lstrcmp(pstrControlName, L"cancel") == 0)
			{
				::SkinCloseWindow(m_hWnd);
			} //end else if (::lstrcmp(
		}
		wprintf(L"Event:%s   ControlName:%s \n", pstrEvent, pstrControlName); 
		return FALSE;
	}
public:
	std::string m_strNewVerFileName;
	std::string m_strOldVerFileName;
	std::string m_strRunAppName;
	std::vector<std::string> m_vcRunList;
private:
	HWND m_hWnd;
};

//回调函数
BOOL CALLBACK SkinEventWindow(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName, POINT *ptMouse, 
					WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		CUpdaterWindow *pThis = (CUpdaterWindow *)pOverlapped;
		return pThis->SkinEvent(pstrEvent, pstrControlName, ptMouse, wParam, lParam);
	}
	return FALSE;
}

//windows消息回调
BOOL CALLBACK SkinWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes, 
	                                             void *pOverlapped)
{
	if (pOverlapped)
	{
		CUpdaterWindow *pThis = (CUpdaterWindow *)pOverlapped;
		return pThis->WindowProc(hWnd, uMsg, wParam, lParam, lRes);
	}
	return FALSE;
}

void ShowUpdateWindow(const char *szOldVerFileName, const char *szNewVerFileName, const char *szRunAppName)
{
	static int UPDATE_WINDOW_WIDTH = 400;
	static int UPDATE_WINDOW_HEIGHT = 300;
	std::string strAppPath;
	GetApplicationPath(strAppPath);
	std::string strFileName = strAppPath;
	strFileName += "skin\\updater.xml"; 
	::SkinCreateFromFile(strFileName.c_str());
	RECT rc = {0};
	CSystemUtils::GetScreenRect(&rc);
	rc.left = (rc.right - rc.left - UPDATE_WINDOW_WIDTH) / 2;
	rc.top = (rc.bottom - rc.top - UPDATE_WINDOW_HEIGHT) / 2;
	rc.right = rc.left + UPDATE_WINDOW_WIDTH;
	rc.bottom = rc.top + UPDATE_WINDOW_HEIGHT;
    CUpdaterWindow Win("updatewindow", "自动升级", NULL,  WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX, 
		    0, &rc, FALSE);
	Win.m_strOldVerFileName = szOldVerFileName;
	Win.m_strNewVerFileName = szNewVerFileName;
	Win.m_strRunAppName = szRunAppName;	
	Win.ShowComment();
	Win.Show(); 
	::SkinApplicationRun();
	::SkinDestroyResource();
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
		ShowUpdateWindow(strOldVerFileName.c_str(), strNewVerFileName.c_str(), strRunAppName.c_str());
	}
}

void CheckPlugins()
{
	typedef BOOL (CALLBACK *CHECK_PLUGINS)(const char *szAppPath);
	std::string strAppPath;
	GetApplicationPath(strAppPath);
	strAppPath += "checkVersion.dll";
	TCHAR szFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(strAppPath.c_str(), szFileName, MAX_PATH - 1);
	HMODULE h = ::LoadLibrary(szFileName);
	if (h != NULL)
	{
		CHECK_PLUGINS pProc = (CHECK_PLUGINS)::GetProcAddress(h, "CheckMainPlugins");
		if (pProc)
			pProc(NULL);
		::FreeLibrary(h);
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
	CoInitialize(NULL);
	CASocket::ASocketInit();
	if (SkinCheckRunOption())
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
	} else
		::MessageBox(NULL, L"本系统需要Direct9以上版本支持", L"提示", MB_OK);
	CASocket::ASocketDestroy();
	::CoUninitialize();
	return 0;
}


#pragma warning(default:4996)
