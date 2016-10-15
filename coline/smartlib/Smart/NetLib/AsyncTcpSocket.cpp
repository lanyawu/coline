#include <Netlib/AsyncTcpSocket.h>
#include <Commonlib/Debuglog.h>

#define RECV_BUFFER_TIME_OUT_INTERVAL 15000  //数据接收超时

CAsyncTcpSocket::CAsyncTcpSocket(void):
                 m_bTerminated(FALSE),
				 m_bConnected(FALSE),
				 m_hConnectEvent(NULL)
{
	m_hSendEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hEmptyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSndThread = CreateThread(NULL, 0, SendDataThread, this, 0, NULL);
}

CAsyncTcpSocket::~CAsyncTcpSocket(void)
{
	if (m_listADI.size() > 0)
	{
		//PRINTDEBUGLOG(dtError, "send leave tcp buff size:%d", m_listADI.size());
		ResetEvent(m_hEmptyEvent);
		SetEvent(m_hSendEvent);
		WaitForSingleObject(m_hEmptyEvent, 5000);
	}
	m_bTerminated = TRUE;
	SetEvent(m_hSendEvent);
	SetEvent(m_hEmptyEvent);
	WaitForSingleObject(m_hSndThread, 5000);
	ClearSendBuffer();
	CloseHandle(m_hEmptyEvent);
	CloseHandle(m_hSendEvent);
	CloseHandle(m_hSndThread);
	Close();
}

void CAsyncTcpSocket::OnSend(int nErrorCode)
{
	SetEvent(m_hSendEvent);
}

void CAsyncTcpSocket::OnConnect(int nErrorCode)
{
	if (nErrorCode == 0) //连接成功
	{
		m_bConnected = TRUE;
		SetEvent(m_hSendEvent); //发送缓存中的信息
	}
	if (m_hConnectEvent)
		SetEvent(m_hConnectEvent);
}

BOOL CAsyncTcpSocket::ConnectPeer(DWORD dwIp, WORD wPort, DWORD dwTimeOut)
{
	if (m_hConnectEvent) //正在处理
		return FALSE;
	if (m_bConnected)
		return TRUE; //已经连接成功
	m_hConnectEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	char szAddr[16] = {0};
	CASocket::IntToIpAddress(szAddr, dwIp);
	Connect(szAddr, wPort);
	WaitForSingleObject(m_hConnectEvent, dwTimeOut);
	CloseHandle(m_hConnectEvent);
	m_hConnectEvent = NULL;
	return m_bConnected;
}

BOOL CAsyncTcpSocket::GetConnected()
{
	return m_bConnected;
}

BOOL CAsyncTcpSocket::IsSendComplete()
{
	return m_listADI.empty();
}

BOOL CAsyncTcpSocket::IsCanWriteBuff(int nTimeOut)
{
	if (m_listADI.size() < MAX_ASYNCTCP_SND_BUFFER_SIZE)
		return TRUE;
	WaitForSingleObject(m_hEmptyEvent, nTimeOut);
	return (m_listADI.size() < MAX_ASYNCTCP_SND_BUFFER_SIZE);
}

BOOL CAsyncTcpSocket::IsCanReadBuff(int nTimeOut)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	fd_set rd;
	FD_ZERO(&rd);
	FD_SET(m_hSocket, &rd);
	timeval val;
	val.tv_sec = nTimeOut / 1000;
	val.tv_usec = (nTimeOut % 1000) * 1000;
	return (::select((int)(m_hSocket + 1), &rd, NULL, NULL, &val) > 0);
}

//发送数据
int CAsyncTcpSocket::SendBuff(char *lpBuff, int &BufLen)
{
	int iResult = BufLen;
	
	if (m_listADI.size() > MAX_ASYNCTCP_SND_BUFFER_SIZE)
	{
		return -1;
	}
    LPADI lpadi = new ADI();
	lpadi->lpBuf = new char[BufLen];
	if (lpadi)
	{
		memmove(lpadi->lpBuf, lpBuff, BufLen);
		lpadi->nBufLen = BufLen;

		CGuardLock::COwnerLock guard(m_lockADI);
		m_listADI.push_back(lpadi);
		SetEvent(m_hSendEvent);
	} else
		PRINTDEBUGLOG(dtError, "Alloc LPADI Error");
	
	return iResult;
}
//
void CAsyncTcpSocket::SetConnectStatus(BOOL bConnectStatus)
{
	m_bConnected = bConnectStatus;
}

//发送数据
int CAsyncTcpSocket::SendBuff(const char *lpBuff, int BufLen, int nTimeOut)
{
	int nPos = 0;
	int nSize;
	while (nPos < BufLen)
	{
		fd_set wrfds;
		FD_ZERO(&wrfds);
		FD_SET(m_hSocket, &wrfds);
		struct timeval tv;
		tv.tv_sec = nTimeOut / 1000;
		tv.tv_usec = (nTimeOut % 1000) * 1000;
		int nSel = ::select((int)(m_hSocket + 1), 0, &wrfds, 0, &tv);
		if (nSel > 0)
		{
			if (FD_ISSET(m_hSocket, &wrfds))
			{
				nSize = ::send(m_hSocket, (lpBuff + nPos), BufLen - nPos, 0);
				if (nSize > 0)
				{
					nPos += nSize;
				} else 
				{
					int n = ::GetLastError();
					break;
				}
			} else
				break;
		} else
			break;
	}
	return nPos;
}

void CAsyncTcpSocket::ClearSendBuffer()
{
	CGuardLock::COwnerLock guard(m_lockADI);
	//PRINTDEBUGLOG(dtError, "TCP Buffer Size:%d", m_listADI.size());
	LPADI lpadi;
	deque<LPADI>::iterator it;
	for (it = m_listADI.begin(); it != m_listADI.end(); it ++)
	{
		lpadi = (*it);
		delete []lpadi->lpBuf;
		delete lpadi;
	}
	m_listADI.clear();
}


int CAsyncTcpSocket::RecvBuffer(char *lpBuff, int BufLen, DWORD dwTimeOut)
{
	DWORD dwStart = GetTickCount();
	int nPos = 0;
	int RecvSize;
	do
	{
		RecvSize = Receive(lpBuff + nPos, BufLen - nPos);
		if (RecvSize > 0)
		{
			nPos += RecvSize;
			dwStart = ::GetTickCount();
		} else if (RecvSize == -1)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				dwStart = ::GetTickCount();
			} else
				return nPos;
		} else
			return nPos;
		if (nPos >= BufLen)
			return nPos;
		IsCanReadBuff(500);
	}while(::GetTickCount() < (dwStart + dwTimeOut));
	return -1;
}

 //数据发送线程
DWORD WINAPI CAsyncTcpSocket::SendDataThread(LPVOID lpParam)
{
	LPADI lpadi;
	int iResult;
	CAsyncTcpSocket *pThis = (CAsyncTcpSocket *)lpParam;
	while(!pThis->m_bTerminated)
	{
		WaitForSingleObject(pThis->m_hSendEvent, INFINITE);
		while((!pThis->m_bTerminated) && pThis->m_bConnected)
		{
			if (pThis->m_listADI.empty())
			{
				break;
			}
			pThis->m_lockADI.Lock();
			lpadi = pThis->m_listADI.front();
			pThis->m_listADI.pop_front();
			pThis->m_lockADI.UnLock();

			iResult = pThis->Send(lpadi->lpBuf, lpadi->nBufLen, 0);
			if (iResult == SOCKET_ERROR)
			{
				//没有发送成功
				int nError = GetLastError();
				PRINTDEBUGLOG(dtError, "TCP 发送数据失败，错误代码:%d", nError);
	            if ( nError == WSAEWOULDBLOCK)
				{
					pThis->m_lockADI.Lock();
					pThis->m_listADI.push_front(lpadi);
					pThis->m_lockADI.UnLock();

					WaitForSingleObject(pThis->m_hSendEvent, 5000);
					//PRINTDEBUGLOG(dtWarning, "TCP 数据发送失败\n");
				}
			} else if (iResult < lpadi->nBufLen) //只发送了一部分数据
			{
				memmove(lpadi->lpBuf, (char *)(lpadi->lpBuf) + iResult, lpadi->nBufLen - iResult);
				lpadi->nBufLen -= iResult;

				pThis->m_lockADI.Lock();
				pThis->m_listADI.push_front(lpadi);
				pThis->m_lockADI.UnLock();
			} else
			{
				delete []lpadi->lpBuf;
				delete lpadi;
			}
		}
		SetEvent(pThis->m_hEmptyEvent);
	}
	return 0;
}


// ==== class CAsyncTcpSocketServer ====
CAsyncTcpSocketServer::CAsyncTcpSocketServer(IAsyncTcpServerApp *pApp):
                       m_pApp(pApp)
{

}

CAsyncTcpSocketServer::~CAsyncTcpSocketServer()
{
	Close();
	m_pApp = NULL;
}

BOOL CAsyncTcpSocketServer::Start(WORD wPort)
{
	if (Create(wPort))
	{
		return Listen();
	}
	return FALSE;
}


void CAsyncTcpSocketServer::OnAccept(int nErrorCode)
{
	if (0 == nErrorCode)
	{
		CAsyncTcpSocket *pSocket = new CAsyncTcpSocket();
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(sockaddr_in));
		addr.sin_family = PF_INET;
        int addrlen = sizeof(struct sockaddr_in);
		if (Accept(*pSocket, (SOCKADDR *)&addr, &addrlen))
		{
			pSocket->SetConnectStatus(TRUE);
			if (m_pApp)
			{
				CNetAddress PeerAddr;
				PeerAddr = addr;
				m_pApp->OnAccept(pSocket, PeerAddr);
			}else
				delete pSocket;
		} else
			delete pSocket;
	}
}
