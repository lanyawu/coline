// asock.cpp : 定义 DLL 应用程序的入口点。
//


#include <tchar.h>
#include <deque>
#include <Netlib/asock.h>
#include <CommonLib/GuardLock.h>
#include <CommonLib/debuglog.h>
#include <MSTcpIP.h>

using namespace std;

#pragma warning(disable:4996)

// == class CNetAddress =====
CNetAddress::CNetAddress()
{
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
}

CNetAddress::~CNetAddress()
{
}

CNetAddress & CNetAddress::operator =(const CNetAddress &addr)
{
	memmove(&m_addr, &(addr.m_addr), sizeof(struct sockaddr_in));
	return *this;
}

CNetAddress & CNetAddress::operator =(sockaddr &addr)
{
	memmove(&m_addr, &addr, sizeof(struct sockaddr));
	return *this;
}

CNetAddress & CNetAddress::operator =(sockaddr_in &addr)
{
	memmove(&m_addr, &addr, sizeof(struct sockaddr_in));
	return *this;
}

void CNetAddress::SetAddress(DWORD dwIp, WORD wPort)
{
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
	m_addr.sin_addr.S_un.S_addr = htonl(dwIp);
	m_addr.sin_family = PF_INET;
	m_addr.sin_port = htons(wPort);
}

void CNetAddress::GetAddress(sockaddr &addr)
{
	memmove(&addr, &m_addr, sizeof(sockaddr));
}

class CASocketKernel
{
public:
	CASocketKernel();
	virtual ~CASocketKernel();
	inline DWORD GetASocketCount(){return m_nEventCount;}
	BOOL AddASocket(CASocket * pASocket,long lEvent);
	bool DeleteASocket(CASocket * pASocket);
	void WaitThreadExit();
	HANDLE m_hKernelThread;
private:
	HANDLE m_ASocketEvent[MAXEVENTCOUNT+1];
	CASocket * m_ASocketObject[MAXEVENTCOUNT+1];
	
	DWORD m_nEventCount;
	CGuardLock m_lockEvent;
	BOOL SearchBin(CASocket * pTarget, DWORD & dwPos);
	void ResetThread();
	HANDLE m_hProcEvent;
	static DWORD WINAPI _ThreadASocketKernelEvent(LPVOID lpvoid);
};

CASocketKernel::CASocketKernel()
{
	DWORD dwThreadID;
	m_nEventCount = 0;
	m_ASocketObject[0]=NULL;
	m_ASocketEvent[0]=CreateEvent(NULL,TRUE,FALSE,NULL);
	m_hKernelThread = CreateThread(NULL,0,_ThreadASocketKernelEvent, (LPVOID)this, THREAD_PRIORITY_NORMAL, &dwThreadID);
}

CASocketKernel::~CASocketKernel()
{
	CloseHandle(m_hKernelThread);
	CloseHandle(m_ASocketEvent[0]);
}


BOOL CASocketKernel::AddASocket(CASocket * pASocket,long lEvent)
{
	if(m_nEventCount >= MAXEVENTCOUNT)
		return FALSE;
	HANDLE hEvent = WSACreateEvent();
	if(hEvent == WSA_INVALID_EVENT)
	{
		return FALSE;
	}
	if(WSAEventSelect(pASocket->m_hSocket, hEvent, lEvent) == SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		WSACloseEvent(hEvent);
		WSASetLastError(nError);
		return FALSE;
	}
	DWORD dwPos;
	m_lockEvent.Lock();
	try
	{
		SearchBin(pASocket,dwPos);
		if(dwPos != m_nEventCount + 1)
		{
			HANDLE * pTemp = new HANDLE[m_nEventCount+1 - dwPos];
			CASocket ** pObj = new CASocket *[m_nEventCount+1 - dwPos];
			memcpy(pTemp, &m_ASocketEvent[dwPos], sizeof(HANDLE) * (m_nEventCount + 1 - dwPos));
			memcpy(&m_ASocketEvent[dwPos + 1], pTemp, sizeof(HANDLE) * (m_nEventCount + 1 - dwPos));
			memcpy(pObj, &m_ASocketObject[dwPos], sizeof(CASocket *) * (m_nEventCount + 1 - dwPos));
			memcpy(&m_ASocketObject[dwPos + 1], pObj, sizeof(CASocket *) * (m_nEventCount + 1 - dwPos));
			delete [] pTemp;
			delete [] pObj;
		}
		m_ASocketEvent[dwPos] = hEvent;
		m_ASocketObject[dwPos] = pASocket;
		m_nEventCount++;
	} catch(...)
	{
		//
	}
	ResetThread();
	m_lockEvent.UnLock();
	return TRUE;
}

bool CASocketKernel::DeleteASocket(CASocket * pASocket)
{
	m_lockEvent.Lock();
	DWORD dwPos;
	BOOL bFind = SearchBin(pASocket,dwPos);
	if(!bFind)
	{
		m_lockEvent.UnLock();
		return false;
	}
	HANDLE hEvent = m_ASocketEvent[dwPos];
	try
	{
		if(dwPos != m_nEventCount)
		{
			HANDLE * pTemp = new HANDLE[m_nEventCount - dwPos];
			CASocket ** pObj = new CASocket * [m_nEventCount - dwPos];
			memcpy(pTemp, &m_ASocketEvent[dwPos + 1], sizeof(HANDLE) * (m_nEventCount - dwPos));
			memcpy(&m_ASocketEvent[dwPos], pTemp, sizeof(HANDLE) * (m_nEventCount - dwPos));
			memcpy(pObj, &m_ASocketObject[dwPos + 1],sizeof(CASocket *) * (m_nEventCount - dwPos));
			memcpy(&m_ASocketObject[dwPos], pObj, sizeof(CASocket *) * (m_nEventCount - dwPos));
			delete [] pTemp;
			delete [] pObj;
		}
		m_ASocketObject[m_nEventCount] = NULL;
		m_ASocketEvent[m_nEventCount] = NULL;
		m_nEventCount--;
	} catch(...)
	{
	
	}
	WSACloseEvent(hEvent);
	ResetThread();
	m_lockEvent.UnLock();
	return true;
}

void CASocketKernel::ResetThread()
{
	SetEvent(m_ASocketEvent[0]);
}

BOOL CASocketKernel::SearchBin(CASocket * pTarget,DWORD & dwPos) //二分查找算法 
{ 
	dwPos = (DWORD)-1;
	DWORD low,mid,high;
	BOOL bResult = FALSE;
	low = 1;
	high = m_nEventCount; //初始化查找区间 
	while ( low <= high )
	{ 
		if ((int)high<0)
		{
			dwPos = 0;
			return FALSE;
		}
		mid = ( low + high )/2 ; //求区间中心的位置 
		if ( m_ASocketObject[mid] == pTarget )
		{
			bResult = TRUE;
			dwPos = mid;
			break;
		}
		if (m_ASocketObject[mid] > pTarget) 
			high = mid - 1 ;//中心节点关键字偏大，查找低半段 
		else 
			low = mid + 1 ; //中心节点关键字偏小，查找高半段 
	}
	if (dwPos == (DWORD)-1)
	{
		dwPos = (low + high) / 2 + 1;
	}
	return bResult; //查找失败，返回失败标志 
}

//主消息泵of CASocketKernel
DWORD WINAPI CASocketKernel::_ThreadASocketKernelEvent(LPVOID lpvoid)
{
	CASocketKernel * pThis = (CASocketKernel *)lpvoid;
	while (TRUE)
	{
		DWORD dwIndex = WSAWaitForMultipleEvents(pThis->m_nEventCount + 1, pThis->m_ASocketEvent, FALSE, WSA_INFINITE, FALSE);

		//int nEventCount = pThis->m_nEventCount;

		pThis->m_lockEvent.Lock();
		WSANETWORKEVENTS networkwvents;
		if (dwIndex == 0)
		{
			ResetEvent(pThis->m_ASocketEvent[0]);
			pThis->m_lockEvent.UnLock();
			continue;
		} else if (dwIndex == WSA_WAIT_FAILED)
		{
			pThis->m_lockEvent.UnLock();
			continue;
		} else if (dwIndex >= pThis->m_nEventCount + 1)
		{
			pThis->m_lockEvent.UnLock();
			continue;
		}

		CASocket * pSocket = pThis->m_ASocketObject[dwIndex];	
		if (pSocket == NULL)
		{
			pThis->m_lockEvent.UnLock();
			continue;
		}

		SOCKET hSocket = pSocket->m_hSocket;
		pThis->m_lockEvent.UnLock();

	
		if (WSAEnumNetworkEvents(hSocket, pThis->m_ASocketEvent[dwIndex], &networkwvents) == SOCKET_ERROR)
		{
			continue;
		}

		while (networkwvents.lNetworkEvents != 0)
		{
			if ((networkwvents.lNetworkEvents & FD_READ) == FD_READ)
			{
				fd_set fds;
				int nReady;
				timeval timeout;
				timeout.tv_sec = 0;
				timeout.tv_usec = 0;

				FD_ZERO(&fds);
				FD_SET(hSocket, &fds);
				nReady = select(0, &fds, NULL, NULL, &timeout);
				if ((nReady == 1) || (networkwvents.iErrorCode != 0))
				{
					if(pSocket == pThis->m_ASocketObject[dwIndex])
					{
						pSocket->OnReceive(networkwvents.iErrorCode[FD_READ_BIT]);
					} else
					{
						break;
					}
				}
				networkwvents.lNetworkEvents ^= FD_READ;
			}
			if ((networkwvents.lNetworkEvents&FD_WRITE) == FD_WRITE)
			{
				if(pSocket == pThis->m_ASocketObject[dwIndex])
				{
					pSocket->OnSend(networkwvents.iErrorCode[FD_WRITE_BIT]);
				} else
				{
					break;
				}
				networkwvents.lNetworkEvents ^= FD_WRITE;
			}
			if ((networkwvents.lNetworkEvents&FD_OOB) == FD_OOB)
			{
				if(pSocket == pThis->m_ASocketObject[dwIndex])
				{
					pSocket->OnOutOfBandData(networkwvents.iErrorCode[FD_OOB_BIT]);
				} else
				{
					break;
				}
				networkwvents.lNetworkEvents ^= FD_OOB;
			}
			if ((networkwvents.lNetworkEvents&FD_ACCEPT) == FD_ACCEPT)
			{
				if(pSocket == pThis->m_ASocketObject[dwIndex])
				{
					pSocket->OnAccept(networkwvents.iErrorCode[FD_ACCEPT_BIT]);
				} else
				{
					break;
				}
				networkwvents.lNetworkEvents ^= FD_ACCEPT;
			}
			if ((networkwvents.lNetworkEvents&FD_CONNECT) == FD_CONNECT)
			{
				if(pSocket == pThis->m_ASocketObject[dwIndex])
				{
					pSocket->OnConnect(networkwvents.iErrorCode[FD_CONNECT_BIT]);
				} else
				{
					break;
				}
				networkwvents.lNetworkEvents ^= FD_CONNECT;
			}
			if ((networkwvents.lNetworkEvents&FD_CLOSE) == FD_CLOSE)
			{
				if (pSocket == pThis->m_ASocketObject[dwIndex])
				{
					pSocket->OnClose(networkwvents.iErrorCode[FD_CLOSE_BIT]);
				} else
				{
					break;
				}
				networkwvents.lNetworkEvents ^= FD_CLOSE;
			} else
			{
				break;
			}
		}
	}
	delete pThis;
	return 0L;
}

class CASocketKernelManagement
{
public:
	CASocketKernelManagement();
	virtual ~CASocketKernelManagement();
	BOOL AddASocket(CASocket * pASocket, long lEvent);
	void DeleteASocket(CASocket * pASocket);
private:
	int m_nMaxKernelCount;
	CGuardLock m_lockKernel;
	CASocketKernel * m_kernel;
};

CASocketKernelManagement::CASocketKernelManagement()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_nMaxKernelCount = 2; //(si.dwNumberOfProcessors * 2 + 2) * 4; 用于客户端，只建一个监听线程
	m_kernel = new CASocketKernel[m_nMaxKernelCount];
}

CASocketKernelManagement::~CASocketKernelManagement()
{
	delete [] m_kernel;
}

BOOL CASocketKernelManagement::AddASocket(CASocket * pASocket, long lEvent)
{
	CASocketKernel * pKernel = NULL;
	BOOL bResult;
	m_lockKernel.Lock();
	DWORD dwMin = MAXEVENTCOUNT;
	for (int i = 0; i < m_nMaxKernelCount; i++)
	{
		if (m_kernel[i].GetASocketCount() < dwMin)
		{
			pKernel = &m_kernel[i];
			dwMin = m_kernel[i].GetASocketCount();
		}
	}
	if (pKernel != NULL)
	{
		bResult = pKernel->AddASocket(pASocket, lEvent);
	} else
	{
		bResult = FALSE;
		SetLastError(NO_ENOUGH_SOCKET_SLOT);
	}
	m_lockKernel.UnLock();
	return bResult;
}

void CASocketKernelManagement::DeleteASocket(CASocket * pASocket)
{
	m_lockKernel.Lock();
	for (int i = 0; i < m_nMaxKernelCount; i++)
	{
		if (m_kernel[i].DeleteASocket(pASocket))
			break;
	}
	m_lockKernel.UnLock();
}

CASocketKernelManagement _ASocketManagement;


CASocket::CASocket(void)
{
	m_hSocket = INVALID_SOCKET;
}

CASocket::~CASocket(void)
{

}

//是否采用阻塞方式
void CASocket::SetBlock(BOOL bBlock)
{
	if (m_hSocket != INVALID_SOCKET)
	{
		if (bBlock)
		{
			AsyncSelect(0);
			u_long arg = 0;
			::ioctlsocket(m_hSocket, FIONBIO, &arg);
		} else
			AsyncSelect();
	}
}

//设置Socket发送区缓存
BOOL CASocket::SetSendBuffSize(DWORD dwBufferSize)
{
	if (m_hSocket != INVALID_SOCKET)
	{
		return (::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, 
			      (char *)&dwBufferSize, sizeof(DWORD)) == 0);
	}
	return FALSE;
}

//设置Socket接收区缓存
BOOL CASocket::SetRecvBuffSize(DWORD dwBufferSize)
{
	if (m_hSocket != INVALID_SOCKET)
	{
		return (::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, 
			      (char *)&dwBufferSize, sizeof(DWORD)) == 0);
	}
	return FALSE;
}
BOOL CASocket::Create(UINT nSocketPort, int nSocketType,long lEvent, char * lpszSocketAddress)
{
	if(m_hSocket != INVALID_SOCKET)
		return FALSE;
	if (Socket(nSocketType, lEvent))
	{
		if (Bind(nSocketPort, lpszSocketAddress))
		{
			m_wBindPort = nSocketPort;
			return TRUE;
		}
		int nResult = WSAGetLastError();
		Close();
		WSASetLastError(nResult);
	}
	return FALSE;
}
	
BOOL CASocket::GetPeerName(char * szPeerAddress, UINT& rPeerPort)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	
	BOOL bResult = (getpeername(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen) != SOCKET_ERROR);
	if(bResult)
	{
		rPeerPort = ntohs(sockAddr.sin_port);
		strcpy(szPeerAddress, inet_ntoa(sockAddr.sin_addr));
	}
	return bResult;
}

BOOL CASocket::GetSockName(char * szSocketAddress, UINT& rSocketPort)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;

	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
    
	sockAddr.sin_family = AF_INET;
	int nSockAddrLen = sizeof(sockAddr);
	
	BOOL bResult = (getsockname(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen)!=SOCKET_ERROR);
	if(bResult)
	{
		rSocketPort = ntohs(sockAddr.sin_port);
		strcpy(szSocketAddress, inet_ntoa(sockAddr.sin_addr));
	}
	return bResult;
}

BOOL CASocket::Accept(CASocket& rConnectedSocket,	SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	if(rConnectedSocket.m_hSocket != INVALID_SOCKET)
		return FALSE;

	SOCKET hTemp = accept(m_hSocket, lpSockAddr, lpSockAddrLen);	
	if (hTemp == INVALID_SOCKET)
	{
		int iResult = WSAGetLastError();
		rConnectedSocket.m_hSocket = INVALID_SOCKET;
		WSASetLastError(iResult);
	} else
	{
		rConnectedSocket.m_hSocket = hTemp;
		if (!rConnectedSocket.AsyncSelect())
		{
			int iResult = WSAGetLastError();
			closesocket(rConnectedSocket.m_hSocket);
			rConnectedSocket.m_hSocket = INVALID_SOCKET;
			WSASetLastError(iResult);
			return FALSE;
		}
	}
	return (hTemp != INVALID_SOCKET);
}

BOOL CASocket::Connect(const char * lpszHostAddress, const UINT nHostPort)
{
	if (m_hSocket == INVALID_SOCKET)
		return FALSE;

	if (lpszHostAddress == NULL)
	{
		return FALSE;
	}

	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);

	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(lpszHostAddress);
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
		{
			WSASetLastError(WSAEINVAL);
			return FALSE;
		}
	}

	sockAddr.sin_port = htons((u_short)nHostPort);

	int nError = connect(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
	if ((nError == SOCKET_ERROR) && (::WSAGetLastError() != WSAEWOULDBLOCK))
		return FALSE;
	return TRUE;
}

BOOL CASocket::Bind(UINT nSocketPort, char * lpszSocketAddress)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	char * lpszAscii;
	if (lpszSocketAddress != NULL)
	{
		lpszAscii = lpszSocketAddress;
		if (lpszAscii == NULL)
		{
			// OUT OF MEMORY
			WSASetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return FALSE;
		}
	} else
	{
		lpszAscii = NULL;
	}

	sockAddr.sin_family = AF_INET;

	if (lpszAscii == NULL)
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		DWORD lResult = inet_addr(lpszAscii);
		if (lResult == INADDR_NONE)
		{
			WSASetLastError(WSAEINVAL);
			return FALSE;
		}
		sockAddr.sin_addr.s_addr = lResult;
	}

	sockAddr.sin_port = htons((u_short)nSocketPort);

	if (SOCKET_ERROR != bind(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)))
	{
		m_wBindPort = nSocketPort;
		return TRUE;
	}
	return FALSE;
}

//TCP服务器相关参数
#define DEFAULT_KEEPALIVETIME		15000
#define DEFAULT_KEEPALIVEINTERVAL   3000

BOOL CASocket::SetKeepLive()
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	tcp_keepalive inTcpLive = {0};
	tcp_keepalive OutTcpLive = {0};
	inTcpLive.onoff = 1;
	inTcpLive.keepaliveinterval = DEFAULT_KEEPALIVEINTERVAL;
	inTcpLive.keepalivetime = DEFAULT_KEEPALIVETIME;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == ::WSAIoctl(m_hSocket, SIO_KEEPALIVE_VALS, &inTcpLive, sizeof(tcp_keepalive), 
		                           &OutTcpLive, sizeof(tcp_keepalive), &dwBytes, NULL, NULL))
	{
		PRINTDEBUGLOG(dtInfo, "Set Socket Keeplive Failed");
		return FALSE;
	}
	return TRUE;
}

BOOL CASocket::Listen(int nConnectionBacklog)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	return (listen(m_hSocket, nConnectionBacklog) !=SOCKET_ERROR); 
}

int CASocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
	if(m_hSocket == INVALID_SOCKET)
		return -1;
	return recv(m_hSocket, (char *)lpBuf, nBufLen, nFlags);
}

int CASocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	if(m_hSocket == INVALID_SOCKET)
		return -1;
	int iResult = send(m_hSocket, (char *)lpBuf, nBufLen, nFlags);
	return iResult;
}

void CASocket::Close()
{
	if(m_hSocket == INVALID_SOCKET)
		return;
	_ASocketManagement.DeleteASocket(this);
	if(m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
	} 
}

BOOL CASocket::AsyncSelect(long lEvent)
{
	return _ASocketManagement.AddASocket(this, lEvent);
}

BOOL CASocket::Socket(int nSocketType, long lEvent, int nProtocolType, int nAddressFormat)
{
	m_hSocket = socket(nAddressFormat, nSocketType, nProtocolType);
	if (m_hSocket != INVALID_SOCKET)
	{
		if(AsyncSelect(lEvent))
		{
			int iBufSize = 2048000;
			if (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&iBufSize, sizeof(iBufSize)) != 0)
			{
				PRINTDEBUGLOG(dtInfo, "设置socket接收缓存失败");
			}

			iBufSize = 2048000;
			if (setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&iBufSize, sizeof(iBufSize)) != 0)
			{
				PRINTDEBUGLOG(dtInfo, "设置socket发送缓存失败");
			}
			return TRUE;
		}
		int nResult = WSAGetLastError();
		closesocket(m_hSocket);
		WSASetLastError(nResult);
	}
	return FALSE;
}

BOOL CASocket::ASocketInit(void)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData ) !=0 )//进行WinSocket的初始化
	{
		return FALSE;
	}
	return TRUE;
}

//清除socket初始化
BOOL CASocket::ASocketDestroy()
{
	if (::WSACleanup() != 0)
	{
		PRINTDEBUGLOG(dtInfo, "WSACleanup failed:%d", ::WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

WORD CASocket::GetBindPort()
{
	return m_wBindPort;
}

static char * ByteToString(char *szByte, BYTE bit)
{
	BYTE v, b = 100;
	bool bHas = false;
	while(b)
	{
		v = bit / b;
		if (v || bHas)
		{
			*szByte++ = v + '0';
			bHas = true;
		}
		bit -= v * b;
		b /= 10;
	}
	if (!bHas)
		*szByte++ = '0';
	return szByte;
}

int CASocket::StringIPToInt(const char *szAddress)
{
	return inet_addr(szAddress);
}

void CASocket::IntToIpAddress(char *szAddress, DWORD dwIp)
{
	BYTE *p = (BYTE *)&dwIp;
	char *pByte = szAddress;
	for (int i = 0; i < 4; i ++)
	{
		pByte = ByteToString(pByte, *p ++);
		*pByte ++ = '.';
	}
	*(--pByte) = '\0';
}

DWORD CASocket::GetIPByHostName(const char *szHostName)
{
	hostent *pHost = ::gethostbyname(szHostName);
	if ((pHost) && (pHost->h_length > 0))
	{
		return (*((DWORD *)(*(pHost->h_addr_list))));
	}
	return 0;
}


WORD CASocket::GetFreeUdpPort(WORD wStartPort, WORD wTryTimes)
{
	struct sockaddr_in addr;
	int addrlen;
	SOCKET h;
	WORD wFreePort = 0;
	for (int i = 0; i < wTryTimes; i ++)
	{
		h = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
		if (h != INVALID_SOCKET)
		{
			addrlen = sizeof(sockaddr_in);
			memset(&addr, 0, addrlen);
			addr.sin_addr.S_un.S_addr = ADDR_ANY;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(wStartPort + i);
			if (bind(h, (struct sockaddr *)&addr, addrlen) ==  0)
			{
				closesocket(h);
				wFreePort = wStartPort + i;
				break;
			}
			closesocket(h);
		}
	}
	return wFreePort;
}

WORD CASocket::GetFreeTcpPort(WORD wStartPort, WORD wTryTimes)
{
	struct sockaddr_in addr;
	int addrlen;
	SOCKET h;
	WORD wFreePort = 0;
	for (int i = 0; i < wTryTimes; i ++)
	{
		h = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (h != INVALID_SOCKET)
		{
			addrlen = sizeof(sockaddr_in);
			memset(&addr, 0, addrlen);
			addr.sin_addr.S_un.S_addr = ADDR_ANY;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(wStartPort + i);
			if (bind(h, (struct sockaddr *)&addr, addrlen) ==  0)
			{
				closesocket(h);
				wFreePort = wStartPort + i;
				break;
			}
			closesocket(h);
		}
	}
	return wFreePort;
}

#pragma warning(default:4996)