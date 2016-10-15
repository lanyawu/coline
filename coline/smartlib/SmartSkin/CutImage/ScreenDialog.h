#ifndef __SCREENDIALOG_H___
#define __SCREENDIALOG_H___

#include <Commonlib/types.h>

typedef enum CapSelectStatus
{
	csNormal = 0, //普通状态
    csSelecting,  //正在选取
	csSelected    //选取完毕
}CAP_SELECT_STATUS;

#define  MIN_SELECT_RECT   5  //最小的选取区域 象素为单位

class CScreenDialog
{
public:
	CScreenDialog(void);
	virtual ~CScreenDialog(void);
public:
	//CWindowWnd function
	virtual LPCTSTR GetWindowClassName() const; 
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnFinalMessage(HWND hWnd);
	virtual UINT GetClassStyle() const;
    //
	int  CaptureScreen(HINSTANCE hRes, HWND hParent, BOOL bHideParent);

	//
	void Close();
private:
	void DisplayInfo();
	BOOL RegisterWindowClass(HINSTANCE hRes);
	//
	int  ShowModal();
	
	static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CAP_SELECT_STATUS m_Status;
	HBITMAP m_hSelected; //选取区域图像
	HDC   m_hMem;
	HWND  m_hWnd;
	int   m_nModalResult;
	WNDPROC  m_OldWndProc;
	HBITMAP m_hScreenBitmap; //
	HBITMAP m_hBkBitmap; //背景图
	HBITMAP m_hOldBitmap;
	SIZE  m_sizeScreen; //屏幕大小
	POINT m_ptStart; //起始点
	POINT m_ptCurr;  //当前点
};

#endif
