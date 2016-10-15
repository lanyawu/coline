#ifndef __TRAYSYSICON_H__
#define __TRAYSYSICON_H__

#define SHIFT_STATE_SHIFT 1   //SHIFT键被按下
#define SHIFT_STATE_CTRL  2   //CTRL 键被按下
#define SHIFT_STATE_ALT   4   //ALT 键被按下

#define TRAY_ICON_ANIMATE_INVERTAL  500  //动画间隔时间


#include <CommonLib/Types.h>
#include <CommonLib/GuardLock.h>
#include <list>
#include <ShellApi.h>

#define TRAY_ICON_NOTIFYMESSAGE     WM_USER + 0x100
#define TRAY_ICON_ID                0

#define  NIIF_NONE            0x00000000
#define  NIIF_INFO            0x00000001
#define  NIIF_WARNING         0x00000002
#define  NIIF_ERROR           0x00000003
#define  NIIF_ICON_MASK       0x0000000F   // Reserved for WinXP
#define  NIIF_NOSOUND         0x00000010   // Reserved for WinXP

typedef struct CNotifyIconDataEx
{
	DWORD cbSize;
	HWND  hWnd;
	UINT  uID;
	UINT  uFlags;
	UINT  uCallbackMessage;
	HICON hIcon;
	WCHAR szTip[128];
	UINT  TimeoutOrVersion;
	WCHAR szInfoTitle[64];
	DWORD dwInfoFlags;
#if (_WIN32_IE >= 0x600)
    GUID guidItem;
#endif
}NOTIFYICONDATAEX, *LPNOTIFYICONDATAEX;

//托盘图标
class COMMONLIB_API CTraySysIcon
{
public:
	CTraySysIcon(HINSTANCE hInstance);
	virtual ~CTraySysIcon(void);
public:
	//鼠标事件  虚函数
	//左键单击
	virtual void OnLButtonClick(int nShiftState,int nX, int nY);
	//右键单击
	virtual void OnRButtonClick(int nShiftState,int nX, int nY);
	//左键双击
	virtual void OnLButtonDblClick(int nShiftState,int nX, int nY);
	//右键双击
	virtual void OnRButtonDblClick(int nShiftState, int nX, int nY);
	//
	virtual void OnBalloonShow();
	//
	virtual void OnBalloonHide();
	//
	virtual void OnBalloonTimeout();
	//
	virtual void OnBalloonUserClick();
	//
	virtual void OnToolTipShow(BOOL bShow);
	//结束消息
	virtual void OnFinalMessage(HWND hWnd);

	//外部调用函数 lpszTip提示信息
	BOOL Animate(const char *lpszTip); //启动动画
	//停止动画
	BOOL StopAnimate(); 
	//显示图标
	BOOL Show();
	//隐藏图标
	BOOL Hide();
	//设置默认图标
	BOOL SetDefaultData(HICON hIcon, const char *lpszTip);
	//设置提示信息
	void SetTip(const char *lpszTip, DWORD dwInfoFlags, const char *szTitle = NULL);
	void AddAnimateIcon(HICON hIcon);

	HWND GetHWND() const;

private:
	static DWORD WINAPI AnimateThread(LPVOID lpParam); //动画线程
	HWND  CreateWnd(HINSTANCE hInstance); //创建一个
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL NotifyIcon(DWORD dwMessage, HICON hIcon, DWORD dwInfoFlags = NIIF_NONE, const char *szTitle = NULL);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetTimerEnable(BOOL bEnable);
private:
	NOTIFYICONDATAEX m_NotifyIconData; //ICON数据
	std::list<HICON> m_pIconList; //动画用的图标列表
	HICON m_hDefaultIcon; //默认的图标
	BOOL m_bAnimate; //是否动画
	WNDPROC m_OldWndProc;
	char *m_lpszTip; //提示信息
	BOOL m_bMouseEnter; //
	POINT m_ptEnter;
	UINT_PTR m_uptrMouseChk;

    //句柄相关
    HWND   m_hWnd;        //句柄
	HANDLE m_hThread;     //动画线程
	HANDLE m_hWaitEvent;  //等待事件
};

#endif