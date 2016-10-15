#include <Netlib/UDTSocket.h>
#include <Commonlib/SystemUtils.h>
#include <commonlib/debuglog.h>

#pragma warning(disable:4996)

#ifndef ADDR_ANY
#define ADDR_ANY 0
#endif

#define RECV_BUFFER_TIME_OUT_INTERVAL 15000  //数据传送超时
#define UDT_PACKET_MSS_VALUE          1408   //UDT包大小
#define UDT_HEADER_SIZE               16     //UDT的header大小

CUDPTransSocket::CUDPTransSocket(void):
                 m_dwInternetIP(0),
				 m_wInternetPort(0)
{
	m_hSocket = UDT::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    int nMss = UDT_PACKET_MSS_VALUE;
	UDT::setsockopt(m_hSocket, 0, UDT_MSS, &nMss, sizeof(int));
	m_hWaitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CUDPTransSocket::~CUDPTransSocket(void)
{
	::SetEvent(m_hWaitEvent);
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		UDT::close(m_hSocket);
		m_hSocket = UDT::INVALID_SOCK;
	}
	::CloseHandle(m_hWaitEvent);
}

void CUDPTransSocket::InitUDT()
{
	UDT::startup();
}

void CUDPTransSocket::CleanUpUDT()
{
	UDT::cleanup();
}

void CUDPTransSocket::Attach(UDTSOCKET u)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		UDT::close(m_hSocket);
	}
    m_hSocket = u;
}

int CUDPTransSocket::SendRawData(char *lpBuffer, int nSize, DWORD nIp, WORD nPort)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		SOCKET h = UDT::getsockethandle(m_hSocket);
		if (h != UDT::INVALID_SOCK)
		{
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_addr.S_un.S_addr = htonl(nIp);
			addr.sin_family = AF_INET;
			addr.sin_port = htons(nPort);
			sendto(h, lpBuffer, nSize, 0, (struct sockaddr *)&addr, sizeof(sockaddr));
			return 0;
		}
	}
	return 0;
}

BOOL CUDPTransSocket::GetSocketInfo(char *szIp, WORD &wPort)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));

		int nSockAddrLen = sizeof(sockAddr);
		if (UDT::getsockname(m_hSocket, (SOCKADDR *)&sockAddr, &nSockAddrLen) != UDT::ERROR)
		{
			wPort = ntohs(sockAddr.sin_port);
		    strcpy(szIp, inet_ntoa(sockAddr.sin_addr));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CUDPTransSocket::GetPeerSocketInfo(char *szIp, WORD &wPort)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));

		int nSockAddrLen = sizeof(sockAddr);
		if (UDT::getpeername(m_hSocket, (SOCKADDR *)&sockAddr, &nSockAddrLen) != UDT::ERROR)
		{
			wPort = ntohs(sockAddr.sin_port);
		    strcpy(szIp, inet_ntoa(sockAddr.sin_addr));
			return TRUE;
		}
	}
	return FALSE;
}

//绑定端口
BOOL CUDPTransSocket::Bind(short nPort)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(sockaddr_in));
		addr.sin_addr.S_un.S_addr = ADDR_ANY;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(nPort);
		return (UDT::bind(m_hSocket, (struct sockaddr *)&addr, sizeof(sockaddr_in)) == 0);
	} else
		return FALSE;
}

//设置缓存区
BOOL CUDPTransSocket::SetSndBufferSize(DWORD dwBufSize)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		try
		{
			return (UDT::setsockopt(m_hSocket, 0, UDT_SNDBUF, &dwBufSize, sizeof(DWORD)) == 0);
		} catch(...)
		{
			return FALSE;
		}
	}
	return FALSE;
}

BOOL CUDPTransSocket::SetRecvBufferSize(DWORD dwBufSize)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		try
		{
			return (UDT::setsockopt(m_hSocket, 0, UDT_RCVBUF, &dwBufSize, sizeof(DWORD)) == 0);
		} catch(...)
		{
			return FALSE;
		}
	}
	return FALSE;
}

//设置异步方式
BOOL CUDPTransSocket::SetSyncBlock(bool bSync)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		try
		{
			BOOL bSucc =  (UDT::setsockopt(m_hSocket, 0, UDT_SNDSYN, &bSync, sizeof(bool)) == 0);
			return ((UDT::setsockopt(m_hSocket, 0, UDT_RCVSYN, &bSync, sizeof(bool)) == 0) && bSucc);
		} catch(...)
		{
			return FALSE;
		}
	}
	return FALSE;
}

//
int CUDPTransSocket::GetUdtSocketName(sockaddr *addr, int *nlen)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		return UDT::getsockname(m_hSocket, addr, nlen);
	}
	return -1;
}

//启动监听
BOOL CUDPTransSocket::Listen()
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		return (UDT::listen(m_hSocket, 5) == 0);
	} else
		return FALSE;
}
	
//发送一个文件
int64_t CUDPTransSocket::SendFile(char *szFileName)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		std::ifstream ifs(szFileName, std::ios::in | std::ios::binary);
		if (ifs)
		{
			ifs.seekg(0, std::ios_base::end);
			int64_t size = ifs.tellg();
			ifs.seekg(0, std::ios_base::beg);
			int64_t sendsize =  UDT::sendfile(m_hSocket, (std::fstream &)ifs, 0, size);
			ifs.close();
			return sendsize;
		} else
			return 0;
	} else
		return 0;
}

//接收一个文件
int64_t CUDPTransSocket::RecvFile(char *szFileName, int64_t size)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		std::ofstream ofs(szFileName, std::ios::out | std::ios::binary | std::ios::trunc);
		int64_t RecvSize = UDT::recvfile(m_hSocket, (std::fstream &)ofs, 0, size);
		ofs.close();
		return RecvSize;
	} else
		return 0;
}

//连接服务器
BOOL CUDPTransSocket::Connect(int nIp, short nPort, DWORD dwTimeOut)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		if (dwTimeOut > 0)
			UDT::setsockopt(m_hSocket, 0, UDT_SNDTIMEO, &dwTimeOut, sizeof(DWORD));
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(sockaddr));
		addr.sin_addr.S_un.S_addr = htonl(nIp);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(nPort);
		return (UDT::connect(m_hSocket, (struct sockaddr *)&addr, sizeof(sockaddr)) == 0);
	} else
		return FALSE;
}

//中止传输
void CUDPTransSocket::Teminate()
{
	SetEvent(m_hWaitEvent);
	UDT::triggerSelect(m_hSocket);
}

//等待缓存发送完毕
void CUDPTransSocket::WaitSendComplete()
{
	if (m_hSocket != UDT::INVALID_SOCK)
		UDT::WaitSendEvent(m_hSocket);
}

//发送数据包
int CUDPTransSocket::SendBuffer(const char *lpBuffer, int nSize)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		DWORD dwStart = GetTickCount();
		int nPos = 0;
		int nSndSize;
		UDT::getlasterror().clear();
		do
		{
			try
			{
				nSndSize = UDT::send(m_hSocket, lpBuffer + nPos, nSize - nPos, 0);
			}catch(...)
			{
				return -1;
			}
			if (nSndSize > 0)
			{
				nPos += nSndSize;
			}
			if (nPos >= nSize)
				return nPos;
			if (UDT::getlasterror().getErrorCode() != CUDTException::SUCCESS)
			{
				PRINTDEBUGLOG(dtInfo, "send udt data failed, msg:%s", UDT::getlasterror().getErrorMessage());
				return nPos;
			}
			if (::WaitForSingleObject(m_hWaitEvent, 500) != WAIT_TIMEOUT)
			{
				return nPos;
			}
		}while( (GetTickCount() - dwStart) < RECV_BUFFER_TIME_OUT_INTERVAL);
	}
	return -1;
}

//开始传输
void CUDPTransSocket::StartTransfer()
{
	::ResetEvent(m_hWaitEvent);
}

//获取缓冲区大小
int CUDPTransSocket::GetCurrSndBufferSize()
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		return (UDT::getCurrSndBufferSize(m_hSocket) * UDT_PACKET_MSS_VALUE - UDT_HEADER_SIZE);
	}
	return -1;
}

int CUDPTransSocket::GetCurrRcvBufferSize()
{
	if (m_hSocket != UDT::INVALID_SOCK)
		return (UDT::getCurrRcvBufferSize(m_hSocket) * UDT_PACKET_MSS_VALUE - UDT_HEADER_SIZE);
	return -1;
}

//是否能读取数据
int CUDPTransSocket::CanRead(const DWORD dwTimeOut)
{
	if (m_hSocket == UDT::INVALID_SOCK)
		return FALSE;
	UDT::UDSET rfds;
	UD_ZERO(&rfds);
	UD_SET(m_hSocket, &rfds);
    timeval val;
	val.tv_sec = dwTimeOut / 1000;
	val.tv_usec = (dwTimeOut % 1000) * 1000;
	UDT::getlasterror().clear();
	if (UDT::select(1, &rfds, NULL, NULL, &val) != UDT::ERROR)
	{
		if (UD_ISSET(m_hSocket, &rfds))
			return SELECT_HAS_DATA_READ;
		else
			return 0;
	} else
	{
		if (UDT::getlasterror().getErrorCode() != CUDTException::SUCCESS)
		{
			PRINTDEBUGLOG(dtInfo, "recv udt data failed, msg:%s", UDT::getlasterror().getErrorMessage());
			return SELECT_READ_ERROR;
		}
	}
	return SELECT_READ_TIME_OUT;
}

//接收到数据包
int CUDPTransSocket::RecvBuffer(char *lpBuffer, int nSize)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		DWORD dwStart = GetTickCount();
		int nPos = 0;
		int nRcvSize;
		UDT::getlasterror().clear();
		do
		{
			try
			{
				nRcvSize = UDT::recv(m_hSocket, lpBuffer + nPos, nSize - nPos, 0);
			}catch(...)
			{
				PRINTDEBUGLOG(dtInfo, "udt recv exception");
				return -1;
			}
			if (nRcvSize > 0)
			{
				nPos += nRcvSize;
			}
			if (nPos >= nSize)
				return nPos;
			if (UDT::getlasterror().getErrorCode() != CUDTException::SUCCESS)
			{
				PRINTDEBUGLOG(dtInfo, "recv udt data failed, msg:%s", UDT::getlasterror().getErrorMessage());
				return nPos;
			}
			if (::WaitForSingleObject(m_hWaitEvent, 500) != WAIT_TIMEOUT)
			{
				return nPos;
			}
		}while( (GetTickCount() - dwStart) < RECV_BUFFER_TIME_OUT_INTERVAL);
	}
	return -1;
}

//虚函数
UDTSOCKET CUDPTransSocket::accept(struct sockaddr* addr, int* addrlen)
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		return UDT::accept(m_hSocket, addr, addrlen);
	} else
		return 0;
}

UDT::ERRORINFO & CUDPTransSocket::GetLastError()
{
	return UDT::getlasterror();
}

//清除统计数据
void CUDPTransSocket::ClearRate()
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		UDT::TRACEINFO rate;
		UDT::perfmon(m_hSocket, &rate, true);
	}
}

int CUDPTransSocket::GetSendRate()
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		UDT::TRACEINFO rate;
		UDT::perfmon(m_hSocket, &rate, false);
		double r = (rate.mbpsSendRate * 1024 * 128/*1024 / 8 */);
		return (int)r;
	} else
		return 0;
}

int CUDPTransSocket::GetRecvRate()
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		UDT::TRACEINFO rate;
		UDT::perfmon(m_hSocket, &rate, false);
		double r = (rate.mbpsRecvRate * 1024 * 128 /*1024 / 8 */);
		return (int)r;
	} else
		return 0;
}

void CUDPTransSocket::Close()
{
	if (m_hSocket != UDT::INVALID_SOCK)
	{
		UDT::close(m_hSocket);
	}
}


// ==========  class CUDPTransSocketServer ===
CUDPTransSocketServer::CUDPTransSocketServer(IUDTTransServerApp *pApp):
                       m_pApp(pApp),
					   m_bTerminated(FALSE),
					   m_wListenPort(0),
					   m_hThread(NULL)
{

}

CUDPTransSocketServer::~CUDPTransSocketServer()
{
	m_bTerminated = TRUE;
	m_UdtSocket.Close();
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, THREAD_WAIT_TIME_OUT_INTERVAL);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

BOOL CUDPTransSocketServer::StartListen(WORD wPort)
{
	if (m_UdtSocket.Bind(wPort))
	{
		if (m_UdtSocket.Listen())
		{
			SOCKADDR_IN addr = {0};
			addr.sin_family = AF_INET;
			int nLen = sizeof(sockaddr);
			if (m_UdtSocket.GetUdtSocketName((sockaddr *)&addr, &nLen) == 0)
			{
				m_wListenPort = ntohs(addr.sin_port);
				PRINTDEBUGLOG(dtInfo, "Get UDT SOCKET Name succ, nPort:%d ListenPort:%d", wPort, m_wListenPort);
			} else
				PRINTDEBUGLOG(dtInfo, "Get UDT Socket Name failed, nPort:%d", wPort);
			m_hThread = CreateThread(NULL, 0, QueryAcceptThread, this, 0, NULL);
			return (m_hThread != NULL);
		}
	}
	return FALSE;
}

WORD CUDPTransSocketServer::GetListenPort()
{
	return m_wListenPort;
}

void CUDPTransSocketServer::SetInternetIP(DWORD dwIp)
{
	m_UdtSocket.m_dwInternetIP = dwIp;
}

void CUDPTransSocketServer::SetInternetPort(WORD wPort)
{
	m_UdtSocket.m_wInternetPort = wPort;
}

DWORD CUDPTransSocketServer::GetInternetIP()
{
	return m_UdtSocket.m_dwInternetIP;
}

WORD CUDPTransSocketServer::GetInternetPort()
{
	return (m_UdtSocket.m_wInternetPort == 0 ? htons(m_wListenPort) : m_UdtSocket.m_wInternetPort);
}

//线程
DWORD WINAPI CUDPTransSocketServer::QueryAcceptThread(LPVOID lpParam)
{
	CUDPTransSocketServer *pThis = (CUDPTransSocketServer *)lpParam;
	UDTSOCKET u;
	struct sockaddr addr;
	int addrlen;
	while (!pThis->m_bTerminated)
	{
		addrlen = sizeof(struct sockaddr);
		memset(&addr, 0, addrlen);
		addr.sa_family = PF_INET;
		u = pThis->m_UdtSocket.accept(&addr, &addrlen);
		if ((u != UDT::INVALID_SOCK) && (pThis->m_pApp))
		{
			CNetAddress PeerAddr;
			PeerAddr = addr;
			pThis->m_pApp->OnAccept(u, PeerAddr);
		}
	}
	return 0;
}

//通过最底层socket发送消息，绕过ＵＤＴ机制
int CUDPTransSocketServer::SendRawData(char *lpBuffer, int nSize, DWORD dwIp, WORD wPort)
{
	return m_UdtSocket.SendRawData(lpBuffer, nSize, dwIp, wPort);
}

//设置发送和接收缓存区大小
BOOL CUDPTransSocketServer::SetSocketBufferSize(int nSndBufSize, int nRcvBufSize)
{
	return (m_UdtSocket.SetSndBufferSize(nSndBufSize) && m_UdtSocket.SetRecvBufferSize(nRcvBufSize));
}

//
BOOL CUDPTransSocketServer::SetSocketSync(bool bSync)
{
	return m_UdtSocket.SetSyncBlock(bSync);
}

#pragma warning(default:4996)
