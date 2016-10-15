#ifndef __GDIPLUSIMAGE_H__
#define __GDIPLUSIMAGE_H__
#include <commonlib/Schedule.h>
#include <CommonLib/Types.h>
#include <CommonLib/GuardLock.h>
#include <GDIPlus.h>
#include <map>
using namespace Gdiplus;

//图像通知事件
class IImageNotifyEvent
{
public:
	virtual void OnInvalidate(LPRECT lprc) = 0; //重绘
};

//GDI图像处理相关
class COMMONLIB_API CGdiPlusImage
{
public:
	CGdiPlusImage(void);
	virtual ~CGdiPlusImage(void);
public:
	//初始化gdiplus
	static void InitGdiPlus();
	//销毁gdiplus
	static void DestroyGdiPlus();
private:
    static ULONG_PTR  m_pGdiplusToken;
	static LONG m_pTokenRef;
};

//GIF文件显示
class COMMONLIB_API CGdiPlusGif : public CGdiPlusImage
{
public:
	CGdiPlusGif(IImageNotifyEvent *pNotify, const char *szFileName, BOOL IsShowAnimate = TRUE);
	CGdiPlusGif(IImageNotifyEvent *pNotify, HBITMAP hBitmap);
	~CGdiPlusGif();
public:
	void Paint(HDC hdc, const RECT &rc);
	int  GetImageWidth();
	int  GetImageHeight();
	void SetTransparent(BOOL bTransparent, COLORREF crTransColor);
	void CopyToClipboard(); //复制到剪切板
private:
	void OnTimer();
    static VOID CALLBACK TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent, DWORD dwTime);
	//static void CALLBACK PaintNextFrame(LPVOID lpParam);
private:
	static std::map<UINT_PTR, CGdiPlusGif *> m_GifList;
	//static CSchedule m_Schedule;
	char m_szSrcFileName[MAX_PATH];
	BOOL m_bIsAlpha;
	COLORREF m_crTransparent;
	BOOL m_bTransparent;
	HDC m_hMem;
	IImageNotifyEvent *m_pNotify;
	HBITMAP m_Bitmap;
 	BOOL m_bTerminated;
	Size m_sizeFrame;
	Size m_sizeImage;
	int  m_nFrameNum; //帧数
	int  m_nCurrPos;  //当前播放第几帧
	UINT *m_pDelays;  //停留时间
	RECT m_rcClient;
	UINT_PTR m_TimerId;
	CGuardLock m_Lock;
};

#endif