#include "StdAfx.h"
#include "SvrInterface.h"
#include "SvrInterfaceImpl.h"

CTestSubSvrInterfaceImpl::CTestSubSvrInterfaceImpl()
{
	//
}

CTestSubSvrInterfaceImpl::~CTestSubSvrInterfaceImpl()
{
	//
}

	 
STDMETHODIMP CTestSubSvrInterfaceImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid,IID_IUnknown) || IsEqualIID(riid,__uuidof(ITestSubSvrInterface)))
	{
		*ppv = (ITestSvrInterface *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CTestSubSvrInterfaceImpl::GetNameById(int nId, char *szName)
{
	std::map<int, std::string>::iterator it = m_Names.find(nId);
	if (it != m_Names.end())
	{
		strcpy(szName, it->second.c_str());
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CTestSubSvrInterfaceImpl::SetNameById (int nId, char *szName)
{
	if (szName)
		m_Names[nId] = szName;
	else
		m_Names.erase(nId);
	return S_OK;
}


CTestSvrInterfaceImpl::CTestSvrInterfaceImpl(void)
{
	//
}

CTestSvrInterfaceImpl::~CTestSvrInterfaceImpl(void)
{
	//
}


STDMETHODIMP CTestSvrInterfaceImpl::QueryInterface(REFIID riid,LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid,IID_IUnknown) || IsEqualIID(riid,__uuidof(ITestSvrInterface)))
	{
		*ppv = (ITestSvrInterface *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(ITestSubSvrInterface)))
	{
		*ppv = (ITestSubSvrInterface *)&m_SubSvr;
		m_SubSvr.AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CTestSvrInterfaceImpl::Square(long *pVal)
{
	long value = *pVal;
	*pVal = value * value;
	return S_OK;
}

STDMETHODIMP CTestSvrInterfaceImpl::Cube(long *pVal)
{
	long value = *pVal;
	*pVal = value * value * value;
	return S_OK;
}
