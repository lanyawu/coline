// GifAnimate.h : CGifAnimate 的声明

#pragma once
#include "resource.h"       // 主符号
#include <atlctl.h>
#include "BaseOleLib.h"
#include <CommonLib/GdiplusImage.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif



// CGifAnimate

class ATL_NO_VTABLE CGifAnimate :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CGifAnimate, &CLSID_GifAnimate>,
	public CComControl<CGifAnimate>,
	public CStockPropImpl<CGifAnimate, IGifAnimate, &IID_IGifAnimate, &LIBID_BaseOleLibLib>,
	public IProvideClassInfo2Impl<&IID_IGifAnimate, NULL, &LIBID_BaseOleLibLib>,
	public IPersistStreamInitImpl<CGifAnimate>,
	public IPersistStorageImpl<CGifAnimate>,
	public IQuickActivateImpl<CGifAnimate>,
	public IOleControlImpl<CGifAnimate>,
	public IOleObjectImpl<CGifAnimate>,
	public IConnectionPointContainerImpl<CGifAnimate>,
	public IOleInPlaceActiveObjectImpl<CGifAnimate>,
	public IViewObjectExImpl<CGifAnimate>,
	public ISpecifyPropertyPagesImpl<CGifAnimate>,
	public IOleInPlaceObjectWindowlessImpl<CGifAnimate>,
	public IImageNotifyEvent,
	public IDataObjectImpl<CGifAnimate>
{
public:


DECLARE_REGISTRY_RESOURCEID(IDR_GIFANIMATE)


BEGIN_COM_MAP(CGifAnimate)
	COM_INTERFACE_ENTRY(IGifAnimate)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY_IMPL(IOleControl)
	COM_INTERFACE_ENTRY_IMPL(IOleObject)
	COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
	COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
	COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
	COM_INTERFACE_ENTRY_IMPL(IDataObject)
	COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CGifAnimate)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)

//	PROP_PAGE(CLSID_About)
END_PROPERTY_MAP()

BEGIN_MSG_MAP(CGifAnimate)
	MESSAGE_HANDLER(WM_CREATE, OnCreate);
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	MESSAGE_HANDLER(WM_DESTROY, OnClose)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
END_MSG_MAP()

BEGIN_CONNECTION_POINT_MAP(CGifAnimate)
    //连接点
	//CONNECTION_POINT_ENTRY(IID_IGifAnimate)
END_CONNECTION_POINT_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		//m_spAdviseSink = NULL;
		memset(&m_rcPos, 0, sizeof(RECT));
		m_hPaintWnd = NULL;
		memset(m_szFileName, 0, MAX_PATH);
		m_pImage = NULL;
		CGdiPlusImage::InitGdiPlus();
		return S_OK;
	}

	void FinalRelease()
	{
		m_spAdviseSink = NULL;
		if (m_pImage)
		{
			delete m_pImage;
			m_pImage = NULL;
		}		
		CGdiPlusImage::DestroyGdiPlus();
	}

public:
	CGifAnimate();
	~CGifAnimate();
	//IGifAnimate
	virtual HRESULT STDMETHODCALLTYPE Play();
	virtual HRESULT STDMETHODCALLTYPE LoadFile(BSTR newVal, ULONG hWnd);
	virtual HRESULT STDMETHODCALLTYPE LoadBitmap(ULONG hBitmap, ULONG hParent);
	//改写虚函数
	HRESULT STDMETHODCALLTYPE SetObjectRects(/* [in] */ LPCRECT lprcPosRect, /* [in] */ LPCRECT lprcClipRect);
	HRESULT STDMETHODCALLTYPE GetRect(/* [in] */ DWORD dwAspect, /* [out] */ __RPC__out LPRECTL pRect);
	//虚函数
    HRESULT OnDraw(ATL_DRAWINFO& di);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	HRESULT IPersistStreamInit_Save(LPSTREAM pStm, BOOL /* fClearDirty */, const ATL_PROPMAP_ENTRY* pMap);
	HRESULT IPersistStreamInit_Load(LPSTREAM pStm, const ATL_PROPMAP_ENTRY* pMap);
    HRESULT InPlaceActivate(LONG iVerb, const RECT* prcPosRect);
	HRESULT OnPosRectChange(LPCRECT lpcRect);
	HRESULT IOleObject_Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection);
	STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
	STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
	STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
	STDMETHOD(CanInPlaceActivate)();
	STDMETHOD(DoVerb)(LONG iVerb, LPMSG /* pMsg */, IOleClientSite* pActiveSite, LONG /* lindex */,
									 HWND hwndParent, LPCRECT lprcPosRect);
	//INotify
    void OnInvalidate(LPRECT lpRect);
private:
	BOOL IsVisible(ATL_DRAWINFO &di);
	void DrawCurrFrame();
	HWND GetContainerWindow();
	HWND GetDisplayWndRect(RECT *pRect);
private:
	CGdiPlusGif *m_pImage;
	BOOL m_bPlay;
	BOOL m_bRunMode;
	HWND m_hParentWnd;
	HWND m_hPaintWnd;
	RECT m_rcPaint;
	time_t m_tmLastTime;
	//RECT m_rcPos;
	//IAdviseSink *m_spAdviseSink;
	char m_szFileName[MAX_PATH];
};

OBJECT_ENTRY_AUTO(__uuidof(GifAnimate), CGifAnimate)
