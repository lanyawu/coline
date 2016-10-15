#ifndef __IOCOMPLETIONSOCKET_H____
#define __IOCOMPLETIONSOCKET_H____

#include <commonlib/types.h>
#include <commonlib/guardlock.h>
#include <netlib/packetstream.h>
#include <queue>
#include <winsock.h>

#define IO_COMPLETION_ACTIVE_TIME_INTERVAL  15000  //15�볬ʱ

class CIoCompletionServer;
class CIoCompletionSocket
{
public:
	CIoCompletionSocket(CIoCompletionServer *pServer);
	virtual ~CIoCompletionSocket(void);
public:
	friend class CIoCompletionServer;
	//�麯��
	virtual BOOL ParseStream(CPacketStream *pStream);
	void Attach(SOCKET h);
	void CloseByPeer(); //���Է��ر�
	BOOL IsValid(); //���SOCKET �Ƿ�Ϸ�
	BOOL Read(CPacketStream *pPacket = NULL); //��ȡ����
	BOOL ReadConnectStatus(CPacketStream *pPacket = NULL); //��ȡ����״̬
    virtual void WriteCompleted(); //д��δ���͵�����
	BOOL WriteBuff(const char *pBuf, int nSize); //д������
	void ClearPackets(); //����������ݰ�
	//���û���
	void SetSocketBufferSize(const int nMaxBufSize = 0, const int nMinBufSize = 0);
	//��ȡsocket Peer IP and Port
	BOOL GetPeerName(char *szPeerIp, WORD &wPeerPort);
	//������
	LONG  AddRef();
	LONG  Release();
	LONG  GetRef();
	SOCKET GetSocket();
    BOOL SocketIsTimeout(); //�����Ƿ�ʱ
    DWORD GetWritePendingSize(); //��ȡ�����Ͷ����еĸ���
protected:
	BOOL ConnectSvr(const char *szIp, const WORD wPort); //���ӷ�����
	
 
protected:
	CIoCompletionServer *m_pServer;
private:
	volatile LONG m_lWritting; //�ж��Ƿ�����д�����
	volatile LONG m_lRef; //������
	BOOL m_bPauseRead; //�Ƿ�����ͣ״̬
	DWORD m_dwActiveTime; //���ʱ��
	CGuardLock m_wLock; //д����
	CGuardLock m_Lock; //������������
	std::deque<CPacketStream *> m_sqList; //�����Ͷ���
	SOCKET m_hSocket;
};

#endif