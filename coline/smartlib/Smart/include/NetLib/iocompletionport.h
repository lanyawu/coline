#ifndef __IOCOMPLETIONPORT_H____
#define __IOCOMPLETIONPORT_H____

#include <commonlib/types.h>

class CIoCompletionPort
{
public:
	//最大连接数 dwMaxCon
	explicit CIoCompletionPort(DWORD dwMaxCon);
	~CIoCompletionPort(void);
public:
	//创建IOCP关联设备
	BOOL AssociateDevice(HANDLE hDevice, ULONG_PTR CompletionKey);
	//传入状态
	void PostStatus(ULONG_PTR dwCompletionKey, DWORD dwNumBytes = 0, LPOVERLAPPED pOverlapped = NULL);
	void GetStatus(PULONG_PTR pCompletionKey, LPDWORD pdwNumBytes, LPOVERLAPPED *ppOverlapped);
	BOOL GetStatus(PULONG_PTR pCompletionKey, LPDWORD pdwNumBytes, LPOVERLAPPED *ppOverlapped, DWORD dwMilliseconds);
	BOOL GetStatus(PULONG_PTR pCompletionKey, LPDWORD pdwNumBytes, LPOVERLAPPED *ppOverlapped, LPDWORD pLastError);
private:
	HANDLE m_hIocp; //
};

#endif