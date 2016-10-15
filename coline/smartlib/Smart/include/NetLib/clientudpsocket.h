#ifndef __CLIENTUDPSOCKET_H__
#define __CLIENTUDPSOCKET_H__
#include <winsock2.h>
#include <commonlib/types.h>
#include <NetLib/MiscClasses.h>
#include <commonlib/MemoryUserList.h>
#include <NetLib/socks5agentsocket.h>

const DWORD RESEND_TIME_INTERVAL   = 3000; //�ط�ʱ�䣬 �Ժ���Ϊ��λ
const DWORD PER_CHECK_COUNT        = 1000; //ÿ�μ�����Ϣ����
const int MESSAGE_TIMEOUT_INTERVAL = 5000; //�������ٺ������ϢΪ������Ϣ��������


class IClientUdpProcessApp
{
public:
	virtual void OnRecvUdpMsg(LPUDPMESSAGEITEM pItem) = 0;
	virtual void OnSendMessageFail(LPUDPMESSAGEITEM pItem) = 0; //��Ϣ����ʧ��
};
class CClientUdpSocket
{
public:
	CClientUdpSocket(IClientUdpProcessApp *pApp);
public:
	~CClientUdpSocket(void);
public:
	//�ⲿ���ú���
	void SendMsgTo(char *lpBuff, int nSize, int nPeerIp, int nPort, int ReSendTimes = 0, DWORD dwToUserId = 0);
	
	//�ⲿ���ú���
	void SendMsg(LPUDPMESSAGEITEM pItem);

	//��ֹ
	void Terminate();
	//��ȡ��SOCKET IP��ַ
	DWORD GetSocketIP();

	//����UDP����
	bool Start(int nPort);
	//�ڲ��˿�
	WORD GetIntranetPort();
	WORD GetInternetPort();
	void SetInternetPort(WORD wPort);
	DWORD GetCurrSeq();
	//������д�����Ϣ
	void Clear();
	//���ô����������Ϣ
	BOOL SetAgentAddr(const char *szAgentIp, const WORD wPort, const char *szAgentName, 
		              const char *szAgentPwd);
private:
	BOOL GetSocketName(char *szAddress, WORD &wPort);
protected:
	//����ʵ�ֵ��麯��
	virtual DWORD GetMessageSeqId(LPUDPMESSAGEITEM pItem) { return 0; } //����Ϣ�л�ȡseq id ��
	virtual BOOL  IsAckPacket(LPUDPMESSAGEITEM pItem) { return FALSE; }; //����Ƿ���һ���ظ���
	virtual void OnRecvUdpMessage(LPUDPMESSAGEITEM pItem);
	virtual void OnSendUdpFail(LPUDPMESSAGEITEM pItem); //��Ϣ����ʧ��	
	
	//���뵽�ط��б�
	void InsertResendList(LPUDPMESSAGEITEM pItem);	
	//����Ƿ��ǺϷ�����
	virtual BOOL IsValidPacket(LPUDPMESSAGEITEM pItem) { return TRUE;  };
private:
	//�̺߳���
	static DWORD WINAPI CheckRcvDataThread(LPVOID pParam);   //����Ƿ�����Ϣ����
	static DWORD WINAPI RcvMsgProcessThread(LPVOID pParam); //������Ϣ�����߳�
	static DWORD WINAPI SndMsgProcessThread(LPVOID pParam); //������Ϣ�����߳�
	static DWORD WINAPI ReSendMessageThread(LPVOID pParam); //�ط���Ϣ�߳�

	BOOL CheckAckMessage(LPUDPMESSAGEITEM pItem);
	void SendMessageItem(LPUDPMESSAGEITEM pItem);
protected:
	//��Ϣ�б�
	CSafeUdpMessageVector  m_pRcvIdleList; //���տ����б�
	CSafeUdpMessageVector  m_pSndIdleList; //���Ϳ����б�
    CSafeUdpMessageDeque   m_pRcvMsgList;  //δ����Ľ�����Ϣ�б�
	CSafeUdpMessageDeque   m_pSndMsgList;  //�����͵���Ϣ�б�	
	
	//�¼��б�
	HANDLE m_RcvEvent;
	HANDLE m_hBreak;	
	HANDLE m_SndEvent;
private:
	IClientUdpProcessApp *m_pApp;
	CSocks5AgentSocket m_AgentSocket; //����
	BOOL m_bAgent; //�Ƿ���ô���
	SOCKET m_hSocket;
	CPointerDataMap m_ReSendList; //�ط���Ϣ�б� 
	CGuardLock m_ReSendLock;
	sockaddr_in m_toAddr;

	//�߳��б�
	HANDLE m_RcvThread;
	HANDLE m_SndThread;
	HANDLE m_ReSndThread;
	HANDLE m_CheckRcvDataThread;
    
	//�����
	CGuardLock m_SeqLock;

	DWORD m_dwSeq;
	WORD m_wIntranetPort; //�ڲ��˿�
	WORD m_wInternetPort; //�ⲿ�˿�
	bool m_bTerminated;
};

#endif