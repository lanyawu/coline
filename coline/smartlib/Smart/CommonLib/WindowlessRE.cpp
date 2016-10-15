#include <CommonLib/DebugLog.h>
#include <CommonLib/WindowlessRE.h>
#include <comdef.h> 
#include <stddef.h>

#pragma warning(disable:4996)

// HIMETRIC units per inch (used for conversion)
#define HIMETRIC_PER_INCH 2540

#define ECO_STYLES (ECO_AUTOVSCROLL | ECO_AUTOHSCROLL | ECO_NOHIDESEL | \
						ECO_READONLY | ECO_WANTRETURN | ECO_SAVESEL | \
						ECO_SELECTIONBAR)
// Convert Himetric along the X axis to X pixels
LONG HimetricXtoDX(LONG xHimetric, LONG xPerInch)
{
	return (LONG) MulDiv(xHimetric, xPerInch, HIMETRIC_PER_INCH);
}

// Convert Himetric along the Y axis to Y pixels
LONG HimetricYtoDY(LONG yHimetric, LONG yPerInch)
{
	return (LONG) MulDiv(yHimetric, yPerInch, HIMETRIC_PER_INCH);
}

// Convert Pixels on the X axis to Himetric
LONG DXtoHimetricX(LONG dx, LONG xPerInch)
{
	return (LONG) MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
}

// Convert Pixels on the Y axis to Himetric
LONG DYtoHimetricY(LONG dy, LONG yPerInch)
{
	return (LONG) MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
}


// These constants are for backward compatibility. They are the 
// sizes used for initialization and reset in RichEdit 1.0

const LONG g_lInitTextMax = (512 * 1024) - 1;
const LONG g_lResetTextMax = (1024 * 1024);


INT	cxBorder, cyBorder;	    // GetSystemMetricx(SM_CXBORDER)...
INT	cxDoubleClk, cyDoubleClk;   // Double click distances
INT	cxHScroll, cxVScroll;	    // Width/height of scrlbar arw bitmap
INT	cyHScroll, cyVScroll;	    // Width/height of scrlbar arw bitmap
INT	dct;			    // Double Click Time in milliseconds
INT nScrollInset;
COLORREF crAuto = 0;

LONG CWindowlessRE::m_xWidthSys = 0;    		            // average char width of system font
LONG CWindowlessRE::m_yHeightSys = 0;				// height of system font
LONG CWindowlessRE::m_yPerInch = 0;				// y pixels per inch
LONG CWindowlessRE::m_xPerInch = 0;				// x pixels per inch


EXTERN_C const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    0x8d33f740,
    0xcf58,
    0x11ce,
    {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

EXTERN_C const IID IID_ITextHost = { /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
    0xc5bdd8d0,
    0xd26e,
    0x11ce,
    {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };


EXTERN_C const IID IID_ITextEditControl = { /* f6642620-d266-11ce-a89e-00aa006cadc5 */
    0xf6642620,
    0xd266,
    0x11ce,
    {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

void GetSysParms(void)
{
	crAuto		= GetSysColor(COLOR_WINDOWTEXT);
	cxBorder	= GetSystemMetrics(SM_CXBORDER);	// Unsizable window border
	cyBorder	= GetSystemMetrics(SM_CYBORDER);	//  widths
	cxHScroll	= GetSystemMetrics(SM_CXHSCROLL);	// Scrollbar-arrow bitmap 
	cxVScroll	= GetSystemMetrics(SM_CXVSCROLL);	//  dimensions
	cyHScroll	= GetSystemMetrics(SM_CYHSCROLL);	//
	cyVScroll	= GetSystemMetrics(SM_CYVSCROLL);	//
	cxDoubleClk	= GetSystemMetrics(SM_CXDOUBLECLK);
	cyDoubleClk	= GetSystemMetrics(SM_CYDOUBLECLK);
	dct			= GetDoubleClickTime();
    
    nScrollInset = GetProfileIntA( "windows", "ScrollInset", DD_DEFSCROLLINSET );
}

HRESULT InitDefaultCharFormat(CHARFORMAT_RE * pcf, HFONT hfont) 
{
	HWND hwnd;
	LOGFONT lf;
	HDC hdc;
	LONG yPixPerInch;

	// Get LOGFONT for default font
	if (!hfont)
		hfont = (HFONT)GetStockObject(SYSTEM_FONT);

	// Get LOGFONT for passed hfont
	if (!GetObject(hfont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	// Set CHARFORMAT structure
	pcf->cbSize = sizeof(CHARFORMAT_RE);
	
	hwnd = GetDesktopWindow();
	hdc = GetDC(hwnd);
	yPixPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
	pcf->yHeight = lf.lfHeight * LY_PER_INCH / yPixPerInch;
	ReleaseDC(hwnd, hdc);

	pcf->yOffset = 0;
	pcf->crTextColor = crAuto;

	pcf->dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
	pcf->dwEffects &= ~(CFE_PROTECTED | CFE_LINK);

	if(lf.lfWeight < FW_BOLD)
		pcf->dwEffects &= ~CFE_BOLD;
	if(!lf.lfItalic)
		pcf->dwEffects &= ~CFE_ITALIC;
	if(!lf.lfUnderline)
		pcf->dwEffects &= ~CFE_UNDERLINE;
	if(!lf.lfStrikeOut)
		pcf->dwEffects &= ~CFE_STRIKEOUT;

	pcf->dwMask = CFM_ALL | CFM_BACKCOLOR;
	pcf->bCharSet = lf.lfCharSet;
	pcf->bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef UNICODE
	_tcscpy(pcf->szFaceName, lf.lfFaceName);
#else
	//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
	MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE) ;
#endif
	
	return S_OK;
}

HRESULT InitDefaultParaFormat(PARAFORMAT_RE * ppf) 
{	
	memset(ppf, 0, sizeof(PARAFORMAT_RE));

	ppf->cbSize = sizeof(PARAFORMAT_RE);
	ppf->dwMask = PFM_ALL;
	ppf->wAlignment = PFA_LEFT;
	ppf->cTabCount = 1;
    ppf->rgxTabs[0] = lDefaultTab;

	return S_OK;
}



LRESULT MapHresultToLresult(HRESULT hr, UINT msg)
{
	LRESULT lres = hr;

	switch(msg)
	{
	case EM_GETMODIFY:
		lres = (hr == S_OK) ? -1 : 0;
		break;

		// These messages must return TRUE/FALSE rather than an hresult.
	case EM_UNDO:
	case WM_UNDO:
	case EM_CANUNDO:
	case EM_CANPASTE:
	case EM_LINESCROLL:
		// Hresults are backwards from TRUE and FALSE so we need
		// to do that remapping here as well.
		lres = (hr == S_OK) ? TRUE : FALSE;
		break;

	case EM_EXLINEFROMCHAR:
	case EM_LINEFROMCHAR:
		// If success, then hr a number. If error, it s/b 0.
		lres = SUCCEEDED(hr) ? (LRESULT) hr : 0;
		break;
			
	case EM_LINEINDEX:
		// If success, then hr a number. If error, it s/b -1.
		lres = SUCCEEDED(hr) ? (LRESULT) hr : -1;
		break;	

	default:
		lres = (LRESULT) hr;		
	}

	return lres;
}


BOOL GetIconic(HWND hwnd) 
{
	while(hwnd)
	{
		if(::IsIconic(hwnd))
			return TRUE;
		hwnd = GetParent(hwnd);
	}
	return FALSE;
}


CRichEditOlePlus::CRichEditOlePlus(IWindowlessRENotify *pNotify):
                  m_pNotify(pNotify)
{
}

CRichEditOlePlus::~CRichEditOlePlus()
{
	m_pNotify = NULL;
}

	//IUnknow接口
HRESULT _stdcall CRichEditOlePlus::QueryInterface(REFIID riid, LPVOID FAR * lplpObj)
{
	HRESULT hr = E_NOINTERFACE;
	*lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown)) 
	{
		*lplpObj = (IRichEditOleCallback *) this;
		hr = S_OK;
	}
	return hr;
}

ULONG _stdcall CRichEditOlePlus::AddRef()
{
	return 1;
}

ULONG _stdcall CRichEditOlePlus::Release()
{
	return 1;
}


// *** IRichEditOleCallback methods ***
HRESULT _stdcall CRichEditOlePlus::GetNewStorage( LPSTORAGE FAR * lplpstg)
{
	return E_NOTIMPL;
}

HRESULT _stdcall CRichEditOlePlus::GetInPlaceContext(LPOLEINPLACEFRAME FAR * lplpFrame, LPOLEINPLACEUIWINDOW FAR * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::ShowContainerUI(BOOL bShow)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg,	LONG cp)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::DeleteObject(LPOLEOBJECT lpoleobj)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR * lpcfFormat, DWORD reco, 
										  BOOL bReally, HGLOBAL hMetaPict)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::ContextSensitiveHelp(BOOL bEnterMode)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::GetClipboardData(CHARRANGE FAR * lpchrg, DWORD reco, LPDATAOBJECT FAR * lplpdataobj)
{
	return E_NOTIMPL;
}

HRESULT _stdcall CRichEditOlePlus::GetDragDropEffect(BOOL bDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	return S_OK;
}

HRESULT _stdcall CRichEditOlePlus::GetContextMenu(WORD seltype, LPOLEOBJECT lpoleobj, 
										 CHARRANGE FAR * lpchrg, HMENU FAR * lphmenu)
{
	return S_OK;
}

//class CWindowless
CWindowlessRE::CWindowlessRE(IWindowlessRENotify *pNotify):
               m_REOlePlus(NULL),
			   m_hWnd(NULL),
			   m_hParent(NULL),
			   m_pTextSrv(NULL),
			   m_pNotify(pNotify),
			   m_dwStyle(0),
			   m_dwExStyle(0),
			   m_bBorder(0),
			   m_bCustRect(0),
			   m_bInBottomless(0),
			   m_bInDialogBox(0),
			   m_bEnableAutoWordSel(0),
			   m_bVertical(0),
			   m_bIconic(0),
               m_bHidden(0),
			   m_bNotSysBkgnd(0),
			   m_bWindowLocked(0),
			   m_bRegisteredForDrop(0),
			   m_bVisible(0),
			   m_bResized(0),
			   m_bWordWrap(0),
			   m_bAllowBeep(0),
			   m_bRich(0),
			   m_bSaveSelection(0),
			   m_bInplaceActive(0),
			   m_bTransparent(0),
			   m_bTimer(0),
			   m_lSelBarWidth(0),
			   m_crBackground(0),
			   m_dwEventMask(0),
			   m_lRefs(0),
			   m_laccelPos(-1),
			   m_lMaxTextLen(g_lInitTextMax)

{
	//初始化
	memset(&m_rcClient, 0, sizeof(RECT));
	memset(&m_rcViewInset, 0, sizeof(RECT));
	memset(&m_sizelExtent, 0, sizeof(SIZEL));
	memset(&m_cfDefault, 0, sizeof(PARAFORMAT_RE));
	memset(&m_pfDefault, 0, sizeof(PARAFORMAT_RE));
	memset(&m_rcInplace, 0, sizeof(RECT) );
	m_chPasswordChar = L'*';
	m_hBkg = ::CreateSolidBrush(RGB(255, 255, 255));
}

CWindowlessRE::~CWindowlessRE()
{
	// Revoke our drop target
	RevokeDragDrop();

	if (m_REOlePlus)
	{
		delete m_REOlePlus;
		m_REOlePlus = NULL;
	}
	if (m_pTextSrv)
	{
		//m_pTextSrv->OnTxInPlaceDeactivate();
		//m_pTextSrv->Release();
		m_pTextSrv = NULL;
	}
	if (m_hBkg)
		::DeleteObject(m_hBkg);
}


////////////////////// Create/Init/Destruct Commands ///////////////////////

bool createTextService(ITextHost *pHost, IUnknown ** ppUnk)
{
	HINSTANCE richHandle = NULL;
	typedef HRESULT  (_stdcall *_CTS)(
		IUnknown *punkOuter,
		ITextHost *pITextHost,
		IUnknown **ppUnk) ; 
	_CTS CTS = NULL;
	richHandle = LoadLibraryW(L"Riched20.dll");
	if(richHandle == NULL)
		return false;
	else
	{
		CTS = (_CTS)GetProcAddress(richHandle, "CreateTextServices");
		if(NULL == CTS)
			return false;
	}
	// Create Text pHost component
	if(FAILED(CTS(NULL, pHost, ppUnk)))
		return false;
	FreeLibrary(richHandle);
	return true;
}

BOOL CWindowlessRE::Init(HWND hParent, HWND hWnd, DWORD dwStyle, DWORD dwExStyle, RECT rcClient)
{
    HDC hdc;
    HFONT hfontOld;
    TEXTMETRIC tm;
	IUnknown *pUnk;
	HRESULT hr;
	RECT rcActive = { 0 };

	// Initialize Reference count
	m_lRefs = 1;
    m_hParent = hParent;	
	m_hWnd = hWnd;
	m_dwStyle = dwStyle;
	m_dwExStyle = dwExStyle;
	// Create and cache CHARFORMAT for this control
	if(FAILED(InitDefaultCharFormat(&m_cfDefault, NULL)))
		goto err;
		
	// Create and cache PARAFORMAT for this control
	if(FAILED(InitDefaultParaFormat(&m_pfDefault)))
		goto err;

 	// edit controls created without a window are multiline by default
	// so that paragraph formats can be
	m_bHidden = TRUE;

	// edit controls are rich by default
	m_bRich = TRUE;
	m_bBorder = ((m_dwStyle & WS_BORDER) > 0);
	m_bWordWrap = (m_dwStyle & (ES_AUTOHSCROLL | WS_HSCROLL)) == 0;
    m_bVertical = (m_dwStyle & (ES_AUTOVSCROLL | WS_VSCROLL)) > 0;
    m_pfDefault.wAlignment = PFA_LEFT;
 
    // Init system metrics
	hdc = GetDC(m_hWnd);
    if(!hdc)
        goto err;

   	hfontOld = (HFONT)SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	if(!hfontOld)
		goto err;

	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, hfontOld);

	m_xWidthSys = (INT) tm.tmAveCharWidth;
    m_yHeightSys = (INT) tm.tmHeight;
	m_xPerInch = GetDeviceCaps(hdc, LOGPIXELSX); 
	m_yPerInch =	GetDeviceCaps(hdc, LOGPIXELSY); 

	ReleaseDC(m_hWnd, hdc);

	// At this point the border flag is set and so is the pixels per inch
	// so we can initalize the inset.
	SetDefaultInset();

	m_bInplaceActive = TRUE;

	// Create Text Services component
	//if(FAILED(CreateTextServices(NULL, this, &pUnk)))
	//	goto err;
	if (!createTextService(this, &pUnk))
		goto err;


	hr = pUnk->QueryInterface(IID_ITextServices,(void **)&m_pTextSrv);
	// Whether the previous call succeeded or failed we are done
	// with the private interface.
	pUnk->Release();
	if(FAILED(hr))
	{
		goto err;
	}

	m_rcClient = rcClient;
	// The extent matches the full client rectangle in HIMETRIC
	m_sizelExtent.cx = DXtoHimetricX(m_rcClient.right - m_rcClient.left - 2 * HOST_BORDER, m_xPerInch);
	m_sizelExtent.cy = DYtoHimetricY(m_rcClient.bottom - m_rcClient.top - 2 * HOST_BORDER, m_yPerInch);

	// notify Text Services that we are in place active
	GetControlRect( &rcActive );
	if( FAILED( m_pTextSrv->OnTxInPlaceActivate(&rcActive) ) ){
		m_bInplaceActive = false;
		goto err;
	}
	

	if (!(m_dwStyle & ES_READONLY))
	{
		// This isn't a read only window so we need a drop target.
		RegisterDragDrop();
	}
    if (!m_REOlePlus)
		m_REOlePlus = new CRichEditOlePlus(m_pNotify);

	m_pTextSrv->TxSendMessage(EM_SETOLECALLBACK, 0, LPARAM(m_REOlePlus), &hr);
	OnSunkenWindowPosChanging(hWnd, NULL);
	return TRUE;

err:
	return FALSE;
}


/////////////////////////////////  IUnknown ////////////////////////////////


HRESULT CWindowlessRE::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = E_NOINTERFACE;
	*ppvObject = NULL;

	if (IsEqualIID(riid, IID_ITextEditControl))
	{
		*ppvObject = (ITextEditControl *) this;
		AddRef();
		hr = S_OK;
	} else if (IsEqualIID(riid, IID_IUnknown) 
		|| IsEqualIID(riid, IID_ITextHost)) 
	{
		AddRef();
		*ppvObject = (ITextHost *) this;
		hr = S_OK;
	}

	return hr;
}

ULONG CWindowlessRE::AddRef(void)
{
	return ++m_lRefs;
}

ULONG CWindowlessRE::Release(void)
{
	ULONG c_Refs = --m_lRefs;

	if (c_Refs == 0)
	{
		delete this;
	}

	return c_Refs;
}


//////////////////////////////// Properties ////////////////////////////////


TXTEFFECT CWindowlessRE::TxGetEffects() const
{
	return (m_dwStyle & ES_SUNKEN) ? TXTEFFECT_SUNKEN : TXTEFFECT_NONE;
}


//////////////////////////// System API wrapper ////////////////////////////

//ITextServices * CWindowlessRE::GetTextServices(void)
//{
//	return m_pTextSrv;
//}

///////////////////////  Windows message dispatch methods  ///////////////////////////////


LRESULT CWindowlessRE::TxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT	lres = 0;
	HRESULT hr;

	switch(msg)
	{
	case WM_NCCALCSIZE:
		// we can't rely on WM_WININICHANGE so we use WM_NCCALCSIZE since
		// changing any of these should trigger a WM_NCCALCSIZE
		GetSysParms();
		return lres;

	case WM_KEYDOWN:
		lres = OnKeyDown((WORD) wparam, (DWORD) lparam);
		if(lres == 0)
			return lres;
		break;		   

	case WM_CHAR:
		lres = OnChar((WORD) wparam, (DWORD) lparam);
		if(lres == 0)
			return lres;
		break;

	case WM_SYSCOLORCHANGE:
		OnSysColorChange();
		// Notify the text services that there has been a change in the 
		// system colors.
		break;

	case WM_GETDLGCODE:
		 return OnGetDlgCode(wparam, lparam);

	case EM_HIDESELECTION:
		if((BOOL)lparam)
		{
			DWORD dwPropertyBits = 0;

			if((BOOL)wparam)
			{
				m_dwStyle &= ~(DWORD) ES_NOHIDESEL;
				dwPropertyBits = TXTBIT_HIDESELECTION;
			}
			else
				m_dwStyle |= ES_NOHIDESEL;

			// Notify text services of change in status.
			m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_HIDESELECTION, 
				dwPropertyBits);
		}
		return lres;

    case EM_LIMITTEXT:
        lparam = wparam;
        // Intentionally fall through. These messages are duplicates.
	case EM_EXLIMITTEXT:
        if (lparam == 0)
        {
            // 0 means set the control to the maximum size. However, because
            // 1.0 set this to 64K will keep this the same value so as not to
            // supprise anyone. Apps are free to set the value to be above 64K.
            lparam = (LPARAM) g_lResetTextMax;
        }
		m_lMaxTextLen = (LONG) lparam;
		m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, 
					TXTBIT_MAXLENGTHCHANGE);
		return lres;

	case EM_SETREADONLY:
		OnSetReadOnly(BOOL(wparam));
		return 1;

	case EM_GETEVENTMASK:
		 return OnGetEventMask();

	case EM_SETEVENTMASK:
		OnSetEventMask((DWORD) lparam);
		break;

	case EM_GETOPTIONS:
		return OnGetOptions();

	case EM_SETOPTIONS:
		OnSetOptions((WORD) wparam, (DWORD) lparam);
		lres = (m_dwStyle & ECO_STYLES);
		if(m_bEnableAutoWordSel)
			lres |= ECO_AUTOWORDSELECTION;
		return lres;

	case WM_SETFONT:
		return OnSetFont((HFONT) wparam);

	case EM_SETRECT:
        OnSetRect((LPRECT)lparam);
        return lres;
        
	case EM_GETRECT:
        OnGetRect((LPRECT)lparam);
        return lres;

	case EM_SETBKGNDCOLOR:
		lres = (LRESULT) m_crBackground;
		m_bNotSysBkgnd = !wparam;
		m_crBackground = (COLORREF) lparam;

		if(wparam)
			m_crBackground = GetSysColor(COLOR_WINDOW);

		// Notify the text services that color has changed
		m_pTextSrv->TxSendMessage(WM_SYSCOLORCHANGE, 0, 0, 0);

		if(lres != (LRESULT) m_crBackground)
			TxInvalidateRect(NULL, TRUE);

		return lres;

	case EM_SETCHARFORMAT:
		{
			UINT cbSize = ((CHARFORMAT_RE *) lparam)->cbSize;
			UINT cb = sizeof(CHARFORMAT_RE);
			if(cbSize != sizeof(CHARFORMAT_RE))
			{
				return 0;
			}

			if(wparam & SCF_SELECTION)
				break;								// Change selection format
			OnSetCharFormat((CHARFORMAT_RE *) lparam);		// Change default format
			return lres;
		}
	case EM_GETCHARFORMAT:
		{
			if (wparam & SCF_SELECTION)
				break;
			CHARFORMAT_RE *p = (CHARFORMAT_RE *)lparam;
			memmove(p, &m_cfDefault, sizeof(CHARFORMAT_RE));
			return lres;
		}
	case EM_SETPARAFORMAT:
		if(((PARAFORMAT_RE *) lparam)->cbSize != sizeof(PARAFORMAT_RE))
		{
			return 0;
		}

		// check to see if we're setting the default.
		// either SCF_DEFAULT will be specified *or* there is no
		// no text in the document (richedit1.0 behaviour).
		if ((wparam & SCF_DEFAULT) == 0)
		{
			hr = m_pTextSrv->TxSendMessage(WM_GETTEXTLENGTH, 0, 0, 0);

			if (hr == 0)
			{
				wparam |= SCF_DEFAULT;
			}
		}

		if ((wparam & SCF_DEFAULT) != 0)
		{								
			OnSetParaFormat((PARAFORMAT_RE *) lparam);	// Change default format
			return lres;
		}
		break;

    case WM_SETTEXT:

        // For RichEdit 1.0, the max text length would be reset by a settext so 
        // we follow pattern here as well.

		hr = m_pTextSrv->TxSendMessage(msg, wparam, lparam, 0);

        if (SUCCEEDED(hr))
        {
            // Update succeeded.
            LONG cNewText = (LONG)_tcslen((LPCTSTR) lparam);

            // If the new text is greater than the max set the max to the new
            // text length.
            if (cNewText > m_lMaxTextLen)
            {
                m_lMaxTextLen = cNewText;                
            }

			lres = 1;
        }

        return lres;

	case WM_SIZE:
		return (LRESULT) OnSize(hwnd, (WORD)wparam, LOWORD(lparam), HIWORD(lparam));
		

	case WM_WINDOWPOSCHANGING:
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);

		if(TxGetEffects() == TXTEFFECT_SUNKEN)
			OnSunkenWindowPosChanging(hwnd, (WINDOWPOS *) lparam);
		return lres;

	case WM_SETCURSOR:
		//Only set cursor when over us rather than a child; this
		//			helps prevent us from fighting it out with an inplace child
		if((HWND)wparam == m_hWnd)
		{
			POINT pt;
			::GetCursorPos( &pt );
			::ScreenToClient(m_hWnd, &pt);
			DoSetCursor( NULL, &pt );
			lres = TRUE;
		}
		return lres;

	case WM_SHOWWINDOW:
		hr = OnTxVisibleChange((BOOL)wparam);
		return lres;

	case WM_NCPAINT:
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
		if(TxGetEffects() == TXTEFFECT_SUNKEN)
		{
			HDC hdc = GetDC(hwnd);
			if(hdc)
			{
				DrawSunkenBorder(hwnd, hdc);
				ReleaseDC(hwnd, hdc);
			}
		}
		return lres;

	case WM_PAINT:
		{
			// Put a frame around the control so it can be seen
			//FrameRect((HDC) wparam, &m_rcClient, (HBRUSH) GetStockObject(BLACK_BRUSH));
			PaintRE((HDC)wparam, (LPRECT)lparam, !m_bInplaceActive, TxGetEffects() == TXTEFFECT_SUNKEN);

		}

		return lres;
	}
	hr = m_pTextSrv->TxSendMessage(msg, wparam, lparam, &lres);
	if (hr == S_FALSE)
	{
		lres = ::DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return lres;
}
	

///////////////////////////////  Keyboard Messages  //////////////////////////////////

void CWindowlessRE::PaintRE(HDC hdc, LPRECT lprc, BOOL bUpdateClient, BOOL bDrawSunken, BOOL bEraseBkg)
{
	RECT rcClient;
	RECT *prc = NULL;
	LONG lViewId = TXTVIEW_ACTIVE;
	if (bUpdateClient)
	{
		GetControlRect(&rcClient);
		prc = &rcClient;
		lViewId = TXTVIEW_INACTIVE;
	}
	
	if (bEraseBkg)
	{ 
		::FillRect(hdc, prc, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}
	// Remember wparam is actually the hdc and lparam is the u pdate
	// rect because this message has been preprocessed by the window.
	m_pTextSrv->TxDraw(
		DVASPECT_CONTENT,  		// Draw Aspect
		0,						// Lindex
		NULL,					// Info for drawing optimazation
		NULL,					// target device information
    	hdc,			// Draw device HDC
    	NULL, 				   	// Target device HDC
		(RECTL *) prc,			// Bounding client rectangle
		NULL, 					// Clipping rectangle for metafiles
		lprc,		// Update rectangle
		NULL, 	   				// Call back function
		NULL,					// Call back parameter
		lViewId);				// What view of the object		 		
    if (::GetObjectType(hdc) !=OBJ_DC && ::GetObjectType(hdc) != OBJ_MEMDC)
	{
		/*RECT p = {0}, u = {0};
		if (prc)
			p = *prc;
		if (lparam)
			u = *((RECT *)lparam);*/
		//PRINTDEBUGLOG(dtError, "Paint error, Boud RECT(left:%d top:%d right:%d bottom:%d), update RECT(left:%d top:%d right:%d bottom:%d), place RECT(left:%d top:%d right:%d bottom:%d) client RECT(left:%d top:%d right:%d bottom:%d) active:%d",
		//	p.left, p.top, p.right, p.bottom, u.left, u.top, u.right, u.bottom, m_rcInplace.left, m_rcInplace.top, 
		//	m_rcInplace.right, m_rcInplace.bottom, m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom, m_bInplaceActive);
	}
	if(bDrawSunken)
	{
		DrawSunkenBorder(m_hWnd, hdc);
	}
}

LRESULT CWindowlessRE::OnKeyDown(WORD vkey, DWORD dwFlags)
{
	switch(vkey)
	{
	case VK_ESCAPE:
		if(m_bInDialogBox)
		{
			PostMessage(m_hParent, WM_CLOSE, 0, 0);
			return 0;
		}
		break;
	
	case VK_RETURN:
		if (m_pNotify)
		{
			if (m_pNotify->OnEnterKeyDown())
				return 0;
		}
		break;
	case 'V':
	case 'v':
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (m_pNotify)
			{
				if (m_pNotify->OnPaste())
					return 0;
			}           
		} 
		break;
	case 'X':
	case 'x':
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			if (m_pNotify)
				if (m_pNotify->OnCut())
					return 0;
		}
		break;
	case 'C':
	case 'c':
		if (GetKeyState(VK_CONTROL) & 0x800)
		{
			if (m_pNotify)
				if (m_pNotify->OnCopy())
					return 0;
		}
		break;
	case VK_TAB:
		if(m_bInDialogBox) 
		{
			SendMessage(m_hParent, WM_NEXTDLGCTL, 	!!(GetKeyState(VK_SHIFT) & 0x8000), 0);
			return 0;
		}
		break;
	}
	return 1;
}

#define CTRL(_ch) (_ch - 'A' + 1)

LRESULT CWindowlessRE::OnChar(WORD vkey, DWORD dwFlags)
{
	switch(vkey)
	{
		// Ctrl-Return generates Ctrl-J (LF), treat it as an ordinary return
		case CTRL('J'):
		case VK_RETURN:
			if(m_bInDialogBox && !(GetKeyState(VK_CONTROL) & 0x8000)
					 && !(m_dwStyle & ES_WANTRETURN))
				return 0;
			break;

		case VK_TAB:
			if(m_bInDialogBox && !(GetKeyState(VK_CONTROL) & 0x8000))
				return 0;
		default:
			if (m_pNotify)
				m_pNotify->OnTextChange();
	}
	
	return 1;
}


////////////////////////////////////  View rectangle //////////////////////////////////////


void CWindowlessRE::OnGetRect(LPRECT prc)
{
    RECT rcInset;

	// Get view inset (in HIMETRIC)
    TxGetViewInset(&rcInset);

	// Convert the himetric inset to pixels
	rcInset.left = HimetricXtoDX(rcInset.left, m_xPerInch);
	rcInset.top = HimetricYtoDY(rcInset.top , m_yPerInch);
	rcInset.right = HimetricXtoDX(rcInset.right, m_xPerInch);
	rcInset.bottom = HimetricYtoDY(rcInset.bottom, m_yPerInch);
    
	// Get client rect in pixels
    TxGetClientRect(prc);

	// Modify the client rect by the inset 
    prc->left += rcInset.left;
    prc->top += rcInset.top;
    prc->right -= rcInset.right;
    prc->bottom -= rcInset.bottom;
}

void CWindowlessRE::OnSetRect(LPRECT prc)
{
	RECT rcClient;
	
	if(!prc)
	{
		SetDefaultInset();
	}	
	else	
    {
    	// For screen display, the following intersects new view RECT
    	// with adjusted client area RECT
    	TxGetClientRect(&rcClient);

        // Adjust client rect
        // Factors in space for borders
        if(m_bBorder)
        {																					  
    	    rcClient.top		+= m_yHeightSys / 4;
    	    rcClient.bottom 	-= m_yHeightSys / 4 - 1;
    	    rcClient.left		+= m_xWidthSys / 2;
    	    rcClient.right	-= m_xWidthSys / 2;
        }
	
        // Ensure we have minimum width and height
        rcClient.right = max(rcClient.right, rcClient.left + m_xWidthSys);
        rcClient.bottom = max(rcClient.bottom, rcClient.top + m_yHeightSys);

        // Intersect the new view rectangle with the 
        // adjusted client area rectangle
        if(!IntersectRect(&m_rcViewInset, &rcClient, prc))
    	    m_rcViewInset = rcClient;

        // compute inset in pixels
        m_rcViewInset.left -= rcClient.left;
        m_rcViewInset.top -= rcClient.top;
        m_rcViewInset.right = rcClient.right - m_rcViewInset.right;
        m_rcViewInset.bottom = rcClient.bottom - m_rcViewInset.bottom;

		// Convert the inset to himetric that must be returned to ITextServices
        m_rcViewInset.left = DXtoHimetricX(m_rcViewInset.left, m_xPerInch);
        m_rcViewInset.top = DYtoHimetricY(m_rcViewInset.top, m_yPerInch);
        m_rcViewInset.right = DXtoHimetricX(m_rcViewInset.right, m_xPerInch);
        m_rcViewInset.bottom = DYtoHimetricY(m_rcViewInset.bottom, m_yPerInch);
    }

    m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE,	TXTBIT_VIEWINSETCHANGE);
	//PRINTDEBUGLOG(dtInfo, "onSetRect");
}



////////////////////////////////////  System notifications  //////////////////////////////////


void CWindowlessRE::OnSysColorChange()
{
	crAuto = GetSysColor(COLOR_WINDOWTEXT);
	if(!m_bNotSysBkgnd)
		m_crBackground = GetSysColor(COLOR_WINDOW);
	if (m_hBkg)
		::DeleteObject(m_hBkg);
	m_hBkg = ::CreateSolidBrush(m_crBackground);
	TxInvalidateRect(NULL, TRUE);
}

LRESULT CWindowlessRE::OnGetDlgCode(WPARAM wparam, LPARAM lparam)
{
	LRESULT lres = DLGC_WANTCHARS | DLGC_WANTARROWS | DLGC_WANTTAB;

	if(m_dwStyle & ES_MULTILINE)
		lres |= DLGC_WANTALLKEYS;

	if(!(m_dwStyle & ES_SAVESEL))
		lres |= DLGC_HASSETSEL;

	if(lparam)
		m_bInDialogBox = TRUE;

	if(lparam && ((WORD) wparam == VK_BACK))
	{
		lres |= DLGC_WANTMESSAGE;
	}

	return lres;
}


/////////////////////////////////  Other messages  //////////////////////////////////////


LRESULT CWindowlessRE::OnGetOptions() const
{
	LRESULT lres = (m_dwStyle & ECO_STYLES);

	if(m_bEnableAutoWordSel)
		lres |= ECO_AUTOWORDSELECTION;
	//PRINTDEBUGLOG(dtInfo, "OnGetOptions: %d", lres);
	return lres;
}

void CWindowlessRE::OnSetOptions(WORD wOp, DWORD eco)
{
	const BOOL bAutoWordSel = !!(eco & ECO_AUTOWORDSELECTION);
	DWORD dwStyleNew = m_dwStyle;
	DWORD dw_Style = 0 ;

	DWORD dwChangeMask = 0;

	// single line controls can't have a selection bar
	// or do vertical writing
	if(!(dw_Style & ES_MULTILINE))
	{
#ifdef DBCS
		eco &= ~(ECO_SELECTIONBAR | ECO_VERTICAL);
#else
		eco &= ~ECO_SELECTIONBAR;
#endif
	}
	dw_Style = (eco & ECO_STYLES);

	switch(wOp)
	{
	case ECOOP_SET:
		dwStyleNew = ((dwStyleNew) & ~ECO_STYLES) | m_dwStyle;
		m_bEnableAutoWordSel = bAutoWordSel;
		break;

	case ECOOP_OR:
		dwStyleNew |= dw_Style;
		if(bAutoWordSel)
			m_bEnableAutoWordSel = TRUE;
		break;

	case ECOOP_AND:
		dwStyleNew &= (dw_Style | ~ECO_STYLES);
		if(m_bEnableAutoWordSel && !bAutoWordSel)
			m_bEnableAutoWordSel = FALSE;
		break;

	case ECOOP_XOR:
		dwStyleNew ^= dw_Style;
		m_bEnableAutoWordSel = (!m_bEnableAutoWordSel != !bAutoWordSel);
		break;
	}

	if(m_bEnableAutoWordSel != (unsigned)bAutoWordSel)
	{
		dwChangeMask |= TXTBIT_AUTOWORDSEL; 
	}

	if(dwStyleNew != dw_Style)
	{
		DWORD dwChange = dwStyleNew ^ dw_Style;
#ifdef DBCS
		USHORT	usMode;
#endif

		m_dwStyle = dwStyleNew;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyleNew);

		if(dwChange & ES_NOHIDESEL)	
		{
			dwChangeMask |= TXTBIT_HIDESELECTION;
		}

		if(dwChange & ES_READONLY)
		{
			dwChangeMask |= TXTBIT_READONLY;

			// Change drop target state as appropriate.
			if (dwStyleNew & ES_READONLY)
			{
				RevokeDragDrop();
			}
			else
			{
				RegisterDragDrop();
			}
		}

		if(dwChange & ES_VERTICAL)
		{
			dwChangeMask |= TXTBIT_VERTICAL;
		}

		// no action require for ES_WANTRETURN
		// no action require for ES_SAVESEL
		// do this last
		if(dwChange & ES_SELECTIONBAR)
		{
			m_lSelBarWidth = 212;
			dwChangeMask |= TXTBIT_SELBARCHANGE;
		}
	}

	if (dwChangeMask)
	{
		DWORD dwProp = 0;
		TxGetPropertyBits(dwChangeMask, &dwProp);
		m_pTextSrv->OnTxPropertyBitsChange(dwChangeMask, dwProp);
	}
	//PRINTDEBUGLOG(dtInfo, "OnSetOptions: %d", dwChangeMask);
}

void CWindowlessRE::OnSetReadOnly(BOOL bReadOnly)
{
	DWORD dwUpdatedBits = 0;

	if(bReadOnly)
	{
		m_dwStyle |= ES_READONLY;

		// Turn off Drag Drop 
		RevokeDragDrop();
		dwUpdatedBits |= TXTBIT_READONLY;
	}
	else
	{
		m_dwStyle &= ~(DWORD) ES_READONLY;

		// Turn drag drop back on
		RegisterDragDrop();	
	}

	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_READONLY, dwUpdatedBits);
}

void CWindowlessRE::OnSetEventMask(DWORD mask)
{
	LRESULT lres = (LRESULT) m_dwEventMask;
	m_dwEventMask = (DWORD) mask;
    //PRINTDEBUGLOG(dtInfo, "OnGetEventMask: %d", m_dwEventMask);
}


LRESULT CWindowlessRE::OnGetEventMask() const
{
	//PRINTDEBUGLOG(dtInfo, "OnGetEventMask: %d", m_dwEventMask);
	return (LRESULT) m_dwEventMask;
}

 
BOOL CWindowlessRE::OnSetFont(HFONT hfont)
{
	if(SUCCEEDED(InitDefaultCharFormat(&m_cfDefault, hfont)))
	{
		m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
			TXTBIT_CHARFORMATCHANGE);
		return TRUE;
	}
	return FALSE;
}

 
BOOL CWindowlessRE::OnSetCharFormat(CHARFORMAT_RE *pcf)
{
	m_cfDefault = *pcf;
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
		TXTBIT_CHARFORMATCHANGE);

	return TRUE;
}

 
BOOL CWindowlessRE::OnSetParaFormat(PARAFORMAT_RE *pPF)
{
	m_pfDefault = *pPF;									// Copy it

	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 
		TXTBIT_PARAFORMATCHANGE);

	return TRUE;
}



////////////////////////////  Event firing  /////////////////////////////////



void * CWindowlessRE::CreateNmhdr(UINT uiCode, LONG cb)
{
	NMHDR *pnmhdr;

	pnmhdr = (NMHDR*) new char[cb];
	if(!pnmhdr)
		return NULL;

	memset(pnmhdr, 0, cb);

	pnmhdr->hwndFrom = m_hWnd;
	pnmhdr->idFrom = GetWindowLong(m_hWnd, GWL_ID);
	pnmhdr->code = uiCode;

	return (VOID *) pnmhdr;
}


////////////////////////////////////  Helpers  /////////////////////////////////////////
void CWindowlessRE::SetDefaultInset()
{
    // Generate default view rect from client rect.
    if(m_bBorder)
    {
        // Factors in space for borders
  	    m_rcViewInset.top = DYtoHimetricY(m_yHeightSys / 4, m_yPerInch);
		m_rcViewInset.bottom	= DYtoHimetricY(m_yHeightSys / 4 - 1, m_yPerInch);
		m_rcViewInset.left = DXtoHimetricX(m_xWidthSys / 2, m_xPerInch);
		m_rcViewInset.right = DXtoHimetricX(m_xWidthSys / 2, m_xPerInch);
    } else
    {
		m_rcViewInset.top = m_rcViewInset.left =
		m_rcViewInset.bottom = m_rcViewInset.right = 0;
	}
}


/////////////////////////////////  Far East Support  //////////////////////////////////////

HIMC CWindowlessRE::TxImmGetContext(void)
{
	HIMC himc;
	himc = ::ImmGetContext( m_hWnd );
	if (himc)
	{  
		COMPOSITIONFORM nPos = {0};
		nPos.dwStyle = CFS_POINT;
		nPos.ptCurrentPos.x = (m_rcClient.left + m_rcClient.right) / 2;
		nPos.ptCurrentPos.y = m_rcClient.bottom;
		::ImmSetCompositionWindow(himc, &nPos);
	}
	return himc;
}

void CWindowlessRE::TxImmReleaseContext(HIMC himc)
{
	ImmReleaseContext( m_hWnd, himc );
}

void CWindowlessRE::RevokeDragDrop(void)
{
	if (m_bRegisteredForDrop)
	{
		::RevokeDragDrop(m_hWnd);
		m_bRegisteredForDrop = FALSE;
	}
}

void CWindowlessRE::RegisterDragDrop(void)
{
	IDropTarget *pdt;

	if(!m_bRegisteredForDrop && m_pTextSrv->TxGetDropTarget(&pdt) == NOERROR)
	{
		HRESULT hr = ::RegisterDragDrop(m_hWnd, pdt);

		if(hr == NOERROR)
		{	
			m_bRegisteredForDrop = TRUE;
		}

		pdt->Release();
	}
}

VOID DrawRectFn(
	HDC hdc, 
	RECT *prc, 
	INT icrTL, 
	INT icrBR,
	BOOL bBot, 
	BOOL bRght)
{
	COLORREF cr;
	COLORREF crSave;
	RECT rc;

	cr = GetSysColor(icrTL);
	crSave = SetBkColor(hdc, cr);

	// top
	rc = *prc;
	rc.bottom = rc.top + 1;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

	// left
	rc.bottom = prc->bottom;
	rc.right = rc.left + 1;
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

	if(icrTL != icrBR)
	{
		cr = GetSysColor(icrBR);
		SetBkColor(hdc, cr);
	}

	// right
	rc.right = prc->right;
	rc.left = rc.right - 1;
	if(!bBot)
		rc.bottom -= cyHScroll;
	if(bRght)
	{
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}

	// bottom
	if(bBot)
	{
		rc.left = prc->left;
		rc.top = rc.bottom - 1;
		if(!bRght)
			rc.right -= cxVScroll;
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	}
	SetBkColor(hdc, crSave);
}

#define cmultBorder 1

VOID CWindowlessRE::OnSunkenWindowPosChanging(HWND hwnd, WINDOWPOS *pwndpos)
{
	if(m_bVisible)
	{
		RECT rc;
		HWND hwndParent;

		GetWindowRect(hwnd, &rc);
		InflateRect(&rc, cxBorder * cmultBorder, cyBorder * cmultBorder);
		hwndParent = GetParent(hwnd);
		MapWindowPoints(HWND_DESKTOP, hwndParent, (POINT *) &rc, 2);
		InvalidateRect(hwndParent, &rc, FALSE);
	}
}


VOID CWindowlessRE::DrawSunkenBorder(HWND hwnd, HDC hdc)
{
	RECT rc;
	RECT rcParent;
	DWORD dwScrollBars;
	HWND hwndParent;

	GetWindowRect(hwnd, &rc);
    hwndParent = GetParent(hwnd);
	rcParent = rc;
	MapWindowPoints(HWND_DESKTOP, hwndParent, (POINT *)&rcParent, 2);
	InflateRect(&rcParent, cxBorder, cyBorder);
	OffsetRect(&rc, -rc.left, -rc.top);

	// draw inner rect
	TxGetScrollBars(&dwScrollBars);
	DrawRectFn(hdc, &rc, COLOR_WINDOWFRAME, COLOR_BTNFACE,
		!(dwScrollBars & WS_HSCROLL), !(dwScrollBars & WS_VSCROLL));

	// draw outer rect
	hwndParent = GetParent(hwnd);
	HDC h = GetDC(hwndParent);
	DrawRectFn(h, &rcParent, COLOR_BTNSHADOW, COLOR_BTNHIGHLIGHT,
		TRUE, TRUE);
	ReleaseDC(hwndParent, h);
}

LRESULT CWindowlessRE::OnSize(HWND hwnd, WORD fwSizeType, int nWidth, int nHeight)
{
	// Update our client rectangle
	m_rcClient.right = m_rcClient.left + nWidth;
	m_rcClient.bottom = m_rcClient.top + nHeight;
    //重新计算 尺寸
	m_sizelExtent.cx = DXtoHimetricX(nWidth - 2 * HOST_BORDER, m_xPerInch);
	m_sizelExtent.cy = DYtoHimetricY(nHeight - 2 * HOST_BORDER, m_yPerInch);

	if(!m_bVisible)
	{
		m_bIconic = GetIconic(hwnd);
		if(!m_bIconic)
			m_bResized = TRUE;
	} else
	{
		if(GetIconic(hwnd))
		{
			m_bIconic = TRUE;
		} else
		{
			m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE, 
				TXTBIT_CLIENTRECTCHANGE);

			if (m_bIconic)
			{
				InvalidateRect(hwnd, NULL, FALSE);
				m_bIconic = FALSE;
			}
			
			if (TxGetEffects() == TXTEFFECT_SUNKEN)	// Draw borders
				DrawSunkenBorder(hwnd, NULL);
		}
	}
	return 0;
}

HRESULT CWindowlessRE::OnTxVisibleChange(BOOL bVisible)
{
	m_bVisible = bVisible;

	if (!m_bVisible && m_bResized)
	{
		RECT rc;
		// Control was resized while hidden,
		// need to really resize now
		TxGetClientRect( &rc );
		m_bResized = FALSE;
		m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE, 
			TXTBIT_CLIENTRECTCHANGE);
	}

	return S_OK;
}



//////////////////////////// ITextHost Interface  ////////////////////////////

HDC CWindowlessRE::TxGetDC()
{
	if (m_bInplaceActive)
	{
		if (::IsWindow(m_hWnd))
			return ::GetDC(m_hWnd);
	}
	return NULL;
}


int CWindowlessRE::TxReleaseDC(HDC hdc)
{
	if (m_bInplaceActive){
		_ASSERT( ::IsWindow(m_hWnd) );
		_ASSERT( ::GetObjectType(hdc) == OBJ_DC || ::GetObjectType(hdc) == OBJ_MEMDC );
		return ( ::ReleaseDC(m_hWnd, hdc) ? 1 : 0 );
	}
	return 0;
}


BOOL CWindowlessRE::TxShowScrollBar(INT nBar, BOOL bShow)
{
	if (m_bInplaceActive && m_pNotify)
		return m_pNotify->RE_ShowScrollBar(nBar, bShow);
	return FALSE;
}

BOOL CWindowlessRE::TxEnableScrollBar (INT uSBFlags, INT uArrowflags)
{
	if (m_bInplaceActive && m_pNotify)
		return m_pNotify->RE_EnableScrollBar(uSBFlags, uArrowflags);
	return FALSE;
}


BOOL CWindowlessRE::TxSetScrollRange(INT nBar, LONG nMinPos, INT nMaxPos, BOOL bRedraw)
{
	if (m_bInplaceActive && m_pNotify)
		return m_pNotify->RE_SetScrollRange(nBar, nMinPos, nMaxPos, bRedraw);
	return FALSE;
}


BOOL CWindowlessRE::TxSetScrollPos (INT nBar, INT nPos, BOOL bRedraw)
{
	if (m_bInplaceActive && m_pNotify)
		return m_pNotify->RE_SetScrollPos(nBar, nPos, bRedraw);
	return FALSE;
}

void CWindowlessRE::TxInvalidateRect(LPCRECT prc, BOOL bMode)
{
	if (prc)
	{
		/*RECT rcUpdate;
		GetControlRect( &rcUpdate );
		if (rcUpdate.left < prc->left)
			rcUpdate.left = prc->left;
		if (rcUpdate.top < prc->top)
			rcUpdate.top = prc->top;
		if (rcUpdate.right > prc->right)
			rcUpdate.right = prc->right;
		if (rcUpdate.bottom > prc->bottom)
			rcUpdate.bottom = prc->bottom; */
		if (m_pNotify)
			m_pNotify->InvalidateRE(prc, bMode);
		else
			::InvalidateRect(m_hWnd, prc, bMode); 
	} 
}

void CWindowlessRE::TxViewChange(BOOL bUpdate) 
{
	if (bUpdate)
	{
		::UpdateWindow(m_hWnd);
	}
}


BOOL CWindowlessRE::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
	if (m_bInplaceActive)
	{
		return ::CreateCaret(m_hWnd, hbmp, xWidth, yHeight);
	}
	return false;
}


BOOL CWindowlessRE::TxShowCaret(BOOL bShow)
{
	if (bShow)
		return ::ShowCaret(m_hWnd);
	else
		return ::HideCaret(m_hWnd);
}

BOOL CWindowlessRE::TxSetCaretPos(INT x, INT y)
{
	if (m_bInplaceActive)
		return ::SetCaretPos(x, y);
	return false;
}


BOOL CWindowlessRE::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	m_bTimer = TRUE;
	return (BOOL)::SetTimer(m_hWnd, idTimer, uTimeout, NULL);
}


void CWindowlessRE::TxKillTimer(UINT idTimer)
{
	::KillTimer(m_hWnd, idTimer);
	m_bTimer = FALSE;
}

void CWindowlessRE::TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll,	LPCRECT lprcClip,	
									  HRGN hrgnUpdate, LPRECT lprcUpdate,	UINT uScroll)	
{
	if (m_bInplaceActive && m_pNotify)
		m_pNotify->RE_TxScrollWindowEx(dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate, uScroll);
}

void CWindowlessRE::TxSetCapture(BOOL bCapture)
{
	if (m_bInplaceActive)
	{
		if (bCapture)
			::SetCapture(m_hWnd);
		else
			::ReleaseCapture();
	}
}

void CWindowlessRE::TxSetFocus()
{
	if (m_bInplaceActive && ::GetFocus() != m_hWnd)
	{
		::SetFocus(m_hWnd);
	}
}

void CWindowlessRE::TxSetCursor(HCURSOR hcur, BOOL fText)
{
	::SetCursor( hcur );
}

BOOL CWindowlessRE::TxScreenToClient(LPPOINT lppt)
{
	_ASSERT( ::IsWindow( m_hWnd ) );
	return ::ScreenToClient(m_hWnd, lppt);	
}

BOOL CWindowlessRE::TxClientToScreen(LPPOINT lppt)
{
	_ASSERT( ::IsWindow( m_hWnd ) );
	return ::ClientToScreen(m_hWnd, lppt);
}

HRESULT CWindowlessRE::TxActivate(LONG *plOldState)
{
	_ASSERT( ::IsWindow( m_hWnd ) );
	return S_OK;
}

HRESULT CWindowlessRE::TxDeactivate(LONG lNewState)
{
    return S_OK;
}
    

HRESULT CWindowlessRE::TxGetClientRect(LPRECT prc)
{
	if( m_bInplaceActive && prc )
	{
		GetControlRect( prc );
		return S_OK;
	}
	return E_FAIL;
}


HRESULT CWindowlessRE::TxGetViewInset(LPRECT prc) 
{
	*prc = m_rcViewInset;
    return S_OK;	
}

HRESULT CWindowlessRE::TxGetCharFormat(const CHARFORMATW **ppCF)
{
	*ppCF =  &m_cfDefault;
	return S_OK;
}

HRESULT CWindowlessRE::TxGetParaFormat(const PARAFORMAT **ppPF)
{
	*ppPF = &m_pfDefault;
	return S_OK;
}


COLORREF CWindowlessRE::TxGetSysColor(int nIndex) 
{
	if (nIndex == COLOR_WINDOW)
	{
		if(!m_bNotSysBkgnd)
			return GetSysColor(COLOR_WINDOW);
		return m_crBackground;
	} else if (nIndex == COLOR_BACKGROUND)
	{
		return RGB(0xFF, 0, 0);
	}
	return GetSysColor(nIndex);
}



HRESULT CWindowlessRE::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
	*pstyle = !m_bTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
	return S_OK;
}


HRESULT CWindowlessRE::TxGetMaxLength(DWORD *pLength)
{
	*pLength = m_lMaxTextLen;
	return S_OK;
}



HRESULT CWindowlessRE::TxGetScrollBars(DWORD *pdwScrollBar)
{
	*pdwScrollBar =  m_dwStyle & ( WS_VSCROLL |  ES_AUTOVSCROLL );
	return S_OK;
}


HRESULT CWindowlessRE::TxGetPasswordChar(TCHAR *pch)
{
#ifdef UNICODE
	*pch = m_chPasswordChar;
#else
	WideCharToMultiByte(CP_ACP, 0, &chPasswordChar, 1, pch, 1, NULL, NULL) ;
#endif
	return S_OK;
}

HRESULT CWindowlessRE::TxGetAcceleratorPos(LONG *pcp)
{
	*pcp = m_laccelPos;
	return S_OK;
} 										   

HRESULT CWindowlessRE::OnTxCharFormatChange(const CHARFORMATW *pcf)
{
	return S_OK;
}


HRESULT CWindowlessRE::OnTxParaFormatChange(const PARAFORMAT *ppf)
{  
	return S_OK;
}


HRESULT CWindowlessRE::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) 
{
	DWORD dwProperties = 0;

	if (m_bRich)
	{
		dwProperties = TXTBIT_RICHTEXT;
	}

	if (m_dwStyle & ES_MULTILINE)
	{
		dwProperties |= TXTBIT_MULTILINE;
	}

	if (m_dwStyle & ES_READONLY)
	{
		dwProperties |= TXTBIT_READONLY;
	}


	if (m_dwStyle & ES_PASSWORD)
	{
		dwProperties |= TXTBIT_USEPASSWORD;
	}

	if (!(m_dwStyle & ES_NOHIDESEL))
	{
		dwProperties |= TXTBIT_HIDESELECTION;
	}

	if (m_bEnableAutoWordSel)
	{
		dwProperties |= TXTBIT_AUTOWORDSEL;
	}

	if (m_bVertical)
	{
		dwProperties |= TXTBIT_VERTICAL;
	}
					
	if (m_bWordWrap)
	{
		dwProperties |= TXTBIT_WORDWRAP;
	}

	if (m_bAllowBeep)
	{
		dwProperties |= TXTBIT_ALLOWBEEP;
	}

	if (m_bSaveSelection)
	{
		dwProperties |= TXTBIT_SAVESELECTION;
	}

	*pdwBits = dwProperties & dwMask; 
	return S_OK;
}


HRESULT CWindowlessRE::TxNotify(DWORD iNotify, void *pv)
{
	if (iNotify == EN_REQUESTRESIZE)
	{
		RECT rc;
		REQRESIZE *preqsz = (REQRESIZE *)pv;
		
		GetControlRect(&rc);
		rc.bottom = rc.top + preqsz->rc.bottom + HOST_BORDER;
		rc.right  = rc.left + preqsz->rc.right + HOST_BORDER;
		rc.top -= HOST_BORDER;
		rc.left -= HOST_BORDER;
		
		SetClientRect(&rc, TRUE);
		
		return S_OK;
	} 
    if (m_pNotify)
		m_pNotify->Notify(iNotify, pv);
 
	return S_OK;
}



HRESULT CWindowlessRE::TxGetExtent(LPSIZEL lpExtent)
{
	// Calculate the length & convert to himetric
	*lpExtent = m_sizelExtent;
	return S_OK;
}

HRESULT	CWindowlessRE::TxGetSelectionBarWidth (LONG *plSelBarWidth)
{
	*plSelBarWidth = m_lSelBarWidth;
	return S_OK;
}


BOOL CWindowlessRE::GetReadOnly()
{
	return (m_dwStyle & ES_READONLY) != 0;
}

void CWindowlessRE::SetReadOnly(BOOL bReadOnly)
{
	if (bReadOnly)
	{
		m_dwStyle |= ES_READONLY;
	} else
	{
		m_dwStyle &= ~ES_READONLY;
	}

	// Notify control of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_READONLY,	bReadOnly ? TXTBIT_READONLY : 0);
}

BOOL CWindowlessRE::GetAllowBeep()
{
	return m_bAllowBeep;
}

void CWindowlessRE::SetAllowBeep(BOOL bAllowBeep)
{
	m_bAllowBeep = bAllowBeep;

	// Notify control of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, 
		m_bAllowBeep ? TXTBIT_ALLOWBEEP : 0);
}

void CWindowlessRE::SetViewInset(RECT *prc)
{
	m_rcViewInset = *prc;
	// Notify control of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, 0);
}

WORD CWindowlessRE::GetDefaultAlign()
{
	return m_pfDefault.wAlignment;
}


void CWindowlessRE::SetDefaultAlign(WORD wNewAlign)
{
	m_pfDefault.wAlignment = wNewAlign;

	// Notify control of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

BOOL CWindowlessRE::GetRichTextFlag()
{
	return m_bRich;
}

void CWindowlessRE::SetRichTextFlag(BOOL bNew)
{
	m_bRich = bNew;

	// Notify control of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_RICHTEXT, 
		m_bRich ? TXTBIT_RICHTEXT : 0);
}

LONG CWindowlessRE::GetDefaultLeftIndent()
{
	return m_pfDefault.dxOffset;
}


void CWindowlessRE::SetDefaultLeftIndent(LONG lNewIndent)
{
	m_pfDefault.dxOffset = lNewIndent;

	// Notify control of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
}

void CWindowlessRE::SetMaxTextLength(int nLength)
{
	m_lMaxTextLen = nLength;
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
}

int  CWindowlessRE::GetMaxTextLength()
{
	return m_lMaxTextLen;
}

void CWindowlessRE::SetClientRect(RECT *prc, BOOL bUpdateExtent) 
{
	// If the extent matches the client rect then we assume the extent should follow
	// the client rect.
	LONG lTestExt = DYtoHimetricY(
		(m_rcClient.bottom - m_rcClient.top)  - 2 * HOST_BORDER, m_yPerInch);

	if (bUpdateExtent && (m_sizelExtent.cy == lTestExt))
	{
		m_sizelExtent.cy = DXtoHimetricX((prc->bottom - prc->top) - 2 * HOST_BORDER, 
			m_xPerInch);
		m_sizelExtent.cx = DYtoHimetricY((prc->right - prc->left) - 2 * HOST_BORDER,
			m_yPerInch);
	}

	m_rcClient = *prc; 
}

BOOL CWindowlessRE::SetSaveSelection(BOOL bSaveSelection)
{
	BOOL bResult = bSaveSelection;
	m_bSaveSelection = bSaveSelection;
	// notify text services of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION, m_bSaveSelection ? TXTBIT_SAVESELECTION : 0);
	return bResult;		
}



HRESULT	CWindowlessRE::OnTxInPlaceActivate(LPCRECT prcClient)
{
	if (prcClient)
		m_rcInplace = *prcClient;
	m_bInplaceActive = 1;
	HRESULT hr = m_pTextSrv->OnTxInPlaceActivate(prcClient);
	if (FAILED(hr))
	{
		m_bInplaceActive = 0;
	}
	return hr;
}


HRESULT	CWindowlessRE::OnTxInPlaceDeactivate()
{
	HRESULT hr = m_pTextSrv->OnTxInPlaceDeactivate();
	if (SUCCEEDED(hr))
	{
		m_bInplaceActive = 0;
	}

	return hr;
}

void CWindowlessRE::OnUIActivate()
{
	if (m_pTextSrv)
		m_pTextSrv->OnTxUIActivate();
}

void CWindowlessRE::OnUIDeactivate()
{
	if (m_pTextSrv)
		m_pTextSrv->OnTxUIDeactivate();
}


BOOL CWindowlessRE::DoSetCursor(RECT *prc, POINT *pt)
{
	RECT rc = prc ? *prc : m_rcClient;

	// Give some space for our border
	rc.top += HOST_BORDER;
	rc.bottom -= HOST_BORDER;
	rc.left += HOST_BORDER;
	rc.right -= HOST_BORDER;

	// Is this in our rectangle?
	if (PtInRect(&rc, *pt))
	{
		RECT *prcClient = (!m_bInplaceActive || prc) ? &rc : NULL;

		HDC hdc = GetDC(m_hWnd);

		m_pTextSrv->OnTxSetCursor(
			DVASPECT_CONTENT,	
			-1,
			NULL,
			NULL,
			hdc,
			NULL,
			prcClient,
			pt->x, 
			pt->y);

		ReleaseDC(m_hWnd, hdc);

		return TRUE;
	}

	return FALSE;
}

void CWindowlessRE::GetControlRect(LPRECT prc)
{
	// Give some space for our border
	prc->top = m_rcClient.top + HOST_BORDER;
	prc->bottom = m_rcClient.bottom - HOST_BORDER;
	prc->left = m_rcClient.left + HOST_BORDER;
	prc->right = m_rcClient.right - HOST_BORDER;
}

void CWindowlessRE::SetTransparent(BOOL bTransparent)
{
	m_bTransparent = bTransparent;

	// notify text services of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
}

LONG CWindowlessRE::SetAccelPos(LONG laccelpos)
{
	LONG laccelposOld = laccelpos;

	m_laccelPos = laccelpos;

	// notify text services of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_SHOWACCELERATOR, 0);

	return laccelposOld;
}

WCHAR CWindowlessRE::SetPasswordChar(WCHAR chPasswordChar)
{
	WCHAR chOldPasswordChar = m_chPasswordChar;

	m_chPasswordChar = chPasswordChar;

	// notify text services of property change
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD,	(m_chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);

	return chOldPasswordChar;
}

void CWindowlessRE::SetDisabled(BOOL bOn)
{
	m_cfDefault.dwMask	  |= CFM_COLOR	   | CFM_DISABLED;
	m_cfDefault.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;

	if (!bOn)
	{
		m_cfDefault.dwEffects &= ~CFE_DISABLED;
	}
	
	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
		TXTBIT_CHARFORMATCHANGE);
}

LONG CWindowlessRE::SetSelBarWidth(LONG lSelBarWidth)
{
	LONG lOldSelBarWidth = m_lSelBarWidth;

	m_lSelBarWidth = lSelBarWidth;

	if (m_lSelBarWidth)
	{
		m_dwStyle |= ES_SELECTIONBAR;
	} else
	{
		m_dwStyle &= (~ES_SELECTIONBAR);
	}

	m_pTextSrv->OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);

	return lOldSelBarWidth;
}

BOOL CWindowlessRE::GetTimerState()
{
	return m_bTimer;
}


HRESULT	CWindowlessRE::TxGetVScroll( LONG *plMin, 
									 LONG *plMax,	
									 LONG *plPos, 
									 LONG *plPage, 
									 BOOL * pfEnabled )
{
	if (m_pTextSrv)
		return m_pTextSrv->TxGetVScroll(plMin, plMax, plPos, plPage, pfEnabled);
	
	*plMin = *plMax = *plPos = *plPage = 0;
	return E_FAIL;
}


//文字操作相关
void CWindowlessRE::SetText(LPCWSTR lpText)
{
	if (m_pTextSrv)
		m_pTextSrv->TxSetText(lpText);
}

char * CWindowlessRE::GetText()
{
	char *szText = NULL;
	if (m_pTextSrv)
	{
		BSTR bstrText;
		if (SUCCEEDED(m_pTextSrv->TxGetText(&bstrText)))
			szText  = _com_util::ConvertBSTRToString(bstrText);
	    SysFreeString(bstrText);
	}
	return szText;
}

#pragma warning(default:4996)