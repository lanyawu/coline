#ifndef __MIRRORVIEWER_H___
#define __MIRRORVIEWER_H___

#include <commonlib/types.h>
#include <commonlib/guardlock.h>
#include <commonlib/Region2D.h>

typedef struct _SWAPCHANGE 
{
	ULONG	ulLength;
	BOOL	bScreen;
	ULONG	ulChangeSize;
	PVOID	pvBuffer;	// comp rectl and byte, format: rectl:bytes,rectl:bytes,...
} SWAPCHANGE, *PSWAPCHANGE;

class COMMONLIB_API CMirrorViewer
{
public:
	CMirrorViewer(void);
	virtual ~CMirrorViewer(void);
	BOOL InitMirror(BOOL bAttach, BOOL bCheckMirror);
	BOOL InitBitmap(BITMAPINFO *pInfo);
	BOOL InitVirtualView(LPCTSTR szVideoFile = NULL);
	void DoRefresh();

protected:	
	void DestroyBitmap();
	BOOL CaptureVideoMemory(LPCTSTR szVideoFile);
	void ReleaseVideoMemory();	
	void ChangeBits(ULONG nSize, PCHAR sBuffer);
    //区域改变通知
	virtual void OnRectChange(const CRegion2D &rcChanged);
protected:
	HDC		m_hDeviceDC;
	PVOID	m_pVideoMemory;
	PVOID	m_pChangeMemory;
	PSWAPCHANGE m_swapChange;
	DWORD	m_dwScreenX;
	DWORD	m_dwScreenY;

	HDC     m_hDesktop;
	DWORD   m_dwByteCount;
	CGuardLock m_DrawLock;
	BITMAPINFO	m_bitmapInfo;
	HDC		m_hMemDC;
	HBITMAP	m_hBitmap;
	char   *m_pBits;
};

#endif

