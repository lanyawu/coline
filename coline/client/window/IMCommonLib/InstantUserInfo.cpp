#include "InstantUserInfo.h"

#pragma warning(disable:4996)

CInstantUserInfo::CInstantUserInfo(void):
                  m_nOrderRef(0)
{
}


CInstantUserInfo::~CInstantUserInfo(void)
{
}

void CInstantUserInfo::Clear()
{
	m_strStatus.Empty();
	m_Params.clear();
}

//IUnknow
STDMETHODIMP CInstantUserInfo::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IInstantUserInfo)))
	{
		*ppv = (IInstantUserInfo *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}


//IInstantUserInfo
//SET
STDMETHODIMP CInstantUserInfo::SetUserInfo(const char *szParam, const char *szValue)
{
	if (szParam)
	{
		if (szValue)
			m_Params[szParam] = szValue;
		else
			m_Params[szParam] = "";
		return S_OK;
	}
	return E_FAIL;
}

//GET
STDMETHODIMP CInstantUserInfo::GetUserInfo(const char *szParam, IAnsiString *szValue)
{
	if (!szValue)
		return E_POINTER;
	if (szParam)
	{
		std::map<CAnsiString_, std::string>::iterator it = m_Params.find(szParam);
		if (it != m_Params.end())
		{
			szValue->SetString(it->second.c_str());  
			return S_OK;
		} //end if (it != m_Params.end())
	} //end if (szParam)
	return E_FAIL;
}

//
BOOL CInstantUserInfo::AssignTo(IInstantUserInfo *pInfo)
{
	std::map<CAnsiString_, std::string>::iterator it;
	for (it = m_Params.begin(); it != m_Params.end(); it ++)
	{
		pInfo->SetUserInfo(it->first.c_str(), it->second.c_str());		
	}
	pInfo->SetUserStatus(m_strStatus.c_str());
	return TRUE;
}

STDMETHODIMP CInstantUserInfo::SetUserStatus(const char *szStatus)
{
	if (szStatus)
	{
		if (m_strStatus.IsEmpty() || (m_strStatus != szStatus))
		{
			m_strStatus = szStatus;
			return S_OK;
		}
	} else
	{
		if (m_strStatus.IsEmpty() || (m_strStatus != ""))
		{
			m_strStatus = "";
			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CInstantUserInfo::GetUserStatus(IAnsiString *szStatus)
{
	if (!szStatus)
		return E_POINTER;
	if ((m_strStatus.GetLength() > 0) && (m_strStatus != " "))
	{
		szStatus->SetString(m_strStatus.c_str());
		return S_OK;
	}
	return E_FAIL;
}

LONG CInstantUserInfo::AddOrderRef()
{
	return ::InterlockedIncrement(&m_nOrderRef);
}

LONG CInstantUserInfo::ReleaseOrderRef()
{
	return ::InterlockedDecrement(&m_nOrderRef);
}

LONG CInstantUserInfo::GetOrderRef()
{
	return m_nOrderRef;
}


#pragma warning(default:4996)
