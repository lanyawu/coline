
#if !defined(_REGISTRAR_H)
#define _REGISTRAR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <stdio.h>
#include <commonlib/stringutils.h>
#include <commonlib/systemutils.h>
#pragma warning(disable:4996)

class CRegistrar
{
protected:
	static BOOL DelFromRegistry(HKEY hRootKey, const char *szSubKey)
	{
		long retCode;
		TCHAR szwSubKey[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szSubKey, szwSubKey, MAX_PATH - 1);
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
		CStringConversion::WideCharToString(pOleStr, strCLSID, nMaxSize); 
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
		bResult = CSystemUtils::WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer, "", strCLSID);
		if (!bResult)
			return FALSE;
		sprintf(szBuffer, "CLSID\\%s", strCLSID);
		char szClass[MAX_PATH] = {0};
		sprintf(szClass,"%s Class", szClassId);
		if (!CSystemUtils::WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer, "", szClass))
			return FALSE;
		sprintf(szClass, "%s.%s", szLibId, szClassId);
		strcat(szBuffer, "\\ProgId");

		return CSystemUtils::WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer, "", szClass) ? TRUE : FALSE;
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
		return CSystemUtils::WriteRegisterKey(HKEY_CLASSES_ROOT, szBuffer,"", szPath);
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

#pragma warning(default:4996)
#endif