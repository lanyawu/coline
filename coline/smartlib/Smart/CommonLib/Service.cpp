#include <CommonLib/service.h>
#include <commonLib/StringUtils.h>

#pragma warning(disable:4996)

CService::CService(const char *szServiceName, const char *szDisplayName)
{
	memset(m_strServiceName, 0, MAX_PATH * sizeof(TCHAR));
	memset(m_strDisplayName, 0, MAX_PATH * sizeof(TCHAR));
	CStringConversion::StringToWideChar(szServiceName, m_strServiceName, MAX_PATH);
	CStringConversion::StringToWideChar(szDisplayName, m_strDisplayName, MAX_PATH);
	m_ServiceStatus.dwWin32ExitCode = 0;
	m_hServiceStatus = NULL;
	m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_POWEREVENT|SERVICE_ACCEPT_PAUSE_CONTINUE;
	m_ServiceStatus.dwWin32ExitCode = 0;
	m_ServiceStatus.dwServiceSpecificExitCode = 0;
	m_ServiceStatus.dwCheckPoint = 0;
	m_ServiceStatus.dwWaitHint = 0;
	_pService = this;
}

CService::~CService(void)
{

}

BOOL CService::Install()
{
	if (IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, L"Couldn't open service manager", m_strServiceName, MB_OK);
		return FALSE;
	}

	// Get the executable file path
	TCHAR szFilePath[_MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

	SC_HANDLE hService = ::CreateService(
		hSCM, m_strServiceName, m_strDisplayName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, L"RPCSS\0", NULL, NULL);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, L"Couldn't create service", m_strServiceName, MB_OK);
		return FALSE;
	}

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL CService::Uninstall()
{
	if (!IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		MessageBox(NULL, L"Couldn't open service manager", m_strServiceName, MB_OK);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, m_strServiceName, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, L"Couldn't open service", m_strServiceName, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)
		return TRUE;

	MessageBox(NULL, L"Service could not be deleted", m_strServiceName, MB_OK);
	return FALSE;
}

BOOL CService::IsInstalled()
{
	BOOL bResult = FALSE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		SC_HANDLE hService = ::OpenService(hSCM, m_strServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

BOOL CService::Init()
{
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = (LPWSTR)m_strServiceName;
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)_ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;

	if (::StartServiceCtrlDispatcher(ServiceTable) == 0)
		m_ServiceStatus.dwWin32ExitCode = GetLastError();
	return m_ServiceStatus.dwWin32ExitCode = 0?TRUE:FALSE;
}

void CService::ServiceMain(DWORD argc, LPTSTR* argv)
{
	m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_POWEREVENT|SERVICE_ACCEPT_PAUSE_CONTINUE;
	m_ServiceStatus.dwWin32ExitCode = 0;
	m_ServiceStatus.dwServiceSpecificExitCode = 0;
	m_ServiceStatus.dwCheckPoint = 0;
	m_ServiceStatus.dwWaitHint = 0;
	m_hServiceStatus = RegisterServiceCtrlHandler(
		m_strServiceName, 
		(LPHANDLER_FUNCTION)_ControlHandler); 
	if (m_hServiceStatus == (SERVICE_STATUS_HANDLE)0) 
	{ 
		m_ServiceStatus.dwWin32ExitCode = GetLastError();
		CloseHandle(m_hServiceStatus);
		return; 
	}  
	if(!InitService())
	{
		m_ServiceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(SERVICE_STOPPED);
		CloseHandle(m_hServiceStatus);
		return;
	}
	m_ServiceStatus.dwWin32ExitCode = Run();
	CloseHandle(m_hServiceStatus);
	return;
}

void CService::ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		OnStop();
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		OnShutdown();
		break;
	case SERVICE_CONTROL_POWEREVENT:
		OnPowerEvent();
		break;
	case SERVICE_CONTROL_PAUSE:
		OnPause();
		break;
	case SERVICE_CONTROL_CONTINUE:
		OnContinue();
		break;
	default:
		SetServiceStatus (m_ServiceStatus.dwCurrentState);
		break;
	}
}

void CService::SetServiceStatus(DWORD dwState)
{
	m_ServiceStatus.dwCurrentState = dwState;
	::SetServiceStatus(m_hServiceStatus, &m_ServiceStatus);
}

BOOL CService::InitService()
{
	return TRUE;
}

int CService::Run()
{
	SetServiceStatus(m_ServiceStatus.dwCurrentState);
	return S_OK;
}

void CService::OnStop()
{
	SetServiceStatus(m_ServiceStatus.dwCurrentState);
}

void CService::OnShutdown()
{
	SetServiceStatus(m_ServiceStatus.dwCurrentState);
}

void CService::OnPowerEvent()
{
	SetServiceStatus(m_ServiceStatus.dwCurrentState);
}

void CService::OnPause()
{
	SetServiceStatus(m_ServiceStatus.dwCurrentState);
}

void CService::OnContinue()
{
	SetServiceStatus(m_ServiceStatus.dwCurrentState);
}

#pragma warning(default:4996)