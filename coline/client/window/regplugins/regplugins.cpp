// regplugins.cpp : Defines the entry point for the console application.
//

#include "stdafx.h" 
#include "../include/Core/CoreInterface.h"
#include "../include/Core/common.h"
#include <Shlobj.h>

#define REGISTER_START_RUN_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REGISTER_START_RUN_NAME "CoLine"

#pragma warning(disable:4996)

int StringToWideChar(const char *szSrc, TCHAR *lpstrDest, int iMaxLen)
{
	if (szSrc && lpstrDest)
		return MultiByteToWideChar(::GetACP(), 0, szSrc, -1, lpstrDest, iMaxLen);
	return FALSE;
}

int WideCharToString(const TCHAR *lpstrSrc, char *szDesc, int iMaxLen)
{
	if (lpstrSrc && szDesc)
		return  WideCharToMultiByte(::GetACP(), 0, lpstrSrc, -1, szDesc, iMaxLen, NULL, NULL);
	return FALSE;
}

BOOL  WriteRegisterKey(HKEY nKey, const char *szPath, const char *szItem, const char *szKey, DWORD dwRegType = REG_SZ)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp;
	WCHAR szTemp[MAX_PATH] = {0};
	StringToWideChar(szPath, szTemp, MAX_PATH);
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
		StringToWideChar(szItem, szSubKey, MAX_PATH);
		switch(dwRegType)
		{
		case REG_SZ:
			{
				TCHAR szwKey[MAX_PATH] = {0};
				StringToWideChar(szKey, szwKey, MAX_PATH - 1);
				
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

//检测文件是否存在
BOOL FileIsExists(const char *szFileName)
{
	int nCode;
#ifdef _UNICODE
	WCHAR szTemp[MAX_PATH] = {0};
	StringToWideChar(szFileName, szTemp, MAX_PATH);
    nCode = ::GetFileAttributesW(szTemp);
#else
	nCode = ::GetFileAttributesA(szFileName);
#endif
	return ((nCode != -1) && ((FILE_ATTRIBUTE_DIRECTORY & nCode) == 0));
}
class CRegistrar
{
protected:
	static BOOL DelFromRegistry(HKEY hRootKey, const char *szSubKey)
	{
		long retCode;
		TCHAR szwSubKey[MAX_PATH] = {0};
		StringToWideChar(szSubKey, szwSubKey, MAX_PATH - 1);
		retCode = RegDeleteKey(hRootKey, szwSubKey);
		if (retCode != ERROR_SUCCESS)
			return FALSE;
		return TRUE;
	}

	static BOOL StrFromCLSID(REFIID riid, char *strCLSID, int nMaxSize)
	{
		LPOLESTR pOleStr = NULL;
		HRESULT hr = ::StringFromCLSID(riid, &pOleStr);
		if(FAILED(hr))
			return FALSE;
		WideCharToString(pOleStr, strCLSID, nMaxSize); 
		return TRUE;
	}
public:
	static BOOL RegisterObject(REFIID riid, const char *szLibId, const char *szClassId)
	{
		char strCLSID[MAX_PATH] = {0};
		char szBuffer[MAX_PATH] = {0};
		
		if (!szClassId)
			return FALSE;

		if (!StrFromCLSID(riid, strCLSID, MAX_PATH - 1))
			return FALSE;
		
		if (!szLibId && (strlen(szClassId) != 0))
			sprintf(szBuffer,"%s.%s\\CLSID",szClassId, szClassId);
		else
			sprintf(szBuffer,"%s.%s\\CLSID",szLibId, szClassId);

		BOOL bResult;
		bResult = WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer, "", strCLSID);
		if (!bResult)
			return FALSE;
		sprintf(szBuffer, "CLSID\\%s", strCLSID);
		char szClass[MAX_PATH] = {0};
		sprintf(szClass,"%s Class", szClassId);
		if (!WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer, "", szClass))
			return FALSE;
		sprintf(szClass, "%s.%s", szLibId, szClassId);
		strcat(szBuffer, "\\ProgId");

		return WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer, "", szClass) ? TRUE : FALSE;
	}

	static BOOL UnRegisterObject(REFIID riid, const char *szLibId, const char *szClassId)
	{
		char strCLSID[MAX_PATH] = {0};
		char szBuffer[MAX_PATH] = {0};
		if (!StrFromCLSID(riid, strCLSID, MAX_PATH - 1))
			return FALSE;

		sprintf(szBuffer,"%s.%s\\CLSID", szLibId, szClassId);
		if( !DelFromRegistry(HKEY_CLASSES_ROOT, szBuffer))
			return FALSE;
		sprintf(szBuffer,"%s.%s", szLibId, szClassId);
		if (!DelFromRegistry(HKEY_CLASSES_ROOT, szBuffer))
			return FALSE;
		sprintf(szBuffer,"CLSID\\%s\\ProgId",strCLSID);
		if (!DelFromRegistry(HKEY_CLASSES_ROOT, szBuffer))
			return FALSE;
		sprintf(szBuffer,"CLSID\\%s", strCLSID);
		return DelFromRegistry(HKEY_CLASSES_ROOT, szBuffer) ? TRUE : FALSE;
	}
};

void RegisterUrlProtocol(const char *szUrlProtoName, const char *szAppName)
{
	char szPath[MAX_PATH] = {0};
	char szKey[MAX_PATH] = {0};
	//注册url
	{
		WriteRegisterKey(HKEY_CLASSES_ROOT, szUrlProtoName, "URL Protocol", "", 1);
	}
	//注册open
	{
		memset(szPath, 0, MAX_PATH);
		memset(szKey, 0, MAX_PATH);
		strcpy(szPath, szUrlProtoName);
		strcat(szPath, "\\shell\\open\\command");
		strcpy(szKey, "\"");
		strcat(szKey, szAppName);
		strcat(szKey, "\" \"%1\""); 
		WriteRegisterKey(HKEY_CLASSES_ROOT, szPath, "", szKey, 1);
	}
	//注册icon
	{
		memset(szPath, 0, MAX_PATH);
		strcpy(szPath, szUrlProtoName);
		strcat(szPath, "\\DefaultIcon");
		WriteRegisterKey(HKEY_CLASSES_ROOT, szPath, "", szAppName, 1);
		//
	}
}

class CDllRegistrar : public CRegistrar
{
public:
	static BOOL RegisterObject(REFIID riid, const char *szLibId, const char *szClassId, const char *szPath)
	{
		if (!CRegistrar::RegisterObject(riid, szLibId, szClassId))
			return FALSE;

		char strCLSID[MAX_PATH] = {0};
		char szBuffer[MAX_PATH] = {0};
		if (!StrFromCLSID(riid, strCLSID, MAX_PATH - 1))
			return FALSE;
		sprintf(szBuffer, "CLSID\\%s\\InProcServer32", strCLSID);
		return WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer,"", szPath);
	}

	static BOOL UnRegisterObject(REFIID riid, const char *szLibId, const char *szClassId)
	{
		char strCLSID[MAX_PATH] = {0};
		char szBuffer[MAX_PATH] = {0};
		if (!StrFromCLSID(riid, strCLSID,  MAX_PATH - 1))
			return FALSE;
		sprintf(szBuffer, "CLSID\\%s\\InProcServer32", strCLSID);
		if (!DelFromRegistry(HKEY_CLASSES_ROOT, szBuffer))
			return FALSE;
		return CRegistrar::UnRegisterObject(riid, szLibId, szClassId);
	}
};
// {9A609B0F-2C86-47D5-8EA8-1E0BF343EAE9}
_declspec(selectany) GUID CLSID_CORE_SMSFRAME  =  { 0x9a609b0f, 0x2c86, 0x47d5, 
                                                  { 0x8e, 0xa8, 0x1e, 0xb, 0xf3, 0x43, 0xea, 0xe9 } };
// {6F459384-19B0-4613-AE3A-3F0127DCBD46}
_declspec(selectany) GUID CLSID_CORE_BCFRAME  = { 0x6f459384, 0x19b0, 0x4613, 
                                                { 0xae, 0x3a, 0x3f, 0x1, 0x27, 0xdc, 0xbd, 0x46 } };
//
_declspec(selectany) GUID CLSID_CORE_CHATFRAME  =  { 0x2d0175dd, 0xc90d, 0x4b73, 
                                                   { 0x94, 0x15, 0x8c, 0x8b, 0xbb, 0x33, 0xd8, 0x95 } };
//
_declspec(selectany) GUID CLSID_CONFIGUREUI  =   { 0xb06dae77, 0x677b, 0x4420, 
                                                 { 0xa1, 0x5, 0xec, 0x62, 0x9f, 0x7, 0x51, 0x94 } };
//
// {1BECC9E4-16DD-4FDC-A0D4-957FD9E30ECB}
_declspec(selectany) GUID CLSID_CORE_ECONTACTPANEL  =  { 0x1becc9e4, 0x16dd, 0x4fdc, 
                                                       { 0xa0, 0xd4, 0x95, 0x7f, 0xd9, 0xe3, 0xe, 0xcb } };

//
_declspec(selectany) GUID CLSID_CORE_ECONTACTS  =  { 0x964acce2, 0x86e2, 0x4c2c, 
                                                   { 0x82, 0x33, 0x6e, 0x29, 0x44, 0x79, 0x92, 0x2a } };

 // {A1B9A290-BF57-4120-94CC-8CA18428D9EA}
_declspec(selectany) GUID CLSID_CORE_EMOTIONFRAME  =  { 0xa1b9a290, 0xbf57, 0x4120, 
                                                      { 0x94, 0xcc, 0x8c, 0xa1, 0x84, 0x28, 0xd9, 0xea } };
//
// {6E7DD74F-183F-40DA-83D4-38520FE17E6C}
_declspec(selectany) GUID CLSID_CORE_FRECONTACTS  =  { 0x6e7dd74f, 0x183f, 0x40da, 
                                                     { 0x83, 0xd4, 0x38, 0x52, 0xf, 0xe1, 0x7e, 0x6c } };
// {998FC37B-02F1-4A18-8FA7-ECA5736D3478}
_declspec(selectany) GUID CLSID_CORE_GROUPFRAME  =  { 0x998fc37b, 0x2f1, 0x4a18, 
                                                    { 0x8f, 0xa7, 0xec, 0xa5, 0x73, 0x6d, 0x34, 0x78 } };
//
_declspec(selectany) GUID CLSID_CORE_LOGIN = { 0x91490d0b, 0xd736, 0x4dd7, 
                                              { 0xbd, 0x14, 0xa3, 0x8a, 0xa5, 0x83, 0x7e, 0x92 } };
//
_declspec(selectany) GUID CLSID_CORE_MAINFRAME = { 0x89703d4c, 0x69f1, 0x43ba, 
                                                 { 0x94, 0xc3, 0x2c, 0x62, 0xd2, 0x45, 0xae, 0x24 } };
//
_declspec(selectany) GUID CLSID_CORE_MINICARD  =  { 0x55509b47, 0xa915, 0x45c5, 
                                                  { 0x82, 0x8f, 0x99, 0x25, 0x7d, 0xd9, 0xe9, 0xe0 } };
//
_declspec(selectany) GUID CLSID_CORE_MSGMGR  =  { 0x4cc95f8a, 0x4587, 0x4716, 
                                                { 0x8e, 0x56, 0x55, 0xcb, 0xc5, 0xa2, 0xd, 0xe3 } };
//
_declspec(selectany) GUID CLSID_CORE_MSGMGRUI  =  { 0x982cabba, 0xe52b, 0x481f,
                                                  { 0xae, 0xd0, 0x50, 0xa8, 0x92, 0x3c, 0x8f, 0x14 } };

//
// {1BB77D09-3647-40A5-9DB8-F4C7AE35BC21}
_declspec(selectany) GUID CLSID_EXTERNAL_OATIP  = { 0x1bb77d09, 0x3647, 0x40a5, 
                                                  { 0x9d, 0xb8, 0xf4, 0xc7, 0xae, 0x35, 0xbc, 0x21 } };
//
_declspec(selectany) GUID CLSID_CORE_TRAYMSG  =  { 0x846e9325, 0x2c01, 0x46f2, 
                                                 { 0xba, 0x39, 0x82, 0x2c, 0x3f, 0xd6, 0xfd, 0x13 } };
//
_declspec(selectany) GUID CLSID_CORE_UIMANAGER = { 0x42555360, 0x5ac8, 0x4be3, 
                                                 { 0x9a, 0x36, 0x1, 0xa3, 0xb5, 0x8a, 0x9, 0x73 } };
//
_declspec(selectany) GUID CLSID_EXTERNAL_HWCALL  = { 0x7a575da8, 0xc2f0, 0x48a5, 
                                                   { 0xa4, 0xfc, 0xcc, 0x72, 0x24, 0xd4, 0x27, 0x52 } };
//
_declspec(selectany) GUID CLSID_HWCALL_INTERFACE = {0x8D9ECEAE, 0xE914, 0x4C1B,
                                                     {0xA4, 0x39, 0xCA, 0xA1, 0xA2, 0x40, 0x89, 0x11}}; 
_declspec(selectany) GUID CLSID_CORE_GPLUS  =  { 0xcd9645d, 0x2ae1, 0x4fc5, 
                                               { 0xb8, 0x28, 0xa6, 0x79, 0xda, 0x9e, 0x2e, 0xb3 } };
class CPluginRegistar :public CDllRegistrar
{
public:
	static BOOL RegisterPlugin(REFIID riid, const char *szLibId, const char *szClassId, const char *szPath, 
		          const char *szDesc, int nType, REFIID riidMain)
	{
		if (CDllRegistrar::RegisterObject(riid, szLibId, szClassId, szPath))
		{
			char strCLSID[MAX_PATH] = {0};
			char szBuffer[MAX_PATH] = {0};
			if (!StrFromCLSID(riid, strCLSID, MAX_PATH - 1))
				return FALSE;
			if (nType > MAX_PLUGIN_TYPE_ID)
				nType = 0;
			sprintf(szBuffer, "%s\\%s\\%s", PLUGIN_REGISTER_DIR, PLUGIN_TYPE_NAMES[nType], szLibId);
			if (WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "guid", strCLSID)
				&& WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "desc", szDesc)
				&& WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "path", szPath))
			{
				char strCLSMain[MAX_PATH] = {0};
				if (StrFromCLSID(riidMain, strCLSMain, MAX_PATH - 1))
					WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "interface", strCLSMain);
				return TRUE;
			}
		}
		return FALSE;
	}

	static BOOL UnRegisterPlugin(REFIID riid, const char *szLibId, const char *szClassId, int nType)
	{
		if (CDllRegistrar::UnRegisterObject(riid, szLibId, szClassId))
		{
			char szBuffer[MAX_PATH] = {0};
			if (nType > MAX_PLUGIN_TYPE_ID)
				nType = 0;
			sprintf(szBuffer, "%s\\%s\\%s",  PLUGIN_REGISTER_DIR, PLUGIN_TYPE_NAMES[nType], szLibId);
			if (DelFromRegistry(HKEY_LOCAL_MACHINE, szBuffer))
				return TRUE;
		}
		return FALSE;
	}
};

char *  GetModuleAppFileName(HMODULE hModule, char *szFileName, int nMaxSize)
{
#ifdef _UNICODE
	WCHAR szTemp[MAX_PATH] = {0};
	::GetModuleFileNameW(hModule, szTemp, MAX_PATH);
	WideCharToString(szTemp, szFileName, nMaxSize - 1);
    return szFileName;
#else
	::GetModuleFileNameA(hModule, szFileName, nMaxSize);
	return szFileName;
#endif
}

#define OS_VERSION_OLD     0
#define OS_VERSION_WIN2000 1
#define OS_VERSION_WINXP   2
#define OS_VERSION_WINXP64 3
#define OS_VERSION_WIN2003 4
#define OS_VERSION_VISTA   5
#define OS_VERSION_VISTA64 6
#define OS_VERSION_WIN7    7
#define OS_VERSION_WIN764  8

//获取操作系统版本
DWORD  GetOSVersion()
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
	   else if ((os.dwMajorVersion == 5) && (os.dwMinorVersion == 0))
		   return OS_VERSION_WIN2000;
	   else if ((os.dwMajorVersion == 5) && (os.dwMinorVersion == 1))
		   return OS_VERSION_WINXP;
	   else if ((os.dwMajorVersion == 5) && (os.dwMinorVersion == 2))
	   {
		   if ((os.wProductType == VER_NT_WORKSTATION) &&
			   ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
			   (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)))
			   return OS_VERSION_WINXP64;
		   else
			   return OS_VERSION_WIN2003;
	   }  else if ((os.dwMajorVersion == 6) && (os.dwMinorVersion == 0))
	   {
		   if ((os.wProductType == VER_NT_WORKSTATION) &&
			   ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
			   (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)))
			   return OS_VERSION_VISTA64;
		   else
			   return OS_VERSION_VISTA;	   
	   } else  if ((os.dwMajorVersion >= 6) && (os.dwMinorVersion == 1))
	   {
		   if ((os.wProductType == VER_NT_WORKSTATION) &&
			   ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
			   (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)))
			   return OS_VERSION_WIN764;
		   else
			   return OS_VERSION_WIN7;
	   }
   }
   return OS_VERSION_OLD;
}


char * ExtractFilePath(const char *szFileName, char *szPath, int nMaxPath)
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
};

//删除注册表某个键值
BOOL DeleteRegisterKey(HKEY nKey, const char *szPath, const char *szItem)
{
	BOOL bSucc = FALSE;
#ifdef _UNICODE
	HKEY nTemp;
	WCHAR szTemp[MAX_PATH] = {0};
	StringToWideChar(szPath, szTemp, MAX_PATH);
	if (::RegOpenKeyExW(nKey, szTemp, 0, KEY_WRITE, &nTemp) == ERROR_SUCCESS)
	{
		WCHAR szSubKey[MAX_PATH] = {0};
		DWORD nRegType = REG_SZ;
		StringToWideChar(szItem, szSubKey, MAX_PATH);
		if (::RegDeleteValue(nTemp, szSubKey) == ERROR_SUCCESS) 
		{
			bSucc = TRUE;
		}
		RegCloseKey(nTemp);
	}
#endif
	return bSucc;
}

BOOL StartShellProcessor(const char *szAppName, const char *szParams, const char *szWorkPath, BOOL bWait)
{ 
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO); 
	TCHAR szwAppName[MAX_PATH]  = {0};
	StringToWideChar(szAppName, szwAppName, MAX_PATH - 1);
	TCHAR *szwParams = NULL;
	BOOL bSucc = FALSE;
	if (szParams)
	{
		int nParamSize = ::strlen(szParams);
		szwParams = new TCHAR[nParamSize + 1];
		memset(szwParams, 0, sizeof(TCHAR) * (nParamSize + 1));
		StringToWideChar(szParams, szwParams, nParamSize);
	}
	
	TCHAR szwWorkPath[MAX_PATH] = {0};
	if (szWorkPath == NULL)
	{
		char szAppPath[MAX_PATH] = {0};
		ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
		StringToWideChar(szAppPath, szwWorkPath, MAX_PATH - 1);
	} else 
	{
		StringToWideChar(szWorkPath, szwWorkPath, MAX_PATH - 1);
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

typedef HRESULT (CALLBACK *DLL_REGISTER)();
int _tmain(int argc, _TCHAR* argv[])
{
	BOOL bUnInstall = FALSE;
	if (argc > 1)
	{
		if (_tcsicmp(argv[1], L"/u") == 0)
			bUnInstall = TRUE;
	}
	// 这个类应该创建标准的注册表入口 
	char szPath[MAX_PATH] = {0};
	char szAppName[MAX_PATH] = {0};
	GetModuleAppFileName(NULL, szAppName, MAX_PATH - 1); 
	ExtractFilePath(szAppName, szPath, MAX_PATH - 1);
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\BCFrame.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_BCFRAME, "BCFrameLib", "BCFrameObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_BCFRAME, "BCFrameLib", "BCFrameObj", szAppName,
			                                  "广播插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IBCFrame));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\ChatFrame.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_CHATFRAME, "ChatFrameLib", "ChatFrameObj",
			                    PLUGIN_TYPE_CHATFRAME);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_CHATFRAME, "ChatFrameLib", "ChatFrameObj", szAppName,
			                                  "聊天窗口插件", PLUGIN_TYPE_CHATFRAME, __uuidof(IChatFrame));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\ConfigureUI.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CONFIGUREUI, "ConfigureUILib", "ConfigureUIObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CONFIGUREUI, "ConfigureUILib", "ConfigureUIObj", szAppName,
			                                  "配置界面插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IConfigureUI));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\ContactPanel.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_ECONTACTPANEL, "ContactPanelLib", "ContactPanelObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_ECONTACTPANEL, "ContactPanelLib", "ContactPanelObj", szAppName,
			                                  "联系人选择插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IContactPanel));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\CoreFrameWork.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CDllRegistrar::UnRegisterObject(CLSID_CORE_FRAMEWORK, "CoreFrameWorkLib", "CoreFrameWorkObj"); 
		else
			CDllRegistrar::RegisterObject(CLSID_CORE_FRAMEWORK, "CoreFrameWorkLib", "CoreFrameWorkObj", szAppName);
	}

	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\eContacts.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_ECONTACTS, "EContactsLib", "EContactsObj",
			                    PLUGIN_TYPE_CONTACTS);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_ECONTACTS, "EContactsLib", "EContactsObj", szAppName,
			                                  "企业联系人插件", PLUGIN_TYPE_CONTACTS, __uuidof(IContacts));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\EmotionFrame.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_EMOTIONFRAME, "EmotionFrameLib", "EmotionFrameObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_EMOTIONFRAME, "EmotionFrameLib", "EmotionFrameObj", szAppName,
			                                  "表情管理插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IEmotionFrame));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\FrameWorkCfg.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_FRAMEWORKCFG, "FrameWorkCFGLib", "FrameWorkCFGObj",
			                    PLUGIN_TYPE_CONFIGURE);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_FRAMEWORKCFG, "FrameWorkCFGLib", "FrameWorkCFGObj", szAppName,
			                                  "配置插件", PLUGIN_TYPE_CONFIGURE, __uuidof(IConfigure));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\FreContacts.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_FRECONTACTS, "FreContactsLib", "FreContactsObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_FRECONTACTS, "FreContactsLib", "FreContactsObj", szAppName,
			                                  "常用联系人插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IFreContacts));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\GroupFrame.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_GROUPFRAME, "GroupFrameLib", "GroupFrameObj",
			                    PLUGIN_TYPE_GROUPFRAME);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_GROUPFRAME, "GroupFrameLib", "GroupFrameObj", szAppName,
			                                  "讨论组插件", PLUGIN_TYPE_GROUPFRAME, __uuidof(IGroupFrame));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\Login.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_LOGIN, "LoginLib", "LoginObj",
			                    PLUGIN_TYPE_LOGINFRAME);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_LOGIN, "LoginLib", "LoginObj", szAppName,
			                                  "登陆插件", PLUGIN_TYPE_LOGINFRAME, __uuidof(ICoreLogin));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\MainFrame.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_MAINFRAME, "MainFrmeLib", "MainFrameObj",
			                    PLUGIN_TYPE_MAINFRAME);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_MAINFRAME, "MainFrmeLib", "MainFrameObj", szAppName,
			                                  "主窗体插件", PLUGIN_TYPE_MAINFRAME, __uuidof(IMainFrame));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\MiniCard.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_MINICARD, "MiniCardLib", "MiniCardObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_MINICARD, "MiniCardLib", "MiniCardObj", szAppName,
			                                  "用户资料信息卡片", PLUGIN_TYPE_EXTERNAL, __uuidof(IMiniCard));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\MsgMgr.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_MSGMGR, "MsgMgrLib", "MsgMgrObj",
			                    PLUGIN_TYPE_MSGMGR);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_MSGMGR, "MsgMgrLib", "MsgMgrObj", szAppName,
			                                  "消息管理插件", PLUGIN_TYPE_MSGMGR, __uuidof(IMsgMgr));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\MsgMgrUI.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_MSGMGRUI, "MsgMgrUILib", "MsgMgrUIObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_MSGMGRUI, "MsgMgrUILib", "MsgMgrUIObj", szAppName,
			                                  "消息管理展现插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IMsgMgrUI));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\OATip.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_EXTERNAL_OATIP, "OATipLib", "OATipObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_EXTERNAL_OATIP, "OATipLib", "OATipObj", szAppName,
			                                  "OA消息提醒插件", PLUGIN_TYPE_EXTERNAL, __uuidof(IOATip));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\SMSFrame.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_SMSFRAME, "SMSFrameLib", "SMSFrameObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_SMSFRAME, "SMSFrameLib", "SMSFrameObj", szAppName,
			                                  "短信发送插件", PLUGIN_TYPE_EXTERNAL, __uuidof(ISMSFrame));
	}

	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\UIManager.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_UIMANAGER, "UIManagerLib", "UIManagerObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_UIMANAGER, "UIManagerLib", "UIManagerObj", szAppName,
			                                  "界面管理插件", PLUGIN_TYPE_UIMANAGER, __uuidof(IUIManager));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\TrayMsg.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_TRAYMSG, "TrayMsgLib", "TrayMsgObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_TRAYMSG, "TrayMsgLib", "TrayMsgObj", szAppName,
			                                  "任务栏消息插件", PLUGIN_TYPE_TRAYMSG, __uuidof(ITrayMsg));
	}
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\HW_Call.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_EXTERNAL_HWCALL, "HWCallLib", "HWCallObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_EXTERNAL_HWCALL, "HWCallLib", "HWCallObj", szAppName,
			                                  "任务栏消息插件", PLUGIN_TYPE_EXTERNAL, CLSID_HWCALL_INTERFACE);
	}
	
	//desktop 插件
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "plugins\\GPlus.plg");
	if (FileIsExists(szAppName))
	{
		if (bUnInstall)
			CPluginRegistar::UnRegisterPlugin(CLSID_CORE_GPLUS, "GPlusLib", "GPlusObj",
			                    PLUGIN_TYPE_EXTERNAL);
		else
			CPluginRegistar::RegisterPlugin(CLSID_CORE_GPLUS, "GPlusLib", "GPlusObj", szAppName,
			                                  "CoLine Plus插件", PLUGIN_TYPE_EXTERNAL,  __uuidof(IGPlus));
	}
	int nVer = GetOSVersion();
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "driver\\");
	//安装显卡驱动
	if (bUnInstall)
	{
		switch(nVer)
		{
			case OS_VERSION_OLD:
				 strcat(szAppName, "xp\\uninstall_silent.bat");
				 break;
			case OS_VERSION_WIN2000:
				 strcat(szAppName, "w2K\\uninstall_silent.bat");
				 break;
			case OS_VERSION_WINXP:
				 strcat(szAppName, "xp\\uninstall_silent.bat");
				 break;
			case OS_VERSION_WINXP64:
				 strcat(szAppName, "xp64\\uninstall_silent.bat");
				 break;
			case OS_VERSION_WIN2003:
				 strcat(szAppName, "w2K\\uninstall_silent.bat");
				 break;
			case OS_VERSION_VISTA:
				 strcat(szAppName, "vista\\uninstall_silent.bat");
				 break;
			case OS_VERSION_VISTA64:
				 strcat(szAppName, "vista64\\uninstall_silent.bat");
				 break;
			case OS_VERSION_WIN7:
				 strcat(szAppName, "vista\\uninstall_silent.bat");
				 break;
			case OS_VERSION_WIN764:
				 strcat(szAppName, "vista64\\uninstall_silent.bat");
				 break;
		}
		
	} else
	{
		switch(nVer)
		{
			case OS_VERSION_OLD:
				 strcat(szAppName, "xp\\install_silent.bat");
				 break;
			case OS_VERSION_WIN2000:
				 strcat(szAppName, "w2K\\install_silent.bat");
				 break;
			case OS_VERSION_WINXP:
				 strcat(szAppName, "xp\\install_silent.bat");
				 break;
			case OS_VERSION_WINXP64:
				 strcat(szAppName, "xp64\\install_silent.bat");
				 break;
			case OS_VERSION_WIN2003:
				 strcat(szAppName, "w2K\\install_silent.bat");
				 break;
			case OS_VERSION_VISTA:
				 strcat(szAppName, "vista\\install_silent.bat");
				 break;
			case OS_VERSION_VISTA64:
				 strcat(szAppName, "vista64\\install_silent.bat");
				 break;
			case OS_VERSION_WIN7:
				 strcat(szAppName, "vista\\install_silent.bat");
				 break;
			case OS_VERSION_WIN764:
				 strcat(szAppName, "vista64\\install_silent.bat");
				 break;
		}
	}
	if (FileIsExists(szAppName))
	{
		StartShellProcessor(szAppName, NULL, NULL, TRUE);
	}
	
	//注册ole控件
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "baseolectrl.dll");  
	if (FileIsExists(szAppName))
	{   
		TCHAR szwTmp[MAX_PATH] = {0};
		StringToWideChar(szAppName, szwTmp, MAX_PATH - 1);
		HMODULE h = ::LoadLibrary(szwTmp);
		if (h != NULL)
		{
			DLL_REGISTER pReg = (DLL_REGISTER)::GetProcAddress(h, "DllRegisterServer");
			if (pReg)
			{
				printf("正在注册组件，请勿关闭此窗口");
				if (SUCCEEDED(pReg()))
				{
					printf("");
				} else
				{
					printf("");
				} //end else if (SUCCEEDED(
			} //end if (pReg)
			::FreeLibrary(h);
		} //end if (h != NULL)
		 
	}  //end if (FileIsExists
	

	::CoInitialize(NULL);
	//删除老版本自启动项
	TCHAR szwStartupDir[MAX_PATH] = {0};
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, SHGFP_TYPE_DEFAULT, szwStartupDir)))
	{
		lstrcat(szwStartupDir, L"\\d9org.lnk");
		::DeleteFile(szwStartupDir);
	}
	//桌面快捷方式 单个用户
	WCHAR szwDesktopPath[MAX_PATH] = {0};
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_DEFAULT, szwDesktopPath)))
	{
		HRESULT hres;
		IShellLink *psl = NULL;
		IPersistFile *pPf = NULL; 
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                           IID_IShellLink, (LPVOID*)&psl);
		TCHAR szLinkName[MAX_PATH] = {0};
		lstrcpy(szLinkName, szwDesktopPath);

		lstrcat(szLinkName, L"\\GoCom融合信息平台.lnk");
		if (SUCCEEDED(hres))
		{
			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&pPf);
			if (SUCCEEDED(hres))
			{
				BOOL bSucc = TRUE;
				if (FAILED(pPf->Load(szLinkName, STGM_READWRITE)))
				{
					memset(szLinkName, 0, sizeof(TCHAR) * MAX_PATH);
					lstrcpy(szLinkName, szwDesktopPath);
					lstrcat(szLinkName, L"\\GoCom统一通讯平台.lnk");
					bSucc = SUCCEEDED(pPf->Load(szLinkName, STGM_READWRITE));
				}
				if (bSucc)
				{
					memset(szAppName, 0, MAX_PATH);
					strcpy(szAppName, szPath);
					strcat(szAppName, "CoLine.exe");
					TCHAR szwTmp[MAX_PATH] = {0};
					StringToWideChar(szAppName, szwTmp, MAX_PATH - 1);
					psl->SetPath(szwTmp); 
					psl->SetIconLocation(szwTmp, 0);
					memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
					StringToWideChar(szPath, szwTmp, MAX_PATH - 1);
					psl->SetWorkingDirectory(szwTmp);
					pPf->Save(szLinkName, TRUE);
				}
				pPf->Release();
			}
			psl->Release();
		} //end if (SUCCEEDED(
	}
	//所有用户的快捷方式
	memset(szwDesktopPath, 0, sizeof(TCHAR) * MAX_PATH);
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_DEFAULT, szwDesktopPath)))
	{
		HRESULT hres;
		IShellLink *psl = NULL;
		IPersistFile *pPf = NULL; 
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                           IID_IShellLink, (LPVOID*)&psl);
		TCHAR szLinkName[MAX_PATH] = {0};
		lstrcpy(szLinkName, szwDesktopPath);

		lstrcat(szLinkName, L"\\GoCom融合信息平台.lnk");
		if (SUCCEEDED(hres))
		{
			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&pPf);
			if (SUCCEEDED(hres))
			{ 
				BOOL bSucc = TRUE;
				if (FAILED(pPf->Load(szLinkName, STGM_READWRITE)))
				{
					memset(szLinkName, 0, sizeof(TCHAR) * MAX_PATH);
					lstrcpy(szLinkName, szwDesktopPath);
					lstrcat(szLinkName, L"\\GoCom统一通讯平台.lnk");
					bSucc = SUCCEEDED(pPf->Load(szLinkName, STGM_READWRITE));
				}
				if (bSucc)
				{
					memset(szAppName, 0, MAX_PATH);
					strcpy(szAppName, szPath);
					strcat(szAppName, "CoLine.exe");
					TCHAR szwTmp[MAX_PATH] = {0};
					StringToWideChar(szAppName, szwTmp, MAX_PATH - 1);
					psl->SetPath(szwTmp); 
					psl->SetIconLocation(szwTmp, 0);
					memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
					StringToWideChar(szPath, szwTmp, MAX_PATH - 1);
					psl->SetWorkingDirectory(szwTmp);
					pPf->Save(szLinkName, TRUE);
				}
				pPf->Release();
			}
			psl->Release();
		} //end if (SUCCEEDED(
	}
	//
	memset(szAppName, 0, MAX_PATH);
	strcpy(szAppName, szPath);
	strcat(szAppName, "CoLine.exe");
	//注册URL协议
	RegisterUrlProtocol("CoLine", szAppName);
	if (bUnInstall)
		DeleteRegisterKey(HKEY_CURRENT_USER, REGISTER_START_RUN_KEY, REGISTER_START_RUN_NAME);
	else
		WriteRegisterKey(HKEY_CURRENT_USER, REGISTER_START_RUN_KEY, REGISTER_START_RUN_NAME, szAppName);
	::CoUninitialize();
	return 0;
}

#pragma warning(default:4996)
