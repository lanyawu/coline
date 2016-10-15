#ifndef __ITMSG_UDPPROCESSOR_H__
#define __ITMSG_UDPPROCESSOR_H__

//UDP��Ϣ������
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <winsock2.h>

#include <netlib/MiscClasses.h>
#include <CommonLib/GuardLock.h>
#include <commonlib/MemoryUserList.h>


const DWORD RESEND_TIME_INTERVAL   = 5000; //�ط�ʱ�䣬 �Ժ���Ϊ��λ
const DWORD PER_CHECK_COUNT        = 1000; //ÿ�μ�����Ϣ����
const int MESSAGE_TIMEOUT_INTERVAL = 5000; //�������ٺ������ϢΪ������Ϣ��������


typedef struct tag_WorkParam
{
	LPVOID MsgInstance;
	HANDLE Event;
}THREADWORDPARAM, *LPTHREADWORKPARAM;

class IUdpProcessApp
{
public:
	virtual void OnRecvUdpMsg(LPUDPMESSAGEITEM pItem) = 0;
	virtual void OnSendMessageFail(LPUDPMESSAGEITEM pItem) = 0; //��Ϣ����ʧ��
};


//class CWorkThread;

class CUdpProcessor
{
public:
	CUdpProcessor(IUdpProcessApp *pApp, WORD dwWorkThdCount);
public:
	~CUdpProcessor(void);
public:
	//�麯��
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
public:
	//�ⲿ���ú���
	void SendMsgTo(const char *lpBuff, const int nSize, const int nPeerIp, const int nPort,
		           const int ReSendTimes = 0, const DWORD dwToUserId = 0);
	//
	void SendMsgTo(const char *pBuf1, const int nBuf1Size, const char *pBuf2, const int nBuf2Size, 
		           const int nPeerIp, const int nPort, const int ReSendTimes = 0, const DWORD dwToUserId = 0);
	//�ⲿ���ú���
	void SendMsg(LPUDPMESSAGEITEM pItem);

	//��ֹ
	void Terminate();
	//����UDP����
	bool Start(int nPort);

	static void IntToIpAddress(char *szAddress, DWORD dwIp);
	static char *ByteToString(char *szByte, BYTE bit);

	//������
	DWORD GetSendMsgSize() { return m_pSndMsgList.GetSize(); };
	DWORD GetRcvMsgSize() { return m_pRcvMsgList.GetSize(); };
protected:
	virtual void OnRecvUdpMessage(LPUDPMESSAGEITEM pItem);
	virtual void OnSendUdpFail(LPUDPMESSAGEITEM pItem); //��Ϣ����ʧ��
	virtual BOOL CheckIsAckMessage(const LPUDPMESSAGEITEM pItem); //�Ƿ��ǻظ���Ϣ
	virtual DWORD GetKeyFromMessage(const LPUDPMESSAGEITEM pItem) = 0; //����Ϣ�л�ȡKEY
private:
	//�̺߳���
	static DWORD WINAPI CheckRcvDataThread(LPVOID pParam);   //����Ƿ�����Ϣ����
	static DWORD WINAPI RcvMsgProcessThread(LPVOID pParam); //������Ϣ�����߳�
	static DWORD WINAPI SndMsgProcessThread(LPVOID pParam); //������Ϣ�����߳�
	static DWORD WINAPI ReSendMessageThread(LPVOID pParam); //�ط���Ϣ�߳�
	static DWORD WINAPI _WorkThread(LPVOID pParam); //�������߳�

	void RcvMsgProcess();
	void SndMsgProcess();

	void CheckAckMessage(LPUDPMESSAGEITEM pItem);
	void SendMessageItem(LPUDPMESSAGEITEM pItem);
	void Clear();
	//���뵽�ط��б�
	void InsertResendList(LPUDPMESSAGEITEM pItem);
	DWORD GetCurrSeq();
protected:
	IUdpProcessApp *m_pApp;
	DWORD m_dwCurrSeq; //��ǰ��Ϣ�����
	CPointerDataMap m_ReSendList; //�ط���Ϣ�б� 
    SOCKET m_hSocket;  //��ǰsocket

	CSafeUdpMessageVector m_pRcvIdleList; //���տ����б�
	CSafeUdpMessageVector m_pSndIdleList; //���Ϳ����б�
    CSafeUdpMessageDeque   m_pRcvMsgList;  //δ����Ľ�����Ϣ�б�
	CSafeUdpMessageDeque   m_pSndMsgList;  //�����͵���Ϣ�б�
    
    vector<HANDLE>       m_WorkThreadEvents;
	//��Ӧ������

	CGuardLock  m_LockWorkEvents; //�����߳���
    CGuardLock m_ReSendLock; //�ط���Ϣ�б���
	CGuardLock m_SeqLock; //�����
	sockaddr_in m_fromAddr;
	sockaddr_in m_toAddr;
	WORD m_dwWorkThdCount; //�����̸߳���
	int m_sockaddrLen;
	HANDLE m_RcvThread; //�����߳�
	HANDLE m_SndThread; //�����߳�
	HANDLE m_ReSndThread; //�ط��߳�
	HANDLE m_CheckRcvDataThread; //��������߳�
	HANDLE m_RcvEvent;  //���յ���Ϣ���¼�
	HANDLE m_SndEvent;  //������Ϣ�¼�
	HANDLE m_hSendFail; //��Ϣ����ʧ���¼�
	HANDLE m_hBreak;   //��ֹ�¼�

	bool m_bTerminated;

	//��С�����ݰ���С
	static int UDP_MIN_PACKET_SIZE;
};



#endif