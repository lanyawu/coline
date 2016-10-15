#ifndef __IOCOMPLETIONPORT_H____
#define __IOCOMPLETIONPORT_H____

#include <commonlib/types.h>

class CIoCompletionPort
{
public:
	//��������� dwMaxCon
	explicit CIoCompletionPort(DWORD dwMaxCon);
	~CIoCompletionPort(void);
public:
	//����IOCP�����豸
	BOOL AssociateDevice(HANDLE hDevice, ULONG_PTR CompletionKey);
	//����״̬
	void PostStatus(ULONG_PTR dwCompletionKey, DWORD dwNumBytes = 0, LPOVERLAPPED pOverlapped = NULL);
	void GetStatus(PULONG_PTR pCompletionKey, LPDWORD pdwNumBytes, LPOVERLAPPED *ppOverlapped);
	BOOL GetStatus(PULONG_PTR pCompletionKey, LPDWORD pdwNumBytes, LPOVERLAPPED *ppOverlapped, DWORD dwMilliseconds);
	BOOL GetStatus(PULONG_PTR pCompletionKey, LPDWORD pdwNumBytes, LPOVERLAPPED *ppOverlapped, LPDWORD pLastError);
private:
	HANDLE m_hIocp; //
};

#endif