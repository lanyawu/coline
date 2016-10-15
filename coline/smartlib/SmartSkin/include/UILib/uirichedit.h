#ifndef __UIRICHEDIT_H__20081104__
#define __UIRICHEDIT_H__20081104__

#include <UILib/UILib.h>
#include <CommonLib/SystemUtils.h>
#include <CommonLib/WindowlessRE.h>
#include <CommonLib/GuardLock.h>
#include <UILib/uiresource.h>
#include <SmartSkin/SmartSkin.h>
#include <map>


typedef void (* PNOTIFY_CALL)(int iNotify);
#define  DEFAULT_INDENT_CONTENT   20  //缩进量

//RichEdit 消息定义
#define EM_EXGETSEL       (WM_USER + 52)
#define EM_EXSETSEL       (WM_USER + 55)
#define EM_GETCHARFORMAT  (WM_USER + 58)
#define EM_GETEVENTMASK   (WM_USER + 59)
#define EM_SETCHARFORMAT  (WM_USER + 68)
#define EM_SETEVENTMASK   (WM_USER + 69)
#define EM_AUTOURLDETECT  (WM_USER + 91)
//{ Richedit v2.0 messages }

//RichEdit Event
#define  ENM_NONE                             0x00000000
#define  ENM_CHANGE                           0x00000001
#define  ENM_UPDATE                           0x00000002
#define  ENM_SCROLL                           0x00000004
#define  ENM_KEYEVENTS                        0x00010000 
#define  ENM_MOUSEEVENTS                      0x00020000 
#define  ENM_REQUESTRESIZE                    0x00040000 
#define  ENM_SELCHANGE                        0x00080000 
#define  ENM_DROPFILES                        0x00100000 
#define  ENM_PROTECTED                        0x00200000 
#define  ENM_CORRECTTEXT                      0x00400000            //  { PenWin specific }
#define  ENM_SCROLLEVENTS                     0x00000008 
#define  ENM_DRAGDROPDONE                     0x00000010 

//{ Far East specific notification mask }

#define  ENM_IMECHANGE                        0x00800000            //  { unused by RE2.0 }
#define  ENM_LANGCHANGE                       0x01000000 
#define  ENM_OBJECTPOSITIONS                  0x02000000 
#define  ENM_LINK                             0x04000000

#define  MAX_OLE_FLAG_SIZE  36

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

typedef struct __CharFormat2w
{
	WORD		wWeight;			// Font weight (LOGFONT value)
	SHORT		sSpacing;			// Amount to space between letters
	COLORREF	crBackColor;		// Background color
	LCID		lcid;				// Locale ID
	DWORD		dwReserved;			// Reserved. Must be 0
	SHORT		sStyle;				// Style handle
	WORD		wKerning;			// Twip size above which to kern char pair
	BYTE		bUnderlineType;		// Underline type
	BYTE		bAnimation;			// Animated text like marching ants
	BYTE		bRevAuthor;			// Revision author index
}CCharFormat2, *LPCharFormat2;
typedef struct __charrange
{
    DWORD dwMin;
    DWORD dwMax;
}CCharRange, *LPCharRange;

typedef struct __CustomLinkItem
{
	CCharRange crg;
	DWORD dwFlag;
}CCustomLinkItem, *LPCustomLinkItem;

//选择状态
typedef enum
{
	RE_SELECT_STATUS_EMPTY = 0, //无选择
	RE_SELECT_STATUS_TEXT, //文本
	RE_SELECT_STATUS_PICTURE //图片
}RE_SELECT_STATUS;

//richedit 应用程序的接口
class IRichEditApp
{
public:
	virtual ~IRichEditApp() {};
	//自定义链接click事件
	virtual void OnCustomLinkClick(char *szText, DWORD dwFlag) = 0;
	//获取自定义图片文件名称
	virtual bool GetCustomPicFile(char *szFileName, char *szTag) = 0;
	//根据TAG获取文件名称 粘贴时保存文件名时使用
	virtual bool GetFileNameByTag(char *szFileName, char *szTag) = 0;
	//获取提示信息前端图标文件
	virtual bool GetTipPicFileName(char *szFileName) = 0; //
	//粘贴文件时，发送文件
	virtual void SendFile(char *szFileName) = 0;
};

typedef struct COleItem
{
	char szFileName[MAX_PATH];
	char szFlag[MAX_OLE_FLAG_SIZE];
}OLE_ITEM, *LPOLE_ITEM;

class CRichEditUI :	public CContainerUI, public IWindowlessRENotify, public IMessageFilterUI
{
public:	
	CRichEditUI(void);
	~CRichEditUI(void);
public:
	//IMessageFilterUI
	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
	//CControlUI虚函数
	LPCTSTR GetClass() const;
	UINT GetControlFlags() const;

	void Init();
	void Attach(IRichEditApp *pApp);
	void Attach(LPSKIN_RICHEDIT_EVENT_CALLBACK pCallback, LPVOID lpOverlapped);

	CStdString GetText() const;
	void SetText(LPCTSTR pstrText);

	void SetEnabled(bool bEnabled);
	void SetVisible(bool bVisible);
	void SetReadOnly(BOOL bReadOnly);
	void SetFocus();
	BOOL IsReadOnly() const;
	void SetTransParent(bool b);
	void Event(TEventUI& event);
	void Notify(TNotifyUI& msg);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	SIZE EstimateSize(SIZE szAvailable);
	bool GetReadOnly();
	void SetPos(RECT rc);
	void DoPaint(HDC hDC, const RECT& rcPaint);
	void Clear();
	int  GetOleText(char *szText, int nMaxLen); //转换后的ole文字串
	void SelectAll(); //全选
    //获取richedit中字数
	int  GetCount();
    //取消一个自定义链接
	void CancelCustomLink(DWORD dwFlag);
	//替换一个图片
	void ReplaceOleObj(const char *szTag, const char *szNewFileName);

	//RichEdit 应用相关 szTag 标志 dwPos 插入位置，文件结果填0
	void InsertGif(const char *lpszFileName, HBITMAP hBitmap, const char *szTag, const int dwPos);
	void InsertImage(char *lpszFileName, char *szTag, DWORD dwPos);
    void InsertBitmap(HBITMAP hBitmap);
	BOOL InsertOlePicture(const char *szFileName);
	//
	void TestNewLine(); //测试是否另起一行
	//
	BOOL InsertText(CHARFORMAT_RE &cfFormat, DWORD dwOffset, char *szText);
	BOOL InsertUnicodeText(CHARFORMAT_RE &cfFormat, DWORD dwOffset, TCHAR *szText);
	BOOL InsertOleText(CHARFORMAT_RE &cfFormat, DWORD dwOffset, const char *szText, BOOL bBullet);
	void Append(CHARFORMAT_RE &cfFormat, const TCHAR *szText); //追加内容
	void Append(const TCHAR *szText); //追加内容
	void AppendCustomLink(const TCHAR *szText, DWORD dwFlag);
	BOOL InsertTip(CCharFontStyle *cfStyle, DWORD dwOffset, const TCHAR *szText);
	//szTip --->发送文件 "%FILE%" 成功
	BOOL InsertFileLink(CCharFontStyle *cfStyle, DWORD dwOffset, const char *szTip, const char *szFileName);
	void SetAutoDetectLink(bool bAuto);  //自动检测超链接 
	BOOL GetSelectImageFileName(char *szFileName, int &nSize);
	RE_SELECT_STATUS GetSelected(); //获取选择的状态
	//虚函数 IWindowlessNotify
	//链接点击事件
	virtual BOOL LinkOnClick(char *szTitle, BOOL bIsCustom, DWORD dwFlag);
	virtual BOOL RE_ShowScrollBar(int nBar, BOOL bShow);
	virtual BOOL RE_EnableScrollBar(int nSBFlags, int nArrowFlags);
	virtual BOOL RE_SetScrollRange(int nBar, LONG nMinPos, int nMaxPos, BOOL bRedraw);
    virtual BOOL RE_SetScrollPos(int nBar, int nPos, BOOL bRedraw);
    virtual void RE_TxScrollWindowEx (int dx, int dy, LPCRECT lprcScroll, LPCRECT lprcClip,	
		           HRGN hrgnUpdate, LPRECT lprcUpdate, UINT uScroll);
    virtual void InvalidateRE(LPCRECT lprc, BOOL bMode);
	virtual BOOL OnEnterKeyDown();
	virtual BOOL OnPaste();
	//复制粘贴相关
	virtual BOOL OnCut();
	virtual BOOL OnCopy();
	//
	BOOL OnSaveAs(HINSTANCE hInstance, HWND hParent);

	//内容发送改变事件
	virtual void OnTextChange(); 
	//获取图标文件路径
	virtual BOOL GetInfoIcon(DWORD dwIconFlag, char *szFileName); 
	//获取所有插入到richedit 内的OLE对象flag
	BOOL GetOleFlags(CStringList_ &Flags);
	//加入一个聊天记录 szUserName, 显示的用户名或者昵称 szTime--时间 szText --内容 cfStyle- 字体
	BOOL AddChatText(const char *szId, const DWORD dwUserId, const char *szUserName, const char *szTime, 
		             const char *szText, const CCharFontStyle &cfStyle, 
					 const int nNickColor, BOOL bIsUTF8 = FALSE, BOOL bAck = FALSE);
	BOOL RichEditCommand_(const char *szCommand, LPVOID lpParams);
	//设置字体
	void SetFontStyle(CCharFontStyle &cfStyle);
	void GetFontStyle(CCharFontStyle &cfStyle);
	//设置最大字符数
	void SetMaxTextLen(int nLength);
	LRESULT PerformMessage(UINT uMsg, WPARAM wParam, LPARAM);


	BOOL GetRichText(DWORD dwStyle, char **pBuf, int &nSize);
	char *GetOleText(DWORD dwStyle);
	void SetRichText(const char *szText, DWORD dwStyle);
	//
	BOOL  GetCurrentChatId(char *szId);
	BOOL  ClearChatMsg(const char *szId);
private:
	void  SetBackGroundColor(COLORREF clr, CCharRange cr);
	void  SetBullet(BOOL bBullet);
	//RichEdit相关辅助函数
	void  SetLeftIndent(DWORD dwOffset, BOOL bBullet = FALSE);
	DWORD GetCustomLinkFlag(DWORD dwMin);
	BOOL  GetCustomAckLink(DWORD dwMin, CCharRange &cr);

	void CharFontStyleToFormat(const CCharFontStyle &cfStyle, CHARFORMAT_RE &cfFormat);
	void CharFormatToStyle(const CHARFORMAT_RE &cfFormat, CCharFontStyle &cfStyle);
	
	//根据oleId获取tag串 bIsFlag 表示是否加上标志串
	bool GetTagFromId(char *szTag, int nOleId, BOOL bIsFlag);
	//解析ole字串
	void ParserOleStr(char *szText);
    //滚动到底部
	void ScrollToBottom();
    //根据位置获取ole的userid
	DWORD GetUserIdByPosition(DWORD dwPos);
	//
	int InsertOleToString(const char *szTemp, char *szText, int nMaxLen);
	//
	void AppendAckLink(CCharRange &cr);
	//
	BOOL GetFileNameLink(DWORD dwMin, std::string &strFileName);
public:
	void Notify(UINT uMsg, void *lpParam);
private:
	//私有变量相关
	CWindowlessRE *m_pWindowlessRE;
	//应用程序接口
	IRichEditApp *m_pApp;
	LPVOID m_lpOverlapped;
	LPSKIN_RICHEDIT_EVENT_CALLBACK m_pCallBack;
	//
    static HMODULE m_hRichEdit2Module;
    static volatile LONG  m_hRichEditRef;
	//RichEdit 应用相关
	std::map<DWORD, LPCustomLinkItem> m_CustomLinkList;  //自定义连接参数
	//自定义的回执
	std::map<DWORD, CCharRange> m_AckLinkList; //
	//打开文件
	std::map<DWORD, std::string> m_FileList;
	CGuardLock m_LinkLock;
	std::map<DWORD, LPOLE_ITEM> m_OleList; //OLe列表
	DWORD m_dwOleSeq; //Ole编号
	CGuardLock m_OleLock; //OLE列表的锁
	IRichEditOle  *m_pRichEditOle;             //实例
	CCharFontStyle m_fcDefaultStyle;          //默认字体样式
	std::map<CStdString, CStdString> m_AttrList; //
	std::map<std::string, CCharRange> m_rcMsgArea; //消息列表
	int m_nTipBitmapId;

	//
	BOOL  m_bAIMsg; //是否智能合并聊天记录
	std::string m_strLastName;
	std::string m_strLastMsgTime;
 
};

#endif