#ifndef __ITMSG_SERVICE_H__
#define __ITMSG_SERVICE_H__

#include <windows.h>
#include <string>
#include <commonlib/types.h>

class CService;
__declspec(selectany) CService* _pService = NULL;

class COMMONLIB_API CService
{
public:
	CService(const char *szServiceName, const char *szDisplayName);
	virtual ~CService(void);
	virtual BOOL Install();
	virtual BOOL Uninstall();
	virtual BOOL IsInstalled();
	virtual BOOL InitService();
	BOOL Init();
	void SetServiceStatus(DWORD dwState);
	void ServiceMain(DWORD argc, LPTSTR* argv);
	void ControlHandler(DWORD request);
protected:
	virtual int Run();
	virtual void OnStop();
	virtual void OnShutdown();
	virtual void OnPowerEvent();
	virtual void OnPause();
	virtual void OnContinue();
protected:
	TCHAR  m_strServiceName[MAX_PATH];
	TCHAR  m_strDisplayName[MAX_PATH];
	SERVICE_STATUS m_ServiceStatus; 
	SERVICE_STATUS_HANDLE m_hServiceStatus;
protected:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
	{
		_pService->ServiceMain(dwArgc, lpszArgv);
	}
	static void WINAPI _ControlHandler(DWORD request)
	{
		_pService->ControlHandler(request);
	}
};

#endif