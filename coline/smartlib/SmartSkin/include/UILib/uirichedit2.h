#ifndef __UIRICHEDIT2_H__
#define __UIRICHEDIT2_H__

#include <CommonLib/Types.h>
#include <UILib/UILib.h>

// Edit Class Name
#define WC_RICHEDITA                "RICHEDIT"
#define WC_RICHEDITW                _T("RICHEDIT")

#ifdef UNICODE
#define WC_RICHEDIT                 WC_RICHEDITW
#else
#define WC_RICHEDIT                 WC_RICHEDITA
#endif

#define  DEFAULT_INDENT_CONTENT   20  //缩进量

//RichEdit 消息定义
const DWORD EM_EXGETSEL      = WM_USER + 52;
const DWORD EM_EXSETSEL      = WM_USER + 55;
const DWORD EM_GETCHARFORMAT = WM_USER + 58;
const DWORD EM_GETEVENTMASK  = WM_USER + 59;
const DWORD EM_SETCHARFORMAT = WM_USER + 68;
const DWORD EM_SETEVENTMASK  = WM_USER + 69;
const DWORD EM_AUTOURLDETECT = WM_USER + 91;
//{ Richedit v2.0 messages }

//RichEdit Event
const DWORD  ENM_NONE                            = 0x00000000;
const DWORD  ENM_CHANGE                          = 0x00000001;
const DWORD  ENM_UPDATE                          = 0x00000002;
const DWORD  ENM_SCROLL                          = 0x00000004; 
const DWORD  ENM_KEYEVENTS                       = 0x00010000; 
const DWORD  ENM_MOUSEEVENTS                     = 0x00020000; 
const DWORD  ENM_REQUESTRESIZE                   = 0x00040000; 
const DWORD  ENM_SELCHANGE                       = 0x00080000; 
const DWORD  ENM_DROPFILES                       = 0x00100000; 
const DWORD  ENM_PROTECTED                       = 0x00200000; 
const DWORD  ENM_CORRECTTEXT                     = 0x00400000;            //  { PenWin specific }
const DWORD  ENM_SCROLLEVENTS                    = 0x00000008; 
const DWORD  ENM_DRAGDROPDONE                    = 0x00000010; 

//{ Far East specific notification mask }

const DWORD  ENM_IMECHANGE                       = 0x00800000;            //  { unused by RE2.0 }
const DWORD  ENM_LANGCHANGE                      = 0x01000000; 
const DWORD  ENM_OBJECTPOSITIONS                 = 0x02000000; 
const DWORD  ENM_LINK                            = 0x04000000;

const DWORD  MAX_TEXT_FACENAME_SIZE               = 32;

//#pragma pack(push)
//#pragma pack(1)
typedef struct __CharFormatA
{
	UINT cbSize;
	DWORD dwMask;
	DWORD dwEffects;
    DWORD dwHeight;
	DWORD dwOffset;
    DWORD dwTextColor;
    BYTE  bCharSet;
	BYTE  bPitchAndFamily;
	TCHAR  szFaceName[MAX_TEXT_FACENAME_SIZE];
}CCharFormat, *LPCharFormat;

typedef struct __CharFontStyle
{
	int nFontSize;
	int nFontStyle;
	COLORREF cfColor;
	TCHAR szFaceName[MAX_TEXT_FACENAME_SIZE];
}CCharFontStyle, *LPCCharFontStyle;

typedef struct __charrange
{
    DWORD dwMin;
    DWORD dwMax;
}CCharRange, *LPCharRange;

//#pragma pack(pop)

class CRichEdit2Wnd;
class  CRichEdit2UI :public CControlUI
{
	friend class CRichEdit2Wnd;
public:
	CRichEdit2UI();
	~CRichEdit2UI();
	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	void Init();

	CStdString GetText() const;
	void SetText(LPCTSTR pstrText);

   void SetEnabled(bool bEnabled);
   void SetVisible(bool bVisible);
   void SetReadOnly(bool bReadOnly);
   void SetFocus(){};
   bool IsReadOnly() const;
   void SetTransParent(bool b);
   void Event(TEventUI& event);
   void Notify(TNotifyUI& msg) {};
   SIZE EstimateSize(SIZE szAvailable);
   void SetPos(RECT rc);
   void SetPos(int left, int top, int right, int bottom);
   void DoPaint(HDC hDC, const RECT& rcPaint);

   static HMODULE m_hRichEdit2Module;
   static volatile LONG  m_hRichEditRef;
public:
	//RichEdit2 参数相关
	DWORD GetSelLength();
	DWORD GetSelStart();
	void SetSelLength(DWORD dwLength);
	void SetSelStart(DWORD dwStart);

	void SetAutioDetect(bool bIsAutio);

	//扩展性能 
	//加入一个聊天记录 szUserName, 显示的用户名或者昵称 szTime--时间 szText --内容 cfStyle- 字体
    BOOL AddChatText(DWORD dwUserId, char *szUserName, char *szTime, char *szText, CCharFontStyle &cfStyle);
	//虚函数 继承类实现 szTitle 选取的文本 bIsCustom 是否是自定义链接 dwFlag 自定义链接标志
    BOOL LinkOnClick(char *szTitle, BOOL bIsCustom, DWORD dwFlag);
	BOOL GetInfoIcon(DWORD dwIconFlag, char *szFileName); //获取图标文件路径
private:
    void CharFontStyleToFormat(CCharFontStyle cfStyle, CCharFormat &cfFormat);
protected:
	CRichEdit2Wnd *m_pWindow; 
	CCharFontStyle m_fcDefaultStyle; //默认用于显示用户名的样式
};


#endif