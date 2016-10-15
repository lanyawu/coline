#include <CommonLib/debuglog.h>
#include <netlib/asock.h>
#include <mstcpip.h>
#include <netlib/Socks5AgentSocket.h>

#define MAX_AGENT_UDP_BUFFER_SIZE   1024  //最大的UDP消息缓存

#define AGENT_SOCKS5_VERSION   0x05   //socks5版本号

CSocks5AgentSocket::CSocks5AgentSocket(void):
                    m_nSocket(INVALID_SOCKET),
					m_hMapSocket(INVALID_SOCKET)
{
	memset(&m_addrProxy, 0, sizeof(sockaddr_in));
	m_addrProxy.sin_family = AF_INET;
}

CSocks5AgentSocket::~CSocks5AgentSocket(void)
{
	if (m_nSocket != INVALID_SOCKET)
		::closesocket(m_nSocket);
	m_nSocket = NULL;
}

BOOL CSocks5AgentSocket::ConnectAgent(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
									  const char *szAgentPwd, SOCKET hMapSocket)
{
	if (m_nSocket != INVALID_SOCKET)
		::closesocket(m_nSocket);
	m_nSocket = (int)::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_nSocket == INVALID_SOCKET)
		return FALSE;
	if (szAgentIp)
		m_strAgentAddr = szAgentIp;
	if (szAgentName)
		m_strAgentName = szAgentName;
	if (szAgentPwd)
		m_strAgentPwd = szAgentPwd;
	m_nAgentPort = wPort;
	m_hMapSocket = hMapSocket;
	do
	{
		timeval tv;
		tv.tv_sec = 100; //超时
		tv.tv_usec = 0;
		//设置超时处理
		if (::setsockopt(m_nSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(timeval)) == SOCKET_ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "set socket timeout value failed");
			break;
		}
		//设置keeplive
		BOOL bAliveOn = 1;
		if (::setsockopt(m_nSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&bAliveOn, sizeof(BOOL)) != SOCKET_ERROR)
		{
			tcp_keepalive alive = {0};
			tcp_keepalive OutKeepLive = {0};
			DWORD dwReturn = 0;
			alive.onoff = 1;
			alive.keepalivetime = 10000; //ms
			alive.keepaliveinterval = 50; //ms
			if (::WSAIoctl(m_nSocket, SIO_KEEPALIVE_VALS, (LPVOID)&alive, sizeof(tcp_keepalive), 
				(LPVOID)&OutKeepLive, sizeof(tcp_keepalive), &dwReturn, NULL, NULL) == SOCKET_ERROR)
			{
				PRINTDEBUGLOG(dtInfo, "set socket alive failed");
			} 
		}
		
		struct sockaddr_in AgentAddr = {0};
		AgentAddr.sin_family = AF_INET;
		AgentAddr.sin_addr.s_addr = ::inet_addr(szAgentIp);
		AgentAddr.sin_port = htons(wPort);
		if (AgentAddr.sin_addr.s_addr == INADDR_NONE)
		{
			struct hostent *p = ::gethostbyname(szAgentIp);
			if (p == NULL)
			{
				PRINTDEBUGLOG(dtInfo, "get host name failed, host:%s", szAgentIp);
			    break;
			}
			AgentAddr.sin_addr.s_addr = ((LPIN_ADDR)p->h_addr)->s_addr;
		}

		//开始连接
		if (::connect(m_nSocket, (struct sockaddr *)&AgentAddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "connect agent server failed");
			break;
		}
		if (!HandShake())
			break;
		if (hMapSocket != INVALID_SOCKET)
		{
			if (!MapChannel(hMapSocket))
			    break;
		}
		return TRUE;
	} while (FALSE);
	if (m_nSocket != INVALID_SOCKET)
	{
		::closesocket(m_nSocket);
		m_nSocket = NULL;
	}
	return FALSE;
}

BOOL CSocks5AgentSocket::HandShake()
{
	char szBuff[256] = {0};
	if (m_strAgentName.empty()) //不需要密码验证
	{
		szBuff[0] = AGENT_SOCKS5_VERSION;
		szBuff[1] = 0x01;
		szBuff[2] = 0x00;
		if (::send(m_nSocket, szBuff, 3, 0) == SOCKET_ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "send empty user failed");
			return FALSE;
		}
		memset(szBuff, 0, 256);
		if (::recv(m_nSocket, szBuff, 256, 0) == SOCKET_ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "Recv user valid failed");
			return FALSE;
		}
		//用户名验证不通过
		if ((szBuff[0] != 0x05) || (szBuff[1] != 0x00))
		{
			PRINTDEBUGLOG(dtInfo, "user or pwd error, Buff[0]:%d Buff[1]:%d", szBuff[0], szBuff[1]);
		    return FALSE;
		} 
	} else
	{
		szBuff[0] = AGENT_SOCKS5_VERSION;
		szBuff[1] = 0x02;
		szBuff[2] = 0x00;
		szBuff[3] = 0x02;
		//发送通讯包
		if (::send(m_nSocket, szBuff, 4, 0) == SOCKET_ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "send agent packet failed");
			return FALSE;
		}

		//接收socks 代理返回
		if (::recv(m_nSocket, szBuff, 256, 0) == SOCKET_ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "recv agent packet failed");
			return FALSE;
		}
		if ((szBuff[0] == AGENT_SOCKS5_VERSION) && (szBuff[1] == 0x02))  //需要用户名和密码验证
		{
			BYTE nAgentNameSize = (BYTE)::strlen(m_strAgentName.c_str());
			BYTE nAgentPwdSize = (BYTE)::strlen(m_strAgentPwd.c_str());
			memset(szBuff, 0,  256);
			szBuff[0] = 0x01;
			szBuff[1] = nAgentNameSize;
			memmove(szBuff + 2, m_strAgentName.c_str(), nAgentNameSize);
			szBuff[nAgentNameSize + 2] = nAgentPwdSize;
			memmove(szBuff + nAgentNameSize + 3, m_strAgentPwd.c_str(), nAgentPwdSize);
			if (::send(m_nSocket, szBuff, nAgentNameSize + nAgentPwdSize + 3, 0) == SOCKET_ERROR)
			{
				PRINTDEBUGLOG(dtInfo, "send user and pwd valid failed");
			    return FALSE;
			}
			//接收用户验证回应
			memset(szBuff, 0, 256);
			if (::recv(m_nSocket, szBuff, 256, 0) == SOCKET_ERROR)
			{
				PRINTDEBUGLOG(dtInfo, "recv user and pwd valid failed");
			    return FALSE;
			}
			//用户名验证不通过
			if ((szBuff[0] != 0x01) || (szBuff[1] != 0x00))
			{
				PRINTDEBUGLOG(dtInfo, "user or pwd error, Buff[0]:%d Buff[1]:%d", szBuff[0], szBuff[1]);
			    return FALSE;
			}
		} else if ((szBuff[0] != AGENT_SOCKS5_VERSION) || (szBuff[1] != 0x00))
		{
			PRINTDEBUGLOG(dtInfo, "need agent user and pwd failed, Buff[0]:%d Buff[1]:%d", szBuff[0], szBuff[1]);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CSocks5AgentSocket::MapChannel(SOCKET hMapSocket)
{
	char szBuff[256] = {0};
	struct sockaddr_in addr = {0};
	int addrlen = sizeof(sockaddr_in);
	getsockname(hMapSocket, (struct sockaddr *)&addr, &addrlen);
	//连接和验证成功
	szBuff[0] = AGENT_SOCKS5_VERSION; //socks5
	szBuff[1] = 0x03; //udp
	szBuff[2] = 0x00;
	szBuff[3] = 0x01; //ipv4
	//目标地址及端口
	memmove(szBuff + 4, &(addr.sin_addr.s_addr), sizeof(DWORD));
	memmove(szBuff +4 + sizeof(DWORD), &(addr.sin_port), sizeof(WORD));
	if (::send(m_nSocket, szBuff, 10, 0) == SOCKET_ERROR)
	{
		PRINTDEBUGLOG(dtInfo, "send ip data failed");
		return FALSE;
	}
	//接收
	memset(szBuff, 0, 256);
	if (::recv(m_nSocket, szBuff, 256, 0) == SOCKET_ERROR)
	{
		PRINTDEBUGLOG(dtInfo, "recv Ip Data Failed");
		return FALSE;
	}
	//
	if ((szBuff[0] != AGENT_SOCKS5_VERSION) || (szBuff[1] != 0x00))
	{
		PRINTDEBUGLOG(dtInfo, "Get Ip Error");
		return FALSE;
	}
	if (szBuff[3] == 0x01)
	{
		memmove(&(m_addrProxy.sin_addr.s_addr), szBuff + 4, sizeof(int));
		memmove(&(m_addrProxy.sin_port), szBuff + 8, sizeof(WORD));
		m_nAddrType = 0x01;
	} else if (szBuff[3] == 0x03)
	{
		int nSize = szBuff[4];
		char *szTemp = new char[nSize + 1];
		memset(szTemp, 0, nSize + 1);
		memmove(szTemp, szBuff + 5, nSize);
		m_addrProxy.sin_addr.s_addr = ::inet_addr(szTemp);
		delete []szTemp;
		//端口
		WORD nPort = 0;
		memmove(&nPort, szBuff + 5 + nSize, sizeof(WORD));
		m_addrProxy.sin_port = nPort;
		m_nAddrType = 0x03;
	} else
	{
		PRINTDEBUGLOG(dtInfo, "Get Ip Type unknown:%d", szBuff[3]);
		return FALSE;
	}
	return TRUE;
}

//TCP连接至远程服务器
BOOL CSocks5AgentSocket::ConnectServer(const char *szServerIp, const WORD wPort)
{
	char szBuff[256] = {0};
	szBuff[0] = AGENT_SOCKS5_VERSION;
	szBuff[1] = 0x01; //连接
	szBuff[2] = 0x00;
	szBuff[3] = 0x01; //ipv4
	int ip = ::inet_addr(szServerIp);
    WORD w = htons(wPort);
	memmove(szBuff + 4, &ip, sizeof(int));
	memmove(szBuff + 8, &w, sizeof(WORD));
	if (::send(m_nSocket, szBuff, 10, 0) == SOCKET_ERROR)
	{
		PRINTDEBUGLOG(dtInfo, "send connect protocol failed");
		return FALSE;
	}

	memset(szBuff, 0, 256);
	if (::recv(m_nSocket, szBuff, 256, 0) == SOCKET_ERROR)
	{
		PRINTDEBUGLOG(dtInfo, "Recv connect protocol failed");
		return FALSE;
	}
    if ((szBuff[0] != 0x05) || (szBuff[1] != 0x00))
	{
		PRINTDEBUGLOG(dtInfo, "agent connect failed");
		return FALSE;
	}
	return TRUE;
}

//发送数据
int CSocks5AgentSocket::SendTcp(const char *szBuff, const int nBufSize)
{
	if (m_nSocket == INVALID_SOCKET)
		return SOCKET_ERROR;
	int nSize = nBufSize;
	return ::send(m_nSocket, szBuff, nSize, 0);
}

//接收数据
int CSocks5AgentSocket::RecvTcp(char *szBuff, int nBufSize)
{
	if (m_nSocket == INVALID_SOCKET)
		return SOCKET_ERROR;
	return ::recv(m_nSocket, szBuff, nBufSize, 0);
}

//发送UDP消息
int CSocks5AgentSocket::SendUdp(SOCKET hSocket, const char *szBuff, const int nBufSize, 
								 const DWORD nDestIp, const WORD wDestPort)
{
	if ((m_nSocket == INVALID_SOCKET) || (hSocket == INVALID_SOCKET))
		return SOCKET_ERROR;
	char *szTemp = new char[nBufSize + 10];
	memset(szTemp, 0, nBufSize + 10);
	szTemp[0] = 0x00;
	szTemp[1] = 0x00;
	szTemp[2] = 0x00;
	szTemp[3] = 0x01; //IPv4
	memmove(szTemp + 4, &nDestIp, sizeof(DWORD));
	memmove(szTemp + 8, &wDestPort, sizeof(WORD));
	memmove(szTemp + 10, szBuff, nBufSize);
	int nRet = ::sendto(hSocket, szTemp, nBufSize + 10, 0, (struct sockaddr *)&m_addrProxy, sizeof(struct sockaddr));
	if (nRet == -1)
	{
		if (::GetLastError() == WSAEWOULDBLOCK)
		{
			if (ReConnect())
				nRet = ::sendto(hSocket, szTemp, nBufSize + 10, 0, 
				              (struct sockaddr *)&m_addrProxy, sizeof(struct sockaddr));
		}
	}
	delete []szTemp;
	return nRet;
}

//接收UDP消息
int CSocks5AgentSocket::RecvUdp(SOCKET hSocket, char *szBuff, int nBufSize, 
								 DWORD &nSrcIp, WORD &wSrcPort)
{
	if ((m_nSocket == INVALID_SOCKET) || (hSocket == INVALID_SOCKET))
		return SOCKET_ERROR;
	int nTempSize = nBufSize + 255;
	char *szTemp = new char[nTempSize];
	memset(szTemp, 0, nTempSize);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	memset(&addr, 0, sizeof(sockaddr));
	int addrlen = sizeof(struct sockaddr_in);
	int nSize = ::recvfrom(hSocket, szTemp, nTempSize, 0, (struct sockaddr *)&addr, &addrlen);
	if ((nSize != SOCKET_ERROR) && (nSize != 0))
	{
		if (szTemp[3] == 0x01)
		{
			memmove(&nSrcIp, szTemp + 4, sizeof(DWORD));
			memmove(&wSrcPort, szTemp + 4 + sizeof(DWORD), sizeof(WORD));
			nSize -= 10;
			memmove(szBuff, szTemp + 10, nSize);
		} else if (szTemp[3] == 0x03)
		{
			int nIpLen = szTemp[4];
			char *szIp = new char[nIpLen + 1];
			memset(szIp, 0, nIpLen + 1);
			memmove(szIp, szTemp + 5, nIpLen);
			nSrcIp = ::inet_addr(szIp);
			delete []szIp;
            memmove(&wSrcPort, szTemp + nIpLen + 5, sizeof(WORD));
			nSize = nSize - nIpLen - 7;
			memmove(szBuff, szTemp + nIpLen + 7, nSize);
		} else 
			nSize = -1;
	} else
	{
		if (::GetLastError() != WSAEWOULDBLOCK)
			ReConnect();
		else
			PRINTDEBUGLOG(dtInfo, "recv from proxy failed:%d", ::WSAGetLastError());
	}
	delete []szTemp;
	return nSize;
}

//是否有数据可读
BOOL CSocks5AgentSocket::CanRead(DWORD dwTimeout)
{
	if (m_nSocket == INVALID_SOCKET)
		return FALSE;
	fd_set rd;
	timeval val;
	val.tv_sec = dwTimeout / 1000;  //5秒
	val.tv_usec = dwTimeout % 1000;
	FD_ZERO(&rd);
	FD_SET(m_nSocket, &rd);
	int nCount = ::select(m_nSocket + 1, &rd, NULL, NULL, &val);
	if (nCount > 0)
		return ::FD_ISSET(m_nSocket, &rd);
	return FALSE;
}


BOOL CSocks5AgentSocket::CheckSocketLive()
{
	
	return TRUE;
}

BOOL CSocks5AgentSocket::ReConnect()
{
	SOCKET hSocket = m_hMapSocket;
	std::string strAgentAddr = m_strAgentAddr;
	std::string strAgentName = m_strAgentName;
	std::string strAgentPwd = m_strAgentPwd;
	WORD wPort = m_nAgentPort;
	if (m_nSocket)
	{
		::closesocket(m_nSocket);
		m_nSocket = INVALID_SOCKET;
	}
	PRINTDEBUGLOG(dtInfo, "reconnect agent server");
	return ConnectAgent(strAgentAddr.c_str(), wPort, strAgentName.c_str(), strAgentPwd.c_str(), hSocket);
}
