#include <commonlib/debuglog.h>
#include <netlib/IoCompletionPort.h>

CIoCompletionPort::CIoCompletionPort(DWORD dwMaxCon)
{
	m_hIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, dwMaxCon);
	if (!m_hIocp)
	{
		PRINTDEBUGLOG(dtInfo, "create io completion port error");
	}
}

CIoCompletionPort::~CIoCompletionPort(void)
{
	if (m_hIocp)
		::CloseHandle(m_hIocp);
}

BOOL CIoCompletionPort::AssociateDevice(HANDLE hDevice, ULONG_PTR dwCompletionKey)
{
	if (m_hIocp != ::CreateIoCompletionPort(hDevice, m_hIocp, dwCompletionKey, 0))
	{
		PRINTDEBUGLOG(dtInfo, "AssociateDevice error:", ::GetLastError());
		return FALSE;
	}
	return TRUE;
}

void CIoCompletionPort::PostStatus(ULONG_PTR dwCompletionKey, DWORD dwNumBytes, LPOVERLAPPED pOverlapped)
{
	if (0 == ::PostQueuedCompletionStatus(m_hIocp, dwNumBytes, dwCompletionKey, pOverlapped))
	{
		PRINTDEBUGLOG(dtInfo, "post completion port error:%d", ::GetLastError());
	}
}
 
void CIoCompletionPort::GetStatus(PULONG_PTR pCompletionKey, LPDWORD lpNumBytes, LPOVERLAPPED *ppOverlapped)
{
	if (0 == ::GetQueuedCompletionStatus(m_hIocp, lpNumBytes, pCompletionKey, ppOverlapped, INFINITE))
	{
		//PRINTDEBUGLOG(dtInfo, "get completion port error:%d", ::GetLastError());
	}
}

BOOL CIoCompletionPort::GetStatus(PULONG_PTR pCompletionKey, PDWORD lpNumBytes, 
								   LPOVERLAPPED *ppOverlapped, DWORD dwMilliseconds)
{
	if (0 == ::GetQueuedCompletionStatus(m_hIocp, lpNumBytes, pCompletionKey, ppOverlapped, dwMilliseconds))
	{
		//PRINTDEBUGLOG(dtInfo, "get completion port error:%d", ::GetLastError());
		if (::GetLastError() != WAIT_TIMEOUT)
			return FALSE;
	}
	return TRUE;
}

BOOL CIoCompletionPort::GetStatus(PULONG_PTR pCompletionKey, PDWORD lpNumBytes, 
								  LPOVERLAPPED *ppOverlapped, LPDWORD pLastError)
{
	if (0 == ::GetQueuedCompletionStatus(m_hIocp, lpNumBytes, pCompletionKey, ppOverlapped, INFINITE))
	{
		if (pLastError)
			*pLastError = ::GetLastError();
		//PRINTDEBUGLOG(dtInfo, "Get completion port error:%d", ::GetLastError());
		return FALSE;
	} else
	{
		if (pLastError)
			*pLastError = 0;
	}
	return TRUE;
}