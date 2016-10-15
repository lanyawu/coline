#include <CommonLib/ImageDataObject.h>

CImageDataObject::CImageDataObject(void):
                  m_ulRefCnt(0),
				  m_pImage(NULL),
				  m_bRelease(FALSE)
{
}

CImageDataObject::~CImageDataObject(void)
{
	if (m_bRelease)
		::ReleaseStgMedium(&m_stgmed);
}

void CImageDataObject::SetBitmap(HBITMAP hBitmap)
{

	STGMEDIUM stgm;
	stgm.tymed = TYMED_GDI;					// Storage medium = HBITMAP handle		
	stgm.hBitmap = hBitmap;
	stgm.pUnkForRelease = NULL;				// Use ReleaseStgMedium

	FORMATETC fm;
	fm.cfFormat = CF_BITMAP;				// Clipboard format = CF_BITMAP
	fm.ptd = NULL;							// Target Device = Screen
	fm.dwAspect = DVASPECT_CONTENT;			// Level of detail = Full content
	fm.lindex = -1;							// Index = Not applicaple
	fm.tymed = TYMED_GDI;					// Storage medium = HBITMAP handle

	SetData(&fm, &stgm, TRUE);		
}

IOleObject *CImageDataObject::GetOleObject(IOleClientSite *pOleClientSite, IStorage *pStorage)
{
	SCODE sc;
	IOleObject *pOleObject;
	sc = ::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT, 
		          &m_format, pOleClientSite, pStorage, (void **)&pOleObject);
	if (sc != S_OK)
		return NULL;
	return pOleObject;
}

void CImageDataObject::OnInvalidate(LPRECT lprc)
{

}

void CImageDataObject::SetImageFileName(char *szFileName)
{
	
}

