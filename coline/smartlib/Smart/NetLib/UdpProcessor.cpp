#include <time.h>
#include <netlib/asock.h>
#include <Netlib/UdpProcessor.h>
#include <CommonLib/DebugLog.h>

CUdpProcessor::CUdpProcessor(IUdpProcessApp *pApp, WORD dwWorkThdCount):
               m_pApp(pApp),
			   m_bTerminated(false),
			   m_dwWorkThdCount(dwWorkThdCount),
			   m_RcvThread(NULL),
			   m_SndThread(NULL),
			   m_CheckRcvDataThread(NULL),
			   m_dwCurrSeq(0),
			   m_ReSndThread(NULL)
{
	m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_hSocket == INVALID_SOCKET)
	{
		PRINTDEBUGLOG(dtError, "�޷�����socket");
	}
	m_RcvEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_SndEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hBreak = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSendFail = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CUdpProcessor::~CUdpProcessor(void)
{
	Terminate();
	SetEvent(m_hSendFail);
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
	if (m_CheckRcvDataThread)
	{
		WaitForSingleObject(m_CheckRcvDataThread, 5000);
		CloseHandle(m_CheckRcvDataThread);
		m_CheckRcvDataThread = NULL;
	}
	//��������߳�

	CloseHandle(m_RcvEvent);
	CloseHandle(m_SndEvent);
	CloseHandle(m_hBreak);
	CloseHandle(m_hSendFail);
	Clear();
	closesocket(m_hSocket);
}

void CUdpProcessor::Terminate()
{
	m_bTerminated = true;
	SetEvent(m_RcvEvent);
	SetEvent(m_SndEvent);
	SetEvent(m_hBreak);
}

void CUdpProcessor::Clear()
{

}

bool CUdpProcessor::Start(int nPort)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	int len = sizeof(sockaddr_in);
	if (0 == bind(m_hSocket, (sockaddr *)&addr, len))
	{

		//����N�������߳�
		    //�����������߳�
		m_LockWorkEvents.Lock();

		HANDLE hEvent;
		LPTHREADWORKPARAM lpParam;
		if (0 == m_dwWorkThdCount)
		{		
			SYSTEM_INFO si;
			GetSystemInfo(&si);
			m_dwWorkThdCount = (WORD)(si.dwNumberOfProcessors * 2);
		}
		for (WORD i = 0; i < m_dwWorkThdCount; i ++)
		{
			lpParam = new THREADWORDPARAM;
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			lpParam->Event = hEvent;
			lpParam->MsgInstance = this;
			CreateThread(NULL, 0, _WorkThread, (LPVOID)lpParam, NULL, NULL);
			m_WorkThreadEvents.push_back(hEvent);
		}
		m_LockWorkEvents.UnLock();
		PRINTDEBUGLOG(dtInfo, "������ %d ��UDP�����߳�", m_dwWorkThdCount);

		//�����߳�
		m_RcvThread = CreateThread(NULL, 0, RcvMsgProcessThread, this, 0, NULL);
		//�����߳�
		m_SndThread = CreateThread(NULL, 0, SndMsgProcessThread, this, 0, NULL);
		//�ط��߳�
		//m_ReSndThread = CreateThread(NULL, 0, ReSendMessageThread, this, 0, NULL);
		//��������߳�
		m_CheckRcvDataThread = CreateThread(NULL, 0, CheckRcvDataThread, this, 0, NULL);
		return true;
	}
	return false;
}


void CUdpProcessor::OnReceive(int nErrorCode)
{
	if (0 == nErrorCode)
	{
		LPUDPMESSAGEITEM pItem = m_pRcvIdleList.GetItem();
		if (pItem)
		{
			m_fromAddr.sin_family = AF_INET;
			m_sockaddrLen = sizeof(sockaddr);
			int nLen = ::recvfrom(m_hSocket, pItem->strMessage, MAXUDPSIZE, 0, (struct sockaddr *)&m_fromAddr, &m_sockaddrLen);
			if ((nLen > 0) && (nLen >= UDP_MIN_PACKET_SIZE))
			{
				pItem->nMsgSize = nLen;
				pItem->nPeerIp = m_fromAddr.sin_addr.S_un.S_addr;
				pItem->nPeerPort = m_fromAddr.sin_port;
				pItem->tmLastSendTime = GetTickCount();
				m_pRcvMsgList.Insert(pItem);
				SetEvent(m_RcvEvent);
			} else
			{
				m_pRcvIdleList.Insert(pItem);
				//PRINTDEBUGLOG(dtWarning, "�����UDP��Ϣ����Ϣ���ȣ�%d\n", nLen);
			}
		} else
		{
			PRINTDEBUGLOG(dtError, "UDP �����߳��޷����뵽�ڴ�");
		}
	}
}

DWORD CUdpProcessor::GetCurrSeq()
{
	DWORD id = 0;
	m_SeqLock.Lock();
	m_dwCurrSeq ++;
	id = m_dwCurrSeq;
	m_SeqLock.UnLock();
	return id;
}

char * CUdpProcessor::ByteToString(char *szByte, BYTE bit)
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

void CUdpProcessor::IntToIpAddress(char *szAddress, DWORD dwIp)
{
	BYTE *p = (BYTE *)&dwIp;
	for (int i = 0; i < 4; i ++)
	{
		szAddress = ByteToString(szAddress, *p ++);
		*szAddress ++ = '.';
	}
	*(--szAddress) = '\0';
}

void CUdpProcessor::OnSend(int nErrorCode)
{
	if (0 == nErrorCode)
	{
		SetEvent(m_hSendFail);
	}
}

void CUdpProcessor::SendMessageItem(LPUDPMESSAGEITEM pItem)
{
	if (pItem)
	{
		memset(&m_toAddr, 0, sizeof(sockaddr_in));
		m_toAddr.sin_addr.S_un.S_addr = pItem->nPeerIp;
		m_toAddr.sin_family = AF_INET;
		m_toAddr.sin_port = pItem->nPeerPort;
		int toLen = sizeof(struct sockaddr);
		int nSendSize = sendto(m_hSocket, pItem->strMessage, pItem->nMsgSize, 0, (struct sockaddr *)&m_toAddr, toLen);
		char szIp[32] = {0};
		CASocket::IntToIpAddress(szIp, pItem->nPeerIp);
		//PRINTDEBUGLOG(dtInfo, "Send packet, IP:%s, Port:%d", szIp, htons(pItem->nPeerPort));
		if (nSendSize != pItem->nMsgSize) //����ʧ��
		{
			if ((pItem->nPeerIp == 0) || (pItem->nPeerIp == -1) || (pItem->nPeerPort = 0))
			{
				PRINTDEBUGLOG(dtInfo, "send udp message error, ip:%d , port:%d", pItem->nPeerIp, pItem->nPeerPort);
			} else
			{
				//����һ��
				LPUDPMESSAGEITEM pCopyItem = m_pSndIdleList.GetItem();
				if (pCopyItem)
				{
					memcpy(pCopyItem, pItem, sizeof(UDPMESSAGEITEM));
					m_pSndMsgList.InsertFront(pCopyItem); //���뵽ǰ��
					PRINTDEBUGLOG(dtWarning, "����UDP��Ϣʧ�ܣ�SOCKET������Ϣ����: %d �ѷ��ͳ���: %d\n", pItem->nMsgSize, nSendSize);
				} else
					PRINTDEBUGLOG(dtInfo, "send udp error, get copy memory fail");
				timeval val;
				val.tv_sec = 0;
				val.tv_usec = 500;
				fd_set writeset;
				FD_ZERO(&writeset);
				FD_SET(m_hSocket, &writeset);
				int nCount = select((int)m_hSocket + 1, NULL, &writeset, NULL, &val);
			}
		} else
			pItem->tmLastSendTime = GetTickCount();
	}
}

void CUdpProcessor::SendMsg(LPUDPMESSAGEITEM pItem)
{
	LPUDPMESSAGEITEM pCurrItem = m_pSndIdleList.GetItem();
	if (pCurrItem)
	{
		memmove(pCurrItem, pItem, sizeof(UDPMESSAGEITEM));
		if (pCurrItem->nReSendTimes > 0)
		{
			//��һ�ݿ���
			LPUDPMESSAGEITEM pCopyItem = m_pSndIdleList.GetItem();
			if (pCopyItem)
			{
				memcpy(pCopyItem, pCurrItem, sizeof(UDPMESSAGEITEM));
				InsertResendList(pCopyItem);
			} else
				PRINTDEBUGLOG(dtError, "Send Msg �ط��߳��޷����뵽�ڴ�ʹ��Ȩ");
		}
		m_pSndMsgList.Insert(pCurrItem);
		SetEvent(m_SndEvent);
	} else
	{
		PRINTDEBUGLOG(dtError, "�����߳��޷����뵽�ڴ�, SendMsg");
	}
}

//
void CUdpProcessor::SendMsgTo(const char *pBuf1, const int nBuf1Size, const char *pBuf2, const int nBuf2Size, 
		           const int nPeerIp, const int nPort, const int ReSendTimes, const DWORD dwToUserId)
{
	LPUDPMESSAGEITEM pItem = m_pSndIdleList.GetItem();
	if (pItem)
	{
		pItem->nMsgSize = nBuf1Size +nBuf2Size;
		pItem->nPeerIp = nPeerIp;
		pItem->nPeerPort = nPort;
		pItem->dwToUserId = dwToUserId;
		pItem->nReSendTimes = ReSendTimes;
		memmove(pItem->strMessage, pBuf1, nBuf1Size);
		memmove(pItem->strMessage + nBuf1Size, pBuf2, nBuf2Size);
		if (pItem->nReSendTimes > 0)
		{		
			//��һ�ݿ���
			LPUDPMESSAGEITEM pCopyItem = m_pSndIdleList.GetItem();
			if (pCopyItem)
			{
				memcpy(pCopyItem, pItem, sizeof(UDPMESSAGEITEM));
				InsertResendList(pCopyItem);
			} else
				PRINTDEBUGLOG(dtError, "SendMsgTo �ط��߳��޷����뵽�ڴ�ʹ��Ȩ");
		}
		m_pSndMsgList.Insert(pItem);
		SetEvent(m_SndEvent);
	} else
	{
		PRINTDEBUGLOG(dtError, "�����߳��޷����뵽�ڴ�, SendMsgTo");
	}
}

void CUdpProcessor::SendMsgTo(const char *lpBuff, const int nSize, const int nPeerIp, const int nPort,
							  const int ReSendTimes, const DWORD dwToUserId)
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
			//��һ�ݿ���
			LPUDPMESSAGEITEM pCopyItem = m_pSndIdleList.GetItem();
			if (pCopyItem)
			{
				memcpy(pCopyItem, pItem, sizeof(UDPMESSAGEITEM));
				InsertResendList(pCopyItem);
			} else
				PRINTDEBUGLOG(dtError, "SendMsgTo �ط��߳��޷����뵽�ڴ�ʹ��Ȩ");
		}
		m_pSndMsgList.Insert(pItem);
		SetEvent(m_SndEvent);
	} else
	{
		PRINTDEBUGLOG(dtError, "�����߳��޷����뵽�ڴ�, SendMsgTo");
	}
}


void CUdpProcessor::InsertResendList(LPUDPMESSAGEITEM pItem)
{
	/*m_ReSendLock.Lock();
	pItem->tmLastSendTime = GetTickCount();
	if (!m_ReSendList.InsertData(GetKeyFromMessage(pItem), pItem))
	{
		PRINTDEBUGLOG(dtInfo, "���뵽�ط��б���ʧ��(ID: %d) �б��С��%d", GetKeyFromMessage(pItem), m_ReSendList.GetCount());
	}
	m_ReSendLock.UnLock(); */
}

void CUdpProcessor::RcvMsgProcess()
{
	HANDLE hEvent;
	while(!m_bTerminated)
	{
        while (m_WorkThreadEvents.empty())  
			Sleep(100); //û�п��еĴ����߳���ȴ�
		m_LockWorkEvents.Lock();
		hEvent = m_WorkThreadEvents.back();
		m_WorkThreadEvents.pop_back();
		m_LockWorkEvents.UnLock();
		SetEvent(hEvent);
		WaitForSingleObject(m_RcvEvent, INFINITE);
	}
}

void CUdpProcessor::SndMsgProcess()
{
	LPUDPMESSAGEITEM pItem;
	while(!m_bTerminated)
	{
		pItem = m_pSndMsgList.GetItem();
		if (pItem)
		{
			SendMessageItem(pItem);
			//����
			m_pSndIdleList.Insert(pItem);
		}else
		{
			WaitForSingleObject(m_SndEvent, INFINITE);
		}
	}
}

DWORD WINAPI CUdpProcessor::CheckRcvDataThread(LPVOID pParam)
{
	CUdpProcessor *pThis = (CUdpProcessor *)pParam;
	fd_set readset, writeset;
	timeval val;
	int nCount;
	val.tv_sec = 5;  //5��
	val.tv_usec = 0;
	while(!pThis->m_bTerminated)
	{
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_SET(pThis->m_hSocket, &readset);
		FD_SET(pThis->m_hSocket, &writeset);
		nCount = select((int)(pThis->m_hSocket) + 1, &readset, NULL, NULL, &val);
		if (nCount > 0)
		{
			if (::FD_ISSET(pThis->m_hSocket, &readset))
				pThis->OnReceive(0);
		}
	}
	return 0;
}

//�Ƿ��ǻظ���Ϣ
BOOL CUdpProcessor::CheckIsAckMessage(const LPUDPMESSAGEITEM pItem) 
{
	return FALSE;
}

DWORD WINAPI CUdpProcessor::_WorkThread(LPVOID lpParam)
{
	LPTHREADWORKPARAM lpWorkParam = (LPTHREADWORKPARAM)lpParam;
	CUdpProcessor *pThis = (CUdpProcessor *)(lpWorkParam->MsgInstance);
	HANDLE hEvent = lpWorkParam->Event;
	delete lpWorkParam;
	LPUDPMESSAGEITEM pItem;
	while(!pThis->m_bTerminated)
	{
		pItem = pThis->m_pRcvMsgList.GetItem();
		if (pItem)
		{   
			if (pThis->CheckIsAckMessage(pItem)) //�ǻظ���Ϣ
				pThis->CheckAckMessage(pItem); //���ط���Ϣ�������
			//��ʱ��Ϣ������
			if ((GetTickCount() - pItem->tmLastSendTime) < MESSAGE_TIMEOUT_INTERVAL)
				pThis->OnRecvUdpMessage(pItem);
			pThis->m_pRcvIdleList.Insert(pItem);
		} else
		{
			pThis->m_LockWorkEvents.Lock();
			pThis->m_WorkThreadEvents.push_back(hEvent);
			pThis->m_LockWorkEvents.UnLock();
			WaitForSingleObject(hEvent, INFINITE);			
		}
	}
	return 0;
}


//�ط�����߳�
DWORD WINAPI CUdpProcessor::ReSendMessageThread(LPVOID lpParam)
{
	CUdpProcessor *pThis = (CUdpProcessor *)lpParam;
	vector<LPUDPMESSAGEITEM> SndFailList; //����ʧ���б�
	const DWORD dwSleep = 5000; //5����ѯһ��
	DWORD dwPerSleep;
	int dwLeaveSleep; //ÿ����ѯ����Ϣʱ��
	while(!pThis->m_bTerminated)
	{
		//PRINTDEBUGLOG(dtInfo, "Count: %d TIME BEGIN: %d", pThis->m_ReSendList.GetCount(), time(NULL));
		if (pThis->m_ReSendList.GetCount() > 0)
		{
			int nPos = 0;
			DWORD dwCurrKey;
			
			//�������ѯһ�θպ�ΪdwSleepʱ��
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
				//                 ��ʼһ����ѯ
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
								pThis->m_pSndMsgList.Insert(pCopyItem); //���뵽�ط��б�
								SetEvent(pThis->m_SndEvent);
							}
							((LPUDPMESSAGEITEM)(pThis->m_ReSendList[nPos].pData))->nReSendTimes --;
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
				//               ���һ����ѯ
				//  ================================================

				vector<LPUDPMESSAGEITEM>::iterator vctit;
				for (vctit = SndFailList.begin(); vctit != SndFailList.end(); vctit ++)
				{
					pThis->OnSendUdpFail((*vctit));
					pThis->m_pSndIdleList.Insert((*vctit));
				}
				SndFailList.clear();
                if (WaitForSingleObject(pThis->m_hBreak, dwPerSleep) != WAIT_TIMEOUT) //�ճ�һ��ʱ�䵵
					break;
				if (i < PER_CHECK_COUNT)
					break;
			} //end for(DWORD j=....
			
			//��Ϣʣ��ʱ�� ��֤һ��ʱ��ΪdwSleep
			dwLeaveSleep = GetTickCount() - dwStart;
			dwLeaveSleep = dwSleep - dwLeaveSleep;
			if (dwLeaveSleep > 0)
			{
				if (WaitForSingleObject(pThis->m_hBreak, dwLeaveSleep) != WAIT_TIMEOUT)
					break;
			}
		} else
		{
			if (WaitForSingleObject(pThis->m_hBreak, dwSleep) != WAIT_TIMEOUT) //5���Ӽ��һ��
				break;
		}
		PRINTDEBUGLOG(dtInfo, "TIME END: %d", time(NULL));
	}
	return 0;
}

void CUdpProcessor::CheckAckMessage(LPUDPMESSAGEITEM pItem)
{
	m_ReSendLock.Lock();
	int nPos = m_ReSendList.FindDataPos(GetKeyFromMessage(pItem));
	if (nPos >= 0)
	{
		m_pSndIdleList.Insert((LPUDPMESSAGEITEM)m_ReSendList[nPos].pData);
		m_ReSendList.Delete(nPos);
	}
	m_ReSendLock.UnLock();
}

void CUdpProcessor::OnRecvUdpMessage(LPUDPMESSAGEITEM pItem)
{
	if (m_pApp)
		m_pApp->OnRecvUdpMsg(pItem);
}

void CUdpProcessor::OnSendUdpFail(LPUDPMESSAGEITEM pItem)
{
	if (m_pApp)
		m_pApp->OnSendMessageFail(pItem);
}

DWORD WINAPI CUdpProcessor::RcvMsgProcessThread(LPVOID pParam)
{
    CUdpProcessor *pThis = (CUdpProcessor *)pParam;
	pThis->RcvMsgProcess();
	return 0;
}

DWORD WINAPI CUdpProcessor::SndMsgProcessThread(LPVOID pParam)
{
	CUdpProcessor *pThis = (CUdpProcessor *)pParam;
	pThis->SndMsgProcess();
	return 0;
}
