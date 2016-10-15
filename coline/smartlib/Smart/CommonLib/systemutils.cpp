#include <Commonlib/SystemUtils.h>
#include <CommonLib/StringUtils.h>
#include <map>
#include <fstream>
#include <time.h>
#include <shellapi.h>
#include <shlobj.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <winsock.h>

#pragma warning(disable:4996)

#pragma comment(lib, "winmm.lib")

DWORD CSystemUtils::m_ScreenPixels = 0;
 

PROCSWITCHTOTHISWINDOW CSystemUtils::m_pSwitchToThisWindow = NULL;
PROCGETLASTINPUTINFO   CSystemUtils::m_pGetLastInputInfo = NULL;
 
__int64 CSystemUtils::GetFileSize(const char *szFileName)
{
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH];
	CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
	__int64 nSize = 0;
	std::ifstream ifs(szTemp);
#endif
	if (ifs.is_open())
	{
		ifs.seekg(0, std::ios_base::end);
		nSize =  ifs.tellg();
		ifs.close();
	}
	return nSize;
}

static void __ByteToString(std::string &strIp, BYTE bit)
{
	BYTE v, b = 100;
	bool bHas = false;
	while(b)
	{
		v = bit / b;
		if (v || bHas)
		{
			strIp.push_back(v + '0');
			bHas = true;
		}
		bit -= v * b;
		b /= 10;
	}
	if (!bHas)
		strIp.push_back('0');
}

//域名转IP
BOOL CSystemUtils::GetIpByHostName(const char *szHostName, std::string &strIp)
{
	hostent *pHost = ::gethostbyname(szHostName);
	if ((pHost) && (pHost->h_length > 0))
	{
		DWORD dwIp = (*((DWORD *)(*(pHost->h_addr_list))));
		BYTE *p = (BYTE *)&dwIp;
		for (int i = 0; i < 4; i ++)
		{
			__ByteToString(strIp, *p ++);
			strIp.push_back('.');
		}
		//delete last "."
		strIp.pop_back();
		return TRUE;
	}
	return FALSE;
}

char *CSystemUtils::GetMyDocumentPath(char *szMyDocPath, int nMaxSize)
{
	return ReadRegisterKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
				"Personal", szMyDocPath, nMaxSize);
}

char *CSystemUtils::GetLocalAppPath(char *szLocalAppPath, int nMaxSize)
{
	return ReadRegisterKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
				"Local AppData", szLocalAppPath, nMaxSize);
}

//获取系统应用程序目录
char *  CSystemUtils::GetSystemAppPath(char *szLocalAppPath, int nMaxSize)
{
	return ReadRegisterKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 
				"AppData", szLocalAppPath, nMaxSize);
}

//获取临时文件目录
char *CSystemUtils::GetSystemTempPath(char *szTmpPath, int nMaxSize)
{
	TCHAR szBuff[MAX_PATH] = {0};
	if (::GetTempPath(nMaxSize, szBuff) > 0)
	{
		CStringConversion::WideCharToString(szBuff, szTmpPath, nMaxSize);
		return szTmpPath;
	}
	return NULL;
}

//获取一个临时文件名称
char *CSystemUtils::GetSystemTempFileName(char *szFileName, int nMaxSize)
{
	TCHAR szBuffPath[MAX_PATH] = {0};
	TCHAR szBuff[MAX_PATH] = {0};
	::GetTempPath(MAX_PATH - 1, szBuffPath);
	if (::GetTempFileName(szBuffPath, L"gcm", 0, szBuff) > 0)
	{
		CStringConversion::WideCharToString(szBuff, szFileName, nMaxSize);
		return szFileName;
	}
	return NULL;
}

//获取安装程序路径
char *CSystemUtils::GetProgramFilesPath(char *szPath, int nMaxSize)
{
	TCHAR szBuff[MAX_PATH] = {0};
	if (::SHGetSpecialFolderPath(NULL, szBuff, CSIDL_PROGRAM_FILES, FALSE))
	{
		CStringConversion::WideCharToString(szBuff, szPath, nMaxSize);
		return szPath;
	} else
		return NULL;
}

//获取系统路径
char * CSystemUtils::GetSystemDirectory(char *szPath, int nMaxSize)
{
	TCHAR szBuff[MAX_PATH] = {0};
	::GetSystemDirectory(szBuff, MAX_PATH);
	CStringConversion::WideCharToString(szBuff, szPath, nMaxSize);
	return szPath;
}

//获取操作系统版本
DWORD  CSystemUtils::GetOSVersion()
{
typedef void (WINAPI *LPGETNATIVESYSTEMINFO)(LPSYSTEM_INFO);
	OSVERSIONINFOEX os;
	SYSTEM_INFO si;
	LPGETNATIVESYSTEMINFO pFun;
    BOOL bOsVersionInfoEx;

	//
	memset(&si, 0, sizeof(SYSTEM_INFO));
	memset(&os, 0, sizeof(OSVERSIONINFOEX));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	bOsVersionInfoEx = ::GetVersionEx((OSVERSIONINFO *)&os);
	if (!bOsVersionInfoEx)
	{
		os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!::GetVersionEx((OSVERSIONINFO *)&os))
			return 0;
	}
    
	pFun = (LPGETNATIVESYSTEMINFO)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "GetNativeSystemInfo");
	if (pFun)
		pFun(&si);
	else
		::GetSystemInfo(&si);
 
   switch(os.dwPlatformId)
   {
   case VER_PLATFORM_WIN32_NT:
	   if (os.dwMajorVersion <= 4)
		   return OS_VERSION_OLD;
	   if ((os.dwMajorVersion == 5) && (os.dwMinorVersion == 0))
		   return OS_VERSION_WIN2000;
	   if ((os.dwMajorVersion == 5) && (os.dwMinorVersion == 1))
		   return OS_VERSION_WINXP;
	   if ((os.dwMajorVersion == 5) && (os.dwMinorVersion == 2))
	   {
		   if ((os.wProductType == VER_NT_WORKSTATION) &&
			   ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
			   (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)))
			   return OS_VERSION_WINXP64;
		   else
			   return OS_VERSION_WIN2003;
	   }
	   if ((os.dwMajorVersion == 6) && (os.dwMinorVersion == 0))
		   return OS_VERSION_VISTA;	   
	   if (os.dwMajorVersion >= 6)
		   return OS_VERSION_WIN7;
   }
   return OS_VERSION_OLD;
}

DWORD CSystemUtils::GetConnectState()
{
	return 0; //::InternetGetConnectedStateEx(
}

//获取网卡mac
BOOL  CSystemUtils::GetNetCardMac(std::string &strMac)
{
	PROCNETBIOS pNetBios = NULL; //网络函数
	HMODULE hModule = ::GetModuleHandle(L"netapi32.dll");
	if (hModule)
	{
		pNetBios = (PROCNETBIOS)::GetProcAddress(hModule, "Netbios");
	}
	if (pNetBios)
	{
		NCB cb = {0};
		LANA_ENUM num = {0};
		ADAPTER_STATUS status = {0};
		strMac = "";
		cb.ncb_command = NCBENUM;
		pNetBios(&cb);

		//
		cb.ncb_buffer = (PUCHAR)&num;
		cb.ncb_length = sizeof(LANA_ENUM);

		if (pNetBios(&cb) != 0)
			return FALSE;

		memset(&cb, 0, sizeof(NCB));
		cb.ncb_command = NCBRESET;
		cb.ncb_lana_num = num.lana[0];
		if (pNetBios(&cb) != 0)
			return FALSE;

		memset(&cb, 0, sizeof(NCB));
		cb.ncb_command = NCBASTAT;
		cb.ncb_lana_num = num.lana[0];
		memmove(cb.ncb_callname, "*", 1);
		cb.ncb_buffer = (PUCHAR)&status;
		cb.ncb_length = sizeof(ADAPTER_STATUS);
		if (pNetBios(&cb) == 0)
		{
			char szTemp[8] = {0};
			for (int i = 0; i < 5; i ++)
			{
				sprintf(szTemp, "%0.2x", status.adapter_address[i]);
				strMac += szTemp;
			}
		}
	}
	if (hModule)
		::FreeLibrary(hModule);
    return TRUE;
}

//获取网卡唯一标识
DWORD CSystemUtils::GetNetCardFlag()
{
	std::string strMac;
	if (GetNetCardMac(strMac))
	{
		char szTemp[9] = {0};
		strncpy(szTemp, strMac.c_str(), 8);
		return (DWORD)HexToInt(szTemp);
	} else
	{
		return ::GetTickCount();
	}
}

DWORD CSystemUtils::GetCpuId()
{
	return 0;
}

int   CSystemUtils::HexToInt(const char *szStr)  
{
	int nR = 0;

	int   i, mid;  
	int   len   = (int)::strlen(szStr);      
	if(len > 8) 
		return nR;
	mid = 0;
	for(i = 0; i < len; i ++)
	{
		if(szStr[i] >= '0' && szStr[i] <= '9')
			mid = szStr[i] - '0';  
		else if(szStr[i] >= 'a' && szStr[i] <= 'f')
			mid = szStr[i] - 'a' + 10;  
		else if(szStr[i] >= 'A' && szStr[i] <= 'F')
			mid = szStr[i] - 'A' + 10;  
		else return nR;
		mid <<= ((len - i - 1) << 2);  
		nR |= mid;      
	}      
	return nR;  
} 
//闪动窗口
void CSystemUtils::FlashWindow(HWND hWnd, BOOL bFlash)
{
	if (bFlash)
	{
		if (::GetForegroundWindow() != hWnd)
		{
			FLASHWINFO fi = {0};
			fi.cbSize = sizeof(FLASHWINFO);
			fi.dwFlags = FLASHW_ALL;
			fi.dwTimeout = 0;
			fi.hwnd = hWnd;
			fi.uCount = DEFAULT_FLAHS_WINDOW_COUNT;
			::FlashWindowEx(&fi); 
		}
	} else
	{
		FLASHWINFO fi = {0};
		fi.cbSize = sizeof(FLASHWINFO);
		fi.dwFlags = FLASHW_STOP;
		fi.dwTimeout = 0;
		fi.hwnd = hWnd;
		fi.uCount = 0;
		::FlashWindowEx(&fi); 
	}
}

//切换窗口到前面
void CSystemUtils::BringToFront(HWND hWnd)
{
	if (!::IsWindowVisible(hWnd))
		::ShowWindow(hWnd, SW_SHOW);
	if (!m_pSwitchToThisWindow)
	{
		HMODULE hModule = ::GetModuleHandle(L"user32");
		m_pSwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)::GetProcAddress(hModule, "SwitchToThisWindow");
	}
	if (m_pSwitchToThisWindow)
		m_pSwitchToThisWindow(hWnd, TRUE);
}

//获取用户电脑最后活动时间 毫秒为单位
DWORD  CSystemUtils::GetUserLastActiveTime()
{
	if (!m_pGetLastInputInfo)
	{
		HMODULE hModule = ::GetModuleHandle(L"user32");
		m_pGetLastInputInfo = (PROCGETLASTINPUTINFO)::GetProcAddress(hModule, "GetLastInputInfo");
	}
	if (m_pGetLastInputInfo)
	{
		LASTINPUTINFO_ info = {0};
		info.cbSize = sizeof(LASTINPUTINFO_);
		m_pGetLastInputInfo(&info);
		return info.dwActTime;
	}
	return 0;
}

//播放声音文件
void CSystemUtils::PlaySoundFile(const char *szFileName, BOOL bLoop)
{
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
	if (bLoop)
		::sndPlaySound(szTemp, SND_LOOP | SND_ASYNC);
	else
		::sndPlaySound(szTemp, SND_ASYNC);
#endif
}

//打开一个链接地址
void CSystemUtils::OpenURL(const char *szURL)
{
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szURL, szTemp, MAX_PATH);
	::ShellExecute(NULL, L"OPEN", szTemp, NULL, NULL, SW_SHOW);
#endif
}

BOOL CSystemUtils::StartShellProcessor(const char *szAppName, const char *szParams, const char *szWorkPath, BOOL bWait)
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
	if (szWorkPath == NULL)
	{
		char szAppPath[MAX_PATH] = {0};
		ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
		CStringConversion::StringToWideChar(szAppPath, szwWorkPath, MAX_PATH - 1);
	} else 
	{
		CStringConversion::StringToWideChar(szWorkPath, szwWorkPath, MAX_PATH - 1);
	}
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

//普通字符串转url串
char *  CSystemUtils::StringToUrlChars(const char *szSrc, char *szUrlChars, int nMaxLen)
{
	size_t n = strlen(szSrc);
	char szTemp[4] = {0};
	std::string str;
	for (size_t i = 0; i < n; i ++)
	{
		sprintf(szTemp, "%%%02X", (BYTE)szSrc[i]);
		str += szTemp;
	}
	strncpy(szUrlChars, str.c_str(), nMaxLen);
	return szUrlChars;
}

char * CSystemUtils::DateTimeToStr(DWORD dwTime, char *szDateTime, BOOL bSep)
{
	time_t tt = dwTime;
	tm *t = localtime(&tt);
	if (bSep)
		sprintf(szDateTime, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	else
		sprintf(szDateTime, "%04d%02d%02d%02d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	return szDateTime;
}

DWORD StringTimeToSecond(const char *strTime)
{
	int nSec = 0;
	if (strTime)
	{
		char szTmp[3] = {0};
		int j = 2;
		int f = 0;
		int n = ::strlen(strTime);
		for (int i = n -1; i >= 0; i --)
		{
			if (strTime[i] ==  ':')
			{
				if (j != 0)
					f =  (szTmp[j] - '0');
				else
					f = ::atoi(szTmp);
				if (nSec == 0)
					nSec = f;
				else
				{
					nSec += (f * 60);
					break;
				}
				memset(szTmp, 0, 3);
				j = 2;
				continue;
			} else
			{
				if (j == 0)
				{
					nSec = 0;
					break;
				} else
				{
					j --;
					szTmp[j] = strTime[i];
				}
			} //end else
		}
	}
	return nSec;
}

//计算两个字符串的日期差 秒为单位 只取最后 分:秒
DWORD  CSystemUtils::MinusTimeString(const char *strTime1, const char *strTime2)
{
	int n1 = StringTimeToSecond(strTime1);
	int n2 = StringTimeToSecond(strTime2);
	if (n1 > n2)
	   return (n1 - n2); 
	return 0xFFFFFF;
}

//日期转换 从time_t 转成字符串 2010-01-01 不转时间
char * CSystemUtils::DateToStr(DWORD dwDate, char *szDate)
{
	time_t tt = dwDate;
	tm *t = localtime(&tt);
    sprintf(szDate, "%02d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	return szDate;
}

char * CSystemUtils::TimeToStr(DWORD dwTime, char *szTime)
{
	time_t tt = dwTime;
	tm *t = localtime(&tt);
    sprintf(szTime, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
	return szTime;
}

//删除一个文件
BOOL CSystemUtils::DeleteFilePlus(const char *szFileName)
{
	if (FileIsExists(szFileName))
	{
#ifdef _UNICODE
		TCHAR szTemp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH - 1);
		::DeleteFile(szTemp);
		return TRUE;
#endif
	}
	return FALSE;
}

//移动一个文件
BOOL CSystemUtils::MoveFilePlus(const char *szSrcFileName, const char *szDestFileName, BOOL bForce)
{
	TCHAR szwSrc[MAX_PATH] = {0};
	TCHAR szwDst[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szSrcFileName, szwSrc, MAX_PATH);
	CStringConversion::StringToWideChar(szDestFileName, szwDst, MAX_PATH);
	DWORD dwFlags = 0;
	if (bForce)
		dwFlags = MOVEFILE_REPLACE_EXISTING;
	return	::MoveFileEx(szwSrc, szwDst, dwFlags);
}

BOOL CSystemUtils::DeleteDirectory( const char* szPath )
{
	if( szPath == NULL )
		return false;

	//检测szPath，只支持目录
	//...

	char szFile[MAX_PATH] = { 0 };
	sprintf( szFile, "%s\\*", szPath );

	WIN32_FIND_DATAA fd;
	HANDLE hFile = ::FindFirstFileA( szFile, &fd );
	if( INVALID_HANDLE_VALUE != hFile ){
		BOOL bFind;
		do{
			BOOL bRet;
			if( strcmp( fd.cFileName, "." ) != 0 
				&& strcmp( fd.cFileName, "..") != 0 ){
				//去除只读

				sprintf( szFile, "%s\\%s", szPath, fd.cFileName );
				bRet = ::SetFileAttributesA( szFile, FILE_ATTRIBUTE_NORMAL );

				if( ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ){
					DeleteDirectory( szFile );
					bRet = ::RemoveDirectoryA( szFile );
				}
				else{
					bRet = ::DeleteFileA( szFile );
				}
			}

			bFind = ::FindNextFileA( hFile, &fd );
		}while( bFind );

		::FindClose( hFile );
	}

	::RemoveDirectoryA( szPath );

	return true;
}


BOOL CSystemUtils::CopyFilePlus(const char *szSrcFileName, const char *szDestFileName, BOOL bForce)
{
	if ((!bForce) && FileIsExists(szDestFileName))
		return FALSE;
#ifdef _UNICODE
	TCHAR szwSrc[MAX_PATH] = {0};
	TCHAR szwDst[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szSrcFileName, szwSrc, MAX_PATH);
	CStringConversion::StringToWideChar(szDestFileName, szwDst, MAX_PATH);
	return ::CopyFile(szwSrc, szwDst, !bForce);
#else
	return ::CopyFile(szSrcFileName, szDestFileName, bForce);
#endif
}

//根据分隔符取数据
BOOL CSystemUtils::GetStringBySep(const char *szString, char *szDest, char c, int nIdx)
{
	int nCount = 0;
	char *p = const_cast<char *>(szString);
	BOOL b = FALSE;
	while(p)
	{
		if (nIdx == nCount)
		{
			p = CStringConversion::GetStringBySep(p, szDest, c);
			b = TRUE;
			break;
		} else
			p = CStringConversion::GetStringBySep(p, NULL, c);
		nCount ++;
	}
    return b;
}

BOOL CSystemUtils::OpenFileDialog(HINSTANCE hInstance, HWND hOwner, char *szTitle, char *szFilter, char *szSelFile, 
								  CStringList_ &FileList, BOOL bIsMultiSelect, BOOL IsSaveDlg)
{
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hOwner;
	ofn.hInstance = hInstance;
#ifdef _UNICODE
	TCHAR szwFilter[1024] = {0};
	TCHAR szwSelFile[1024] = {0};
	TCHAR szwTitle[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFilter, szwFilter, 1024 - 1);
	//替换
	int n = ::lstrlen(szwFilter);
	for (int i = 0; i < n; i ++)
	{
		if (szwFilter[i] == _T('|'))
			szwFilter[i] = _T('\0');
	}
	CStringConversion::StringToWideChar(szSelFile, szwSelFile, MAX_PATH);
	CStringConversion::StringToWideChar(szTitle, szwTitle, MAX_PATH);
	
	ofn.lpstrFilter = szwFilter;
	ofn.lpstrFile = szwSelFile;
	ofn.lpstrTitle = szwTitle;
#else
	ofn.lpstrFilter = szFileter;
	ofn.lpstrFile = szSelFile;
	ofn.lpstrTitle = szTitle;
#endif
	ofn.nMaxFile = 1024;
	ofn.Flags = 0;
	if (IsSaveDlg)
	{
		ofn.Flags |= OFN_EXPLORER;
	} else
	{
		ofn.Flags = ofn.Flags | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST  | OFN_EXPLORER;
		if (bIsMultiSelect)
			ofn.Flags = ofn.Flags | OFN_ALLOWMULTISELECT; 
	}
	BOOL bSucc = FALSE;
	if (IsSaveDlg)
		bSucc = ::GetSaveFileName(&ofn);
	else
		bSucc = ::GetOpenFileName(&ofn);
	if (bSucc)
	{
#ifdef _UNICODE
		TCHAR szPath[MAX_PATH] = {0};
		::lstrcpyn(szPath, szwSelFile, ofn.nFileOffset);
		int nLen = ::lstrlen(szPath);
		if (szPath[nLen - 1]  != '\\')
			::lstrcat(szPath, TEXT("\\"));
		TCHAR *p = szwSelFile + ofn.nFileOffset;
		//获取扩展名称
		char szExt[MAX_PATH] = {0};
		char szExtTemp[MAX_PATH] = {0};
		int nSep = ofn.nFilterIndex * 2 - 1;
		GetStringBySep(szFilter, szExtTemp, '|', nSep); 
		if (strcmp(szExtTemp, "*.*") != 0)
		{
			if (szExtTemp[0] == '*')
				memmove(szExt, szExtTemp + 1, strlen(szExtTemp) - 1);
			else
				strcpy(szExt, szExtTemp);
		}
		TCHAR szwFileName[MAX_PATH];
		while (*p)
		{
			char *szFileName = new char[MAX_PATH];
			memset(szwFileName, 0, MAX_PATH * sizeof(TCHAR));
			memset(szFileName, 0, MAX_PATH);
			::lstrcat(szwFileName, szPath); //文件路径
			::lstrcat(szwFileName, p); //文件名
			p += ::lstrlen(p) + 1;
			CStringConversion::WideCharToString(szwFileName, szFileName, MAX_PATH - 1);
			if (IsSaveDlg && (szExt[0] != '\0'))
			{
				char szSrcExt[32] = {0};
				ExtractFileExtName(szFileName, szSrcExt, 31);
				if (strcmp(szSrcExt, (szExt + 1)) != 0)
					::strcat(szFileName, szExt);
			}
			FileList.push_back(szFileName);
			delete []szFileName;
		}
#else
		strncpy(szSelFiles, ofn.lpstrFile, MAX_PATH);
#endif
		return TRUE;
	}
	return FALSE;
}

//选取一个目录
BOOL CSystemUtils::SelectFolder(HWND hOwner, char *szSelPath)
{
#ifdef _UNICODE
    BROWSEINFO bi;                         //BROWSEINFO结构体  
    TCHAR szBuffer[MAX_PATH] = L"";  
    TCHAR szFullPath[MAX_PATH] = L"";  
    bi.hwndOwner = hOwner;        
    bi.pidlRoot = NULL;  
    bi.pszDisplayName = szBuffer;    //返回选择的目录名的缓冲区  
    bi.lpszTitle = L"请选择保存的目录";   //弹出的窗口的文字提示  
    bi.ulFlags = BIF_RETURNONLYFSDIRS;   //只返回目录
    bi.lpfn = NULL;   //回调函数 
    bi.lParam = 0;  
    bi.iImage = 0;  
    ITEMIDLIST* pidl = ::SHBrowseForFolder (&bi);   //显示弹出窗口，ITEMIDLIST很重要  
    if(::SHGetPathFromIDList(pidl, szFullPath))     //在ITEMIDLIST中得到目录名的整个路径  
    {
		CStringConversion::WideCharToString(szFullPath, szSelPath, MAX_PATH);
		return TRUE;
    }
#endif
	return FALSE;
}

//打开一个字体选择框
BOOL CSystemUtils::OpenFontDialog(HINSTANCE hInstance, HWND hOwner, int &nFontSize, int &nFontStyle, 
	               int &nFontColor, char *szFontName)
{
	if (m_ScreenPixels == 0)
	{
		HDC hdc = ::GetDC(::GetDesktopWindow());
		m_ScreenPixels = ::GetDeviceCaps(hdc, LOGPIXELSY);
		::ReleaseDC(::GetDesktopWindow(), hdc);
	}
	CHOOSEFONT cf = {0};
	LOGFONT LogFont = {0};
	LogFont.lfHeight = -::MulDiv(nFontSize, m_ScreenPixels, 72);
	LogFont.lfItalic = nFontStyle & FONT_STYLE_ITALIC;
	LogFont.lfUnderline = nFontStyle & FONT_STYLE_UNDERLINE;
    LogFont.lfQuality = DEFAULT_QUALITY;
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	LogFont.lfPitchAndFamily = DEFAULT_PITCH;
	LogFont.lfCharSet = DEFAULT_CHARSET;
	if (nFontStyle & FONT_STYLE_BOLB)
		LogFont.lfWeight = FW_BOLD;
	else
		LogFont.lfWeight = FW_NORMAL;
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hInstance = hInstance;
	cf.hwndOwner = hOwner;
	cf.hDC = 0;
	cf.lpLogFont = &LogFont;
	cf.rgbColors = nFontColor;
	cf.nSizeMin = 8;
	cf.nSizeMax = 18;
	cf.Flags = CF_LIMITSIZE | CF_EFFECTS| CF_BOTH | CF_FORCEFONTEXIST |  CF_INITTOLOGFONTSTRUCT ;
#ifdef _UNICODE
	CStringConversion::StringToWideChar(szFontName, LogFont.lfFaceName, 32);
#else
	strncpy(LogFont.lfFaceName, szFontName, 32);
#endif
	if (::ChooseFont(&cf))
	{
		nFontSize = -::MulDiv(LogFont.lfHeight, 72, m_ScreenPixels);
		nFontStyle = 0;
		if (LogFont.lfItalic)
			nFontStyle |= FONT_STYLE_ITALIC;
		if (LogFont.lfUnderline)
			nFontStyle |= FONT_STYLE_UNDERLINE;
		if (LogFont.lfWeight == FW_BOLD)
			nFontStyle |= FONT_STYLE_BOLB;
		nFontColor = cf.rgbColors;
#ifdef _UNICODE
		CStringConversion::WideCharToString(LogFont.lfFaceName, szFontName, 32);
#else
		strncpy(szFontName, LogFont.lfFaceName, 32);
#endif
		return TRUE;
	}
	return FALSE;
}

//打开颜色选择框
BOOL  CSystemUtils::OpenColorDialog(HINSTANCE hInstance, HWND hOwner, COLORREF &cr)
{
	static COLORREF clref[16] = {0x00ff0000};
	CHOOSECOLOR cc = {0};
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hOwner;
	cc.rgbResult = cr;
	cc.lpCustColors = clref;
	cc.Flags = 0;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL; 
	if (::ChooseColor(&cc))
	{
		cr = cc.rgbResult;
		return TRUE;
	}
	return FALSE;
}

//获取本机网卡IP地址
DWORD CSystemUtils::GetLocalIP()
{
	char szHostName[MAX_PATH] = {0};
	int nStatus = ::gethostname(szHostName, MAX_PATH);
	if (nStatus == SOCKET_ERROR)
	{
		return 0;
	}
	HOSTENT *pHost = ::gethostbyname(szHostName);
	if (pHost)
	{
		if (pHost->h_length > 0)
			return (*((DWORD *)(pHost->h_addr_list[0])));
	}
	return 0;
}



BOOL CSystemUtils::ForceDirectories(const char *szPath)
{
	if (!szPath)
		return FALSE;
	char szTemp[MAX_PATH] = {0};
	if (szPath[strlen(szPath) - 1] == '\\')
		strncpy(szTemp, szPath, strlen(szPath) - 1);
	else
		strcpy(szTemp, szPath);
	char szParentPath[MAX_PATH] = {0};
	if ((strlen(szTemp) < 3) || (DirectoryIsExists(szTemp)))
		return TRUE;
	if (ExtractFilePath(szTemp, szParentPath, MAX_PATH) != NULL)
	{
        if (ForceDirectories(szParentPath))
			return CreateDir(szPath);
	}
	return FALSE;
}

BOOL CSystemUtils::DirectoryIsExists(const char *szPath)
{
	int nCode;
#ifdef _UNICODE
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
    nCode = ::GetFileAttributesW(szTemp);
#else
	nCode = ::GetFileAttributesA(szPath);
#endif
	return ((nCode != -1) && ((FILE_ATTRIBUTE_DIRECTORY & nCode) != 0));
}

//检测文件是否存在
BOOL CSystemUtils::FileIsExists(const char *szFileName)
{
	int nCode;
#ifdef _UNICODE
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
    nCode = ::GetFileAttributesW(szTemp);
#else
	nCode = ::GetFileAttributesA(szFileName);
#endif
	return ((nCode != -1) && ((FILE_ATTRIBUTE_DIRECTORY & nCode) == 0));
}

BOOL CSystemUtils::CreateDir(const char *szPath)
{
	if (DirectoryIsExists(szPath)) //目录已经存在
		return TRUE;
#ifdef _UNICODE
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
	return ::CreateDirectoryW(szTemp, NULL);
#else
	return ::CreateDirectoryA(szPath, NULL);
#endif
}

//读取注册表项值
BOOL CSystemUtils::ReadRegisterItems(HKEY nKey, const char *szPath, std::vector<std::string> &Items)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp;
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
	if (::RegOpenKeyExW(nKey, szTemp, 0, KEY_READ, &nTemp) == ERROR_SUCCESS)
	{
		int nIdx = 0;
		do
		{
			WCHAR szSubKey[MAX_PATH] = {0};
			WCHAR szValue[MAX_PATH] = {0};
			DWORD dwValueSize = MAX_PATH - 1;
			DWORD dwSize = MAX_PATH - 1;
			DWORD dwRegType = REG_SZ;
			if (RegEnumKeyW(nTemp, nIdx, szSubKey, dwSize) == ERROR_SUCCESS)
			{
				char strKey[MAX_PATH] = {0};
				CStringConversion::WideCharToString(szSubKey, strKey, MAX_PATH - 1);
				if (::strlen(strKey) > 0)
					Items.push_back(strKey);
				nIdx ++;
			} else
				break;
		} while (TRUE);
		bSucc = TRUE;
		RegCloseKey(nTemp);
	}
#else
#endif
	return bSucc;
}

char * CSystemUtils::ReadRegisterKey(HKEY nKey, const char *szPath, const char *szItem, char *szResult, int nMaxSize)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp;
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
	if (::RegOpenKeyExW(nKey, szTemp, 0, KEY_READ, &nTemp) == ERROR_SUCCESS)
	{
		WCHAR szSubKey[MAX_PATH] = {0};
		WCHAR szData[MAX_PATH] = {0};
		DWORD nRegType = REG_SZ;
		DWORD dwSize = MAX_PATH;
		CStringConversion::StringToWideChar(szItem, szSubKey, MAX_PATH);
		if (::RegQueryValueExW(nTemp, szSubKey, NULL, &nRegType, (LPBYTE)szData, &dwSize) == ERROR_SUCCESS) 
		{
			CStringConversion::WideCharToString(szData, szResult, nMaxSize);
			bSucc = TRUE;
		}
		RegCloseKey(nTemp);
	}
#else
	HKEY nTemp;
	if (::RegOpenKeyExA(nKey, szTemp, 0, KEY_READ, &nTemp) == ERROR_SUCCESS)
	{
		DWORD nRegType = REG_NONE;
		DWROD dwSize;
		if (nMaxSize > 0)
			dwSize = nMaxSize;
		else
			dwSize = 0;
		if (::RegQueryValueExA(nTemp, szItem, NULL, &nRegType, (LPBYTE)szResult, &dwSize) == ERROR_SUCCESS) 
		{
			bSucc = TRUE;
		}
		RegCloseKey(nTemp);
	}
#endif
    if (bSucc)
		return szResult;
	else
		return NULL;
}

//创建一个子键
BOOL CSystemUtils::CreateChildKey(HKEY nKey, const char *szPath, const char *szChildKey)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp = (HKEY)0;
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
	if (::RegCreateKey(nKey, szTemp, &nTemp) == ERROR_SUCCESS)
	{
		bSucc = TRUE;		 
		RegCloseKey(nTemp);
	}
#else
	HKEY nTemp;
	if (::RegCreateKey(nKey, szTemp, &nTemp) == ERROR_SUCCESS)
	{
		bSucc = TRUE;		 
		RegCloseKey(nTemp);
	}
#endif
	return bSucc;
}

BOOL  CSystemUtils::WriteRegisterKey(HKEY nKey, const char *szPath, const char *szItem, const char *szKey, DWORD dwRegType)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp;
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
	bSucc = ::RegOpenKeyExW(nKey, szTemp, 0, KEY_WRITE, &nTemp) == ERROR_SUCCESS;
	if (!bSucc)
	{
		DWORD dwDisposition;
		bSucc = RegCreateKeyEx(nKey, szTemp, 0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_WRITE, NULL, &nTemp, &dwDisposition) == ERROR_SUCCESS;
	}
	if (bSucc)
	{
		WCHAR szSubKey[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szItem, szSubKey, MAX_PATH);
		switch(dwRegType)
		{
		case REG_SZ:
			{
				TCHAR szwKey[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(szKey, szwKey, MAX_PATH - 1);
				
				if (::RegSetValueEx(nTemp, szSubKey, NULL, dwRegType, (LPBYTE)szwKey, (DWORD)(::lstrlen(szwKey) * sizeof(TCHAR))) == ERROR_SUCCESS) 
					bSucc = TRUE;
				break;
			}
		case REG_DWORD:
			{
				if (::RegSetValueEx(nTemp, szSubKey, NULL, dwRegType, (LPBYTE)szKey, sizeof(DWORD)) == ERROR_SUCCESS)
					bSucc = TRUE;
				break;
			}
		}		
		RegCloseKey(nTemp);
	}
#else
	HKEY nTemp;
	if (::RegOpenKeyExA(nKey, "", 0, KEY_ALL_ACCESS, &nTemp) == ERROR_SUCCESS)
	{
		DWORD nRegType = REG_NONE;
		if (::RegSetValueExA(nTemp, szItem, NULL, nRegType, (LPBYTE)szKey,strlen(szData)) == ERROR_SUCCESS) 
		{
			bSucc = TRUE;
		}
		RegCloseKey(nTemp);
	}
#endif
	return bSucc;
}

//注册关联的Web协议
BOOL CSystemUtils::RegisterWebProtocol(const char *szProtoName, const char *szAppName, const int iIconIdx)
{
	if (szProtoName && (::strlen(szProtoName) > 0) && szAppName && (::strlen(szAppName) > 0))
	{
		char szTmp[MAX_PATH] = {0};
		sprintf(szTmp, "%sProtocol", szProtoName);
		CreateChildKey(HKEY_CLASSES_ROOT, szProtoName, NULL);
		WriteRegisterKey(HKEY_CLASSES_ROOT, szProtoName, NULL, szTmp, REG_SZ);
		WriteRegisterKey(HKEY_CLASSES_ROOT, szProtoName, "URL Protocol", szAppName, REG_SZ);
		memset(szTmp, 0, MAX_PATH);
		sprintf(szTmp, "%s\\DefaultIcon", szProtoName);
		CreateChildKey(HKEY_CLASSES_ROOT, szTmp, NULL);
		char szValue[MAX_PATH] = {0};
		sprintf(szValue, "%s,%d", szAppName, iIconIdx);
		WriteRegisterKey(HKEY_CLASSES_ROOT, szTmp, "", szValue, REG_SZ);
		memset(szTmp, 0, MAX_PATH);
		sprintf(szTmp, "%s\\shell\\open\\command", szProtoName);
		CreateChildKey(HKEY_CLASSES_ROOT, szTmp, NULL);
		memset(szValue, 0, MAX_PATH);
		sprintf(szValue, "\"%s\" \"%%1\"", szAppName);
        WriteRegisterKey(HKEY_CLASSES_ROOT, szTmp, "", szValue, REG_SZ);
		return TRUE;
	}
	return FALSE;
}

//删除注册表某个键值
BOOL CSystemUtils::DeleteRegisterKey(HKEY nKey, const char *szPath, const char *szItem)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp;
	WCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szTemp, MAX_PATH);
	if (::RegOpenKeyExW(nKey, szTemp, 0, KEY_WRITE, &nTemp) == ERROR_SUCCESS)
	{
		WCHAR szSubKey[MAX_PATH] = {0};
		DWORD nRegType = REG_SZ;
		CStringConversion::StringToWideChar(szItem, szSubKey, MAX_PATH);
		if (::RegDeleteValue(nTemp, szSubKey) == ERROR_SUCCESS) 
		{
			bSucc = TRUE;
		}
		RegCloseKey(nTemp);
	}
#endif
	return bSucc;
}

char * CSystemUtils::GetApplicationFileName(char *szFileName, int nMaxSize)
{
	return GetModuleAppFileName(NULL, szFileName, nMaxSize);
}

char *  CSystemUtils::GetModuleAppFileName(HMODULE hModule, char *szFileName, int nMaxSize)
{
#ifdef _UNICODE
	WCHAR szTemp[MAX_PATH] = {0};
	::GetModuleFileNameW(hModule, szTemp, MAX_PATH);
	CStringConversion::WideCharToString(szTemp, szFileName, nMaxSize - 1);
    return szFileName;
#else
	::GetModuleFileNameA(hModule, szFileName, nMaxSize);
	return szFileName;
#endif
}

char * CSystemUtils::ExtractFilePath(const char *szFileName, char *szPath, int nMaxPath)
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

char *CSystemUtils::ExtractFileName(const char *szFileName, char *szDest, int nMaxSize)
{
	if (szFileName)
	{
		int nPos = (int)strlen(szFileName);
		while (nPos > 0)
		{
			if (szFileName[nPos - 1] == '\\')
				break;
			nPos --;
		}
		if (nPos > 0)
		{
			int nSize = (int)strlen(szFileName) - nPos;
			if (nSize > 0)
			{
				if (nSize > nMaxSize)
					nSize = nMaxSize;
				strncpy(szDest, szFileName + nPos, nSize);
			} else
				return NULL;
		} else
			strncpy(szDest, szFileName, nMaxSize);
		return szDest;
	}
	return NULL;
}

char *CSystemUtils::ExtractFileExtName(const char *szFileName, char *szExt, int nMaxSize)
{
	if (szFileName)
	{
		int nPos = (int)strlen(szFileName);
		while(nPos > 0)
		{
			if (szFileName[nPos - 1] == '\\')
			{
				nPos = (int)strlen(szFileName);
				break;
			} else if(szFileName[nPos -1] == '.')
			{
				break;
			}
			nPos --;
		}
		if (nPos > 0)
		{
			int nSize = (int)strlen(szFileName) - nPos;
			if (nSize > 0)
			{
				if (nSize > nMaxSize)
					nSize = nMaxSize;
				strncpy(szExt, szFileName + nPos, nSize);
				return szExt;
			}
		}
	}
	return NULL;
}

//去除文件路径分界符"/"
char * CSystemUtils::DeletePathDelimiter(const char *szPath, char *szDest, int nMaxPath)
{
	if (szPath)
	{
		int nCpyLen = (int)::strlen(szPath);
		if (szPath[nCpyLen - 1] == '\\')
			nCpyLen --;
		if (nCpyLen > nMaxPath)
			nCpyLen = nMaxPath;
		strncpy(szDest, szPath, nCpyLen);
		return szDest;
	}
	return NULL;
}

char * CSystemUtils::IncludePathDelimiter(const char *szPath, char *szDest, int nMaxPath)
{
	if (szPath)
	{
		strncpy(szDest, szPath, nMaxPath);
		if ((int)strlen(szDest) < nMaxPath)
		{
			if (szDest[strlen(szDest) - 1] != '\\')
				strcat(szDest, "\\");
			return szDest;
		} else
			return NULL;
	} 
	return NULL;
}


BOOL  CSystemUtils::GetWindowsVersion(char *szWinVer, int &nWinVer, char *szMajorBuilder)
{
	if (!szWinVer  || !szMajorBuilder)
		return FALSE;
	strcpy(szWinVer, "Unknown Windows Version");
	nWinVer = 0;

	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osinfo))
		return FALSE;

	DWORD dwPlatformId   = osinfo.dwPlatformId;
	DWORD dwMinorVersion = osinfo.dwMinorVersion;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;
	DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;	// Win 95 needs this

	sprintf(szMajorBuilder, "%u.%u.%u", dwMajorVersion, dwMinorVersion, dwBuildNumber);

	if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
	{
		if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
		{
			strcpy(szWinVer, "Windows 95");
			nWinVer = 1;
		}
		else if ((dwMinorVersion < 10) && 
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
		{
			strcpy(szWinVer, "Windows 95 SP1");
			nWinVer = 2;
		}
		else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
		{
			strcpy(szWinVer, "Windows 95 OSR2");
			nWinVer = 3;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
		{
			strcpy(szWinVer, "Windows 98");
			nWinVer = 4;
		}
		else if ((dwMinorVersion == 10) && 
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
		{
			strcpy(szWinVer, "Windows 98 SP1");
			nWinVer = 5;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
		{
			strcpy(szWinVer, "Windows 98 SE");
			nWinVer = 6;
		}
		else if (dwMinorVersion == 90)
		{
			strcpy(szWinVer, "Windows Me");
			nWinVer = 7;
		}
	} else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
		{
			strcpy(szWinVer, "Windows NT 3.51");
			nWinVer = 101;
		}
		else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
		{
			strcpy(szWinVer, "Windows NT 4.0");
			nWinVer = 102;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
		{
			strcpy(szWinVer, "Windows 2000");
			nWinVer = 103;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
		{
			strcpy(szWinVer, "Windows XP");
			nWinVer = 104;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
		{
			strcpy(szWinVer, "Windows 2003 Server");
			nWinVer = 105;
		}
	}	else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
	{
		strcpy(szWinVer, "Windows CE");
		nWinVer = 201;
	}
	return TRUE;
}

//合并成一个flag
DWORD CSystemUtils::MakeCustomFlag(DWORD dwFileId, BYTE byteLink)
{
	DWORD dwFlag = (dwFileId << 8) & 0xFFFFFF00;
	dwFlag |= byteLink;
    return dwFlag;
}

//分解一个flag
void  CSystemUtils::ParserCustomFlag(DWORD dwFlag, DWORD &dwFileId, BYTE &byteLink)
{
	dwFileId = (dwFlag >> 8) & 0x00FFFFFF;
	byteLink = (BYTE)(dwFlag & 0x000000FF);
}

char * ParserCommandLine(char *p, std::string &strParam)
{
	int i, len;
	char *pStart, *pQ;
	strParam = "";
	while (true)
	{
		while ((*p) && ((*p) <= ' '))
			p = ::CharNextA(p);
		if (((*p) == '"') && ((*(p + 1) == '"')))
			p += 2;
		else
			break;
	}
	len = 0;
	pStart = p;
	while ((*p) > ' ')
	{
		if ((*p) == '"')
		{
			p = ::CharNextA(p);
			while((*p) && ((*p) != '"'))
			{
				pQ = ::CharNextA(p);
				len = len + (int) (pQ - p);
				p = pQ;
			}
			if ((*p))
				p = ::CharNextA(p);
		} else
		{
			pQ = ::CharNextA(p);
			len = len + (int) (pQ - p);
			p = pQ;
		}
	}
	p = pStart;
    i = 0;
	while ((*p) > ' ')
	{
		if ((*p) == '"')
		{
			p = ::CharNextA(p);
			while ((*p) && ((*p) != '"'))
			{
				pQ = ::CharNextA(p);
				while (p < pQ)
				{
					strParam += (*p);
					p ++;
				}
			}
			if ((*p))
				p = ::CharNextA(p);
		} else
		{
			pQ = ::CharNextA(p);
			while (p < pQ)
			{
				strParam += (*p);
				p ++;
			}
		}
	}
	return p;
}

//获取命令行参数个数
int CSystemUtils::GetParamCount()
{
	char szParams[1024] = {0};
	std::string strParam;
	CStringConversion::WideCharToString(::GetCommandLine(), szParams, 1023);
	char *p = ParserCommandLine(szParams, strParam);
	int nCount = 0;
    while (true)
	{
		p = ParserCommandLine(p, strParam);
		if (strParam == "")
			break;
		nCount ++;
	}
	return nCount;
}

//获取命令行参数
std::string CSystemUtils::GetParamStr(int idx)
{
	std::string strParam;
	if (idx == 0)
	{
		char szAppName[MAX_PATH] = {0};
		GetApplicationFileName(szAppName, MAX_PATH - 1);
		strParam = szAppName;
	} else
	{
		char szParams[1024] = {0};
		CStringConversion::WideCharToString(::GetCommandLine(), szParams, 1023);
		char *p = szParams;
		int nCount = 0;
		while (true)
		{
			p = ParserCommandLine(p, strParam);
			if ((idx == 0) || (strParam == ""))
				break;
			idx --;
		}
	}
	return strParam;
}

//屏幕保护程序枚举回调函数
BOOL CALLBACK KillScreenSaverFunc(HWND hWnd, LPARAM lParam)
{
	::PostMessage(hWnd, WM_CLOSE, 0, 0);
	return TRUE;
}

//杀掉屏幕保护程序
BOOL CSystemUtils::KillScreenSaver()
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!::GetVersionEx(&os))
		return FALSE;
	switch(os.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_WINDOWS:
			{
				HWND hWnd = ::FindWindow(L"WindowsScreenSaverClass", NULL);
				if (hWnd != NULL)
					::PostMessage(hWnd, WM_CLOSE, 0, 0);
				return TRUE;
			}
		case VER_PLATFORM_WIN32_NT:
			{
				HDESK hDesk = ::OpenDesktop(L"Screen-saver", 0, FALSE, DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS);
				if (hDesk != NULL)
				{
					::EnumDesktopWindows(hDesk, (WNDENUMPROC) &KillScreenSaverFunc, 0);
					::CloseDesktop(hDesk);
					//等待足够的时间让保护程序关闭
					Sleep(2000);
					//重置保护程序
					::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE);
					return TRUE;
				} // end if (hDesk...
			} // end case ver_platform
	} //end switch(...
	return FALSE;
 }

//设置剪切板的文本数据
BOOL CSystemUtils::SetClipboardText(HWND hOwner, const char *szText)
{
	::OpenClipboard(hOwner);
	::EmptyClipboard();
	size_t nTextSize = ::strlen(szText);
	HGLOBAL hMem = ::GlobalAlloc(GMEM_SHARE, nTextSize + 1);
	::GlobalLock(hMem);
	strncpy((char *)hMem, szText, nTextSize);
	::GlobalUnlock(hMem);
	BOOL b = ::SetClipboardData(CF_TEXT, hMem) == hMem;
	::CloseClipboard();
	return b;
}

//执行一个操作系统外部程序
BOOL CSystemUtils::ExecuteCommand(const char *szCommand, const char *szParam)
{
	if ((!szCommand) || (::strlen(szCommand) == 0))
		return FALSE;
#ifdef _UNICODE
	TCHAR szwCommand[MAX_PATH] = {0};
	TCHAR szwParam[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szCommand, szwCommand, MAX_PATH - 1);
	if (szParam)
	{
		CStringConversion::StringToWideChar(szParam, szwParam, MAX_PATH - 1);
	}
	::ShellExecute(NULL, L"OPEN", szwCommand, szwParam, NULL, SW_SHOW);
#else
	::ShellExecute(NULL, "OPEN", szCommand, szParam, NULL, SW_SHOW);
#endif
	return TRUE;
}

BOOL CSystemUtils::DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BOOL bAlphaChannel, BYTE uFade, 
        bool hole, bool xtiled, bool ytiled)
{
	if (!hBitmap)
		return FALSE;
	HDC hCloneDC = ::CreateCompatibleDC(hDC);
    HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
    ::SetStretchBltMode(hDC, COLORONCOLOR);

    RECT rcTemp = {0};
    RECT rcDest = {0};
    if (lpAlphaBlend && (bAlphaChannel || uFade < 255))
	{
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
        // middle
        if (!hole)
		{
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if (::IntersectRect(&rcTemp, &rcPaint, &rcDest))
			{
                if (!xtiled && !ytiled)
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, 
                        rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, 
                        rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
                } else if ( xtiled && ytiled ) 
				{
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                    int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for ( int j = 0; j < iTimesY; ++j ) 
					{
                        LONG lDestTop = rcDest.top + lHeight * j;
                        LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                        LONG lDrawHeight = lHeight;
                        if ( lDestBottom > rcDest.bottom )
						{
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        for ( int i = 0; i < iTimesX; ++i ) 
						{
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if ( lDestRight > rcDest.right ) 
							{
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            lpAlphaBlend(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, 
                                lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, 
                                rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, lDrawWidth, lDrawHeight, bf);
                        }
                    }
                } else if ( xtiled )
				{
                    LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                    int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                    for ( int i = 0; i < iTimes; ++i ) 
					{
                        LONG lDestLeft = rcDest.left + lWidth * i;
                        LONG lDestRight = rcDest.left + lWidth * (i + 1);
                        LONG lDrawWidth = lWidth;
                        if ( lDestRight > rcDest.right ) 
						{
                            lDrawWidth -= lDestRight - rcDest.right;
                            lDestRight = rcDest.right;
                        }
                        lpAlphaBlend(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom, 
                            hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                            lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
                    }
                } else 
				{ // ytiled
                    LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                    int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                    for ( int i = 0; i < iTimes; ++i ) 
					{
                        LONG lDestTop = rcDest.top + lHeight * i;
                        LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                        LONG lDrawHeight = lHeight;
                        if ( lDestBottom > rcDest.bottom ) 
						{
                            lDrawHeight -= lDestBottom - rcDest.bottom;
                            lDestBottom = rcDest.bottom;
                        }
                        lpAlphaBlend(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop, 
                            hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
                            rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, bf);                    
                    }  //end for (int i 
                } //end else
            } //end
        }

        // left-top
        if ( rcCorners.left > 0 && rcCorners.top > 0 ) 
		{
            rcDest.left = rc.left;
            rcDest.top = rc.top;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) )
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
                    rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, bf);
            }
        }
        // top
        if ( rcCorners.top > 0 ) 
		{
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.top;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - 
                    rcCorners.left - rcCorners.right, rcCorners.top, bf);
            } //
        }
        // right-top
        if ( rcCorners.right > 0 && rcCorners.top > 0 ) 
		{
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.top;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rcCorners.top;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, bf);
            }
        }
        // left 
        if ( rcCorners.left > 0 ) 
		{
            rcDest.left = rc.left;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - 
                    rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
            }
        }
        // right
        if ( rcCorners.right > 0 ) 
		{
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.top + rcCorners.top;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) )
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, 
                    rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
            }
        }
        // left-bottom
        if ( rcCorners.left > 0 && rcCorners.bottom > 0 ) 
		{
            rcDest.left = rc.left;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rcCorners.left;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, bf);
            }
        }
        // bottom
        if ( rcCorners.bottom > 0 )
		{
            rcDest.left = rc.left + rcCorners.left;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, 
                    rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, bf);
            }
        }
        // right-bottom
        if ( rcCorners.right > 0 && rcCorners.bottom > 0 ) 
		{
            rcDest.left = rc.right - rcCorners.right;
            rcDest.top = rc.bottom - rcCorners.bottom;
            rcDest.right = rcCorners.right;
            rcDest.bottom = rcCorners.bottom;
            rcDest.right += rcDest.left;
            rcDest.bottom += rcDest.top;
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
			{
                rcDest.right -= rcDest.left;
                rcDest.bottom -= rcDest.top;
                lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                    rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, 
                    rcCorners.bottom, bf);
            }
        }
    }  else 
    {
        if (rc.right - rc.left == rcBmpPart.right - rcBmpPart.left 
            && rc.bottom - rc.top == rcBmpPart.bottom - rcBmpPart.top 
            && rcCorners.left == 0 && rcCorners.right == 0 && rcCorners.top == 0 && rcCorners.bottom == 0)
        {
            if ( ::IntersectRect(&rcTemp, &rcPaint, &rc) ) 
			{
                ::BitBlt(hDC, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, 
                    hCloneDC, rcBmpPart.left + rcTemp.left - rc.left, rcBmpPart.top + rcTemp.top - rc.top, SRCCOPY);
            }
        }  else
        {
            // middle
            if ( !hole ) 
			{
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    if ( !xtiled && !ytiled ) 
					{
                        rcDest.right -= rcDest.left;
                        rcDest.bottom -= rcDest.top;
                        ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                            rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, 
                            rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, 
                            rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                    } else if ( xtiled && ytiled )
					{
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                        int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for ( int j = 0; j < iTimesY; ++j ) 
						{
                            LONG lDestTop = rcDest.top + lHeight * j;
                            LONG lDestBottom = rcDest.top + lHeight * (j + 1);
                            LONG lDrawHeight = lHeight;
                            if( lDestBottom > rcDest.bottom )
							{
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            for ( int i = 0; i < iTimesX; ++i ) 
							{
                                LONG lDestLeft = rcDest.left + lWidth * i;
                                LONG lDestRight = rcDest.left + lWidth * (i + 1);
                                LONG lDrawWidth = lWidth;
                                if ( lDestRight > rcDest.right )
								{
                                    lDrawWidth -= lDestRight - rcDest.right;
                                    lDestRight = rcDest.right;
                                }
                                ::BitBlt(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, 
                                    lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, 
                                    rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, SRCCOPY);
                            }
                        }
                    } else if ( xtiled ) 
					{
                        LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
                        int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
                        for ( int i = 0; i < iTimes; ++i ) 
						{
                            LONG lDestLeft = rcDest.left + lWidth * i;
                            LONG lDestRight = rcDest.left + lWidth * (i + 1);
                            LONG lDrawWidth = lWidth;
                            if ( lDestRight > rcDest.right ) 
							{
                                lDrawWidth -= lDestRight - rcDest.right;
                                lDestRight = rcDest.right;
                            }
                            ::StretchBlt(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom, 
                                hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, 
                                lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                        }
                    }  else
					{ 
						// ytiled
                        LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
                        int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
                        for ( int i = 0; i < iTimes; ++i ) 
						{
                            LONG lDestTop = rcDest.top + lHeight * i;
                            LONG lDestBottom = rcDest.top + lHeight * (i + 1);
                            LONG lDrawHeight = lHeight;
                            if ( lDestBottom > rcDest.bottom ) 
							{
                                lDrawHeight -= lDestBottom - rcDest.bottom;
                                lDestBottom = rcDest.bottom;
                            }
                            ::StretchBlt(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop, 
                                hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, 
                                rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, SRCCOPY);                    
                        }
                    }
                }
            }
            
            // left-top
            if ( rcCorners.left > 0 && rcCorners.top > 0 ) 
			{
                rcDest.left = rc.left;
                rcDest.top = rc.top;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, SRCCOPY);
                }
            }
            // top
            if ( rcCorners.top > 0 )
			{
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.top;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - 
                        rcCorners.left - rcCorners.right, rcCorners.top, SRCCOPY);
                }
            }
            // right-top
            if ( rcCorners.right > 0 && rcCorners.top > 0 )
			{
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.top;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rcCorners.top;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, SRCCOPY);
                }
            }
            // left
            if ( rcCorners.left > 0 ) 
			{
                rcDest.left = rc.left;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - 
                        rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                }
            }
            // right
            if ( rcCorners.right > 0 ) 
			{
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.top + rcCorners.top;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, 
                        rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
                }
            }
            // left-bottom
            if ( rcCorners.left > 0 && rcCorners.bottom > 0 ) 
			{
                rcDest.left = rc.left;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rcCorners.left;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, SRCCOPY);
                }
            }
            // bottom
            if ( rcCorners.bottom > 0 ) 
			{
                rcDest.left = rc.left + rcCorners.left;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, 
                        rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, SRCCOPY);
                }
            }

            // right-bottom
            if ( rcCorners.right > 0 && rcCorners.bottom > 0 ) 
			{
                rcDest.left = rc.right - rcCorners.right;
                rcDest.top = rc.bottom - rcCorners.bottom;
                rcDest.right = rcCorners.right;
                rcDest.bottom = rcCorners.bottom;
                rcDest.right += rcDest.left;
                rcDest.bottom += rcDest.top;
                if ( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) 
				{
                    rcDest.right -= rcDest.left;
                    rcDest.bottom -= rcDest.top;
                    ::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, 
                        rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, 
                        rcCorners.bottom, SRCCOPY);
                }
            }
        }
    }

    ::SelectObject(hCloneDC, hOldBitmap);
    ::DeleteDC(hCloneDC);
	return TRUE;
}

//区域图像变暗
BOOL CSystemUtils::AreaGray(HDC hDC, const RECT *prc)
{
	/*static BYTE pBits[] = {
		                   0x55, 0x0, 0xAA, 0x0, 
		                   0x55, 0x0, 0xAA, 0x0, 
					       0x55, 0x0, 0xAA, 0x0, 
				           0x55, 0x0, 0xAA, 0x0
	};

	static HBITMAP hBitmap = ::CreateBitmap(8, 8, 1, 1, pBits);
    HBRUSH hBrush = ::CreatePatternBrush(hBitmap);
	::SelectObject(hDC, hBrush);
	//0xA000C9
	::PatBlt(hDC, prc->left, prc->top, prc->right, prc->bottom, 0xA000C9);
	::DeleteObject(hBrush);*/
	COLORREF clr;
	int y;
	for (int i = prc->left; i < prc->right; i ++)
	{
		for (int j = prc->top; j < prc->bottom; j ++)
		{
			clr = ::GetPixel(hDC, i, j);
			y = (9798   *   GetRValue(clr)   +   19235   *   GetGValue(clr)   +   3735   *   GetBValue(clr)) / 32768;
			::SetPixel(hDC, i, j, RGB(y, y, y));
		}
	}
	return TRUE;
}

//获取GUID字符串
BOOL CSystemUtils::GetGuidString(char *strGuid, int *nSize)
{
	GUID guid = {0};
	HRESULT hr = ::CoCreateGuid(&guid);
	if (SUCCEEDED(hr) && nSize && (*nSize > 36))
	{
		sprintf(strGuid, "{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
			guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
		return TRUE;
	}
	return FALSE;
}

BOOL CSystemUtils::RectToString(const RECT &rc, std::string &strRect)
{
	char szTmp[128] = {0};
	sprintf(szTmp, "%d %d %d %d", rc.left, rc.top, rc.right, rc.bottom);
	strRect = szTmp;
	return TRUE;
}

BOOL CSystemUtils::StringToRect(RECT *prc, const char *strRect)
{
	if (!prc)
		return FALSE;
	char *p = new char[strlen(strRect) + 1];
	strcpy(p, strRect);
	p[strlen(strRect)] = '\0';
	char *p1 = p;
	do  
	{
		prc->left = ::strtol(p1, &p1, 10);
		if (p == NULL)
			break;
		prc->top = ::strtol(p1 + 1, &p1, 10);
        if (p == NULL)
			break;
		prc->right = ::strtol(p1 + 1, &p1, 10); 
		if (p == NULL)
			break;
		prc->bottom = ::strtol(p1 + 1, &p1, 10); 
		delete []p;
		return TRUE;
	} while (FALSE);
	delete []p;
	return FALSE;
}

//获取屏幕大小
BOOL CSystemUtils::GetScreenRect(RECT *lprc)
{
	if (lprc)
	{
		lprc->right = ::GetSystemMetrics(SM_CXSCREEN);
		lprc->bottom = ::GetSystemMetrics(SM_CYSCREEN);
		lprc->left = 0;
		lprc->top = 0;
		return TRUE;
	}
	return FALSE;
}

BOOL CSystemUtils::IsMobileNumber(const char *szNumber)
{
	if (szNumber && (::strlen(szNumber) == 11) && (szNumber[0] == '1')) //mobile phone number is 11
	{
		for (int i = 1; i < 11; i ++)
		{
			if (szNumber[i] < '0' || szNumber[i] > '9')
				return FALSE;
		}
		return TRUE;
	} 
	return FALSE;
}

BOOL CSystemUtils::GetScreenCenterRect(int nWidth, int nHeight, RECT &rc)
{
	RECT rcScreen = {0};  
	if (GetScreenRect(&rcScreen))
	{
		rc.left = (rcScreen.right - rcScreen.left - nWidth) / 2;
		rc.top = (rcScreen.bottom - rcScreen.top - nHeight) / 2;
		rc.right = rc.left + nWidth;
		rc.bottom = rc.top + nHeight;
		return TRUE;
	} else
		return FALSE;
}
BOOL CALLBACK EnumUserWindowsCB(HWND hWnd, LPARAM lParam)
{
	LONG lFlags = ::GetWindowLong(hWnd, GWL_STYLE);
	if ((lFlags & WS_VISIBLE) == 0)
		return TRUE;
 
	HWND hSndWnd = ::FindWindowEx(hWnd, NULL, L"SHELLDLL_DefView", NULL);
	if (hSndWnd == NULL)
		return TRUE;
	HWND hTargetWnd = ::FindWindowEx(hSndWnd, NULL, L"SysListView32", L"FolderView");
	if (hTargetWnd == NULL)
		return TRUE;
	HWND *h = (HWND *)lParam;
	*h = hTargetWnd;
	return FALSE;
}


HWND CSystemUtils::FindDesktopWindow()
{ 
	HWND hWnd = NULL;
	EnumWindows(EnumUserWindowsCB, (LPARAM) &hWnd);
	return hWnd; 
}

BOOL CSystemUtils::Show2ToolBar(HWND hWnd, BOOL bShow)
{
	ITaskbarList *pTaskbarList;

    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,  
            IID_ITaskbarList, ( void** )&pTaskbarList );
     
	if (SUCCEEDED(hr))
	{
		pTaskbarList->HrInit(); 

	    if (bShow)
	    {
	        hr = pTaskbarList->AddTab(hWnd);
	    } else
	    {
	        hr = pTaskbarList->DeleteTab(hWnd);
	    } 
	    pTaskbarList->Release( ); 
		return SUCCEEDED(hr);
	} 
	return FALSE;
}
#pragma warning(default:4996)
