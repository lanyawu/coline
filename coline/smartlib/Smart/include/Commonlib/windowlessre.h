#ifndef __WINDOWLESSRE_H__
#define __WINDOWLESSRE_H__
#include <CommonLib/Types.h>
#include <richedit.h>
#include <textserv.h>
#include <RichOle.h>

#define HOST_BORDER 5

#define LY_PER_INCH   1440

#define WINDOWLESSRE_2
#ifdef WINDOWLESSRE_2
#define  CHARFORMAT_RE CHARFORMAT2
#define  PARAFORMAT_RE PARAFORMAT2  
#endif

EXTERN_C const IID IID_ITextEditControl;

/**************************************************************************
 	TXTEFFECT
 
 	@enum	Defines different background styles control
**************************************************************************/
enum TXTEFFECT {
	TXTEFFECT_NONE = 0,				//@emem	no special backgoround effect
	TXTEFFECT_SUNKEN,				//@emem	draw a "sunken 3-D" look
};


interface ITextEditControl : public IUnknown
{
	virtual LRESULT	TxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) = 0;
};

class IWindowlessRENotify
{
public:
	virtual void Notify(UINT uMsg, void *lpParam) = 0;
	//滚动条相关
	virtual BOOL RE_ShowScrollBar(int nBar, BOOL bShow) = 0;
	virtual BOOL RE_EnableScrollBar(int nSBFlags, int nArrowFlags) = 0;
	virtual BOOL RE_SetScrollRange(int nBar, LONG nMinPos, int nMaxPos, BOOL bRedraw) = 0;
    virtual BOOL RE_SetScrollPos(int nBar, int nPos, BOOL bRedraw) = 0;
	virtual void RE_TxScrollWindowEx (int dx, int dy, LPCRECT lprcScroll, LPCRECT lprcClip,	
		           HRGN hrgnUpdate, LPRECT lprcUpdate, UINT uScroll) = 0;
	virtual BOOL OnPaste() = 0; //截取键盘粘贴事件
	virtual BOOL OnCut() = 0;   //键盘剪切事件
	virtual BOOL OnCopy() = 0;  //键盘复制事件
	virtual void OnTextChange() = 0; //内容发送改变事件
	virtual BOOL OnEnterKeyDown() = 0; //回车键按下
	virtual void InvalidateRE(LPCRECT lprc, BOOL bMode) = 0; //重绘RichEdit窗口
};

class COMMONLIB_API CRichEditOlePlus: public IRichEditOleCallback
{
public:
	CRichEditOlePlus(IWindowlessRENotify *pNotify);
	~CRichEditOlePlus();
public:
    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR * lplpObj) ;
    STDMETHOD_(ULONG,AddRef) ()  ;
    STDMETHOD_(ULONG,Release) () ;

    // *** IRichEditOleCallback methods ***
	STDMETHOD(GetNewStorage) ( LPSTORAGE FAR * lplpstg) ;
    STDMETHOD(GetInPlaceContext) ( LPOLEINPLACEFRAME FAR * lplpFrame,
								  LPOLEINPLACEUIWINDOW FAR * lplpDoc,
								  LPOLEINPLACEFRAMEINFO lpFrameInfo) ;
	STDMETHOD(ShowContainerUI) (BOOL fShow) ;
	STDMETHOD(QueryInsertObject) (LPCLSID lpclsid, LPSTORAGE lpstg,
									LONG cp) ;
	STDMETHOD(DeleteObject) (LPOLEOBJECT lpoleobj) ;
	STDMETHOD(QueryAcceptData) ( LPDATAOBJECT lpdataobj,
								CLIPFORMAT FAR * lpcfFormat, DWORD reco,
								BOOL fReally, HGLOBAL hMetaPict) ;
	STDMETHOD(ContextSensitiveHelp) ( BOOL fEnterMode) ;
	STDMETHOD(GetClipboardData) ( CHARRANGE FAR * lpchrg, DWORD reco,
									LPDATAOBJECT FAR * lplpdataobj) ;
	STDMETHOD(GetDragDropEffect) ( BOOL fDrag, DWORD grfKeyState,
									LPDWORD pdwEffect) ;
	STDMETHOD(GetContextMenu) ( WORD seltype, LPOLEOBJECT lpoleobj,
									CHARRANGE FAR * lpchrg,
									HMENU FAR * lphmenu) ;
private:
	IWindowlessRENotify *m_pNotify;
};

class COMMONLIB_API  CWindowlessRE: public ITextHost, public ITextEditControl
{
public:
	CWindowlessRE(IWindowlessRENotify *pNotify);
	~CWindowlessRE(void);
public:
	BOOL Init(HWND hParent, HWND hWnd, DWORD dwStyle, DWORD dwExStyle, RECT rcClient);
	//
//	ITextServices * GetTextServices(void);
	void SetClientRect(RECT *prc, BOOL fUpdateExtent = TRUE);
	void GetControlRect(LPRECT prc);

	BOOL GetWordWrap(void);
	void SetWordWrap(BOOL bWordWrap);
	BOOL GetReadOnly();
	void SetReadOnly(BOOL bReadOnly);
	BOOL GetAllowBeep();
	void SetAllowBeep(BOOL bAllowBeep);
	void SetViewInset(RECT *prc);
	WORD GetDefaultAlign();
	void SetDefaultAlign(WORD wNewAlign);
	BOOL GetRichTextFlag();
	void SetRichTextFlag(BOOL bNew);
	LONG GetDefaultLeftIndent();
	void SetDefaultLeftIndent(LONG lNewIndent);
	BOOL SetSaveSelection(BOOL bSaveSelection);
	void SetMaxTextLength(int nLength);
	int  GetMaxTextLength();
	SIZEL *GetExtent(void);
	BOOL GetActiveState(void);
	BOOL DoSetCursor(RECT *prc, POINT *pt);
	void SetTransparent(BOOL fTransparent);
	
	LONG SetAccelPos(LONG laccelpos);
	WCHAR SetPasswordChar(WCHAR chPasswordChar);
	void SetDisabled(BOOL fOn);
	LONG SetSelBarWidth(LONG lSelBarWidth);
	BOOL GetTimerState();
	//wrappers for ITextServices methods
	void OnUIActivate();
	void OnUIDeactivate();
	HRESULT OnTxInPlaceDeactivate();
	HRESULT OnTxInPlaceActivate(LPCRECT prcClient);
	HRESULT	TxGetVScroll( LONG *plMin, LONG *plMax,	LONG *plPos, LONG *plPage, BOOL * pfEnabled );

	static LONG GetXPerInch(void);
	static LONG	GetYPerInch(void);
	
	//文字相关
	void SetText(LPCWSTR lpText);
	//返回的指针需要释放 delete []Text;
	char *GetText();

	//debug
	int  GetInactive() { return m_bInplaceActive;};
	// -----------------------------
	//	IUnknown interface
	// -----------------------------
	virtual HRESULT _stdcall QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG _stdcall AddRef(void);
	virtual ULONG _stdcall Release(void);

	// -----------------------------
	//	ITextEditControl interface
	// -----------------------------
	virtual LRESULT TxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// -----------------------------
	//	ITextHost interface
	// -----------------------------
	virtual HDC TxGetDC();
	virtual INT TxReleaseDC(HDC hdc);
	virtual BOOL TxShowScrollBar(INT nBar, BOOL bShow);
	virtual BOOL TxEnableScrollBar (INT uSBFlags, INT uArrowflags);
	virtual BOOL TxSetScrollRange(INT nBar, LONG nMinPos, INT nMaxPos, BOOL bRedraw);
	virtual BOOL TxSetScrollPos (INT nBar, INT nPos, BOOL bRedraw);
	virtual void TxInvalidateRect(LPCRECT prc, BOOL bMode);
	virtual void TxViewChange(BOOL bUpdate);
	virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
	virtual BOOL TxShowCaret(BOOL bShow);
	virtual BOOL TxSetCaretPos(INT x, INT y);
	virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
	virtual void TxKillTimer(UINT idTimer);
	virtual void TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT uScroll);
	virtual void TxSetCapture(BOOL fCapture);
	virtual void TxSetFocus();
	virtual void TxSetCursor(HCURSOR hcur, BOOL bText);
	virtual BOOL TxScreenToClient (LPPOINT lppt);
	virtual BOOL TxClientToScreen (LPPOINT lppt);
	virtual HRESULT TxActivate( LONG * plOldState );
   	virtual HRESULT TxDeactivate( LONG lNewState );
	virtual HRESULT TxGetClientRect(LPRECT prc);
	virtual HRESULT TxGetViewInset(LPRECT prc);
	virtual HRESULT TxGetCharFormat(const CHARFORMATW **ppCF );
	virtual HRESULT TxGetParaFormat(const PARAFORMAT **ppPF);
	virtual COLORREF TxGetSysColor(int nIndex);
	virtual HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle);
	virtual HRESULT TxGetMaxLength(DWORD *plength);
	virtual HRESULT TxGetScrollBars(DWORD *pdwScrollBar);
	virtual HRESULT TxGetPasswordChar(TCHAR *pch);
	virtual HRESULT TxGetAcceleratorPos(LONG *pcp);
    virtual HRESULT TxGetExtent(LPSIZEL lpExtent);
	virtual HRESULT OnTxCharFormatChange (const CHARFORMATW * pcf);
	virtual HRESULT OnTxParaFormatChange (const PARAFORMAT * ppf);
	virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
	virtual HRESULT TxNotify(DWORD iNotify, void *pv);
	virtual HIMC TxImmGetContext();
	virtual void TxImmReleaseContext(HIMC himc);
	virtual HRESULT TxGetSelectionBarWidth (LONG *lSelBarWidth);

	//
	void PaintRE(HDC hdc, LPRECT lprc, BOOL bUpdateClient, BOOL bDrawSuken, BOOL bEraseBkg = FALSE);
private:
	//debug
	RECT m_rcInplace;
	// helpers
	void *CreateNmhdr(UINT uiCode, LONG cb);
	
	void RevokeDragDrop(void);
	void RegisterDragDrop(void);
	void DrawSunkenBorder(HWND hwnd, HDC hdc);
	VOID  OnSunkenWindowPosChanging(HWND hwnd, WINDOWPOS *pwndpos);
	LRESULT OnSize(HWND hwnd, WORD wSizeType, int nWidth, int nHeight);
	TXTEFFECT TxGetEffects() const;
	HRESULT	OnTxVisibleChange(BOOL bVisible);
	void	SetDefaultInset();
	// Keyboard messages
	LRESULT	OnKeyDown(WORD wKey, DWORD dwFlags);
	LRESULT	OnChar(WORD wKey, DWORD dwFlags);
	// System notifications
	void 	OnSysColorChange();
	LRESULT OnGetDlgCode(WPARAM wparam, LPARAM lparam);
	// Other messages
	LRESULT OnGetOptions() const;
	void	OnSetOptions(WORD wOp, DWORD eco);
	LRESULT	OnGetEventMask() const;
	void    OnSetEventMask(DWORD dwMask);
	void	OnSetReadOnly(BOOL fReadOnly);
	BOOL	OnSetFont(HFONT hFont);
	BOOL	OnSetCharFormat(CHARFORMAT_RE *pcf);
	BOOL	OnSetParaFormat(PARAFORMAT_RE *ppf);
	void	OnGetRect(LPRECT prc);
	void	OnSetRect(LPRECT prc);
private:
	static LONG m_xWidthSys;
    static LONG m_yHeightSys;
	static LONG m_xPerInch;
	static LONG m_yPerInch;
    
	ULONG	m_lRefs;					 // Reference Count
	HWND	m_hWnd;					     // control window
	HWND	m_hParent;			         // parent window
	ITextServices	*m_pTextSrv;		// pointer to Text Services object
    IWindowlessRENotify *m_pNotify;     //通知接口
	CRichEditOlePlus *m_REOlePlus;

	// Properties
	DWORD		m_dwStyle;				// style bits
	DWORD		m_dwExStyle;		    // extended style bits
	unsigned	m_bBorder			:1;	// control has border
	unsigned	m_bCustRect			:1;	// client changed format rect
	unsigned	m_bInBottomless		:1;	// inside bottomless callback
	unsigned	m_bInDialogBox		:1;	// control is in a dialog box
	unsigned	m_bEnableAutoWordSel	:1;	// enable Word style auto word selection?
	unsigned	m_bVertical			:1;	// vertical writing
	unsigned	m_bIconic			:1;	// control window is iconic
	unsigned	m_bHidden			:1;	// control window is hidden
	unsigned	m_bNotSysBkgnd		:1;	// not using system background color
	unsigned	m_bWindowLocked		:1;	// window is locked (no update)
	unsigned	m_bRegisteredForDrop	:1; // whether host has registered for drop
	unsigned	m_bVisible			:1;	// Whether window is visible or not.
	unsigned	m_bResized			:1;	// resized while hidden
	unsigned	m_bWordWrap			:1;	// Whether control should word wrap
	unsigned	m_bAllowBeep		:1;	// Whether beep is allowed
	unsigned	m_bRich				:1;	// Whether control is rich text
	unsigned	m_bSaveSelection	:1;	// Whether to save the selection when inactive
	unsigned	m_bInplaceActive	:1; // Whether control is inplace active
	unsigned	m_bTransparent		:1; // Whether control is transparent
	unsigned	m_bTimer			:1;	// A timer is set
	LONG		m_lSelBarWidth;			// Width of the selection bar
	COLORREF 	m_crBackground;			// background color
	LONG  		m_lMaxTextLen;			// maximum text size
	DWORD		m_dwEventMask;			// Event mask to pass on to parent window
	RECT		m_rcClient;				// Client Rect for this control
    RECT        m_rcViewInset;           // view rect inset 
	SIZEL		m_sizelExtent;			// Extent array
	CHARFORMAT_RE	m_cfDefault;			// Default character format
	PARAFORMAT_RE	m_pfDefault;			// Default paragraph format
	LONG		m_laccelPos;			// Accelerator position
	WCHAR		m_chPasswordChar;		// Password character	
	HBRUSH      m_hBkg;
};

#endif