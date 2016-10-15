#include <Netlib/ClientUdpSocket.h>
#include <Commonlib/Debuglog.h>

#pragma warning(disable:4996)

CClientUdpSocket::CClientUdpSocket(IClientUdpProcessApp *pApp):
                  m_pApp(pApp),
				  m_dwSeq(0),
				  m_RcvThread(NULL),
				  m_SndThread(NULL),
				  m_ReSndThread(NULL),
				  m_CheckRcvDataThread(NULL),
				  m_bAgent(FALSE),
				  m_bTerminated(false)
{
	m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_hSocket == INVALID_SOCKET)
	{
		PRINTDEBUGLOG(dtError, "无法创建socket");
	}
	m_RcvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_SndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hBreak = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CClientUdpSocket::~CClientUdpSocket(void)
{
	Terminate();
	if (m_RcvThread)
	{
	   WaitForSingleObject(m_RcvThread, 5000);
	   CloseHandle(m_RcvThread);
	}
	if (m_SndThread)
	{
	   WaitForSingleObject(m_SndThread, 5000);
	   CloseHandle(m_SndThread);
	}
	if (m_ReSndThread)
	{
		WaitForSingleObject(m_ReSndThread, 5000);
		CloseHandle(m_ReSndThread);
	}
	closesocket(m_hSocket);
	if (m_CheckRcvDataThread)
	{
		WaitForSingleObject(m_CheckRcvDataThread, 5000);
		CloseHandle(m_CheckRcvDataThread);
		m_CheckRcvDataThread = NULL;
	}

	//清除工作线程
	CloseHandle(m_RcvEvent);
	CloseHandle(m_SndEvent);
	CloseHandle(m_hBreak);
	Clear();
}

DWORD CClientUdpSocket::GetCurrSeq()
{
	DWORD dwId;
	CGuardLock::COwnerLock guard(m_SeqLock);
    m_dwSeq ++;
	dwId = m_dwSeq;
	return dwId;
}

BOOL CClientUdpSocket::GetSocketName(char *szAddress, WORD &wPort)
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
		if (szAddress)
			::strcpy(szAddress, inet_ntoa(sockAddr.sin_addr));
	}
	return bResult;
}

bool CClientUdpSocket::Start(int nPort)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.S_un.S_addr = 0;
	int len = sizeof(sockaddr_in);
	if (0 == bind(m_hSocket, (sockaddr *)&addr, len))
	{
		if (!GetSocketName(NULL, m_wIntranetPort))
			PRINTDEBUGLOG(dtInfo, "GetSocketName Error, nPort:%d", nPort);
		else
			PRINTDEBUGLOG(dtInfo, "Start Port Succ, nPort:%d IntranetPort:%d", nPort, m_wIntranetPort);
		//接收线程
		m_RcvThread = CreateThread(NULL, 0, RcvMsgProcessThread, this, 0, NULL);
		
		//发送线程
		m_SndThread = CreateThread(NULL, 0, SndMsgProcessThread, this, 0, NULL);

		//检测数据线程
		m_CheckRcvDataThread = CreateThread(NULL, 0, CheckRcvDataThread, this, 0, NULL);		
		
		//重发线程
		m_ReSndThread = CreateThread(NULL, 0, ReSendMessageThread, this, 0, NULL);
		return true;
	}
	return false;
}

//设置代理服务器信息
BOOL CClientUdpSocket::SetAgentAddr(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
	              const char *szAgentPwd)
{
	m_bAgent = FALSE;
	if ((szAgentIp != NULL) && (strlen(szAgentIp) > 0))
		m_bAgent = m_AgentSocket.ConnectAgent(szAgentIp, wPort, szAgentName, szAgentPwd, m_hSocket);
	return m_bAgent;
}

void CClientUdpSocket::Terminate()
{
	m_bTerminated = true;
	SetEvent(m_RcvEvent);
	SetEvent(m_SndEvent);
	SetEvent(m_hBreak);
}

DWORD CClientUdpSocket::GetSocketIP()
{
	if(m_hSocket == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
    
	sockAddr.sin_family = AF_INET;
	int nSockAddrLen = sizeof(sockAddr);
	
	BOOL bResult = (getsockname(m_hSocket, (SOCKADDR*)&sockAddr, &nSockAddrLen)!=SOCKET_ERROR);
	if(bResult)
	{
		return sockAddr.sin_addr.S_un.S_addr;
	}
	return 0;
}

void CClientUdpSocket::SendMsgTo(char *lpBuff, int nSize, int nPeerIp, int nPort, int ReSendTimes, DWORD dwToUserId)
{
	LPUDPMESSAGEITEM pItem = m_pSndIdleList.GetItem();
	if (pItem)
	{
		pItem->nMsgSize = nSize;
		pItem->nPeerIp = nPeerIp;
		pItem->nPeerPort = nPort;
		pItem->dwToUserId = dwToUserId;
		pItem->nReSendTimes = ReSendTimes;
		memmove(pItem->strMessage, lpBuff, nSize);
		if (pItem->nReSendTimes > 0)
		{		
			//做一份拷贝
			LPUDPMESSAGEITEM pCopyItem = m_pSndIdleList.GetItem();
			if (pCopyItem)
			{
				memcpy(pCopyItem, pItem, sizeof(UDPMESSAGEITEM));
				pCopyItem->tmLastSendTime = ::GetTickCount();
				InsertResendList(pCopyItem);
			} else
				PRINTDEBUGLOG(dtError, "SendMsgTo 重发线程无法申请到内存使用权");
		}
		m_pSndMsgList.Insert(pItem);
		SetEvent(m_SndEvent);
	} else
	{
		PRINTDEBUGLOG(dtError, "发送线程无法申请到内存, SendMsgTo");
	}
}

void CClientUdpSocket::SendMsg(LPUDPMESSAGEITEM pItem)
{
	LPUDPMESSAGEITEM pCurrItem = m_pSndIdleList.GetItem();
	if (pCurrItem)
	{
		memmove(pCurrItem, pItem, sizeof(UDPMESSAGEITEM));
		if (pCurrItem->nReSendTimes > 0)
		{
			//做一份拷贝
			LPUDPMESSAGEITEM pCopyItem = m_pSndIdleList.GetItem();
			if (pCopyItem)
			{
				memcpy(pCopyItem, pCurrItem, sizeof(UDPMESSAGEITEM));
				pCopyItem->tmLastSendTime = ::GetTickCount();
				InsertResendList(pCopyItem);
			} else
				PRINTDEBUGLOG(dtError, "Send Msg 重发线程无法申请到内存使用权");
		}
		m_pSndMsgList.Insert(pCurrItem);
		SetEvent(m_SndEvent);
	} else
	{
		PRINTDEBUGLOG(dtError, "发送线程无法申请到内存, SendMsg");
	}
}

void CClientUdpSocket::SendMessageItem(LPUDPMESSAGEITEM pItem)
{
	if (pItem)
	{		
		int nSendSize = 0;
		int toLen = 0;
		if (m_bAgent)
			m_AgentSocket.SendUdp(m_hSocket, pItem->strMessage, pItem->nMsgSize, pItem->nPeerIp, pItem->nPeerPort);
		else
		{
			memset(&m_toAddr, 0, sizeof(sockaddr_in));
			m_toAddr.sin_addr.S_un.S_addr = pItem->nPeerIp;
			m_toAddr.sin_family = AF_INET;
			m_toAddr.sin_port = pItem->nPeerPort;
			toLen = sizeof(struct sockaddr);
			nSendSize = sendto(m_hSocket, pItem->strMessage, pItem->nMsgSize, 0, (struct sockaddr *)&m_toAddr, toLen);
		}
		if ((!m_bAgent) && (nSendSize != pItem->nMsgSize)) //发送失败
		{
			//PRINTDEBUGLOG(dtWarning, "发送UDP消息失败，SOCKET错误，消息长度: %d 已发送长度: %d\n", pItem->nMsgSize, nSendSize);
			timeval val;
			while(!m_bTerminated)
			{
				val.tv_sec = 1;
				val.tv_usec = 0;
				fd_set writeset;
				FD_ZERO(&writeset);
				FD_SET(m_hSocket, &writeset);
				int nCount = select((int)m_hSocket + 1, NULL, &writeset, NULL, &val);
				if ((nCount > 0) && FD_ISSET(m_hSocket, &writeset))
				{
					nSendSize = sendto(m_hSocket, pItem->strMessage, pItem->nMsgSize, 0, (struct sockaddr *)&m_toAddr, toLen);
					if (nSendSize != pItem->nMsgSize)
					{
						PRINTDEBUGLOG(dtInfo, "error address ip: %d port: %d", pItem->nPeerIp, pItem->nPeerPort);
					}
					break;
				}
			}
		} else
			pItem->tmLastSendTime = GetTickCount();
	}
}

BOOL CClientUdpSocket::CheckAckMessage(LPUDPMESSAGEITEM pItem)
{
	CGuardLock::COwnerLock guard(m_ReSendLock);
	int nPos = m_ReSendList.FindDataPos(GetMessageSeqId(pItem));
	if (nPos >= 0)
	{
		m_pSndIdleList.Insert((LPUDPMESSAGEITEM)m_ReSendList[nPos].pData);
		m_ReSendList.Delete(nPos);
		return TRUE;
	}
	return FALSE;
}

void CClientUdpSocket::Clear()
{
	m_pSndIdleList.Clear();
	m_pRcvIdleList.Clear();
	m_pRcvMsgList.Clear();
	m_pSndMsgList.Clear();
	//重发
	m_ReSendLock.Lock();
	DWORD dwCount = m_ReSendList.GetCount();
	for (DWORD i = 0; i < dwCount; i ++)
	{
		LPUDPMESSAGEITEM pItem = (LPUDPMESSAGEITEM)(m_ReSendList[i].pData);
		delete pItem;
	}
	m_ReSendList.Clear();
	m_ReSendLock.UnLock();
}

WORD CClientUdpSocket::GetInternetPort()
{
	return m_wInternetPort;
}

WORD CClientUdpSocket::GetIntranetPort()
{
	return m_wIntranetPort;
}

void CClientUdpSocket::SetInternetPort(WORD wPort)
{
	m_wInternetPort = wPort;
}

void CClientUdpSocket::InsertResendList(LPUDPMESSAGEITEM pItem)
{
	CGuardLock::COwnerLock guard(m_ReSendLock);
	pItem->tmLastSendTime = GetTickCount();
	if (!m_ReSendList.InsertData(GetMessageSeqId(pItem), pItem))
	{
		PRINTDEBUGLOG(dtInfo, "插入到重发列表中失败(ID: %d) 列表大小：%d", GetMessageSeqId(pItem), m_ReSendList.GetCount());
	}
}


//线程
DWORD WINAPI CClientUdpSocket::CheckRcvDataThread(LPVOID pParam)
{
	CClientUdpSocket *pThis = (CClientUdpSocket *)pParam;
	fd_set readset, writeset;
	timeval val;
	int nCount;
	val.tv_sec = 5;  //5秒
	val.tv_usec = 0;
	sockaddr_in fromAddr;
	int sockaddrLen;
	while(!pThis->m_bTerminated)
	{
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_SET(pThis->m_hSocket, &readset);
		FD_SET(pThis->m_hSocket, &writeset);
		nCount = ::select((int)(pThis->m_hSocket) + 1, &readset, NULL, NULL, &val);
		if (nCount > 0)
		{
			if (pThis->m_bAgent)
			{
				//有数据需要接收
				LPUDPMESSAGEITEM pItem = pThis->m_pRcvIdleList.GetItem();
				int nMsgSize = 1500;
				int nLen = pThis->m_AgentSocket.RecvUdp(pThis->m_hSocket, pItem->strMessage, nMsgSize, pItem->nPeerIp, pItem->nPeerPort);
				if (nLen > 0)
				{
					pItem->nMsgSize = nLen;
					pItem->tmLastSendTime = GetTickCount();
					if (pThis->IsValidPacket(pItem))
					{
						pThis->m_pRcvMsgList.Insert(pItem);
						SetEvent(pThis->m_RcvEvent);
					} else
					{
						pThis->m_pRcvIdleList.Insert(pItem);
						//PRINTDEBUGLOG(dtWarning, "非法UDP消息， 消息长度: %d\n", nLen);
					}
				} else
				{
					pThis->m_pRcvIdleList.Insert(pItem);
					//PRINTDEBUGLOG(dtWarning, "错误的UDP消息，消息长度：%d\n", nLen);
				}
			} else
			{
				if (::FD_ISSET(pThis->m_hSocket, &readset))
				{
					//有数据需要接收
					LPUDPMESSAGEITEM pItem = pThis->m_pRcvIdleList.GetItem();
					if (pItem)
					{
						fromAddr.sin_family = AF_INET;
						sockaddrLen = sizeof(sockaddr);
						int nLen = ::recvfrom(pThis->m_hSocket, pItem->strMessage, MAXUDPSIZE, 0, (struct sockaddr *)&fromAddr, &sockaddrLen);
						if (nLen > 0) //检测是否是合法数据
						{
							pItem->nMsgSize = nLen;
							pItem->nPeerIp = fromAddr.sin_addr.S_un.S_addr;
							pItem->nPeerPort = fromAddr.sin_port;
							pItem->tmLastSendTime = GetTickCount();
							if (pThis->IsValidPacket(pItem))
							{
								pThis->m_pRcvMsgList.Insert(pItem);
								SetEvent(pThis->m_RcvEvent);
							} else
							{
								pThis->m_pRcvIdleList.Insert(pItem);
								PRINTDEBUGLOG(dtWarning, "非法UDP消息， 消息长度: %d\n", nLen);
							}
						} else
						{
							pThis->m_pRcvIdleList.Insert(pItem);
							PRINTDEBUGLOG(dtWarning, "错误的UDP消息，消息长度：%d\n", nLen);
						}
					} else
					{
						PRINTDEBUGLOG(dtError, "UDP 接收线程无法申请到内存");
					}
				}
			}
		}
	}
	return 0;
}

DWORD WINAPI CClientUdpSocket::RcvMsgProcessThread(LPVOID pParam)
{
    CClientUdpSocket *pThis = (CClientUdpSocket *)pParam;
	while(!pThis->m_bTerminated)
	{
		LPUDPMESSAGEITEM pItem;
		while(!pThis->m_bTerminated)
		{
			pItem = pThis->m_pRcvMsgList.GetItem();
			if (pItem)
			{   
				//PRINTDEBUGLOG(dtInfo, "Recv Packet");
				if (pThis->IsAckPacket(pItem)) //是回复信息
				{
					if (!pThis->CheckAckMessage(pItem)) //从重发消息中做检测
					{
						PRINTDEBUGLOG(dtInfo, "invalid ack message, size: %d", pItem->nMsgSize);
						pThis->m_pRcvIdleList.Insert(pItem);
						continue;
					}
				}
				pThis->OnRecvUdpMessage(pItem);
				pThis->m_pRcvIdleList.Insert(pItem);
			} else
				break;
		}        
		WaitForSingleObject(pThis->m_RcvEvent, INFINITE);
	}
	return 0;
}

DWORD WINAPI CClientUdpSocket::ReSendMessageThread(LPVOID pParam)
{
	CClientUdpSocket *pThis = (CClientUdpSocket *)pParam;
	vector<LPUDPMESSAGEITEM> SndFailList; //发送失败列表
	const DWORD dwSleep = 2000; //5秒轮询一次
	DWORD dwPerSleep;
	int dwLeaveSleep; //每次轮询的休息时间
	while(!pThis->m_bTerminated)
	{
		if (pThis->m_ReSendList.GetCount() > 0)
		{
			int nPos = 0;
			DWORD dwCurrKey;
			
			//计算出轮询一次刚好为dwSleep时间
			DWORD dwPer = (pThis->m_ReSendList.GetCount() / PER_CHECK_COUNT) + 1;
			dwPerSleep = (dwSleep) / dwPer;
            dwPerSleep -= 100;
			if ((dwPerSleep < 100) || (dwPerSleep > dwSleep))
				dwPerSleep = 100; 

			dwLeaveSleep = dwSleep;
			DWORD dwStart = GetTickCount();
			for(DWORD j = 0; j < dwPer; j ++ )
			{
				DWORD now = GetTickCount();
				SndFailList.clear();
				LPUDPMESSAGEITEM pCopyItem;
				
				// ========================================
				//                 开始一次轮询
				// ========================================
				pThis->m_ReSendLock.Lock();
				if (nPos > (int)pThis->m_ReSendList.GetCount())
					nPos = pThis->m_ReSendList.FindInsertPos(dwCurrKey);
				if (nPos > (int)pThis->m_ReSendList.GetCount())
					break;
				int i;
				for (i = 0; i < PER_CHECK_COUNT; i ++)
				{
					if ((now > ((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData))->tmLastSendTime) 
						&& ((now - ((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData))->tmLastSendTime) > RESEND_TIME_INTERVAL))
					{
						if (((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData))->nReSendTimes <= 1)
						{
							SndFailList.push_back((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData));
							pThis->m_ReSendList.Delete(nPos);
						}else 
						{
							pCopyItem = pThis->m_pSndIdleList.GetItem();
							if (pCopyItem)
							{
								memcpy(pCopyItem, pThis->m_ReSendList[nPos].pData, sizeof(UDPMESSAGEITEM));
								pThis->m_pSndMsgList.Insert(pCopyItem); //插入到重发列表
								SetEvent(pThis->m_SndEvent);
							}
							((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData))->nReSendTimes --;
							((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData))->tmLastSendTime = ::GetTickCount();
							nPos ++;
						} 						
						
					} else
						nPos ++;
					if ((pThis->m_ReSendList.GetCount() == 0) || (nPos >= (int)pThis->m_ReSendList.GetCount()))
						break;
				} 
				if (i >= PER_CHECK_COUNT)
				   dwCurrKey = pThis->m_ReSendList[nPos].dwKey;
				pThis->m_ReSendLock.UnLock();

				//  ================================================
				//               完成一次轮询
				//  ================================================

				vector<LPUDPMESSAGEITEM>::iterator vctit;
				for (vctit = SndFailList.begin(); vctit != SndFailList.end(); vctit ++)
				{
					pThis->OnSendUdpFail((*vctit));
					pThis->m_pSndIdleList.Insert((*vctit));
				}
				SndFailList.clear();
                if (WaitForSingleObject(pThis->m_hBreak, dwPerSleep) != WAIT_TIMEOUT) //空出一个时间档
					break;
				if (i < PER_CHECK_COUNT)
					break;
			} //end for(DWORD j=....
			
			//休息剩余时间 保证一轮时间为dwSleep
			dwLeaveSleep = GetTickCount() - dwStart;
			dwLeaveSleep = dwSleep - dwLeaveSleep;
			if (dwLeaveSleep > 0)
			{
				if (WaitForSingleObject(pThis->m_hBreak, dwLeaveSleep) != WAIT_TIMEOUT)
					break;
			}
		} else
		{
			if (WaitForSingleObject(pThis->m_hBreak, dwSleep) != WAIT_TIMEOUT) //5秒钟检测一次
				break;
		}
	}
	return 0;
}

DWORD WINAPI CClientUdpSocket::SndMsgProcessThread(LPVOID pParam)
{
	CClientUdpSocket *pThis = (CClientUdpSocket *)pParam;
	LPUDPMESSAGEITEM pItem;
	while(!pThis->m_bTerminated)
	{
		pItem = pThis->m_pSndMsgList.GetItem();
		if (pItem)
		{
			pThis->SendMessageItem(pItem);
			//回收
			pThis->m_pSndIdleList.Insert(pItem);
		}else
		{
			WaitForSingleObject(pThis->m_SndEvent, INFINITE);
		}
	}
	return 0;
}

//应用程序处理函数
void CClientUdpSocket::OnRecvUdpMessage(LPUDPMESSAGEITEM pItem)
{
	if (m_pApp)
		m_pApp->OnRecvUdpMsg(pItem);
}

void CClientUdpSocket::OnSendUdpFail(LPUDPMESSAGEITEM pItem)
{
	if (m_pApp)
		m_pApp->OnSendMessageFail(pItem);
}

#pragma warning(default:4996)
