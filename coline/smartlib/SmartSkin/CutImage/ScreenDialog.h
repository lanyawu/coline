#ifndef __SCREENDIALOG_H___
#define __SCREENDIALOG_H___

#include <Commonlib/types.h>

typedef enum CapSelectStatus
{
	csNormal = 0, //��ͨ״̬
    csSelecting,  //����ѡȡ
	csSelected    //ѡȡ���
}CAP_SELECT_STATUS;

#define  MIN_SELECT_RECT   5  //��С��ѡȡ���� ����Ϊ��λ

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
	HBITMAP m_hSelected; //ѡȡ����ͼ��
	HDC   m_hMem;
	HWND  m_hWnd;
	int   m_nModalResult;
	WNDPROC  m_OldWndProc;
	HBITMAP m_hScreenBitmap; //
	HBITMAP m_hBkBitmap; //����ͼ
	HBITMAP m_hOldBitmap;
	SIZE  m_sizeScreen; //��Ļ��С
	POINT m_ptStart; //��ʼ��
	POINT m_ptCurr;  //��ǰ��
};

#endif
