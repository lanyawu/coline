#ifndef __PACKETSTREAM_H___
#define __PACKETSTREAM_H___

#include <commonlib/basestream.h> 
#include <commonlib/guardlock.h>
#include <vector>
#include <Winsock2.h>

#define DEBUG_PRINT_ALLOC_COUNT 

class CPacketStream;

//���ݰ��ڴ������
class CPacketAllocator
{
public:
	CPacketAllocator(DWORD dwMaxBufCount, DWORD dwPacketSize);
	~CPacketAllocator();
public:
	CPacketStream *AllocPacket();
	//����
	void RecyclePacket(CPacketStream *pPacket);
	//�����ڴ�
	void Flush();
	//��ȡ���ݰ���С
	DWORD GetPacketSize();

	//debug
	void GetAllocStatus(int &nAllocCount, int &nReleaseCount, int &nListCount, int &nNewCount);
private:
	std::vector<CPacketStream *> m_FreeList;
	CGuardLock m_Lock;
	DWORD m_dwMaxBufCount;
	DWORD m_dwPacketSize;
#ifdef DEBUG_PRINT_ALLOC_COUNT
	//debug
	volatile LONG m_nAllocCount;   //�Ѿ�����ĸ���
	volatile LONG m_nReleaseCount; //�Ѿ��ͷŵĸ���
	volatile LONG m_nNewCount;  //New 
#endif
};

typedef std::vector<CPacketStream *> CPacketList;

class CPacketStream :public CBaseStream
{
public:
	explicit CPacketStream(CPacketAllocator *pAlloctor, DWORD dwPacketSize);
	~CPacketStream(void);
	//IO��������
	enum CIoOperationType
	{
		IO_TYPE_READ_REQUEST,   //������
		IO_TYPE_READ_COMPLETE,  //�����
		IO_TYPE_WRITE_REQUEST,  //д����
		IO_TYPE_WRITE_COMPLETE, //д���
		IO_TYPE_ACCEPT_COMPLETE, //��������
		IO_TYPE_CONNECT_REQUEST, //��������
		IO_TYPE_CONNECT_COMPLETE, //�������
		IO_TYPE_CLOSE   //�ر�
	};
public:
	LONG AddRef();
	LONG Release();
	LONG GetRef();

	int Read(char *pBuff, int nSize);  //��ȡ����
	int Write(const char *pBuff, int nSize); //׷��д������

	void SetIoOperation(CPacketStream::CIoOperationType nType);
	CPacketStream::CIoOperationType GetIoOperator();
	int GetLeaveSize(); //��ȡʣ��Ĵ�С
	void SetupRead(); //���ö����� ���WSABUF�ṹ
    void SetupWrite(); //����д���� ���WSABUF�ṹ
	void SetupAccept(); //���ý������Ӳ��� ���WSABUF�ṹ
	WSABUF *GetWSABUF() const; //
	void Initialize(); //��ʼ��
	void SetOwnerList(CPacketList *pList); //���������ĸ�����
	void RemoveFromList();  //�Ƴ��б�
	void RemoveData(int nSize); //�Ƴ���������
	void SetSocket(SOCKET h);
	void SetListenSocket(SOCKET h);
	SOCKET GetSocket();
	SOCKET GetListenSocket();
	char *GetData() const;
	void PrintStream(); //��ӡ�ڴ�����
public:
	OVERLAPPED m_Overlapped;
private:
    char *m_pBuff;
	CPacketAllocator *m_pAllocator; //�ڴ������
	WSABUF m_WsaBuf;
	long   m_lRef;
	CIoOperationType m_IoType;
	CPacketList *m_pOwnerList;

	//����״���йز���
	SOCKET m_hSocket; //����SOCKET
	SOCKET m_hListen; //����SOCKET
};



#endif