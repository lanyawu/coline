#ifndef __SMARTSTREAMIMPL_H___
#define __SMARTSTREAMIMPL_H___

#include <ComBase.h>
#include <SmartUtils/SmartUtilsInterface.h>
#include <commonlib/classes.h>

class CSmartStreamImpl :public CComBase<>, public InterfaceImpl<ISmartStream> 
{
public:
	CSmartStreamImpl(void);
	virtual ~CSmartStreamImpl(void);
public:
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
	//
	STDMETHOD (Read)( 
		/* [length_is][size_is][out] */ void *pv,
		/* [in] */ ULONG cb,
		/* [out] */ ULONG *pcbRead);

	STDMETHOD (Write)( 
		/* [size_is][in] */ const void *pv,
		/* [in] */ ULONG cb,
		/* [out] */ ULONG *pcbWritten);
	STDMETHOD (Seek)( 
		/* [in] */ LARGE_INTEGER dlibMove,
		/* [in] */ DWORD dwOrigin,
		/* [out] */ ULARGE_INTEGER *plibNewPosition);

	STDMETHOD (SetSize)( 
		/* [in] */ ULARGE_INTEGER libNewSize);

	STDMETHOD (CopyTo)( 
		/* [unique][in] */ IStream *pstm,
		/* [in] */ ULARGE_INTEGER cb,
		/* [out] */ ULARGE_INTEGER *pcbRead,
		/* [out] */ ULARGE_INTEGER *pcbWritten);

	STDMETHOD (Commit)(
		/* [in] */ DWORD grfCommitFlags);

	STDMETHOD (Revert)( void);

	STDMETHOD (LockRegion) ( 
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType);

	STDMETHOD (UnlockRegion)( 
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType);

	STDMETHOD (Stat)( 
		/* [out] */ STATSTG *pstatstg,
		/* [in] */ DWORD grfStatFlag);

	STDMETHOD (Clone)( 
		/* [out] */ IStream **ppstm);
private:
	CMemoryStream m_Stream;
};

#endif
