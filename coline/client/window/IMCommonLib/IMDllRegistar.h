#include <Registrar.h>
#include "../include/Core/CoreInterface.h"
#include "../include/Core/common.h"

#pragma warning(disable:4996)

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
			if (CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "guid", strCLSID)
				&& CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "desc", szDesc)
				&& CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "path", szPath))
			{
				char strCLSMain[MAX_PATH] = {0};
				if (StrFromCLSID(riidMain, strCLSMain, MAX_PATH - 1))
					CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szBuffer, "interface", strCLSMain);
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

#pragma warning(default:4996)
