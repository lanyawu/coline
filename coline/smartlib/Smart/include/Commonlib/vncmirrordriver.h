#ifndef __VNCMIRRORDRIVER_H___
#define __VNCMIRRORDRIVER_H___

#include <commonlib/types.h>
#include <commonlib/Region2D.h>
#include <commonlib/guardlock.h>
#include <commonlib/ColorTranslate.h>

#define MAX_MONITOR_NAME_SIZE 32

struct VNCMonitorInfo
{
	int nWidth;
	int nHeight;
	int nDepth;
	char szDeviceName[MAX_MONITOR_NAME_SIZE];
	int nOffsetX;
	int nOffsetY;
};

class COMMONLIB_API CVNCMirrorDriver
{
public:
	CVNCMirrorDriver(void);
	virtual ~CVNCMirrorDriver(void);
public:
	BOOL HardwareCursor();
	BOOL NoHardwareCursor();
	virtual BOOL StartCapture();
	void Stop();

    //刷新一次界面
	void DoRefresh();
	//初始化bitmap
	BOOL InitBufferBitmap();
	//删除Bitmap
	void DestroyBitmap();
	//
	BOOL GetBitmapInfo(DESKTOP_BITMAP_INFO *pInfo);

	RECT GetDesktopRect();
	//截取屏幕数据
    BOOL CaptureScreen(char *lpBuff, DWORD &dwBufSize, const CRegionRect &rc, BOOL bTranslate);
	//截鼠标
	BOOL CaptureMouse(char *lpBuff, DWORD &dwBufSize, RECT &rc, HCURSOR &hOld);
	static BOOL GetDllProductVersion(char * szDllName, char *szBuffer, int nSize);
	static BOOL CheckVideoDriver(bool bBox);
	//
	static int GetNrMonitors();
	//
	void CheckMonitors();
	//获取颜色索引表值
	BOOL GetColorMap(RGBQUAD *pClrMap, DWORD &dwSize);
	//产生颜色对应表
	BOOL CreateMapColor(); 
protected:
    BOOL Start(int x, int y, int w, int h);
	BOOL MirrorDriverAttachXP(int x,int y,int w,int h);
	void MirrorDriverDetachXP();
	BOOL MirrorDriverVista(DWORD dwAttach,int x,int y,int w,int h);
	char *VideoMemoryGetSharedMemory(void);
	void VideoMemoryReleaseSharedMemory(char *pVideoMemory);
	HDC  GetDcMirror();
    //区域改变通知
	virtual void OnRectChange(const CRegion2D &rcChanged);
	//获取某一象素点值
	COLORREF CapturePixel(int x, int y);
	//更新到缓存
	BOOL  RefreshMemDC();
private:
	//拷贝某一区域数据
	void CopyClipBitmap(DWORD dwClip, CRegion2D &rcChanged);

	//
	static BOOL GetEnumDisplayDevicesFun();
	void GetPrimaryDevice();
	void GetSecondaryDevice();
protected:
	//桌面相关
	RECT  m_rcDesktop;
	DESKTOP_BITMAP_INFO m_bmInfo;
	DWORD m_dwBytesPerRow;
	HDC   m_hDesktop;          //桌面DC
	HDC   m_hDeviceDC;         //设备DC
	HDC   m_hMemDC;            //内存DC
	HBITMAP m_hBufferBitmap;   //缓存BITMAP
	BOOL  m_bInitDriverSucc;   //初始化驱动成功
	DWORD m_dwOSVer;
	BOOL  m_bDrawCursor;    //是否绘制鼠标
	char  *m_pBits;  //DIB数据
	CGuardLock m_Lock;
	BOOL m_bTerminated; //是否已经中止
private:
	//VNC驱动相关参数
	DWORD m_dwOld;       //上次更新位置
	char *m_pVideoMem;
	char *m_pFrameBuffer;
	LPCHANGERS_BUFFER m_pChangeBufs;	
	TCHAR m_szDeviceName[MAX_PATH];
    //控制台
	VNCMonitorInfo m_Monitors[3];
	DWORD m_dwMonitorCount;
    //颜色转换
	CColorTranslate m_Translate;
	//函数
	static pEnumDisplayDevices m_pEnumDisplayDevices;
};


#endif