#ifndef __IOCOMPLETIONSERVER_H___
#define __IOCOMPLETIONSERVER_H___

#include <netlib/iocompletionport.h>
#include <netlib/iocompletionsocket.h>
#include <vector>
#include <MSWSock.h>

#define DEFAULT_PACKET_SIZE  4096  //Ĭ�����ݰ���С
#define DEFAULT_ACCEPT_TIMES 16

//TCP��������ز���
#define DEFAULT_KEEPALIVETIME		15000
#define DEFAULT_KEEPALIVEINTERVAL   3000

//IOCP������
class CIoCompletionServer
{
public:
	CIoCompletionServer(DWORD dwPktCount, DWORD dwPktSize);
	~CIoCompletionServer(void);
public:
    BOOL Start(const char *szServerIp, WORD wPort);  //��������
	BOOL Stop(); //ֹͣ����
    
	//����һ��SOCKET
    virtual CIoCompletionSocket * AllocateSocket(SOCKET s,sockaddr_in * lpLocalAddr,sockaddr_in * lpRemoteAddr) = 0;
	//��������
	virtual BOOL ParseStream(CPacketStream *pStream, CIoCompletionSocket *pSocket);
    //����һ��SOCKET ��IOCP
	virtual BOOL AssociateDevice(SOCKET h, CIoCompletionSocket *pSocket);
	//����һ��SOCKET ��IOCP �����Ƿ����ӳɹ�
	virtual BOOL AssociateConnectStatus(SOCKET h, CIoCompletionSocket *pSocket);
    //����һ���ڴ�
	CPacketStream *AllocatePacket();
	DWORD GetPacketSize();
	//debug ��ӡ�ڴ�״̬
	void PrintAllocatorStatus();
	//��Ϣ����
	//Post IO����
	void PostIoOperation(CPacketStream::CIoOperationType nType, CIoCompletionSocket *pSocket, CPacketStream *pStream);
	//IO����
	void DoIoOperation(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//��ȡ������Ϣ
	void DoReadRequest(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//��ȡ���
	void DoReadCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//д������
	void DoWriteRequest(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
    //д�����
	void DoWriteCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//���յ���������
	void DoAcceptCompletion(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
    //���ӳɹ�
	void DoSocketConnect(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
	//�������
	void DoConnectComplete(CIoCompletionSocket *pSocket, CPacketStream *pStream, DWORD dwIoSize, DWORD dwLastError);
protected:
	//�¼�
	virtual void OnClientClose(CIoCompletionSocket *pSocket, DWORD dwError); //�ͻ��˹ر�
public:
	LPFN_ACCEPTEX   m_pAcceptEx;							// AcceptEx����ָ�� 
	LPFN_GETACCEPTEXSOCKADDRS m_pGetAcceptExSockaddrs;	    // GetAcceptExSockaddrs����ָ��
private:
	//��ȡ����
	void Read(CIoCompletionSocket *pSocket, CPacketStream *pStream);
	//д������
	void Write(CIoCompletionSocket *pSocket, CPacketStream *pStream);
	//������������
	void PostAcceptEx(SOCKET s, int n); //
    //��ʼ������ָ��
	void GetFunctionPointer();
	//�߳�
	static DWORD WINAPI ListenThread(LPVOID lpParam); //�����߳�
	static DWORD WINAPI WorkThread(LPVOID lpParam); //�����߳�
private:
	CIoCompletionPort m_IOCP;
	//�ڴ������
	CPacketAllocator m_Allocator;
	BOOL m_bTerminated; //�Ƿ���ֹ

	static GUID m_GUIDAcceptEx;
	static GUID m_GUIDGetAcceptExSockAddrs;
	//�����йصı���
	SOCKET m_hListenSocket;
	HANDLE m_hListenEvent;
	HANDLE m_hListen;

	//�����߳����
	std::vector<HANDLE> m_WorkThreadList;

	//debug
	volatile LONG m_nAllocSocket;
	volatile LONG m_nFreeSocket;
};

#endif