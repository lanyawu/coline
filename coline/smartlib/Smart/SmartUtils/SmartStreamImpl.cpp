#include "StdAfx.h"
#include "SmartStreamImpl.h"

CSmartStreamImpl::CSmartStreamImpl(void)
{
}

CSmartStreamImpl::~CSmartStreamImpl(void)
{
}

STDMETHODIMP CSmartStreamImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ISmartStream)))
	{
		*ppv = (ISmartStream *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, IID_IStream))
	{
		*ppv = (IStream *)this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, IID_ISequentialStream))
	{
		*ppv = (ISequentialStream *)this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

//
STDMETHODIMP CSmartStreamImpl::Read(void *pv, ULONG cb,  ULONG *pcbRead)
{
	*pcbRead = m_Stream.Read(pv,cb);
	return S_OK;
}


STDMETHODIMP CSmartStreamImpl::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	*pcbWritten = m_Stream.Write(pv, cb);
	return S_OK;
}

STDMETHODIMP CSmartStreamImpl::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	plibNewPosition->QuadPart = m_Stream.Seek(dlibMove.QuadPart, (SEEK_ORIGIN)dwOrigin);
	return S_OK;
}


STDMETHODIMP CSmartStreamImpl::SetSize(ULARGE_INTEGER libNewSize)
{
	m_Stream.SetSize(libNewSize.QuadPart);
	return S_OK;
}

STDMETHODIMP CSmartStreamImpl::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	ULONG uWritten = 0;
	if (cb.QuadPart < (ULONGLONG)m_Stream.GetSize())
		uWritten = (ULONG) cb.QuadPart;
	else
		uWritten = (ULONG) m_Stream.GetSize();
	ULONG lWritten = 0;
	pcbWritten->QuadPart = uWritten;
	HRESULT hr = pstm->Write(m_Stream.GetMemory(), uWritten, &lWritten);
	return hr;
}

STDMETHODIMP CSmartStreamImpl::Commit(DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSmartStreamImpl::Revert(void)
{
	return E_NOTIMPL;
}


STDMETHODIMP CSmartStreamImpl::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSmartStreamImpl::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSmartStreamImpl::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSmartStreamImpl::Clone(IStream **ppstm)
{
	return E_NOTIMPL;
}
