#ifndef __ASYNCTCPSOCKET_H__
#define __ASYNCTCPSOCKET_H__

#include <deque>
#include <Netlib/asock.h>
#include <Commonlib/GuardLock.h>

#define MAX_ASYNCTCP_SND_BUFFER_SIZE 32 //��󻺴��
using namespace std;


//�������ݻ���
typedef struct tagAsyncDataItem
{
	char *lpBuf;
	int nBufLen;
}ADI, *LPADI;

//�첽TCP Socket�����߳�
class CAsyncTcpSocket :public CASocket
{
public:
	CAsyncTcpSocket(void);
	~CAsyncTcpSocket(void);
public:
	//CASocket �����麯��
	virtual void OnSend(int nErrorCode);
    virtual void OnConnect(int nErrorCode);

    //��������
    int SendBuff(char *lpBuff, int &BufLen);
	//��������
	int SendBuff(const char *lpBuff, int BufLen, int nTimeOut);
	//��������
	int RecvBuffer(char *lpBuff, int BufLen, DWORD dwTimeOut);
	BOOL IsSendComplete(); //�����Ƿ��Ѿ��������
	//���ӷ�����
	virtual BOOL ConnectPeer(DWORD dwIp, WORD wPort, DWORD dwTimeOut);
	//
	BOOL GetConnected();
	//
	BOOL IsCanWriteBuff(int nTimeOut);
	//
	BOOL IsCanReadBuff(int nTimeOut);
	//
	void SetConnectStatus(BOOL bConnectStatus);
private:
	static DWORD WINAPI SendDataThread(LPVOID lpParam); //���ݷ����߳�
	void ClearSendBuffer(); //������ͻ�����
protected:
	BOOL m_bTerminated; //�Ƿ��Ѿ���ֹ
	BOOL   m_bConnected; //�Ƿ��Ѿ�����
private:
	deque<LPADI> m_listADI; //�����͵������б�
	CGuardLock  m_lockADI; //������
	HANDLE m_hConnectEvent; //����֪ͨ�¼�
	HANDLE m_hSendEvent; //����֪ͨ�¼�
	HANDLE m_hEmptyEvent;
	HANDLE m_hSndThread; //�����߳�
};

class IAsyncTcpServerApp
{
public:
	//���յ���������
	virtual void OnAccept(CAsyncTcpSocket *pSocket, CNetAddress &addr) = 0;
	//���յ�����
	virtual void OnReceiveBuffer(char *lpBuffer, int nBufSize, CAsyncTcpSocket *pSocket) = 0;
};

class CAsyncTcpSocketServer: public CAsyncTcpSocket
{
public:
	CAsyncTcpSocketServer(IAsyncTcpServerApp *pApp);
	~CAsyncTcpSocketServer();
public:
	void OnAccept(int nErrorCode);
	BOOL Start(WORD wPort);
protected:
	IAsyncTcpServerApp *m_pApp;
};

#endif