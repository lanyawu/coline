#ifndef __IMAGEDATAOBJECT_H__
#define __IMAGEDATAOBJECT_H__

#include <atlbase.h>
#include <atlcom.h>
#include <CommonLib/GdiPlusImage.h>

class COMMONLIB_API CImageDataObject: public IDataObject,
	                    public IImageNotifyEvent
{
public:
	CImageDataObject(void);
	~CImageDataObject(void);
private:
	ULONG	m_ulRefCnt;
	BOOL	m_bRelease;
	STGMEDIUM m_stgmed;
	FORMATETC m_format;

	//image
	CGdiPlusGif *m_pImage;
public:
		
	// Methods of the IUnknown interface
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		if (iid == IID_IUnknown || iid == IID_IDataObject)
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		else
			return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)(void)
	{
		m_ulRefCnt++;
		return m_ulRefCnt;
	}

	STDMETHOD_(ULONG, Release)(void)
	{
		if (--m_ulRefCnt == 0)
		{
			delete this;
		}

		return m_ulRefCnt;
	}

	// Methods of the IDataObject Interface
	STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) 
	{
		HANDLE hDst;
		hDst = ::OleDuplicateData(m_stgmed.hBitmap, CF_BITMAP, NULL);
		if (hDst == NULL)
		{
			return E_HANDLE;
		}

		pmedium->tymed = TYMED_GDI;
		pmedium->hBitmap = (HBITMAP)hDst;
		pmedium->pUnkForRelease = NULL;

		return S_OK;
	}

	STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM*  pmedium ) 
	{
		return E_NOTIMPL;
	}

	STDMETHOD(QueryGetData)(FORMATETC*  pformatetc ) {
		return E_NOTIMPL;
	}

	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  pformatectIn ,FORMATETC* pformatetcOut ) 	
	{
		return E_NOTIMPL;
	}

	STDMETHOD(SetData)(FORMATETC* pformatetc , STGMEDIUM*  pmedium , BOOL  fRelease ) 
	{
		m_format = *pformatetc;
		m_stgmed = *pmedium;
		return S_OK;
	}

	STDMETHOD(EnumFormatEtc)(DWORD  dwDirection , IEnumFORMATETC**  ppenumFormatEtc ) 
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
		DWORD *pdwConnection) 
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DUnadvise)(DWORD dwConnection) 
	{
		return E_NOTIMPL;
	}

	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) 
	{
		return E_NOTIMPL;
	}

	// Some Other helper functions
	void SetBitmap(HBITMAP hBitmap);
	void SetImageFileName(char *szFileName);
	IOleObject *GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage);
	//
	virtual void OnInvalidate(LPRECT lprc);
};

#endif