#include <commonlib/debuglog.h>
#include <commonlib/MirrorViewer.h>
#include <commonlib/stringutils.h>

#define VIDEO_SIZE_MUL	3

#define	DESKTOP_SIZE	1

#define SCREEN_X		1024
#define SCREEN_Y		768

#define VIDEO_FILE			_T("c:\\video.dat")
#define VIDEO_FILE_DRV		L"\\??\\c:\\video.dat"
#define VIDEO_CHANGE_DRV	L"\\??\\c:\\videochange.dat"
#define DRIVER_NAME			_T("Microsoft Mirror Driver")

#define REFRESH_TIMER		80
#define CHANGE_RECT_SIZE	1024


#define ESCAPE_VIEW_CHANGE	0x20001 
#define ESCAPE_GET_CHANGE	0x20002

#pragma warning(disable:4996)

CMirrorViewer::CMirrorViewer(void):
               m_dwScreenX(0),
               m_dwScreenY(0),
			   m_dwByteCount(1),
			   m_hDesktop(NULL),
			   m_hDeviceDC(NULL),
			   m_pChangeMemory(NULL),
			   m_swapChange(NULL),
			   m_pVideoMemory(NULL),
			   m_hMemDC(NULL),
			   m_hBitmap(NULL),
			   m_pBits(NULL)
{
	//
}

CMirrorViewer::~CMirrorViewer(void)
{
	InitMirror(FALSE, FALSE);
	ReleaseVideoMemory();
	DestroyBitmap();

	if (m_pChangeMemory)
		delete[] m_pChangeMemory;

	if (m_hDeviceDC)
		::DeleteDC(m_hDeviceDC);
}


BOOL CMirrorViewer::CaptureVideoMemory(LPCTSTR szVideoFile)
{
   HANDLE hFile, hMappingFile;

   hFile = ::CreateFile(szVideoFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
   {
	   PRINTDEBUGLOG(dtInfo, "CaptureVideoMemory: CreateFile failed");
	   return FALSE;
   }

   DWORD dwFileSize = ::GetFileSize(hFile, NULL);
   if (dwFileSize < m_dwScreenX * m_dwScreenY * 4)
   {
	   ::CloseHandle(hFile);
	   return FALSE;
   }

   hMappingFile = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
   if (hMappingFile)
   {
	   m_pVideoMemory = ::MapViewOfFile(hMappingFile, FILE_MAP_READ, 0, 0, 0);
	   ::CloseHandle(hMappingFile);
   }

   ::CloseHandle(hFile);

   return (m_pVideoMemory != NULL);
}

void CMirrorViewer::ReleaseVideoMemory()
{
	if (m_pVideoMemory)
	{
		::UnmapViewOfFile(m_pVideoMemory);
		m_pVideoMemory = NULL;
	}
}

const char *dispCode[7] =
{
   "Change Successful",
  "Must Restart",
   "Bad Flags",
   "Bad Parameters",
   "Failed",
   "Bad Mode",
   "Not Updated"
};

const char *GetDispCode(INT code)
{
   switch (code) 
   {   
	   case DISP_CHANGE_SUCCESSFUL:
		    return dispCode[0];

	   case DISP_CHANGE_RESTART: 
		    return dispCode[1];
	   
	   case DISP_CHANGE_BADFLAGS: 
		    return dispCode[2];
	   
	   case DISP_CHANGE_BADPARAM: 
		    return dispCode[3];
	   
	   case DISP_CHANGE_FAILED: 
		    return dispCode[4];
	   
	   case DISP_CHANGE_BADMODE: 
		    return dispCode[5];
	   
	   case DISP_CHANGE_NOTUPDATED: 
		    return dispCode[6];	   
	   default:
		   {
			   static char tmp[MAX_PATH] = {0};
			   sprintf(tmp, "Unknown code:%08x\n", code);
			   return tmp;
		   }  
   }   
   return NULL;   // can't happen
}


BOOL CMirrorViewer::InitMirror(BOOL bAttach, BOOL bCheckMirror)
{
typedef BOOL (WINAPI* LPEnumDisplayDevices)(PVOID,DWORD,PVOID,DWORD);
    BOOL  bED   = TRUE;
    DEVMODE devmode;
    LPEnumDisplayDevices pDisplayFun = NULL;
    FillMemory(&devmode, sizeof(DEVMODE), 0);
    devmode.dmSize = sizeof(DEVMODE);
    devmode.dmDriverExtra = 0;

	HMODULE hModule = ::LoadLibrary(L"user32");
	if (hModule)
		pDisplayFun = (LPEnumDisplayDevices)::GetProcAddress(hModule, "EnumDisplayDevicesW");
	if (!pDisplayFun)
		return FALSE;
    BOOL change = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
    devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION ;
	if (change)
	{
		DISPLAY_DEVICE dispDevice = {0};   
        FillMemory(&dispDevice, sizeof(DISPLAY_DEVICE), 0);   
        dispDevice.cb = sizeof(DISPLAY_DEVICE);   
        LPTSTR deviceName = NULL;
        devmode.dmDeviceName[0] = _T('\0');
        INT devNum = 0;
        BOOL bRes;
        DWORD cxPrimary = 0xFFFFFFFF;
        DWORD cyPrimary = 0xFFFFFFFF;
        while (bRes = pDisplayFun(NULL, devNum, &dispDevice, 0))
		{
			char szName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(dispDevice.DeviceString, szName, MAX_PATH - 1);
			PRINTDEBUGLOG(dtInfo, "device string:%s", szName);
			if (dispDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			{
				// Primary device. Find out its dmPelsWidht and dmPelsHeight.
				EnumDisplaySettings(dispDevice.DeviceName,
					ENUM_CURRENT_SETTINGS, &devmode);

				cxPrimary = devmode.dmPelsWidth;
				cyPrimary = devmode.dmPelsHeight;
				break;
			}
			devNum++;
			memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
			dispDevice.cb = sizeof(DISPLAY_DEVICE);
		}

		if (!bRes)
		{
			PRINTDEBUGLOG(dtInfo, "Primary Device not found");
			return FALSE;
		}

        if (cxPrimary == 0xFFFFFFFF || cyPrimary == 0xFFFFFFFF)
        {
            PRINTDEBUGLOG(dtInfo, "cxPrimary or cyPrimary not valid\n");

			return FALSE;
        }

		m_dwScreenX = cxPrimary;
		m_dwScreenY = cyPrimary;

        // Enumerate again for the mirror driver:
        devNum = 0;
		memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
		dispDevice.cb = sizeof(DISPLAY_DEVICE);
        while (bRes = pDisplayFun(NULL, devNum, &dispDevice, 0))
        {
			char szName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(dispDevice.DeviceString, szName, MAX_PATH - 1);
			PRINTDEBUGLOG(dtInfo, "device string:%s", szName);
			if (_tcscmp(dispDevice.DeviceString, DRIVER_NAME) == 0)
				break;
			devNum++;
			memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
			dispDevice.cb = sizeof(DISPLAY_DEVICE);
        }

        // error check       
        if (!bRes)
        {
			char szName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(DRIVER_NAME, szName, MAX_PATH - 1);
			PRINTDEBUGLOG(dtInfo, "No '%s' found. LastError:%d", szName, GetLastError());
			return FALSE;
        }
 
        TCHAR deviceNum[MAX_PATH];
        LPTSTR deviceSub;

        // Simply extract 'DEVICE#' from registry key.  This will depend
        // on how many mirrored devices your driver has and which ones
        // you intend to use.

        _tcsupr(&dispDevice.DeviceKey[0]);
        deviceSub = _tcsstr(&dispDevice.DeviceKey[0], _T("\\DEVICE"));

        if (!deviceSub) 
			::lstrcpyn(deviceNum, _T("DEVICE0"), sizeof(deviceNum));
        else
			::lstrcpyn(deviceNum, ++deviceSub, sizeof(deviceNum));

        // Reset the devmode for mirror driver use:
        FillMemory(&devmode, sizeof(DEVMODE), 0);
        devmode.dmSize = sizeof(DEVMODE);
        devmode.dmDriverExtra = 0;
        devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION;
		::lstrcpyn(&devmode.dmDeviceName[0], _T("mirror"), sizeof(devmode.dmDeviceName));
        deviceName = (LPTSTR)&dispDevice.DeviceName[0];
		if (bED)
		{
            if (bAttach)
            {
				devmode.dmPelsWidth = cxPrimary;
                devmode.dmPelsHeight = cyPrimary;
            } else
            {
                devmode.dmPelsWidth = 0;
                devmode.dmPelsHeight = 0;
            }
            // Update the mirror device's registry data with the devmode. Dont
            // do a mode change. 
            INT code =  ChangeDisplaySettingsEx(deviceName,  &devmode, NULL, (CDS_UPDATEREGISTRY | CDS_NORESET),  NULL);
            // effect.
            code = ChangeDisplaySettingsEx(NULL, NULL,  NULL,  0,  NULL);
        }

		if (bCheckMirror)
		{
			if (!m_hDeviceDC)
				m_hDeviceDC = CreateDC(_T("DISPLAY"),  deviceName, NULL,  NULL);
			if (!m_pChangeMemory)
			{
				m_pChangeMemory = new char[m_dwScreenX * m_dwScreenY * 4 + sizeof(SWAPCHANGE)];
				m_swapChange = (PSWAPCHANGE)m_pChangeMemory;
			}
		}
	}
	return TRUE;
}

BOOL CMirrorViewer::InitVirtualView(LPCTSTR szVideoFile)
{
	if (!szVideoFile)
		return CaptureVideoMemory(VIDEO_FILE);
	return CaptureVideoMemory(szVideoFile);
}


BOOL CMirrorViewer::InitBitmap(BITMAPINFO *pInfo)
{
	int iScreenX = (int)m_dwScreenX;
	int iScreenY = (int)m_dwScreenY;	
	iScreenY = -abs(iScreenY);
	if (pInfo)
		memmove(&m_bitmapInfo, pInfo, sizeof(BITMAPINFO));
	else
	{
		FillMemory(&m_bitmapInfo, sizeof(BITMAPINFO), 0);
		m_bitmapInfo.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
		m_bitmapInfo.bmiHeader.biBitCount = 32;
		m_bitmapInfo.bmiHeader.biWidth    = iScreenX;
		m_bitmapInfo.bmiHeader.biHeight   = iScreenY;
		m_bitmapInfo.bmiHeader.biPlanes   = 1;
		m_bitmapInfo.bmiHeader.biCompression = BI_RGB;
	}
	m_dwByteCount = m_bitmapInfo.bmiHeader.biBitCount / 8;
	m_hDesktop = ::GetDC(::GetDesktopWindow());
	if (!m_hDesktop)
		return FALSE;

	m_hMemDC = CreateCompatibleDC(m_hDesktop);
	m_hBitmap = ::CreateDIBSection(m_hMemDC, &m_bitmapInfo, DIB_RGB_COLORS,
		(void **)&m_pBits, NULL, 0);	
    if (m_hBitmap == NULL)
	{
		PRINTDEBUGLOG(dtInfo, "create dibsection failed, error:%d", ::GetLastError());
	}
	return TRUE;
}

void CMirrorViewer::DestroyBitmap()
{
	if (m_hBitmap)
	{
		::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_hMemDC)
	{
		::DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}
	if (m_hDesktop)
	{
		::ReleaseDC(::GetDesktopWindow(), m_hDesktop);
		m_hDesktop = NULL;
	}
}


void CMirrorViewer::DoRefresh()
{
	if (!m_pBits)
		return;
	CGuardLock::COwnerLock guard(m_DrawLock);
	::GdiFlush();
	if (!m_hDeviceDC || !m_swapChange)
	{
		memcpy(m_pBits, m_pVideoMemory, m_dwScreenX * m_dwScreenY * m_dwByteCount);
	} else
	{
		memset (m_swapChange, 0, sizeof(SWAPCHANGE));
		int iRet = ::ExtEscape(m_hDeviceDC, ESCAPE_GET_CHANGE, 0, NULL, 
			             m_dwScreenX * m_dwScreenY * m_dwByteCount + sizeof(SWAPCHANGE), (LPSTR)m_swapChange);
		if (iRet)
		{
			if (m_swapChange->bScreen)
			{
				memcpy (m_pBits, m_pVideoMemory, m_dwScreenX * m_dwScreenY * m_dwByteCount);
			}
			else if (m_swapChange->ulChangeSize > 0)
			{
				PCHAR pChange = (PCHAR)m_swapChange + sizeof(SWAPCHANGE);
				ChangeBits(m_swapChange->ulChangeSize, pChange);
			}
		}
	}
}

void CMirrorViewer::ChangeBits(ULONG nSize, PCHAR sBuffer)
{
	CRegion2D rcChanged; //
	for (ULONG i = 0; i < nSize; i++)
	{
		RECT rc;
		memcpy (&rc, sBuffer, sizeof(RECT));
		sBuffer += sizeof(RECT);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		PCHAR p = (PCHAR)m_pBits;
		p += rc.top * m_dwScreenX * m_dwByteCount;
		for (int k = 0; k < h; k++)
		{
			PCHAR p2 = p;
			p2 += rc.left * m_dwByteCount;
			memcpy (p2, sBuffer, w * m_dwByteCount);
			sBuffer += w * m_dwByteCount;
			p += m_dwScreenX * m_dwByteCount;
		}
		rcChanged.Union(rc);
	}
	OnRectChange(rcChanged);
}

    //区域改变通知
void CMirrorViewer::OnRectChange(const CRegion2D &rcChanged)
{
	//
}

#pragma warning(default:4996)
