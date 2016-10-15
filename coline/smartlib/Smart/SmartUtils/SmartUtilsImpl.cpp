#include "StdAfx.h"
#include "SmartUtilsImpl.h"

#include "SmartStreamImpl.h"

CSmartUtilsImpl::CSmartUtilsImpl(void)
{
}

CSmartUtilsImpl::~CSmartUtilsImpl(void)
{
}

STDMETHODIMP CSmartUtilsImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (IsEqualIID(riid, __uuidof(ISmartStream)))
	{
		*ppv = (ISmartStream *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, IID_IStream))
	{
		*ppv = (IStream *)this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}