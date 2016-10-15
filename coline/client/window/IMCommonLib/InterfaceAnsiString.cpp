#include "InterfaceAnsiString.h"

#pragma warning(disable:4996)

CInterfaceAnsiString::CInterfaceAnsiString(void)
{

}


CInterfaceAnsiString::~CInterfaceAnsiString(void)
{

}

//IUnknown
STDMETHODIMP CInterfaceAnsiString::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IAnsiString)))
	{
		*ppv = (IAnsiString *) this;
		_AddRef();
		return S_OK;
	} 
	return E_NOINTERFACE;
}

//IAnsiString
STDMETHODIMP CInterfaceAnsiString::SetString(const char *strInput) 
{
	if (strInput)
		m_strAnsi = strInput;
	else
		m_strAnsi = "";
	return S_OK;
}

STDMETHODIMP CInterfaceAnsiString::AppendString(const char *strAppend)
{
	if (strAppend)
	{
		m_strAnsi += strAppend;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CInterfaceAnsiString::GetString(char *szOutput, int *nSize)
{
	if (!nSize)
		return E_POINTER;
	if (*nSize >= (int) m_strAnsi.size())
	{
		*nSize = (int) m_strAnsi.size();
		strncpy(szOutput, m_strAnsi.c_str(), *nSize);
		return S_OK;
	} else
	{
		*nSize = (int) m_strAnsi.size();
		return S_FALSE;
	}
}

STDMETHODIMP_(int) CInterfaceAnsiString::GetSize()
{ 
	return(int) m_strAnsi.size();
}

const char *CInterfaceAnsiString::GetData()
{
	return m_strAnsi.c_str();
}

#pragma warning(default:4996)
