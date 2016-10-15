#if !defined(AFX_UICONTROLS_H__20050423_DB94_1D69_A896_0080AD509054__INCLUDED_)
#define AFX_UICONTROLS_H__20050423_DB94_1D69_A896_0080AD509054__INCLUDED_

#pragma once

#include <UILib/UIBase.h>
#include <UILib/uiresource.h>
#include <map>
/////////////////////////////////////////////////////////////////////////////////////
//

class CControlUI;

/////////////////////////////////////////////////////////////////////////////////////
//

typedef enum EVENTTYPE_UI
{
   UIEVENT__FIRST = 0,
   UIEVENT_MOUSEMOVE,
   UIEVENT_MOUSELEAVE,
   UIEVENT_MOUSEENTER,
   UIEVENT_MOUSEHOVER,
   UIEVENT_DRAG,
   UIEVENT_DRAGEND,
   UIEVENT_KEYDOWN,
   UIEVENT_KEYUP,
   UIEVENT_CHAR,
   UIEVENT_SYSKEY,
   UIEVENT_KILLFOCUS,
   UIEVENT_SETFOCUS,
   UIEVENT_BUTTONDOWN,
   UIEVENT_BUTTONUP,
   UIEVENT_RBUTTONDOWN,
   UIEVENT_RBUTTONUP,
   UIEVENT_DBLCLICK,
   UIEVENT_CONTEXTMENU,
   UIEVENT_VSCROLL,
   UIEVENT_HSCROLL,
   UIEVENT_SCROLLWHEEL,
   UIEVENT_WINDOWSIZE,
   UIEVENT_SETCURSOR,
   UIEVENT_MEASUREITEM,
   UIEVENT_DRAWITEM,
   UIEVENT_TIMER,
   UIEVENT_NOTIFY,
   UIEVENT_COMMAND,
   UIEVENT_LINKNOTIFY,
   UIEVENT_WINDOWPOSCHANGING,
   UIEVENT__LAST
};

#define UIEVENT__MOUSEBEGIN UIEVENT_MOUSEMOVE
#define UIEVENT__MOUSEEND   UIEVENT_MOUSEHOVER
#define UIEVENT__KEYBEGIN   UIEVENT_KEYDOWN
#define UIEVENT__KEYEND     UIEVENT_SYSKEY
typedef enum
{
   UIFONT__FIRST = 0,
   UIFONT_NORMAL,
   UIFONT_BOLD,
   UIFONT_CAPTION,
   UIFONT_MENU,
   UIFONT_LINK,
   UIFONT_TITLE,
   UIFONT_HEADLINE,
   UIFONT_SUBSCRIPT,
   UIFONT__LAST,
} UITYPE_FONT;

typedef enum
{
   UICOLOR__FIRST = 0,
   UICOLOR_WINDOW_BACKGROUND,
   UICOLOR_WINDOW_TEXT,
   UICOLOR_DIALOG_BACKGROUND,
   UICOLOR_DIALOG_TEXT_NORMAL,
   UICOLOR_DIALOG_TEXT_DARK,
   UICOLOR_MENU_BACKGROUND,
   UICOLOR_MENU_TEXT_NORMAL,
   UICOLOR_MENU_TEXT_HOVER,
   UICOLOR_MENU_TEXT_SELECTED,
   UICOLOR_TEXTCOLOR_NORMAL,
   UICOLOR_TEXTCOLOR_HOVER,
   UICOLOR_TAB_BACKGROUND_NORMAL,
   UICOLOR_TAB_BACKGROUND_SELECTED,
   UICOLOR_TAB_FOLDER_NORMAL,
   UICOLOR_TAB_FOLDER_SELECTED,
   UICOLOR_TAB_BORDER,
   UICOLOR_TAB_TEXT_NORMAL,
   UICOLOR_TAB_TEXT_SELECTED,
   UICOLOR_TAB_TEXT_DISABLED,
   UICOLOR_TAB_BUTTON_BGNORMAL,//tab页标按钮的背景
   UICOLOR_TAB_BUTTON_BGHOVER,//tab页标按钮的背景
   UICOLOR_TAB_BUTTON_BGPUSHED,//tab页标按钮的背景
   UICOLOR_TAB_BUTTON_BGSELECTED,//tab页标按钮的背景
   UICOLOR_NAVIGATOR_BACKGROUND,
   UICOLOR_NAVIGATOR_BUTTON_HOVER,
   UICOLOR_NAVIGATOR_BUTTON_PUSHED,
   UICOLOR_NAVIGATOR_BUTTON_SELECTED,
   UICOLOR_NAVIGATOR_BORDER_NORMAL,
   UICOLOR_NAVIGATOR_BORDER_SELECTED,
   UICOLOR_NAVIGATOR_TEXT_NORMAL,
   UICOLOR_NAVIGATOR_TEXT_SELECTED,
   UICOLOR_NAVIGATOR_TEXT_PUSHED,
   UICOLOR_BUTTON_BACKGROUND_NORMAL,
   UICOLOR_BUTTON_BACKGROUND_DISABLED,
   UICOLOR_BUTTON_BACKGROUND_PUSHED,
   UICOLOR_BUTTON_TEXT_NORMAL,
   UICOLOR_BUTTON_TEXT_PUSHED,
   UICOLOR_BUTTON_TEXT_DISABLED,
   UICOLOR_BUTTON_BORDER_LIGHT,
   UICOLOR_BUTTON_BORDER_DARK,
   UICOLOR_BUTTON_BORDER_DISABLED,
   UICOLOR_BUTTON_BORDER_FOCUS,
   UICOLOR_CONTROL_BACKGROUND_NORMAL,
   UICOLOR_CONTROL_BACKGROUND_SELECTED,
   UICOLOR_CONTROL_BACKGROUND_DISABLED,
   UICOLOR_CONTROL_BACKGROUND_READONLY,
   UICOLOR_CONTROL_BACKGROUND_HOVER,
   UICOLOR_CONTROL_BACKGROUND_SORTED,
   UICOLOR_CONTROL_BACKGROUND_EXPANDED,
   UICOLOR_CONTROL_BORDER_NORMAL,
   UICOLOR_CONTROL_BORDER_SELECTED,
   UICOLOR_CONTROL_BORDER_DISABLED,
   UICOLOR_CONTROL_TEXT_NORMAL,
   UICOLOR_CONTROL_TEXT_SELECTED,
   UICOLOR_CONTROL_TEXT_DISABLED,
   UICOLOR_CONTROL_TEXT_READONLY,
   UICOLOR_TOOL_BACKGROUND_NORMAL,
   UICOLOR_TOOL_BACKGROUND_DISABLED,
   UICOLOR_TOOL_BACKGROUND_HOVER,
   UICOLOR_TOOL_BACKGROUND_PUSHED,
   UICOLOR_TOOL_BORDER_NORMAL,
   UICOLOR_TOOL_BORDER_DISABLED,
   UICOLOR_TOOL_BORDER_HOVER,
   UICOLOR_TOOL_BORDER_PUSHED,
   UICOLOR_EDIT_BACKGROUND_NORMAL,
   UICOLOR_EDIT_BACKGROUND_HOVER,
   UICOLOR_EDIT_BACKGROUND_DISABLED,
   UICOLOR_EDIT_BACKGROUND_READONLY,
   UICOLOR_EDIT_TEXT_NORMAL,
   UICOLOR_EDIT_TEXT_DISABLED,
   UICOLOR_EDIT_TEXT_READONLY,
   UICOLOR_EDIT_TEXT_TIPTEXT,//edit中的默认提示文字
   UICOLOR_TITLE_BACKGROUND,
   UICOLOR_TITLE_TEXT,
   UICOLOR_TITLE_BORDER_LIGHT,
   UICOLOR_TITLE_BORDER_DARK,
   UICOLOR_HEADER_BACKGROUND,
   UICOLOR_HEADER_BORDER,
   UICOLOR_HEADER_SEPARATOR,
   UICOLOR_HEADER_TEXT,
   UICOLOR_TASK_BACKGROUND,
   UICOLOR_TASK_CAPTION,
   UICOLOR_TASK_BORDER,
   UICOLOR_TASK_TEXT,
   UICOLOR_LINK_TEXT_HOVER,
   UICOLOR_LINK_TEXT_NORMAL,
   UICOLOR_STANDARD_BLACK,
   UICOLOR_STANDARD_YELLOW,
   UICOLOR_STANDARD_RED,
   UICOLOR_STANDARD_GREY,
   UICOLOR_STANDARD_LIGHTGREY,
   UICOLOR_STANDARD_WHITE,
   UICOLOR_STANDARD_FRAME_BORDER,
   UICOLOR_TOOLBAR_BORDER,
   UICOLOR_TOOLBAR_BACKGROUND,
   UICOLOR_GIFGRIDPANEL_SEPLINE,
   UICOLOR_SCROLLBAR_BACK,
   UICOLOR__LAST,
   UICOLOR__INVALID,
} UITYPE_COLOR;

const UINT IMGID_INVALID_ = 0xFFFFFFFF;

//string having "#abcd" format can be converted to COLORREF value
inline
COLORREF 
StringToColor( LPCTSTR pstrValue )
{
	COLORREF clr = 0;
	if( *pstrValue == '#' ){
		pstrValue++;
		COLORREF clrColor = _tcstol( pstrValue, const_cast<LPTSTR*>(&pstrValue), 16 );
		clr = RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor));
	}
	return clr;
}
/////////////////////////////////////////////////////////////////////////////////////
//

// Styles for the DoPaintArcCaption() helper
#define UIARC_GRIPPER        0x00000001
#define UIARC_EXPANDBUTTON   0x00000002
#define UIARC_COLLAPSEBUTTON 0x00000004

// Flags for CControlUI::GetControlFlags()
#define UIFLAG_TABSTOP       0x00000001
#define UIFLAG_SETCURSOR     0x00000002
#define UIFLAG_WANTRETURN    0x00000004
#define UIFLAG_FOCUS		 0x00000008

// Flags for FindControl()
#define UIFIND_ALL           0x00000000
#define UIFIND_VISIBLE       0x00000001
#define UIFIND_ENABLED       0x00000002
#define UIFIND_HITTEST       0x00000004
#define UIFIND_ME_FIRST      0x80000000

// Flags for Draw Style
#define UIDRAWSTYLE_INPLACE  0x00000001
#define UIDRAWSTYLE_FOCUS    0x00000002

// Flags for DoAnumateWindow()
#define UIANIM_FADE          0x00000001
#define UIANIM_HIDE          0x00000002

// Flags for the CDialogLayout stretching
#define UISTRETCH_NEWGROUP   0x00000001
#define UISTRETCH_NEWLINE    0x00000002
#define UISTRETCH_MOVE_X     0x00000004
#define UISTRETCH_MOVE_Y     0x00000008
#define UISTRETCH_SIZE_X     0x00000010
#define UISTRETCH_SIZE_Y     0x00000020

// Flags used for controlling the paint
#define UISTATE_FOCUSED      0x00000001
#define UISTATE_SELECTED     0x00000002
#define UISTATE_DISABLED     0x00000004
#define UISTATE_HOT          0x00000008
#define UISTATE_PUSHED       0x00000010
#define UISTATE_CHECKED      0x00000020
#define UISTATE_READONLY     0x00000040
#define UISTATE_CAPTURED     0x00000080
// Styles for the DoPaintFrame() helper
#define UIFRAME_ROUND        0x00000100
#define UIFRAME_FOCUS        0x00000200

/////////////////////////////////////////////////////////////////////////////////////
//
typedef enum{
	TABALIGN_TOP = 1,
	TABALIGN_LEFT,
	TABALIGN_RIGHT,
	TABALIGN_BOTTOM,
	TABALIGN_HIDDEN,
	TABALIGN__INVALID
}TABALIGN;

/////////////////////////////////////////////////////////////////////////////////////
//
// Structure for notifications from the system
// to the control implementation.
typedef struct tagTEventUI
{
   int Type;
   CControlUI* pSender;
   DWORD dwTimestamp;
   POINT ptMouse;
   TCHAR chKey;
   WORD wKeyState;
   WPARAM wParam;
   LPARAM lParam;
} TEventUI;

// Structure for notifications to the outside world
typedef struct tagTNotifyUI 
{
	tagTNotifyUI() : 
		pSender( NULL ), 
		dwTimestamp( 0 ), 
		wParam( 0 ), 
		lParam( 0 ), 
		bHandled( FALSE ){}
	CStdString sType;
	CControlUI* pSender;
	DWORD dwTimestamp;
	POINT ptMouse;
	WPARAM wParam;
	LPARAM lParam;
	BOOL	bHandled;
} TNotifyUI;

// Structure for adding alpha bitmaps on top of the window
typedef struct tagTPostPaintUI
{
   HBITMAP hBitmap;
   RECT rc;
   BYTE iAlpha;
} TPostPaintUI;

// System settings
typedef struct tagTSystemSettingsUI
{
   bool bShowKeyboardCues;
   bool bScrollLists;
} TSystemSettingsUI;

// Various system settings
typedef struct tagTSystemMetricsUI
{
   INT cxvscroll;
} TSystemMetricsUI;

// Listener interface
class INotifyUI
{
public:
   virtual void Notify(TNotifyUI& msg) = 0;
};

// MessageFilter interface
class IMessageFilterUI
{
public:
   virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CControlGroup
{
public:
	CControlGroup() : m_pCurSel( NULL ){}
	~CControlGroup(){}

	bool Add( CControlUI* pCtrl ){ return m_aControls.Add(pCtrl); }
	int GetCount() const { return m_aControls.GetSize(); }

	CControlUI* GetCurSelUI() const { return m_pCurSel; }
	void SetCurSelUI( CControlUI* pCtrl ) { m_pCurSel = pCtrl; }
private:
	CControlUI* m_pCurSel;
	CStdPtrArray m_aControls;
};
/////////////////////////////////////////////////////////////////////////////////////
//
class CAnimJobUI;
class CMenuUI;
class CImageResourceItem;
class CPaintManagerUI
{
public:
   CPaintManagerUI();
   virtual ~CPaintManagerUI();

public:
   typedef std::map<int, CControlGroup*> MapCtrlGroups;
   typedef MapCtrlGroups::iterator MapCtrlGroupsIt;

   void Init(HWND hWnd);
   void UpdateLayout();
   void SetBkgndColor(COLORREF clr);
   static COLORREF GetBkgndColor();
   void Invalidate(RECT rcItem);

   HDC GetPaintDC() const;
   HWND GetPaintWindow() const;

   POINT GetMousePos() const;
   SIZE GetClientSize() const;

   void SetMinMaxInfo(int cx, int cy);

   static HINSTANCE GetResourceInstance();
   static HINSTANCE GetLanguageInstance();
   static void SetResourceInstance(HINSTANCE hInst);
   static void SetLanguageInstance(HINSTANCE hInst);
   static void ShiftSystemTextColor(BYTE r, BYTE g, BYTE b);
   HPEN GetThemePen(UITYPE_COLOR Index) const;
   HFONT GetThemeFont(UITYPE_FONT Index) const;
   HBRUSH GetThemeBrush(UITYPE_COLOR Index) const;
   COLORREF GetThemeColor(UITYPE_COLOR Index) const;
   static COLORREF GetTransparentColor(UINT index);
   virtual int  GetGraphicLinkImageId(LPCTSTR lpszLink);
   const TEXTMETRIC& GetThemeFontInfo(UITYPE_FONT Index) const;
   bool GetThemeColorPair(UITYPE_COLOR Index, COLORREF& clr1, COLORREF& clr2) const;

   bool AttachDialog(CControlUI* pControl);
   bool InitControls(CControlUI* pControl, CControlUI* pParent = NULL);
   void ReapObjects(CControlUI* pControl);

   CControlUI* GetFocus() const;
   HFONT GetFont(int idx);
   void SetFocus(CControlUI* pControl);

   bool SetNextTabControl(bool bForward = true);
   void SetCapture();
   void ReleaseCapture();
   bool IsCaptured();

   bool SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse);
   bool KillTimer(CControlUI* pControl, UINT nTimerID);

   bool AddNotifier(INotifyUI* pControl);
   bool RemoveNotifier(INotifyUI* pControl);   
   void SendNotify(TNotifyUI& Msg);
   void SendNotify(CControlUI* pControl, LPCTSTR pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0);

   bool AddMessageFilter(IMessageFilterUI* pFilter);
   bool RemoveMessageFilter(IMessageFilterUI* pFilter);

   bool AddAnimJob(const CAnimJobUI& job);
   bool AddPostPaintBlit(const TPostPaintUI& job);

   CControlUI* FindControl(POINT pt) const;
   CControlUI* FindControl(LPCTSTR pstrName);
   BOOL RemoveFromHashName(LPCTSTR pstrName);

   static void MessageLoop();
   static bool TranslateMessage(const LPMSG pMsg);
   static void Terminate();
   static void GetHSL(short *H, short *S, short *L); 
   static void CloseForm(CWindowWnd *pWindow);
   static void SetMainForm(CWindowWnd *pWindow);
   static BOOL CheckMainFormIsExists();
   static void ReInitAppRun();

   bool MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
   bool PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);

   TSystemMetricsUI GetSystemMetrics() const;
   TSystemSettingsUI GetSystemSettings() const;
   void SetSystemSettings(const TSystemSettingsUI Config);
   
   //overridables
   virtual CPaintManagerUI* CreateInstance();
   virtual HWND HintWindow(HWND hParent);
   virtual BOOL GetImage(DWORD dwImageId, LPUI_IMAGE_ITEM *lpImage) const;
   virtual UINT GetMenuCheckImage() const;
   virtual void GetScrollBarImage( UINT& nPrior, UINT& nMid, UINT& nNext, BOOL bVert ) const;
   virtual CMenuUI* LoadMenuUI( LPCWSTR lpszMenu );
   virtual CMenuUI* LoadMenuUI( const std::string& sMenu );
   virtual void ReleaseMenuUI( CMenuUI** pMenu );
   //operations
   CControlGroup* RegisterGroup( int iGroupID );
   CControlGroup* FindGroup( int iGroupID );
   //第一次启动时，是否启用默认focus控件
   void EnableFirstFocus(bool bEnable);
private:
   static CControlUI* CALLBACK __FindControlFromNameHash(CControlUI* pThis, LPVOID pData);
   static CControlUI* CALLBACK __FindControlFromCount(CControlUI* pThis, LPVOID pData);
   static CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData);
   static CControlUI* CALLBACK __FindControlFromTab(CControlUI* pThis, LPVOID pData);
   static CControlUI* CALLBACK __FindControlFromShortcut(CControlUI* pThis, LPVOID pData);

   //
   virtual void DoAlphaTopForm(HDC hdc, const RECT &rcPaint);
   //
   virtual void DoShiftBackground(HDC hdc, RECT rc);
private:
   HWND m_hWndPaint;
   HDC m_hDcPaint;
   HDC m_hDcOffscreen;
   HBITMAP m_hbmpOffscreen;
   HWND m_hwndTooltip;
   TOOLINFO m_ToolTip;
   //
   CControlUI* m_pRoot;
   CControlUI* m_pFocus;
   CControlUI* m_pEventHover;
   CControlUI* m_pEventClick;
   CControlUI* m_pEventKey;
   //
   POINT m_ptLastMousePos;
   SIZE m_szMinWindow;
   UINT m_uMsgMouseWheel;
   UINT m_uTimerID;
   bool m_bFirstLayout;
   bool m_bResizeNeeded;
   bool m_bFocusNeeded;
   bool m_bOffscreenPaint;
   bool m_bMouseTracking;
   bool m_bMouseCapture;
   //
   TSystemMetricsUI m_SystemMetrics;
   TSystemSettingsUI m_SystemConfig;
   //
   CStdPtrArray m_aNotifiers;
   CStdPtrArray m_aNameHash;
   CStdPtrArray m_aTimers;
   CStdValArray m_aPostPaint;
   CStdPtrArray m_aMessageFilters;
   CStdPtrArray m_aDelayedCleanup;
   //
   MapCtrlGroups m_mapCtrlGroups;
   //
   static HINSTANCE m_hLangInst;
   static HINSTANCE m_hInstance;
   static CStdPtrArray m_aPreMessages;
   static BOOL m_bApplicationTerminated;
   static CWindowWnd *m_pMainForm;
   //
   static short m_H, m_S, m_L;
   //debug
   int m_nTotalTime;
   int m_nTimes;
   static COLORREF m_crBkgndColor;
};


/////////////////////////////////////////////////////////////////////////////////////
//

typedef CControlUI* (CALLBACK* FINDCONTROLPROC)(CControlUI*, LPVOID);

class  CControlUI : public INotifyUI
{
public:
   CControlUI();
   virtual ~CControlUI();

public:
   virtual CStdString GetName() const;
   virtual void SetName(LPCTSTR pstrName);
   virtual LPVOID GetInterface(LPCTSTR pstrName);

   virtual bool Activate();
   virtual CControlUI* GetParent() const;

   virtual CStdString GetText() const;
   virtual void SetText(LPCTSTR pstrText);

   virtual CStdString GetToolTip() const;
   virtual void SetToolTip(LPCTSTR pstrText);

   virtual TCHAR GetShortcut() const;
   virtual void SetShortcut(TCHAR ch);

   virtual UINT_PTR GetTag() const;
   virtual void SetTag(UINT_PTR pTag);

   virtual void SetFocus();

   virtual bool IsVisible() const;
   virtual bool IsEnabled() const;
   virtual bool IsFocused() const;
   virtual void SetVisible(bool bVisible);
   virtual void ParentChangeVisible(bool bVisible);
   virtual void SetEnabled(bool bEnable);
   virtual bool HasBorder() const;
   virtual void SetBorder(bool bBorder);
   virtual void SetBorderColor( COLORREF clrBorder );
   virtual void SetBkgndColor( COLORREF clrBkgnd );
   virtual COLORREF GetBorderColor() const { return m_clrBorder; }
   virtual COLORREF GetBkgndColor() const { return m_clrBkgnd; }

   virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

   virtual CPaintManagerUI* GetManager() const;
   virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent);
   COLORREF GetAdjustColor(COLORREF clr);
   virtual RECT GetPos() const;
   virtual void SetPos(RECT rc);
   virtual UINT GetControlFlags() const;
   virtual RECT GetInset() const;
   virtual void SetInset(const RECT&);
   
   void Invalidate();
   void UpdateLayout();

   virtual CMenuUI *GetPopMenu();
   virtual void Init();
   virtual void Event(TEventUI& event);
   virtual void Notify(TNotifyUI& msg);

   virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
   virtual BOOL GetAttribute(LPCTSTR pstrName, TCHAR *szValue, int &nMaxValueSize);
   CControlUI* ApplyAttributeList(LPCTSTR pstrList);
   int GetWidth() { return m_cxyFixed.cx;};
   int GetHeight() { return m_cxyFixed.cy;};
   virtual void SetWidth(int nW);
   virtual void SetHeight(int nH);

   virtual LPCTSTR GetClass() const = 0;
   virtual SIZE EstimateSize(SIZE szAvailable) = 0;
   virtual void DoPaint(HDC hDC, const RECT& rcPaint);
   void DoPaintBorder(HDC hDC, const RECT &rcPaint);
   static  UINT GetMsgTypeFromEvent(int nType);
   static  UINT GetMsgTypeFromEventName(LPCTSTR lpszType);
protected:
   CPaintManagerUI* m_pManager;
   CControlUI* m_pParent;
   TCHAR m_chShortcut;
   CStdString m_sName;
   CStdString m_sText;
   CStdString m_PopMenuName;
   CMenuUI *m_pPopMenu;
   CStdString m_sToolTip;
   UINT_PTR m_pTag; 
   SIZE m_cxyFixed;
   RECT m_rcItem;
   RECT m_rcInset;
   RECT m_rcPaint;
   bool m_bVisible;
   bool m_bEnabled;
   bool m_bFocused;
   bool m_bBorder;
   //Border
   int  m_nBorderImageId;
   RECT m_rcBorderCorner;
   BOOL m_bBorderHole;
   COLORREF m_clrBorder;
   COLORREF m_clrBkgnd;
   BOOL m_bTransparent;
   BOOL m_bColorHSL;
};

#endif // !defined(AFX_UICONTROLS_H__20050423_DB94_1D69_A896_0080AD509054__INCLUDED_)

