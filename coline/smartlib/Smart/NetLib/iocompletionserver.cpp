#include <netlib/IoCompletionServer.h>
#include <commonlib/debuglog.h>
#include <MSTcpIP.h>

#define EAGAIN 11 
#define EINTR  4

#define SOFT_ERROR(e)	((e) == WSAEINTR || \
	(e) == WSA_IO_PENDING || \
	(e) == WSAEWOULDBLOCK || \
	(e) == EINTR || \
	(e) == EAGAIN || \
	(e) == 0)

GUID CIoCompletionServer::m_GUIDAcceptEx= WSAID_ACCEPTEX;
GUID CIoCompletionServer::m_GUIDGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

CIoCompletionServer::CIoCompletionServer(DWORD dwPktCount, DWORD dwPktSize):
                     m_IOCP(0),
					 m_pAcceptEx(NULL),
					 m_hListenSocket(INVALID_SOCKET),
					 m_hListenEvent(NULL),
					 m_pGetAcceptExSockaddrs(NULL),
#ifdef DEBUG_PRINT_ALLOC_COUNT
					 m_nAllocSocket(0),
					 m_nFreeSocket(0),
#endif
					 m_Allocator(dwPktCount, dwPktSize)
{

}


CIoCompletionServer::~CIoCompletionServer(void)
{
	Stop();
}

//初始化函数指针
void CIoCompletionServer::GetFunctionPointer()
{
	SOCKET s;
	unsigned long dwResult;
	s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (INVALID_SOCKET == s ) 
	{
		PRINTDEBUGLOG(dtInfo, "create socket error:%d", ::WSAGetLastError());
	}
    do
	{
		if (SOCKET_ERROR == WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &m_GUIDAcceptEx, sizeof(m_GUIDAcceptEx),
    							  &m_pAcceptEx, sizeof(m_pAcceptEx), &dwResult, NULL, NULL))
		{
			PRINTDEBUGLOG(dtInfo, "get acceptex function error:%d", ::WSAGetLastError());
			break;
		}

		if (SOCKET_ERROR == WSAIoctl(s,	SIO_GET_EXTENSION_FUNCTION_POINTER,	&m_GUIDGetAcceptExSockAddrs, sizeof(m_GUIDGetAcceptExSockAddrs),
                    			&m_pGetAcceptExSockaddrs,	sizeof(m_pGetAcceptExSockaddrs), &dwResult, NULL, NULL))
		{
			PRINTDEBUGLOG(dtInfo, "get acceptexsockaddrs function error:%d", ::WSAGetLastError());
			break;
		}
	} while (FALSE);
	::closesocket(s);
}

//启动监听
BOOL CIoCompletionServer::Start(const char *szServerIp, WORD wPort)
{
    m_bTerminated = FALSE;
    GetFunctionPointer();
	m_hListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_hListenSocket == INVALID_SOCKET)
	{
		PRINTDEBUGLOG(dtInfo, "create listen socket error:%d", ::WSAGetLastError());
		return FALSE;
	}
	m_hListenEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_hListenEvent)
	{
		PRINTDEBUGLOG(dtInfo, "Create Listen Event error:%d", ::GetLastError());
		return FALSE;
	}
	do
	{
		//
		sockaddr_in addr;
		memset(&addr, 0, sizeof(sockaddr_in));
		if (szServerIp && (::strlen(szServerIp) > 0))
			addr.sin_addr.s_addr = inet_addr(szServerIp);
		else
			addr.sin_addr.s_addr = ADDR_ANY;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(wPort);
		if (SOCKET_ERROR == ::bind(m_hListenSocket, (struct sockaddr *)&addr, sizeof(addr)))
		{
			PRINTDEBUGLOG(dtInfo, "bind socket error:%d", ::WSAGetLastError());
		}
		if (SOCKET_ERROR == ::listen(m_hListenSocket, SOMAXCONN))
		{
			PRINTDEBUGLOG(dtInfo, "listen socket error:%d", ::WSAGetLastError());
		}
		if (SOCKET_ERROR == ::WSAEventSelect(m_hListenSocket, m_hListenEvent, FD_ACCEPT))
		{
			PRINTDEBUGLOG(dtInfo, "Event Select error:%d", ::WSAGetLastError());
		}
		//关联到IOCP
		PostAcceptEx(m_hListenSocket, DEFAULT_ACCEPT_TIMES);
		m_IOCP.AssociateDevice(reinterpret_cast<HANDLE>(m_hListenSocket), 0);
		
		//

		//创建工作线程和监听线程
		m_hListen = ::CreateThread(NULL, 0, ListenThread, this, 0, NULL);
		if (!m_hListen)
		{
			PRINTDEBUGLOG(dtInfo, "create listen thread error:%d", ::GetLastError());
		}

		//创建工作线程
		SYSTEM_INFO systemInfo = {0};
		::GetSystemInfo(&systemInfo);
		DWORD dwWorkCount = systemInfo.dwNumberOfProcessors * 2 + 2;
		for (DWORD i = 0; i < dwWorkCount; i ++)
		{
			HANDLE hThread = ::CreateThread(NULL, 0, WorkThread, this, 0, NULL);
			if (!hThread)
			{
				PRINTDEBUGLOG(dtInfo, "create work thread error:%d", ::GetLastError());
			} else
				m_WorkThreadList.push_back(hThread);
		}
		return TRUE;
	}while (FALSE);
	if (m_hListenSocket != INVALID_SOCKET)
	{
		::closesocket(m_hListenSocket);
		m_hListenSocket = INVALID_SOCKET;
	}
	if (m_hListenEvent)
	{
		::CloseHandle(m_hListenEvent);
		m_hListenEvent = NULL;
	}
	return FALSE;
}

BOOL CIoCompletionServer::Stop()
{
	//停止监听线程
	m_bTerminated = TRUE;
	if (m_hListenEvent)
		SetEvent(m_hListenEvent);
	if (m_hListenSocket != INVALID_SOCKET)
		::closesocket(m_hListenSocket);
	if (m_hListenEvent)
		::CloseHandle(m_hListenEvent);

	//停止工作线程
	std::vector<HANDLE>::iterator itWorkThread;
	for (itWorkThread = m_WorkThreadList.begin(); itWorkThread != m_WorkThreadList.end(); itWorkThread ++)
	{
		m_IOCP.PostStatus(NULL, 0, NULL);
	}
	for (itWorkThread = m_WorkThreadList.begin(); itWorkThread != m_WorkThreadList.end(); itWorkThread ++)
	{
		::WaitForSingleObject((*itWorkThread), 1000);
		::CloseHandle((*itWorkThread));
	}
	m_WorkThreadList.clear();
	m_pAcceptEx = NULL;
	m_pGetAcceptExSockaddrs = NULL;
	return TRUE;
}

void CIoCompletionServer::PostAcceptEx(SOCKET s, int n)
{
	SOCKET hAccept;
	DWORD dwBytes = 0;
	int i = 0;
	while (i < n)
	{
		hAccept = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (INVALID_SOCKET == hAccept)
		{
			PRINTDEBUGLOG(dtInfo, "accept socket create error:%d", ::WSAGetLastError());
		}
		CPacketStream *lpPacket = m_Allocator.AllocPacket();
		//设置参数
		lpPacket->SetIoOperation(CPacketStream::IO_TYPE_ACCEPT_COMPLETE);
		lpPacket->SetupAccept();
		lpPacket->SetSocket(hAccept);
		lpPacket->SetListenSocket(s);
        
		//调用内核函数
		if (FALSE == m_pAcceptEx(s, hAccept, (lpPacket->GetWSABUF()->buf), 0, sizeof(SOCKADDR_IN) + 16, 
			sizeof(SOCKADDR_IN) + 16, &dwBytes, &(lpPacket->m_Overlapped)))
		{
			//如果返回是 ERROR_IO_PENDING 是有效的
			//如果返回是 WSAECONNRESET 被重置
			int nCode = ::WSAGetLastError();
			if (nCode != ERROR_IO_PENDING)
			{
				lpPacket->Release();
				::closesocket(hAccept);
				if (SOFT_ERROR(nCode))
				{
					continue;
				} else
				{
					PRINTDEBUGLOG(dtInfo, "accept error:%d", nCode);
					break;
				}
			}
		} 
		++i;
	}
}

//分配一段内存
CPacketStream *CIoCompletionServer::AllocatePacket()
{
	return m_Allocator.AllocPacket();
}

DWORD CIoCompletionServer::GetPacketSize()
{
	return m_Allocator.GetPacketSize();
}

//监听线程
DWORD WINAPI CIoCompletionServer::ListenThread(LPVOID lpParam)
{
	CIoCompletionServer *pThis = (CIoCompletionServer *)lpParam;

	DWORD dwResult = 0;

	while (TRUE)
	{
		dwResult = ::WaitForSingleObject(pThis->m_hListenEvent, 1000);
		if ((pThis->m_bTerminated) || (WAIT_FAILED == dwResult))
		{
			break;
		}
		if (WAIT_TIMEOUT == dwResult) //超时
		{
			continue;
		}
		pThis->PostAcceptEx(pThis->m_hListenSocket, DEFAULT_ACCEPT_TIMES);
	} 
    return 0;
}

//客户端关闭
void CIoCompletionServer::OnClientClose(CIoCompletionSocket *pSocket, DWORD dwError)
{
	//
}

//工作线程
DWORD WINAPI CIoCompletionServer::WorkThread(LPVOID lpParam)
{
	CIoCompletionServer *pThis = (CIoCompletionServer *)lpParam;
	DWORD dwIoSize = 0;
	CIoCompletionSocket *pSocket = NULL;
	CPacketStream *pBuffer = NULL;
	DWORD dwError = 0;
	LPOVERLAPPED lpOverlapped = NULL;

	while(TRUE)
	{
		if (!pThis->m_IOCP.GetStatus((PULONG_PTR)&pSocket, &dwIoSize, (LPOVERLAPPED *)&lpOverlapped, &dwError))
		{
			//PRINTDEBUGLOG(dtInfo, "get status error, Error:%d", dwError);
			//两种情况，一种是连接异常，一种是关闭listen时候出现，需要区分对待
			if (pSocket)
			{	
				pThis->OnClientClose(pSocket, dwError);	
				//这里要删除
				if (lpOverlapped)
					pBuffer = CONTAINING_RECORD(lpOverlapped, CPacketStream, m_Overlapped);
				else
					pBuffer = NULL;
				if (pSocket->Release() == 0)
				{
					//PRINTDEBUGLOG(dtInfo, "delete socket in onclientclose, socket:%d", (int) pSocket->GetSocket());
#ifdef DEBUG_PRINT_ALLOC_COUNT
					::InterlockedIncrement(&pThis->m_nFreeSocket);
#endif
					delete pSocket;
					//PRINTDEBUGLOG(dtInfo, "get failed");
					//pThis->PrintAllocatorStatus();
				} else
				{
					PRINTDEBUGLOG(dtInfo, "no delete socket in onclientclose, ref:%d, socket:%d", pSocket->GetRef(), (int)pSocket->GetSocket());
				}
				
				if (pBuffer) 
				{
					pBuffer->Release();
					PRINTDEBUGLOG(dtInfo, "Onclientclose Buffer Ref:%d ", pBuffer->GetRef());
					lpOverlapped = NULL;
					pBuffer = NULL;
				}
				pSocket = NULL;
				continue;
			} //end if (pSocket)			
		} //end if (!pThis->...
		if (!pSocket && !lpOverlapped)
		{
			PRINTDEBUGLOG(dtInfo, "GetPortStatus Error, Socket or Overlapped is NULL, Error:%d", dwError);
			break;
		}
		pBuffer = CONTAINING_RECORD(lpOverlapped, CPacketStream, m_Overlapped);
		pThis->DoIoOperation(pSocket, pBuffer, dwIoSize, dwError);

		//处理完毕后释放内存
		if (pSocket)
		{
			if (pSocket->Release() == 0)
			{
				//PRINTDEBUGLOG(dtInfo, "socket is release equal zero, Socket:%d", (int) pSocket->GetSocket());
#ifdef DEBUG_PRINT_ALLOC_COUNT
				::InterlockedIncrement(&pThis->m_nFreeSocket);
#endif
				delete pSocket;	
				//pThis->PrintAllocatorStatus();
			} else if (pSocket->GetRef() < 0)
			{
				PRINTDEBUGLOG(dtInfo, "socket not free, socket:%d, ref:%d", (int) pSocket->GetSocket(), pSocket->GetRef());
			}
			pSocket = NULL;
		}
		if (pBuffer) 
		{
			pBuffer->Release();
			lpOverlapped = NULL;
			pBuffer = NULL;
		}
	} //end while

	if (pSocket)
	{
		if (pSocket->Release() == 0)
		{
			//PRINTDEBUGLOG(dtInfo, "socket is release equal zero, Socket:%d", (int) pSocket->GetSocket());
#ifdef DEBUG_PRINT_ALLOC_COUNT
			::InterlockedIncrement(&pThis->m_nFreeSocket);
#endif
			delete pSocket;
		} else if (pSocket->GetRef() < 0)
		{
			PRINTDEBUGLOG(dtInfo, "socket is release failed, socket:%d ref:%d", (int) pSocket->GetSocket(), pSocket->GetRef());
		}
		pSocket = NULL;

	}
	if (pBuffer) 
	{
		pBuffer->Release();
		lpOverlapped = NULL;
		pBuffer = NULL;
	} 
	return 0;
}


//消息处理
//投递一个消息
void CIoCompletionServer::PostIoOperation(CPacketStream::CIoOperationType nType, CIoCompletionSocket *pSocket, CPacketStream *pStream)
{
	pStream->SetIoOperation(nType);
	pSocket->AddRef();
	pStream->AddRef();
	m_IOCP.PostStatus((ULONG_PTR)pSocket, 0, &(pStream->m_Overlapped));
}

//IO操作处理
void CIoCompletionServer::DoIoOperation(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	CPacketStream::CIoOperationType nType = pStream->GetIoOperator();
	switch(nType)
	{
		case CPacketStream::IO_TYPE_READ_REQUEST:
			{
				DoReadRequest(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_READ_COMPLETE:
			{
				DoReadCompletion(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_WRITE_REQUEST:
			{
				DoWriteRequest(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_WRITE_COMPLETE:
			{
				DoWriteCompletion(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_ACCEPT_COMPLETE:
			{
				DoAcceptCompletion(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_CONNECT_REQUEST:
			{
				//
				DoSocketConnect(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_CONNECT_COMPLETE:  //连接完成
			{
				DoConnectComplete(pSocket, pStream, dwIoSize, dwLastError);
				break;
			}
		case CPacketStream::IO_TYPE_CLOSE:
			{
				break;
			}
	}
}

//处理数据
BOOL CIoCompletionServer::ParseStream(CPacketStream *pStream, CIoCompletionSocket *pSocket)
{
	//默认丢弃所有数据 DEBUG
	char *szTemp = new char[pStream->GetPosition() + 1];
	memmove(szTemp, pStream->GetData(), pStream->GetPosition());
	szTemp[pStream->GetPosition()] = '\0';
	PRINTDEBUGLOG(dtInfo, "Recv Client Data(SOCKET:%d):%s", pSocket->m_hSocket, szTemp);
	delete []szTemp;
	pStream->Initialize();
	static const char *szRecv = "ack message\n";
	pSocket->WriteBuff(szRecv, (int)::strlen(szRecv));
	return TRUE;
}

//读取数据
void CIoCompletionServer::Read(CIoCompletionSocket *pSocket, CPacketStream *pStream)
{
	pSocket->AddRef();
	pStream->AddRef();
	pStream->SetIoOperation(CPacketStream::IO_TYPE_READ_COMPLETE);
	pStream->SetupRead();
	

	CGuardLock::COwnerLock guard(pSocket->m_wLock);
	DWORD dwNumBytes = 0;
	DWORD dwFlags =0;
	if (SOCKET_ERROR == ::WSARecv(pSocket->m_hSocket, pStream->GetWSABUF(), 1, &dwNumBytes, 
		     &dwFlags, &pStream->m_Overlapped, NULL))
	{
		DWORD dwError = ::WSAGetLastError();
		if (ERROR_IO_PENDING != dwError)
		{
			pSocket->Release();
			pStream->Release();
		}
	}
}

//写入数据
void CIoCompletionServer::Write(CIoCompletionSocket *pSocket, CPacketStream *pStream)
{
	pSocket->AddRef();
	pStream->AddRef();
	pStream->SetIoOperation(CPacketStream::IO_TYPE_WRITE_COMPLETE);
	pStream->SetupWrite();
	

	DWORD dwFlags = 0;
	DWORD dwSendNumBytes = 0;

	if (SOCKET_ERROR == ::WSASend(pSocket->m_hSocket, pStream->GetWSABUF(), 1, &dwSendNumBytes, dwFlags, &pStream->m_Overlapped, NULL))
	{
		DWORD dwError = ::WSAGetLastError();
		if (ERROR_IO_PENDING != dwError)
		{
			//写入错误
			pSocket->Release();
			pStream->Release();
		}
	}
}

//读取请求消息
void CIoCompletionServer::DoReadRequest(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	Read(pSocket, pStream);
}

//连接成功
void CIoCompletionServer::DoSocketConnect(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	pSocket->AddRef();
	pStream->AddRef();
	pStream->SetIoOperation(CPacketStream::IO_TYPE_CONNECT_COMPLETE);
	pStream->SetupWrite();
	

	DWORD dwFlags = 0;
	DWORD dwSendNumBytes = 0;

	if (SOCKET_ERROR == ::WSASend(pSocket->m_hSocket, pStream->GetWSABUF(), 1, &dwSendNumBytes, dwFlags, &pStream->m_Overlapped, NULL))
	{
		DWORD dwError = ::WSAGetLastError();
		if (ERROR_IO_PENDING != dwError)
		{
			//写入错误
			pSocket->Release();
			pStream->Release();
		}
	}
}

//连接完成
void CIoCompletionServer::DoConnectComplete(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	//
}

//读取完成
void CIoCompletionServer::DoReadCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	pStream->Seek(dwIoSize, CBaseStream::STREAM_CURR); //
	if (dwIoSize != 0)
	{
		//处理数据
		if (ParseStream(pStream, pSocket))
		{
			if (!pSocket->Read(pStream))
			{
				//读取出现错误
				PRINTDEBUGLOG(dtInfo, "read stream from socket failed");
				OnClientClose(pSocket, dwLastError);
			}
		} else
		{
			//收到错误的协议包，关闭连接
			PRINTDEBUGLOG(dtInfo, "Parse Stream Fail");
			OnClientClose(pSocket, dwLastError);
			pSocket->CloseByPeer();
			//这里不做删除
			//pSocket->Release();
		}
	} else if (0 == dwLastError)
	{
		//客户端被关闭
		//PRINTDEBUGLOG(dtInfo, "Client Socket is Closed in ReadCompletion");
		OnClientClose(pSocket, dwLastError);
		//pSocket->Release();
	}	
}

//写入请求
void CIoCompletionServer::DoWriteRequest(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	Write(pSocket, pStream);
}

//写入完成
void CIoCompletionServer::DoWriteCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	pStream->Seek(dwIoSize, CBaseStream::STREAM_CURR);
	if (pStream->GetWSABUF()->len > dwIoSize)
	{
		//继续发送剩余数据
		PRINTDEBUGLOG(dtInfo, "write leave size, TotalSize:%d Sent Size:%d", pStream->GetWSABUF()->len, dwIoSize);
		pStream->RemoveData(dwIoSize);
		Write(pSocket, pStream);
		return;
	}

	pSocket->WriteCompleted();
}

//接收到连接请求
void CIoCompletionServer::DoAcceptCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError)
{
	SOCKET hAccept = pStream->GetSocket();
	SOCKET hListen = pStream->GetListenSocket();
	tcp_keepalive inTcpLive = {0};
	tcp_keepalive OutTcpLive = {0};
	DWORD  dwBytes  = 0;
	int   nLen = 0;
	sockaddr_in * lpRemoteAddr;
	sockaddr_in * lpLocalAddr;
	CIoCompletionSocket * lpAcceptSocket = NULL; 

	if (NOERROR != dwLastError)
	{
		PRINTDEBUGLOG(dtInfo, "Accept completion error:%d", dwLastError);
		return;
	}

	if (SOCKET_ERROR == ::setsockopt(hAccept, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&hListen, sizeof(hListen)))
	{
		PRINTDEBUGLOG(dtInfo, "setsockopt error:%d", ::WSAGetLastError());
		return;
	}

	inTcpLive.onoff = 1;
	inTcpLive.keepaliveinterval = DEFAULT_KEEPALIVEINTERVAL;
	inTcpLive.keepalivetime = DEFAULT_KEEPALIVETIME;
	if (SOCKET_ERROR == ::WSAIoctl(hAccept, SIO_KEEPALIVE_VALS, &inTcpLive, sizeof(tcp_keepalive), &OutTcpLive, sizeof(tcp_keepalive), 
		                       &dwBytes, NULL, NULL))
	{
		PRINTDEBUGLOG(dtInfo, "ctrl keeplive time error:%d", ::WSAGetLastError());
		return;
	}
 
    nLen = sizeof(sockaddr_in);
	m_pGetAcceptExSockaddrs(pStream->GetWSABUF()->buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		         (sockaddr **)&lpLocalAddr, &nLen, (sockaddr **)&lpRemoteAddr, &nLen);
	lpAcceptSocket = AllocateSocket(hAccept, lpLocalAddr, lpRemoteAddr);
#ifdef DEBUG_PRINT_ALLOC_COUNT
	::InterlockedIncrement(&m_nAllocSocket);
#endif
	if (!lpAcceptSocket)
	{
		PRINTDEBUGLOG(dtInfo, "AllocateSocket error");
		return;
	}	
	if (AssociateDevice(hAccept, lpAcceptSocket))
	{
		lpAcceptSocket->Release();
		//pStream->Release();
		//PRINTDEBUGLOG(dtInfo, "AssociateDevice Stream Ref:%d", pStream->GetRef());
	} else //关联出错
	{
#ifdef DEBUG_PRINT_ALLOC_COUNT
		::InterlockedIncrement(&m_nFreeSocket);
#endif
		delete lpAcceptSocket;
		//pStream->Release();
		//PRINTDEBUGLOG(dtInfo, "AssociateDevice Failed, Stream Ref:%d", pStream->GetRef());
	}
}

//关联一个SOCKET 到IOCP
BOOL CIoCompletionServer::AssociateDevice(SOCKET h, CIoCompletionSocket *pSocket)
{
	if (m_IOCP.AssociateDevice(reinterpret_cast<HANDLE>(h), (ULONG_PTR)pSocket))
	{
		return pSocket->Read();
	}
	return FALSE;
}

//关联一个SOCKET 到IOCP 测试是否连接成功
BOOL CIoCompletionServer::AssociateConnectStatus(SOCKET h, CIoCompletionSocket *pSocket)
{
	if (m_IOCP.AssociateDevice(reinterpret_cast<HANDLE>(h), (ULONG_PTR)pSocket))
	{
		return pSocket->ReadConnectStatus();
	}
	return FALSE;
}

//debug 打印内存状态
void CIoCompletionServer::PrintAllocatorStatus()
{
#ifdef DEBUG_PRINT_ALLOC_COUNT
	int nAllocCount = 0, nReleaseCount = 0, nListCount = 0, nNewCount = 0;
	m_Allocator.GetAllocStatus(nAllocCount, nReleaseCount, nListCount, nNewCount);
	PRINTDEBUGLOG(dtInfo, "Allocator Status, AllocCount:%d ReleaseCount:%d ListCount:%d NewCount:%d Alloc Socket:%d Free Socket:%d", 
		nAllocCount, nReleaseCount, nListCount, nNewCount, m_nAllocSocket, m_nFreeSocket);
#endif
}