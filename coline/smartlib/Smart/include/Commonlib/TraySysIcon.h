#ifndef __TRAYSYSICON_H__
#define __TRAYSYSICON_H__

#define SHIFT_STATE_SHIFT 1   //SHIFT��������
#define SHIFT_STATE_CTRL  2   //CTRL ��������
#define SHIFT_STATE_ALT   4   //ALT ��������

#define TRAY_ICON_ANIMATE_INVERTAL  500  //�������ʱ��


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

//����ͼ��
class COMMONLIB_API CTraySysIcon
{
public:
	CTraySysIcon(HINSTANCE hInstance);
	virtual ~CTraySysIcon(void);
public:
	//����¼�  �麯��
	//�������
	virtual void OnLButtonClick(int nShiftState,int nX, int nY);
	//�Ҽ�����
	virtual void OnRButtonClick(int nShiftState,int nX, int nY);
	//���˫��
	virtual void OnLButtonDblClick(int nShiftState,int nX, int nY);
	//�Ҽ�˫��
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
	//������Ϣ
	virtual void OnFinalMessage(HWND hWnd);

	//�ⲿ���ú��� lpszTip��ʾ��Ϣ
	BOOL Animate(const char *lpszTip); //��������
	//ֹͣ����
	BOOL StopAnimate(); 
	//��ʾͼ��
	BOOL Show();
	//����ͼ��
	BOOL Hide();
	//����Ĭ��ͼ��
	BOOL SetDefaultData(HICON hIcon, const char *lpszTip);
	//������ʾ��Ϣ
	void SetTip(const char *lpszTip, DWORD dwInfoFlags, const char *szTitle = NULL);
	void AddAnimateIcon(HICON hIcon);

	HWND GetHWND() const;

private:
	static DWORD WINAPI AnimateThread(LPVOID lpParam); //�����߳�
	HWND  CreateWnd(HINSTANCE hInstance); //����һ��
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL NotifyIcon(DWORD dwMessage, HICON hIcon, DWORD dwInfoFlags = NIIF_NONE, const char *szTitle = NULL);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetTimerEnable(BOOL bEnable);
private:
	NOTIFYICONDATAEX m_NotifyIconData; //ICON����
	std::list<HICON> m_pIconList; //�����õ�ͼ���б�
	HICON m_hDefaultIcon; //Ĭ�ϵ�ͼ��
	BOOL m_bAnimate; //�Ƿ񶯻�
	WNDPROC m_OldWndProc;
	char *m_lpszTip; //��ʾ��Ϣ
	BOOL m_bMouseEnter; //
	POINT m_ptEnter;
	UINT_PTR m_uptrMouseChk;

    //������
    HWND   m_hWnd;        //���
	HANDLE m_hThread;     //�����߳�
	HANDLE m_hWaitEvent;  //�ȴ��¼�
};

#endif