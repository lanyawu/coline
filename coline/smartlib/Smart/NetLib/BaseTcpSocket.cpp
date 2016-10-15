#include <commonlib/debuglog.h>
#include <NetLib/BaseTcpSocket.h>
#include <netlib/asock.h>

#pragma warning(disable:4996)

CBaseTcpSocket::CBaseTcpSocket(void):
                m_bAgent(FALSE)
{
	m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_hSocket == INVALID_SOCKET)
	{
		PRINTDEBUGLOG(dtInfo, "create basetcpsocket failed:%d", WSAGetLastError());
	}
}

CBaseTcpSocket::~CBaseTcpSocket(void)
{
	if (m_hSocket)
		::closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
}

SOCKET CBaseTcpSocket::Detach()
{
	SOCKET h = m_hSocket;
	m_hSocket = INVALID_SOCKET;
	return h;
}

int CBaseTcpSocket::RecvBuffer(char *lpBuff, int nBufSize, int nTimeOut)
{
	if (m_hSocket == INVALID_SOCKET)
		return 0;
	int nPos = 0;
	int nSize = 0;
	fd_set rd;
	while (nPos < nBufSize)
	{
		if (m_bAgent)
		{
			if (m_AgentSocket.CanRead(nTimeOut))
			{
				nSize = m_AgentSocket.RecvTcp(lpBuff + nPos, nBufSize - nPos);
			} 
		}else
		{							
			FD_ZERO(&rd);
			FD_SET(m_hSocket, &rd);
			struct timeval tv;
			tv.tv_sec = nTimeOut / 1000;
			tv.tv_usec = (nTimeOut % 1000) * 1000;
			int nSel = ::select((int)(m_hSocket + 1), &rd, NULL, NULL, &tv);
			if (nSel > 0)
			{
				if (FD_ISSET(m_hSocket, &rd))
				{
					nSize = ::recv(m_hSocket, (lpBuff + nPos), nBufSize - nPos, 0);
				}
			} else
				PRINTDEBUGLOG(dtInfo, "other socket is can read");
		} 				
		if (nSize > 0)
		{
			nPos += nSize;
		} else 
		{
			PRINTDEBUGLOG(dtInfo, "recv data failed, Total:%d Pos:%d error:%d", nBufSize, nPos, ::WSAGetLastError());
			break;
		}
	}
	return nPos;
}

//设置代理服务器
BOOL CBaseTcpSocket::SetAgent(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
	          const char *szAgentPwd)
{
	m_bAgent = FALSE;
	if (szAgentIp)
		m_bAgent = m_AgentSocket.ConnectAgent(szAgentIp, wPort, szAgentName, szAgentPwd, INVALID_SOCKET);
	return m_bAgent;
}

BOOL CBaseTcpSocket::IsCanReadBuff(int nTimeOut)
{
	if (m_hSocket == INVALID_SOCKET)
		return FALSE;
	if (m_bAgent)
	{
		return m_AgentSocket.CanRead(nTimeOut);
	} else
	{
		fd_set rd;
		FD_ZERO(&rd);
		FD_SET(m_hSocket, &rd);
		struct timeval tv;
		tv.tv_sec = nTimeOut / 1000;
		tv.tv_usec = (nTimeOut % 1000) * 1000;
		int nSel = ::select((int)(m_hSocket + 1), &rd, NULL, NULL, &tv);
		if (nSel > 0)
		{
			if (FD_ISSET(m_hSocket, &rd))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

int CBaseTcpSocket::SendBuffer(const char *lpBuff, int nBufSize, int nTimeOut)
{
	if (m_hSocket == INVALID_SOCKET)
		return 0;
	int nPos = 0;
	int nSize = 0;
	fd_set wrfds;
	while (nPos < nBufSize)
	{
		if (m_bAgent)
		{
			nSize = m_AgentSocket.SendTcp(lpBuff + nPos, nBufSize - nPos); 
		} else
		{							
			FD_ZERO(&wrfds);
			FD_SET(m_hSocket, &wrfds);
			struct timeval tv;
			tv.tv_sec = nTimeOut / 1000;
			tv.tv_usec = (nTimeOut % 1000) * 1000;
			int nSel = ::select((int)(m_hSocket + 1), NULL, &wrfds, NULL, &tv);
			if (nSel > 0)
			{
				if (FD_ISSET(m_hSocket, &wrfds))
				{
					nSize = ::send(m_hSocket, (lpBuff + nPos), nBufSize - nPos, 0);
				}
			} else 
				PRINTDEBUGLOG(dtInfo, "other socket can write");
		} 	
		//
		if (nSize > 0)
		{
			nPos += nSize;
		} else 
		{
			PRINTDEBUGLOG(dtInfo, "send data failed, Total:%d Pos:%d error:%d", nBufSize, nPos, ::WSAGetLastError());
			break;
		}
	}
	return nPos;
}

BOOL CBaseTcpSocket::Connect(const char *szIp, int nPort, int nTimeOut)
{
	int nIp = inet_addr(szIp);
	return Connect(nIp, nPort, nTimeOut);
}

BOOL CBaseTcpSocket::GetSocketInfo(char *szIp, WORD &wPort)
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
		wPort = ntohs(sockAddr.sin_port);
		strcpy(szIp, inet_ntoa(sockAddr.sin_addr));
	}
	return bResult;
}

BOOL CBaseTcpSocket::GetPeerSocketInfo(char *szIp, WORD &wPort)
{
	if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	
	BOOL bResult = (getpeername(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen) != SOCKET_ERROR);
	if(bResult)
	{
		wPort = ntohs(sockAddr.sin_port);
		strcpy(szIp, inet_ntoa(sockAddr.sin_addr));
	}
	return bResult;
}

BOOL CBaseTcpSocket::Connect(int nIp, int nPort, int nTimeOut)
{
    if(m_hSocket == INVALID_SOCKET)
		return FALSE;
	
	m_dwPeerIp = nIp;
	m_wPeerPort = htons(nPort);
	if (m_bAgent)
	{
		char ip[16] = {0};
		CASocket::IntToIpAddress(ip, nIp);
		return m_AgentSocket.ConnectServer(ip, nPort);
	} else
	{
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
	 
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = nIp;
		sockAddr.sin_port = htons((u_short)nPort);

		int nError = ::connect(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
		if (nError != SOCKET_ERROR)
		{
			fd_set wrfds;
			FD_ZERO(&wrfds);
			FD_SET(m_hSocket, &wrfds);
			struct timeval tv;
			tv.tv_sec = nTimeOut / 1000;
			tv.tv_usec = (nTimeOut % 1000) * 1000;
			if (::select((int)(m_hSocket + 1), 0, &wrfds, 0, &tv) > 0)
			{
				if (FD_ISSET(m_hSocket, &wrfds))
				{
					u_long arg = 1;
					::ioctlsocket(m_hSocket, FIONBIO, &arg);
					return TRUE;
				}
			}
			PRINTDEBUGLOG(dtInfo, "socket connect failed in select timeout");
		} else
		{
			PRINTDEBUGLOG(dtInfo, "socket connect failed, error:%d", ::WSAGetLastError());
		}
	}
	return FALSE;
}

BOOL CBaseTcpSocket::SetSendBuffSize(int nBufSize)
{
	if (m_hSocket != INVALID_SOCKET)
	{
		return (::setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, 
			      (char *)&nBufSize, sizeof(int)) == 0);
	}
	return FALSE;
}

BOOL CBaseTcpSocket::SetRecvBuffSize(int nBufSize)
{
	if (m_hSocket != INVALID_SOCKET)
	{
		return (::setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, 
			      (char *)&nBufSize, sizeof(int)) == 0);
	}
	return FALSE;
}

#pragma warning(default:4996)
