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

    //ˢ��һ�ν���
	void DoRefresh();
	//��ʼ��bitmap
	BOOL InitBufferBitmap();
	//ɾ��Bitmap
	void DestroyBitmap();
	//
	BOOL GetBitmapInfo(DESKTOP_BITMAP_INFO *pInfo);

	RECT GetDesktopRect();
	//��ȡ��Ļ����
    BOOL CaptureScreen(char *lpBuff, DWORD &dwBufSize, const CRegionRect &rc, BOOL bTranslate);
	//�����
	BOOL CaptureMouse(char *lpBuff, DWORD &dwBufSize, RECT &rc, HCURSOR &hOld);
	static BOOL GetDllProductVersion(char * szDllName, char *szBuffer, int nSize);
	static BOOL CheckVideoDriver(bool bBox);
	//
	static int GetNrMonitors();
	//
	void CheckMonitors();
	//��ȡ��ɫ������ֵ
	BOOL GetColorMap(RGBQUAD *pClrMap, DWORD &dwSize);
	//������ɫ��Ӧ��
	BOOL CreateMapColor(); 
protected:
    BOOL Start(int x, int y, int w, int h);
	BOOL MirrorDriverAttachXP(int x,int y,int w,int h);
	void MirrorDriverDetachXP();
	BOOL MirrorDriverVista(DWORD dwAttach,int x,int y,int w,int h);
	char *VideoMemoryGetSharedMemory(void);
	void VideoMemoryReleaseSharedMemory(char *pVideoMemory);
	HDC  GetDcMirror();
    //����ı�֪ͨ
	virtual void OnRectChange(const CRegion2D &rcChanged);
	//��ȡĳһ���ص�ֵ
	COLORREF CapturePixel(int x, int y);
	//���µ�����
	BOOL  RefreshMemDC();
private:
	//����ĳһ��������
	void CopyClipBitmap(DWORD dwClip, CRegion2D &rcChanged);

	//
	static BOOL GetEnumDisplayDevicesFun();
	void GetPrimaryDevice();
	void GetSecondaryDevice();
protected:
	//�������
	RECT  m_rcDesktop;
	DESKTOP_BITMAP_INFO m_bmInfo;
	DWORD m_dwBytesPerRow;
	HDC   m_hDesktop;          //����DC
	HDC   m_hDeviceDC;         //�豸DC
	HDC   m_hMemDC;            //�ڴ�DC
	HBITMAP m_hBufferBitmap;   //����BITMAP
	BOOL  m_bInitDriverSucc;   //��ʼ�������ɹ�
	DWORD m_dwOSVer;
	BOOL  m_bDrawCursor;    //�Ƿ�������
	char  *m_pBits;  //DIB����
	CGuardLock m_Lock;
	BOOL m_bTerminated; //�Ƿ��Ѿ���ֹ
private:
	//VNC������ز���
	DWORD m_dwOld;       //�ϴθ���λ��
	char *m_pVideoMem;
	char *m_pFrameBuffer;
	LPCHANGERS_BUFFER m_pChangeBufs;	
	TCHAR m_szDeviceName[MAX_PATH];
    //����̨
	VNCMonitorInfo m_Monitors[3];
	DWORD m_dwMonitorCount;
    //��ɫת��
	CColorTranslate m_Translate;
	//����
	static pEnumDisplayDevices m_pEnumDisplayDevices;
};


#endif