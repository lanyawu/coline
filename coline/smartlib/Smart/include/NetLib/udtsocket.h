#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#define THREAD_WAIT_TIME_OUT_INTERVAL 5000  //�̵߳ȴ���ʱ
#include <NetLib/ASock.h>
#include <Commonlib/Types.h>
#include <Netlib/udt.h>

#define SELECT_HAS_DATA_READ  1  //�����ݿɶ�ȡ
#define SELECT_READ_TIME_OUT  0  //��ʱ
#define SELECT_READ_ERROR     -1 //socket��

//UDT������ʵ���ӿڶ���
class IUDTTransServerApp
{
public:
	virtual void OnAccept(UDTSOCKET u, CNetAddress &addr) = 0;
};

class CUDPTransSocket
{
public:
	CUDPTransSocket(void);
	~CUDPTransSocket(void);

public:
	static void InitUDT();
	static void CleanUpUDT();
	BOOL Bind(short nPort); //�󶨶˿�
	BOOL Listen(); //��������
	int64_t SendFile(char *szFileName);
	int64_t RecvFile(char *szFileName, int64_t size);
    BOOL Connect(int nIp, short nPort, DWORD dwTimeOut);
	void Close();
	//�Ƿ��ܶ�ȡ����
	int CanRead(const DWORD dwTimeOut);
	int SendBuffer(const char *lpBuffer, int nSize);
	int RecvBuffer(char *lpBuffer, int nSize);
    BOOL GetSocketInfo(char *szIp, WORD &wPort);
	BOOL GetPeerSocketInfo(char *szIp, WORD &wPort);
	//�麯��
	virtual UDTSOCKET accept(struct sockaddr* addr, int* addrlen);

	//ͨ����ײ�socket������Ϣ���ƹ��գģԻ���
	int SendRawData(char *lpBuffer, int nSize, DWORD dwIp, WORD wPort);
    
	//�������� byte/s
	int GetSendRate();
	//�����ٶ� byte/s
	int GetRecvRate();
	//���ͳ������
	void ClearRate();
	//��ֵ
	void Attach(UDTSOCKET u); //��ֵһ���µ�UDT ���
	//��ʼ����
	void StartTransfer();
	//��ֹ����
	void Teminate();
	//�ȴ����淢�����
	void WaitSendComplete();
	//��ȡ����
	static UDT::ERRORINFO & GetLastError();
	//
	int GetUdtSocketName(sockaddr *addr, int *nlen);
	//���û�����
	BOOL SetSndBufferSize(DWORD dwBufSize);
	BOOL SetRecvBufferSize(DWORD dwBufSize);
	//�����첽��ʽ
	BOOL SetSyncBlock(bool bSync);
	//��ȡ��������С
	int  GetCurrSndBufferSize();
	int  GetCurrRcvBufferSize();
public:
	DWORD m_dwInternetIP; //��������ɣе�ַ
	WORD m_wInternetPort; //��������˿�
private:
	HANDLE m_hWaitEvent; //�ȴ��¼�
	UDTSOCKET m_hSocket;
};

//UDT��������
class CUDPTransSocketServer
{
public:
	CUDPTransSocketServer(IUDTTransServerApp *pApp);
	~CUDPTransSocketServer();
public:
	void SetInternetIP(DWORD dwIp);
	void SetInternetPort(WORD wPort);
	DWORD GetInternetIP();
	WORD  GetInternetPort();
	BOOL  StartListen(WORD wPort);
	WORD  GetListenPort();
	//ͨ����ײ�socket������Ϣ���ƹ��գģԻ���
	int SendRawData(char *lpBuffer, int nSize, DWORD dwIp, WORD wPort);
	//���÷��ͺͽ��ջ�������С
	BOOL SetSocketBufferSize(int nSndBufSize, int nRcvBufSize);
	//
	BOOL SetSocketSync(bool bSync);
private:
	static DWORD WINAPI QueryAcceptThread(LPVOID lpParam);
private:
	CUDPTransSocket m_UdtSocket;
	IUDTTransServerApp *m_pApp;
	WORD m_wListenPort;
	HANDLE m_hThread; //�߳�
	BOOL m_bTerminated;

};

#endif