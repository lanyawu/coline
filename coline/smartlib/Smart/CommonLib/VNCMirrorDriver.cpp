#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include <commonlib/stringutils.h>
#include <commonlib/graphicplus.h>
#include <commonlib/VNCMirrorDriver.h>

#define CURSOR_MAP1   1030
#define CURSOR_UNMAP1 1031
#define CURSOR_EN     1060
#define CURSOR_DIS    1061

const char *szVideoDriverName = "mv video hook driver2";
pEnumDisplayDevices CVNCMirrorDriver::m_pEnumDisplayDevices = NULL;
#pragma warning(disable:4996)
//debug
void SaveBitmapToFile(HBITMAP hBitmap)
{
	CGraphicPlus g;
	g.LoadFromBitmap(hBitmap);
	char szFileName[256] = {0};
	sprintf(szFileName, "F:\\Cap\\%d.jpg", ::GetTickCount());
	g.SaveToFile(szFileName, GRAPHIC_TYPE_JPG);
}

void SaveBufferToFile(const RECT *prc, char *pBuffer, DWORD dwBitCount, BOOL bFlip)
{
	CGraphicPlus g;
	g.LoadFromDIB(pBuffer, prc->right - prc->left, prc->bottom - prc->top, dwBitCount, 
					 (prc->right - prc->left) * dwBitCount / 8, bFlip);
	char szFileName[256] = {0};
	sprintf(szFileName, "F:\\Cap\\%d.jpg", ::GetTickCount());
	g.SaveToFile(szFileName, GRAPHIC_TYPE_JPG);
}

void SaveBufferToBmpFile(HDC hDC, HDC hDrawDC, char *pBuffer, const DESKTOP_BITMAP_INFO &bmInfo, WORD wWidth, WORD wHeight)
{
	DESKTOP_BITMAP_INFO Info = {0};
	memmove(&Info, &bmInfo, sizeof(DESKTOP_BITMAP_INFO));
	Info.bmInfo.bmiHeader.biHeight = wHeight;
	Info.bmInfo.bmiHeader.biHeight = - Info.bmInfo.bmiHeader.biHeight;
	Info.bmInfo.bmiHeader.biWidth = wWidth;
	Info.bmInfo.bmiHeader.biSizeImage = wWidth * wHeight * Info.bmInfo.bmiHeader.biBitCount / 8;
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, wWidth, wHeight);
	::SetDIBits(hDrawDC, hBitmap, 0, wHeight, pBuffer, &(Info.bmInfo),DIB_RGB_COLORS);
	char szFileName[256] = {0};
	sprintf(szFileName, "F:\\Cap\\%d.jpg", ::GetTickCount());
	CGraphicPlus g;
	g.LoadFromBitmap(hBitmap);
	g.SaveToFile(szFileName, GRAPHIC_TYPE_JPG);
	::DeleteObject(hBitmap);
}

//检测
///////////////////////////////////////////////////////////////////
BOOL CVNCMirrorDriver::GetDllProductVersion(char * szDllName, char *szBuffer, int nSize)
{
	TCHAR *szVerInfo;
	void *lpBuf;
	UINT dwBytes;
	DWORD dwReadBuffer;
	if (!szDllName || !szBuffer)
		return FALSE;
    
	TCHAR szwDllName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szDllName, szwDllName, MAX_PATH - 1);
	DWORD dwVerSize = ::GetFileVersionInfoSize(szwDllName, &dwReadBuffer);
	if (dwVerSize == 0)
		return FALSE;
	szVerInfo = new TCHAR[dwVerSize];
	memset(szVerInfo, 0, sizeof(TCHAR) * dwVerSize);
	BOOL bRes = ::GetFileVersionInfo(szwDllName, NULL, dwVerSize, szVerInfo);
	BOOL bValue = ::VerQueryValue(szVerInfo, L"\\StringFileInfo\\040904b0\\ProductVersion", &lpBuf, &dwBytes);
    if (!bValue)
		bValue = ::VerQueryValue(szVerInfo, L"\\StringFileInfo\\000004b0\\ProductVersion", &lpBuf, &dwBytes);
	if (bValue)
	{
		CStringConversion::WideCharToString((TCHAR *)lpBuf, szBuffer, nSize);
		delete []szVerInfo;
		return TRUE;
	} else
	{
		delete []szVerInfo;
		return FALSE;
	}
}

void CVNCMirrorDriver::GetSecondaryDevice()
{
	int i;
    if (GetEnumDisplayDevicesFun())
    {
        DISPLAY_DEVICE dd;
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
		for (i = 0; m_pEnumDisplayDevices(NULL, i, &dd, 0); i ++)
		{
			if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
				&& (!(dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
				&& (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)))
				CStringConversion::WideCharToString(dd.DeviceName, m_Monitors[1].szDeviceName, MAX_MONITOR_NAME_SIZE - 1);
		}
	}
}

void CVNCMirrorDriver::GetPrimaryDevice()
{
	int i;
    if (GetEnumDisplayDevicesFun())
    {
        DISPLAY_DEVICE dd;
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
		for (i = 0; m_pEnumDisplayDevices(NULL, i, &dd, 0); i ++)
		{
			if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
				&& (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
				&& (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)))
				CStringConversion::WideCharToString(dd.DeviceName, m_Monitors[0].szDeviceName, MAX_MONITOR_NAME_SIZE - 1);
		}
	}
}

//
void CVNCMirrorDriver::CheckMonitors()
{
	m_dwMonitorCount = GetNrMonitors();
	DWORD dwOSVer = CSystemUtils::GetOSVersion();
	DEVMODE devMode = {0};
	devMode.dmSize = sizeof(DEVMODE);
	//
	if (m_dwMonitorCount > 0)
	{
		if ((dwOSVer == OS_VERSION_OLD) || (dwOSVer == OS_VERSION_WINXP) || (dwOSVer == OS_VERSION_WIN2003))
			GetPrimaryDevice();
		if ((dwOSVer == OS_VERSION_OLD) || (dwOSVer == OS_VERSION_WINXP) || (dwOSVer == OS_VERSION_WIN2003))
		{
			TCHAR szDeviceName[MAX_MONITOR_NAME_SIZE] = {0};
			CStringConversion::StringToWideChar(m_Monitors[0].szDeviceName, szDeviceName, MAX_MONITOR_NAME_SIZE - 1);
			::EnumDisplaySettings(szDeviceName, ENUM_CURRENT_SETTINGS, &devMode);
		} else
			::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
		m_Monitors[0].nOffsetX = devMode.dmPosition.x;
		m_Monitors[0].nOffsetY = devMode.dmPosition.y;
		m_Monitors[0].nWidth = devMode.dmPelsWidth;
		m_Monitors[0].nHeight = devMode.dmPelsHeight;
		m_Monitors[0].nDepth = devMode.dmBitsPerPel;
	}
	if (m_dwMonitorCount > 1)
	{
		GetSecondaryDevice();
		TCHAR szDeviceName[MAX_MONITOR_NAME_SIZE] = {0};
		CStringConversion::StringToWideChar(m_Monitors[1].szDeviceName, szDeviceName, MAX_MONITOR_NAME_SIZE - 1);
		::EnumDisplaySettings(szDeviceName, ENUM_CURRENT_SETTINGS, &devMode);
        m_Monitors[1].nOffsetX = devMode.dmPosition.x;
		m_Monitors[1].nOffsetY = devMode.dmPosition.y;
		m_Monitors[1].nWidth = devMode.dmPelsWidth;
		m_Monitors[1].nHeight = devMode.dmPelsHeight;
		m_Monitors[1].nDepth = devMode.dmBitsPerPel;
	}
	m_Monitors[2].nOffsetX = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_Monitors[2].nOffsetY = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_Monitors[2].nWidth = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
	m_Monitors[2].nHeight = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
	m_Monitors[2].nDepth = m_Monitors[0].nDepth;
}

//
int CVNCMirrorDriver::GetNrMonitors()
{
	DWORD dwOSVer = CSystemUtils::GetOSVersion();
	if ((dwOSVer == OS_VERSION_WIN2000) || (dwOSVer == OS_VERSION_WINXP64))
		return 1;
	int i, j = 0;
	if (GetEnumDisplayDevicesFun())
	{
		DISPLAY_DEVICE dd;
		::ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
		for (i = 0; m_pEnumDisplayDevices(NULL, i, &dd, 0); i ++)
		{
			if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) 
				&& (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)))
				j ++;
		}
	}
	return j;
}

///////////////////////////////////////////////////////////////////

BOOL CVNCMirrorDriver::GetEnumDisplayDevicesFun()
{
	if (!m_pEnumDisplayDevices)
	{
		HMODULE hUser32 = ::LoadLibrary(L"USER32");
		if (hUser32)
			m_pEnumDisplayDevices = (pEnumDisplayDevices)::GetProcAddress(hUser32, "EnumDisplayDevicesW");
	}
	return m_pEnumDisplayDevices != NULL;
}

BOOL CVNCMirrorDriver::CheckVideoDriver(bool bBox)
{
	HDC hdc = NULL;
	BOOL bDriverFound;
	DEVMODE mode;
    memset(&mode, 0, sizeof(DEVMODE));
	mode.dmSize = sizeof(DEVMODE);
	mode.dmDriverExtra = 0;
	BOOL bChange = ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);
	mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (GetEnumDisplayDevicesFun())
	{
		char *szDeviceName = NULL;
		DISPLAY_DEVICE dd;
		memset(&dd, 0, sizeof(DISPLAY_DEVICE));
		dd.cb = sizeof(DISPLAY_DEVICE);
		int nDevNum = 0;
		BOOL bRes;
		bDriverFound = FALSE;
		char szDeviceString[256];
		while (bRes = m_pEnumDisplayDevices(NULL, nDevNum, &dd, 0))
		{
			memset(szDeviceString, 0, 256);
			CStringConversion::WideCharToString(dd.DeviceString, szDeviceString, 255);
			if (strcmp(szDeviceString, szVideoDriverName) == 0)
			{
				bDriverFound = TRUE;
				break;
			}
			nDevNum ++;
		}
		if (bDriverFound)
		{
			if (bBox)
			{
				char szBuf[512] = {0};
				GetDllProductVersion("mv2.dll", szBuf, 512);
				if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
				{
					strcat(szBuf, " dirver active");
					HDC hTestDc = NULL;
					hTestDc = ::CreateDC(L"DISPLAY", dd.DeviceName, NULL, NULL);
					if (hTestDc)
					{
						::DeleteDC(hTestDc);
						strcat(szBuf, " access ok");
					} else
						strcat(szBuf, " access denied, permission problem");
				} else
					strcat(szBuf, " driver not active");
				PRINTDEBUGLOG(dtInfo, "Check Video Driver:%s", szBuf);
			} 
			return TRUE;
		} //end if (bDriverFound)
	} // end if (pFun)
	return FALSE;
}



CVNCMirrorDriver::CVNCMirrorDriver(void):
                  m_pVideoMem(NULL),
			      m_pFrameBuffer(NULL),
				  m_hDesktop(NULL),
				  m_hDeviceDC(NULL),
				  m_hMemDC(NULL),
				  m_bInitDriverSucc(FALSE),
				  m_dwMonitorCount(0),
				  m_pBits(NULL),
				  m_bDrawCursor(FALSE),
				  m_bTerminated(FALSE),
				  m_hBufferBitmap(NULL),
			      m_pChangeBufs(NULL)
{
	memset(&m_bmInfo, 0, sizeof(DESKTOP_BITMAP_INFO));
	memset(m_szDeviceName, 0, sizeof(TCHAR) * MAX_PATH);
}

CVNCMirrorDriver::~CVNCMirrorDriver(void)
{
	Stop();
	DestroyBitmap();
}
 
BOOL CVNCMirrorDriver::HardwareCursor()
{
	m_bDrawCursor = TRUE;
	m_Lock.Lock();
	int nResult = 0;
	if (m_bInitDriverSucc && m_pVideoMem)
	{
		HDC hdc;
		hdc = ::GetDC(NULL);
		nResult = ::ExtEscape(hdc, CURSOR_MAP1, 0, NULL, 0, NULL);
		nResult = ::ExtEscape(hdc, CURSOR_EN, 0, NULL, 0, NULL);
		::ReleaseDC(NULL, hdc);
	}
	m_Lock.UnLock();
	return (nResult > 0);
}

BOOL CVNCMirrorDriver::NoHardwareCursor()
{
	m_bDrawCursor = FALSE;
	m_Lock.Lock();
	int nResult = 0;
	if (m_bInitDriverSucc && m_pVideoMem)
	{
		HDC hdc;
		hdc = ::GetDC(NULL);
		nResult = ::ExtEscape(hdc, CURSOR_DIS, 0, NULL, 0, NULL);
		::ReleaseDC(NULL, hdc);
	}
	m_Lock.UnLock();
	return (nResult > 0);;
}


BOOL CVNCMirrorDriver::StartCapture()
{
	int w = ::GetSystemMetrics(SM_CXSCREEN);
	int h = ::GetSystemMetrics(SM_CYSCREEN);
	CheckMonitors(); //检测
	m_dwMonitorCount = GetNrMonitors();
	PRINTDEBUGLOG(dtInfo, "Monitors count:%d", m_dwMonitorCount);
	return Start(0, 0, w, h);
}

BOOL CVNCMirrorDriver::Start(int x, int y, int w, int h)
{
	PRINTDEBUGLOG(dtError, "Entry Attach VNC Driver Lock");
	m_Lock.Lock();
	PRINTDEBUGLOG(dtError, "Leave Attach VNC Driver Lock");
	try
	{
		m_bInitDriverSucc = FALSE;
		m_dwOld = 1;
		m_pVideoMem = NULL;
		m_dwOSVer = CSystemUtils::GetOSVersion();
		PRINTDEBUGLOG(dtInfo, "get os version:%d", m_dwOSVer);
		if ((m_dwOSVer == OS_VERSION_WIN2000)
			 || (m_dwOSVer == OS_VERSION_WIN2003)
			 || (m_dwOSVer == OS_VERSION_WINXP))
		{
			PRINTDEBUGLOG(dtInfo, "XP attach");
			if (MirrorDriverAttachXP(x, y, w, h))
			{
				HDC hdc = GetDcMirror();
				if (hdc != NULL)
				{
					::DeleteDC(hdc);
					m_pVideoMem = VideoMemoryGetSharedMemory();
					m_pChangeBufs = (LPCHANGERS_BUFFER)m_pVideoMem;
					m_pFrameBuffer = m_pVideoMem + sizeof(CHANGERS_BUFFER);
					m_bInitDriverSucc = TRUE;
				} else
					m_pVideoMem = NULL;
			} else
				m_pVideoMem = NULL;
		} else if ((m_dwOSVer == OS_VERSION_VISTA) || (m_dwOSVer == OS_VERSION_WIN7))
		{
			PRINTDEBUGLOG(dtInfo, "Vista attach 00000");
			if (MirrorDriverVista(1, x, y, w, h))
			{
				HDC hdc = GetDcMirror();
				if (hdc != NULL)
				{
					::DeleteDC(hdc);
					m_pVideoMem = VideoMemoryGetSharedMemory();
					m_pChangeBufs = (LPCHANGERS_BUFFER)m_pVideoMem;
					m_pFrameBuffer = m_pVideoMem + sizeof(CHANGERS_BUFFER);
					m_bInitDriverSucc = TRUE;
				} else
				{
					if (MirrorDriverAttachXP(x, y, w, h))
					{
						hdc = GetDcMirror();
						if (hdc != NULL)
						{
							::DeleteDC(hdc);
							m_pVideoMem = VideoMemoryGetSharedMemory();
							m_pChangeBufs = (LPCHANGERS_BUFFER)m_pVideoMem;
							m_pFrameBuffer = m_pVideoMem + sizeof(CHANGERS_BUFFER);
							m_bInitDriverSucc = TRUE;
						}  else //end if (GetDcMirror..
							PRINTDEBUGLOG(dtInfo, "Get DC Mirror Failed in Vista attach xp");
					} else //end if (MirrorDirverAttachXP(..
						PRINTDEBUGLOG(dtInfo, "Attach XP Failed in vista");
				}// end if else...
			} else // end if MirrorDriverVista(...
				PRINTDEBUGLOG(dtInfo, "Mirror Driver vista failed");
		} else
			PRINTDEBUGLOG(dtInfo, "no support os version:%d", m_dwOSVer);//end if else (m_OSVer...
	}catch(...)
	{
		PRINTDEBUGLOG(dtInfo, "Start Driver Exception");
	}
	PRINTDEBUGLOG(dtError, "End Attach VNC Driver");
	m_Lock.UnLock();
	return m_bInitDriverSucc;
}

void CVNCMirrorDriver::Stop()
{
	PRINTDEBUGLOG(dtError, "Entry Stop Capture Driver Lock");
	m_Lock.Lock();
	PRINTDEBUGLOG(dtError, "Leave Stop Capture Driver Lock");
	try
	{
		m_dwOSVer = CSystemUtils::GetOSVersion();
		if ((m_dwOSVer == OS_VERSION_WIN2000)
			|| (m_dwOSVer == OS_VERSION_WIN2003)
			|| (m_dwOSVer == OS_VERSION_WINXP))
		{
			MirrorDriverDetachXP();
			PRINTDEBUGLOG(dtInfo, "xp detach driver succ ");
		} else if ((m_dwOSVer == OS_VERSION_VISTA) || (m_dwOSVer == OS_VERSION_WIN7))
		{
			if (MirrorDriverVista(0, 0, 0, 0, 0))
				PRINTDEBUGLOG(dtInfo, "vista detach Succ");
			else
				PRINTDEBUGLOG(dtInfo, "Vista detach failed");
		}
		if (m_pVideoMem != NULL)
			VideoMemoryReleaseSharedMemory(m_pVideoMem);
		m_pVideoMem = NULL;
		m_pFrameBuffer = NULL;
		m_pChangeBufs = NULL;
	} catch(...)
	{
		PRINTDEBUGLOG(dtInfo, "Stop Driver Exception");
	}
	m_Lock.UnLock();
	PRINTDEBUGLOG(dtError, "End Stop Driver");
}

 

BOOL CVNCMirrorDriver::MirrorDriverAttachXP(int x,int y,int w,int h)
{
	HDESK hInput;
	HDESK hCurrent;

 	//
	DEVMODE mode;
	memset(&mode, 0, sizeof(DEVMODE));
    mode.dmSize = sizeof(DEVMODE);
	mode.dmDriverExtra = 0;
	BOOL bChange = ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);
	mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_POSITION | DM_PELSHEIGHT;
	if ((mode.dmBitsPerPel != 8) && (mode.dmBitsPerPel != 16) && (mode.dmBitsPerPel != 32))
	{	
		return FALSE;
	}
	if (bChange && GetEnumDisplayDevicesFun())
	{
		DISPLAY_DEVICE dispDevice;
		memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
		dispDevice.cb = sizeof(DISPLAY_DEVICE);
		char *szDeviceName = NULL;
		int nDevNum = 0;
		char szDeviceString[256] = {0};
		BOOL bRes;
		while (bRes = (m_pEnumDisplayDevices)(NULL, nDevNum, &dispDevice, 0))
		{
			memset(szDeviceString, 0, 256);
			CStringConversion::WideCharToString(dispDevice.DeviceString, szDeviceString, 256);
			if (strcmp(szDeviceString, szVideoDriverName) == 0)
				break;
			memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
			dispDevice.cb = sizeof(DISPLAY_DEVICE);
			nDevNum ++;
		}
		if (!bRes)
		{
			PRINTDEBUGLOG(dtInfo, "Driver(%s) not Found", szVideoDriverName);
			return FALSE;
		}
        
		char szDeviceKey[256] = {0};
		char szDeviceNum[MAX_PATH] = {0};
		CStringConversion::WideCharToString(dispDevice.DeviceKey, szDeviceKey, 256);
		strupr(szDeviceKey);
		char *szDeviceSub = ::strstr(szDeviceKey, "\\DEVICE");
		if (!szDeviceSub)
			strcpy(szDeviceNum, "DEVICE0");
		else
			strncpy(szDeviceNum, ++szDeviceSub, MAX_PATH);

		char szRegPath[512] = {0};
		sprintf(szRegPath, "SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Services\\mv2\\%s", szDeviceNum);
		if (!CSystemUtils::CreateChildKey(HKEY_LOCAL_MACHINE, szRegPath, NULL))
		{
			return FALSE;
		}
		DWORD dwOne = 1;
		if (!CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szRegPath, "Attach.ToDesktop", (char *)&dwOne, REG_DWORD))
		{
			return FALSE;
		}
		hCurrent = ::GetThreadDesktop(::GetCurrentThreadId());
		if (hCurrent != NULL)
		{
			hInput = ::OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
			if (hInput != NULL)
				::SetThreadDesktop(hInput);
		}
		mode.dmPelsWidth = w;
		mode.dmPelsHeight = h;
		mode.dmPosition.x = x;
		mode.dmPosition.y = y;
		::lstrcpy(mode.dmDeviceName, L"mv2");
		LPCWSTR lpstrDeviceName = dispDevice.DeviceName;
		int nCode = ::ChangeDisplaySettingsEx(lpstrDeviceName, &mode, NULL, CDS_UPDATEREGISTRY, NULL);
		if (nCode != DISP_CHANGE_SUCCESSFUL) //失败
		{
			PRINTDEBUGLOG(dtInfo, "ChangeDisplaySettingsEx Failed, Error Code: %d in attach xp", nCode);
		} else
		{
			nCode = ::ChangeDisplaySettingsEx(lpstrDeviceName, &mode, NULL, 0, NULL);
			if (nCode != DISP_CHANGE_SUCCESSFUL)
			{
				PRINTDEBUGLOG(dtInfo, "after attach ChangeDisplaySettingsEx Failed, Error Code: %d in attach xp", nCode);
			}
			memset(m_szDeviceName, 0, sizeof(TCHAR) * MAX_PATH);
			::lstrcpyn(m_szDeviceName, lpstrDeviceName, MAX_PATH);
		}
		::SetThreadDesktop(hCurrent);
		::CloseDesktop(hInput);
        if (nCode != DISP_CHANGE_SUCCESSFUL)
		{
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

void CVNCMirrorDriver::MirrorDriverDetachXP()
{
	HDESK hInput;
	HDESK hCurrent;
 
	DEVMODE mode;
	memset(&mode, 0, sizeof(DEVMODE));
	mode.dmSize = sizeof(DEVMODE);
    mode.dmDriverExtra = 0;
	BOOL bChange = ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);
	mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    
	if (bChange && GetEnumDisplayDevicesFun())
	{
		DISPLAY_DEVICE dispDevice;
		memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
		dispDevice.cb = sizeof(DISPLAY_DEVICE);
		char *szDeviceName = NULL;
        int nDevNum = 0;
		BOOL bRes;
		char szDeviceString[256] = {0};
		while (bRes = m_pEnumDisplayDevices(NULL, nDevNum, &dispDevice, 0))
		{
			memset(szDeviceString, 0, 256);
			CStringConversion::WideCharToString(dispDevice.DeviceString, szDeviceString, 256);
			if (strcmp(szDeviceString, szVideoDriverName) == 0)
				break;
			memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
			dispDevice.cb = sizeof(DISPLAY_DEVICE);
			nDevNum ++;
		}
		if (!bRes)
		{
			return;
		}
        
		char szDeviceNum[MAX_PATH] = {0};
		char *szDeviceSub;
		char szDeviceKey[256] = {0};
		CStringConversion::WideCharToString(dispDevice.DeviceKey, szDeviceKey, 256);
        strupr(szDeviceKey);
		szDeviceSub = strstr(szDeviceKey, "\\DEVICE");
        if (!szDeviceSub)
			strcpy(szDeviceNum, "DEVICE0");
		else
			strncpy(szDeviceNum, ++szDeviceSub, 255);

		char szRegKey[512] = {0};
		sprintf(szRegKey, "SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Services\\mv2\\%s", szDeviceNum);
		if (!CSystemUtils::CreateChildKey(HKEY_LOCAL_MACHINE, szRegKey, NULL))
		{
			return;
		}
		DWORD dwOne = 0;
		if (!CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szRegKey, "Attach.ToDesktop", (char *)&dwOne, REG_DWORD))
		{
			return;
		}
        szDeviceSub = strstr(szDeviceKey, "SYSTEM");
		if (szDeviceSub)
		{
			sprintf(szRegKey, "SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\%s", szDeviceSub);
			if (!CSystemUtils::CreateChildKey(HKEY_LOCAL_MACHINE, szRegKey, NULL))
			{
				return ;
			}
	        if (!CSystemUtils::WriteRegisterKey(HKEY_LOCAL_MACHINE, szRegKey, "Attach.ToDesktop", "0"))
			{
				return ;
			} 
		}

		lstrcpy(mode.dmDeviceName, L"mv2");
		mode.dmBitsPerPel = 32;
		hCurrent = ::GetThreadDesktop(::GetCurrentThreadId());
		if (hCurrent != NULL)
		{
			hInput = ::OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
			if (hInput != NULL)
				::SetThreadDesktop(hInput);
		}

		int nCode = ::ChangeDisplaySettingsEx(dispDevice.DeviceName, &mode, NULL, CDS_UPDATEREGISTRY, NULL);
		if (nCode != DISP_CHANGE_SUCCESSFUL) //失败
		{
			PRINTDEBUGLOG(dtInfo, "ChangeDisplaySettingsEx Failed, Error Code: %d in dettach xp", nCode);
		}
		nCode = ::ChangeDisplaySettingsEx(dispDevice.DeviceName, &mode, NULL, 0, NULL);
		if (nCode != DISP_CHANGE_SUCCESSFUL)
		{
			PRINTDEBUGLOG(dtInfo, "after ChangeDisplaySettingsEx Failed, Error Code: %d in dettach xp", nCode);
		}
		::SetThreadDesktop(hCurrent);
		::CloseDesktop(hInput);
	}
}

BOOL CVNCMirrorDriver::MirrorDriverVista(DWORD dwAttach, int x, int y, int w, int h)
{
	HDESK hInput;
	HDESK hCurrent;
	//
	DEVMODE mode;
	memset(&mode, 0, sizeof(DEVMODE));
    mode.dmSize = sizeof(DEVMODE);
	mode.dmDriverExtra = 0;
	BOOL bChange = ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);
	mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION;
    
    if (bChange && GetEnumDisplayDevicesFun())
	{
		DISPLAY_DEVICE dispDevice;
		memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
		dispDevice.cb = sizeof(DISPLAY_DEVICE);
		char *szDeviceName = NULL;
		int nDevNum = 0;
		BOOL bRes;
		DWORD cxPrimary = 0xFFFFFFFF;
		DWORD cyPrimary = 0xFFFFFFFF;
		while (bRes = m_pEnumDisplayDevices(NULL, nDevNum, &dispDevice, 0))
		{
			if (dispDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			{
				::EnumDisplaySettings(dispDevice.DeviceName, ENUM_CURRENT_SETTINGS, &mode);
				cxPrimary = mode.dmPelsWidth;
				cyPrimary = mode.dmPelsHeight;
				PRINTDEBUGLOG(dtInfo, "Width:%0X, Height:%0X", mode.dmPelsWidth, mode.dmPelsHeight);
				break;
			}
			nDevNum ++;
        }
		if ((mode.dmBitsPerPel != 8) && (mode.dmBitsPerPel != 16) && (mode.dmBitsPerPel != 32))
		{
			PRINTDEBUGLOG(dtInfo, "attach vista bitsperpel(%d) failed", mode.dmBitsPerPel);
			return FALSE;
		}
		
		if (!bRes)
		{
			PRINTDEBUGLOG(dtInfo, "not Find Prmary device in attach vista");
			return FALSE;
		}
        if ((cxPrimary == 0xFFFFFFFF) || (cyPrimary == 0xFFFFFFFF))
		{
			PRINTDEBUGLOG(dtInfo, "cxPrimary failed in attach vista");
			return false;
		}
        
		nDevNum = 0;
		char szDeviceString[256];
		while (bRes = m_pEnumDisplayDevices(NULL, nDevNum, &dispDevice, 0))
		{
			memset(szDeviceString, 0, 256);
			CStringConversion::WideCharToString(dispDevice.DeviceString, szDeviceString, 256);
			if (strcmp(szDeviceString, szVideoDriverName) == 0)
				break;
			memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
			dispDevice.cb = sizeof(DISPLAY_DEVICE);
			nDevNum ++;
		}
		if (!bRes)
		{
			PRINTDEBUGLOG(dtInfo, "not find mv2 driver");
			return FALSE;
		}
        
		char szDeviceNum[MAX_PATH] = {0};
		char *szDeviceSub;
		char szDeviceKey[256] = {0};
		CStringConversion::WideCharToString(dispDevice.DeviceKey, szDeviceKey, 256);
		strupr(szDeviceKey);
		szDeviceSub = strstr(szDeviceKey, "\\DEVICE");
		if (!szDeviceSub)
			strcpy(szDeviceNum, "DEVICE0");
		else
			strncpy(szDeviceNum, ++szDeviceSub, 255);

		//
		mode.dmSize = sizeof(DEVMODE);
		mode.dmDriverExtra = 0;
		mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_POSITION;
       
		::lstrcpy(mode.dmDeviceName, L"mv2");
        //设置 
		if (dwAttach == 0)
		{
			mode.dmPelsWidth = 0;
			mode.dmPelsHeight = 0;
		} else
		{
			mode.dmPelsWidth = w;
			mode.dmPelsHeight = h;
			mode.dmPosition.x = x;
			mode.dmPosition.y = y;
		}

		hCurrent = ::GetThreadDesktop(::GetCurrentThreadId());
		if (hCurrent != NULL)
		{
			hInput = ::OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
			if (hInput != NULL)
			{
				::SetThreadDesktop(hInput);
			}
		}
		int nCode = ::ChangeDisplaySettingsEx(dispDevice.DeviceName, &mode, NULL, CDS_UPDATEREGISTRY | CDS_RESET | CDS_GLOBAL, NULL);
		if (nCode != DISP_CHANGE_SUCCESSFUL)
		{
			PRINTDEBUGLOG(dtInfo, "ChangeDisplaySettingsEx Failed, Error Code: %d in attach vista", nCode);
		} else
		{
			nCode = ::ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);
			if (nCode != DISP_CHANGE_SUCCESSFUL)
			{
				PRINTDEBUGLOG(dtInfo, "after ChangeDisplaySettingsEx Failed, Error Code: %d in attach vista", nCode);
			}
			memset(m_szDeviceName, 0, sizeof(TCHAR) * MAX_PATH);
			::lstrcpyn(m_szDeviceName, dispDevice.DeviceName, MAX_PATH);
		}
		::SetThreadDesktop(hCurrent);
		::CloseDesktop(hInput);
		if (nCode != DISP_CHANGE_SUCCESSFUL)
		{
			PRINTDEBUGLOG(dtInfo, "change displaysetting failed");
			return FALSE;
		}            
		return TRUE;
	} else
	{
		PRINTDEBUGLOG(dtInfo, "not change, change:%d", bChange);
	}
	return FALSE;
}

char *CVNCMirrorDriver::VideoMemoryGetSharedMemory(void)
{
	char *pVideoMem = NULL;
	HANDLE hMapFile, hFile;
	//创建文件
	hFile = ::CreateFile(L"C:\\video0.dat",  GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = ::CreateFile(L"C:\\video1.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		hMapFile = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
		if (hMapFile && hMapFile != INVALID_HANDLE_VALUE)
		{
			pVideoMem = (char *)::MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
			::CloseHandle(hMapFile);
		}
		::CloseHandle(hFile);
	} else
	{
		PRINTDEBUGLOG(dtInfo, "Create Video.data Fail:%d", ::GetLastError());
	}
	return pVideoMem;
}

void CVNCMirrorDriver::VideoMemoryReleaseSharedMemory(char * pVideoMemory)
{
	::UnmapViewOfFile(pVideoMemory);
}

HDC  CVNCMirrorDriver::GetDcMirror()
{
	HDC hdc = NULL;
	if (GetEnumDisplayDevicesFun())
	{		 
		DEVMODE mode;
		FillMemory(&mode, sizeof(DEVMODE), 0);
		mode.dmSize = sizeof(DEVMODE);
		mode.dmDriverExtra = 0;
		BOOL change = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);
		mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		if (::lstrlen(m_szDeviceName) > 0)
		{
			hdc = ::CreateDC(_T("DISPLAY"), m_szDeviceName, NULL, NULL);
			if (!hdc)
			{
				PRINTDEBUGLOG(dtInfo, "Create DC Failed, Error:%d", ::GetLastError());
			}
		}
	}
	return hdc;
}

void CVNCMirrorDriver::DoRefresh()
{
	if (!m_pChangeBufs)
		return ;
	if (m_dwOld == m_pChangeBufs->dwCounter) //时间段内没有变化
		return;
	if ((m_pChangeBufs->dwCounter < 1) || (m_pChangeBufs->dwCounter > MAXCHANGES_BUF - 1))
		return;
	CRegion2D rcChanged;
	DWORD dwCounter = m_pChangeBufs->dwCounter;
	if (m_dwOld < dwCounter)		
	{
		for (DWORD i = m_dwOld; i < dwCounter; i ++)
			CopyClipBitmap(i, rcChanged);
	} else
	{
		DWORD i = 0;
		for (i = m_dwOld + 1; i < MAXCHANGES_BUF; i ++)
			CopyClipBitmap(i, rcChanged);
		for (i = 1; i < dwCounter; i ++)
			CopyClipBitmap(i, rcChanged);
	}
    m_dwOld = dwCounter;
	if (!m_bTerminated)
		OnRectChange(rcChanged);
}

inline bool ClipRect(int *x, int *y, int *w, int *h,   int cx, int cy, int cw, int ch) 
{
	if (*x < cx) 
	{
		*w -= (cx - *x);
		*x = cx;
	}
	if (*y < cy) 
	{
		*h -= (cy - *y);
		*y = cy;
	}
	if ((*x + *w) > (cx + cw)) 
	{
		*w = (cx + cw) - *x;
	}
	if ((*y + *h) > (cy + ch)) 
	{
		*h = (cy + ch) - *y;
	}
	return (*w > 0) && (*h > 0);
}

void CVNCMirrorDriver::CopyClipBitmap(DWORD dwClip, CRegion2D &rcChanged)
{
	int x = m_pChangeBufs->Data[dwClip].rc.left;
	int w = m_pChangeBufs->Data[dwClip].rc.right - m_pChangeBufs->Data[dwClip].rc.left;
	int y = m_pChangeBufs->Data[dwClip].rc.top;
	int h = m_pChangeBufs->Data[dwClip].rc.bottom - m_pChangeBufs->Data[dwClip].rc.top;
	if (!ClipRect(&x, &y, &w, &h, m_rcDesktop.left, m_rcDesktop.top, m_rcDesktop.right - m_rcDesktop.left,
		m_rcDesktop.bottom - m_rcDesktop.top))
		return;
	RECT rc = {x, y, x + w, y + h};
	switch(m_pChangeBufs->Data[dwClip].type)
	{
	case SCREEN_SCREEN: //全屏变化
		 if (m_pChangeBufs->Data[dwClip].pt.x == 0 || m_pChangeBufs->Data[dwClip].pt.y == 0)
		 {

			//
		 }
		 //PRINTDEBUGLOG(dtInfo, "full screen changed");
		 rcChanged.Union(rc);
		 break;
	case BLIT:
	case SOLIDFILL:
	case BLEND:
	case TRANS:
	case PLG:
	case TEXTOUT:
		 rcChanged.Union(rc);
		 break;		  
	}
}

//区域改变通知
void CVNCMirrorDriver::OnRectChange(const CRegion2D &rcChanged)
{
	//子类继承
}

//产生颜色对应表
BOOL CVNCMirrorDriver::CreateMapColor()
{
	PRINTDEBUGLOG(dtError, "Entry Create MapColor");
	BOOL bSucc = TRUE;
	//初始化颜色索引表
	if (m_bInitDriverSucc && m_pFrameBuffer)
	{
		m_Translate.InitTranslate(m_pFrameBuffer, (WORD)m_bmInfo.bmInfo.bmiHeader.biWidth, 
    		(WORD)abs(m_bmInfo.bmInfo.bmiHeader.biHeight), (WORD)m_bmInfo.bmInfo.bmiHeader.biBitCount / 8);
	} else
	{
		if (m_pBits)
		{
			RefreshMemDC();
			m_Translate.InitTranslate(m_pBits, (WORD)m_bmInfo.bmInfo.bmiHeader.biWidth, 
						   (WORD)abs(m_bmInfo.bmInfo.bmiHeader.biHeight), (WORD)m_bmInfo.bmInfo.bmiHeader.biBitCount / 8);
		} else
		{
			PRINTDEBUGLOG(dtInfo, "DIBits Failed");
			bSucc = FALSE;
		}
	}
	PRINTDEBUGLOG(dtError, "Leave Create MapColor");
	return bSucc;
}

//初始化bitmap
BOOL CVNCMirrorDriver::InitBufferBitmap()
{
	PRINTDEBUGLOG(dtError, "Entry InitBufferBitmap Lock");
	m_Lock.Lock();
	PRINTDEBUGLOG(dtError, "Leave InitBufferBitmap Lock");
	BOOL bSucc = TRUE;
	if (!m_hDesktop)
	{
		try
		{
			do
			{
				//桌面相关
				m_rcDesktop.left = 0;
				m_rcDesktop.top = 0;
				m_rcDesktop.right = ::GetSystemMetrics(SM_CXSCREEN);
				m_rcDesktop.bottom = ::GetSystemMetrics(SM_CYSCREEN);
			    
				m_hDesktop = ::CreateDC(L"DISPLAY", NULL, 0, NULL);
				if (!m_hDesktop)
					break;
				if (m_bInitDriverSucc)
				{
					m_hDeviceDC = GetDcMirror();
					if (!m_hDeviceDC)
						break;
				}
				if (m_hDeviceDC)
					m_hMemDC = ::CreateCompatibleDC(m_hDeviceDC);
				else
					m_hMemDC = ::CreateCompatibleDC(m_hDesktop);
				if (!m_hMemDC)
					break;
				//创建内存bitmap
				if (m_hDeviceDC)
					m_hBufferBitmap = ::CreateCompatibleBitmap(m_hDeviceDC, m_rcDesktop.right, m_rcDesktop.bottom);
				else
					m_hBufferBitmap = ::CreateCompatibleBitmap(m_hDesktop, m_rcDesktop.right, m_rcDesktop.bottom);
				if (!m_hBufferBitmap)
				{
					PRINTDEBUGLOG(dtInfo, "Create Buffer Bitmap Failed");
					break;
				}
				//设置参数
				//检测是否提供BITBLT方式
				if ((::GetDeviceCaps(m_hDesktop, RASTERCAPS) & RC_BITBLT) == 0)
					break;
				if ((::GetDeviceCaps(m_hMemDC, RASTERCAPS) & RC_DI_BITMAP) == 0)
					break;	
			 
				memset(&m_bmInfo, 0, sizeof(m_bmInfo));
				m_bmInfo.bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				m_bmInfo.bmInfo.bmiHeader.biBitCount = 0;
				if (::GetDIBits(m_hMemDC, m_hBufferBitmap, 0, 1, NULL, &(m_bmInfo.bmInfo), DIB_RGB_COLORS) == 0)
					break;
				 
				if (::GetDIBits(m_hMemDC, m_hBufferBitmap,  0, 1, NULL, &(m_bmInfo.bmInfo), DIB_RGB_COLORS) == 0)
					break;
				 
				m_bmInfo.bmInfo.bmiHeader.biCompression = BI_RGB;
				m_bmInfo.bmInfo.bmiHeader.biHeight = -abs(m_bmInfo.bmInfo.bmiHeader.biHeight); //
				//每行的图像信息字节数
				m_dwBytesPerRow = m_rcDesktop.right * m_bmInfo.bmInfo.bmiHeader.biBitCount / 8;
				if (!m_bInitDriverSucc)
				{
					HBITMAP hBitmap = ::CreateDIBSection(m_hMemDC, &m_bmInfo.bmInfo, DIB_RGB_COLORS, (void **)&m_pBits, NULL, 0);
					if (hBitmap)
					{
						::DeleteObject(m_hBufferBitmap);
						m_hBufferBitmap = hBitmap;
					} else
					{
						PRINTDEBUGLOG(dtInfo, "CreateDIBSection Failed:%d", GetLastError());
						break;
					}
				}
				m_Lock.UnLock();
				return CreateMapColor();
			} while (FALSE);

			if (m_hDesktop)
				::ReleaseDC(::GetDesktopWindow(), m_hDesktop);
			if (m_hDeviceDC)
				::DeleteDC(m_hDeviceDC);
			if (m_hMemDC)
				::DeleteDC(m_hMemDC);
			if (m_hBufferBitmap)
				::DeleteObject(m_hBufferBitmap);
			m_hDesktop = NULL;
			m_hDeviceDC = NULL;
			m_hMemDC = NULL;
			m_hBufferBitmap = NULL;
			m_pBits = NULL;
			PRINTDEBUGLOG(dtInfo, "init Buffer Bitmap Failed");
		} catch(...)
		{
			PRINTDEBUGLOG(dtInfo, "InitBuffer Exception");
		}
		m_Lock.UnLock();
		return FALSE;
	}
	m_Lock.UnLock();
	return TRUE;
}

//删除Bitmap
void CVNCMirrorDriver::DestroyBitmap()
{
	m_Lock.Lock();
	try
	{
		if (m_hBufferBitmap)
			::DeleteObject(m_hBufferBitmap);
		if (m_hMemDC)
			::DeleteDC(m_hMemDC);
		if (m_hDeviceDC)
			::DeleteDC(m_hDeviceDC);
		if (m_hDesktop)
			::DeleteDC(m_hDesktop);
		m_hDesktop = NULL;
		m_hDeviceDC = NULL;
		m_hMemDC = NULL;
		m_hBufferBitmap = NULL;
	} catch(...)
	{
		PRINTDEBUGLOG(dtError, "DestroyBitmap Exception");
	}
	m_Lock.UnLock();
}

//更新到缓存
BOOL  CVNCMirrorDriver::RefreshMemDC()
{
	HBITMAP hOld = (HBITMAP) ::SelectObject(m_hMemDC, m_hBufferBitmap);
	BOOL bSucc = ::BitBlt(m_hMemDC, 0, 0, m_rcDesktop.right, m_rcDesktop.bottom,
				 m_hDesktop, 0, 0, SRCCOPY | CAPTUREBLT);
	if (m_bDrawCursor)
	{
		//鼠标
		CURSORINFO CursorInfo = {0};
		CursorInfo.cbSize = sizeof(CURSORINFO);
		if (::GetCursorInfo(&CursorInfo))
		{
			::DrawIconEx(m_hMemDC, CursorInfo.ptScreenPos.x, CursorInfo.ptScreenPos.y, CursorInfo.hCursor,
						0, 0, 0, NULL, DI_NORMAL | DI_COMPAT);
		}
	}
	//恢复原状
	::SelectObject(m_hMemDC, hOld);
	return bSucc;
}

//截取屏幕数据
BOOL CVNCMirrorDriver::CaptureScreen(char *lpBuff, DWORD &dwBufSize, const CRegionRect &rc, BOOL bTranslate)
{
	m_Lock.Lock();
	BOOL bSucc = FALSE;
	try
	{
		char *pFrameBuffer = NULL;
		if (m_bInitDriverSucc && m_pFrameBuffer)
			pFrameBuffer = m_pFrameBuffer;
		else if (m_pBits)
		{
			//PRINTDEBUGLOG(dtInfo, "Capture Screen(l:%d t:%d r:%d b:%d)", rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetBottom());
			//RefreshMemDC();
			pFrameBuffer = m_pBits;
		}
		if (pFrameBuffer)
		{
			if (bTranslate)
			{
				bSucc = m_Translate.Translate256(pFrameBuffer, (WORD)m_bmInfo.bmInfo.bmiHeader.biWidth,
						   (WORD)abs(m_bmInfo.bmInfo.bmiHeader.biHeight), (WORD)(m_bmInfo.bmInfo.bmiHeader.biBitCount / 8),
									  &(rc.GetRect()), lpBuff, dwBufSize);
			} else
			{
				dwBufSize = 0;
		        
				//拷贝图像，从下往上
				//PRINTDEBUGLOG(dtInfo, "Get Screen, Rect(Left:%d Top:%d Right:%d Bottom:%d", rc.GetLeft(), rc.GetTop(), rc.GetRight(), rc.GetBottom());
				char *pDestBuff = lpBuff;
				int nBytesPerPixel = m_bmInfo.bmInfo.bmiHeader.biBitCount / 8;
				char *pSrcBuff = pFrameBuffer;
				pSrcBuff += (m_dwBytesPerRow * rc.GetTop() + nBytesPerPixel * rc.GetLeft());
				int nWidthBytes = rc.Width() * nBytesPerPixel;
				nWidthBytes = (nWidthBytes + 3) / 4 * 4;
				for (int y = rc.GetTop(); y < rc.GetBottom(); y ++)
				{
					memcpy(pDestBuff, pSrcBuff , nWidthBytes);
					pSrcBuff += m_dwBytesPerRow;
					pDestBuff += nWidthBytes;
					dwBufSize += nWidthBytes;
				} 
				//SaveBufferToBmpFile(m_hDesktop, m_hMemDC, lpBuff, m_bmInfo, rc.Width(), rc.Height());
				bSucc = TRUE; 
			}			
		} else
		{
			PRINTDEBUGLOG(dtInfo, "Get Screen Image Failed");
			//::GetDIBits(m_hMemDC, m_hBufferBitmap, 
		}	

	} catch(...)
	{
	}
	m_Lock.UnLock();
	return bSucc;
}

//获取某一象素点值
COLORREF CVNCMirrorDriver::CapturePixel(int x, int y)
{
	if (m_pBits)
	{
		COLORREF cr = 0;
		memmove(&cr, m_pBits + y * m_dwBytesPerRow + x * m_bmInfo.bmInfo.bmiHeader.biBitCount / 8, m_bmInfo.bmInfo.bmiHeader.biBitCount / 8);
		return cr;
	}
	return 0;
}

//获取颜色索引表值
BOOL CVNCMirrorDriver::GetColorMap(RGBQUAD *pClrMap, DWORD &dwSize)
{
	return m_Translate.GetColorMap(pClrMap, dwSize);
}

//截鼠标
BOOL CVNCMirrorDriver::CaptureMouse(char *lpBuff, DWORD &dwBufSize, RECT &rc, HCURSOR &hOld)
{
	return FALSE;
}

//
BOOL CVNCMirrorDriver::GetBitmapInfo(DESKTOP_BITMAP_INFO *pInfo)
{
	memmove(pInfo, &m_bmInfo, sizeof(DESKTOP_BITMAP_INFO));
	return TRUE;
}

RECT CVNCMirrorDriver::GetDesktopRect()
{
	return m_rcDesktop;
}

