#ifndef __PACKETSTREAM_H___
#define __PACKETSTREAM_H___

#include <commonlib/basestream.h> 
#include <commonlib/guardlock.h>
#include <vector>
#include <Winsock2.h>

#define DEBUG_PRINT_ALLOC_COUNT 

class CPacketStream;

//数据包内存分配器
class CPacketAllocator
{
public:
	CPacketAllocator(DWORD dwMaxBufCount, DWORD dwPacketSize);
	~CPacketAllocator();
public:
	CPacketStream *AllocPacket();
	//回收
	void RecyclePacket(CPacketStream *pPacket);
	//更新内存
	void Flush();
	//获取数据包大小
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
	volatile LONG m_nAllocCount;   //已经分配的个数
	volatile LONG m_nReleaseCount; //已经释放的个数
	volatile LONG m_nNewCount;  //New 
#endif
};

typedef std::vector<CPacketStream *> CPacketList;

class CPacketStream :public CBaseStream
{
public:
	explicit CPacketStream(CPacketAllocator *pAlloctor, DWORD dwPacketSize);
	~CPacketStream(void);
	//IO操作类型
	enum CIoOperationType
	{
		IO_TYPE_READ_REQUEST,   //读请求
		IO_TYPE_READ_COMPLETE,  //读完成
		IO_TYPE_WRITE_REQUEST,  //写请求
		IO_TYPE_WRITE_COMPLETE, //写完成
		IO_TYPE_ACCEPT_COMPLETE, //接收连接
		IO_TYPE_CONNECT_REQUEST, //连接请求
		IO_TYPE_CONNECT_COMPLETE, //连接完成
		IO_TYPE_CLOSE   //关闭
	};
public:
	LONG AddRef();
	LONG Release();
	LONG GetRef();

	int Read(char *pBuff, int nSize);  //读取数据
	int Write(const char *pBuff, int nSize); //追加写入数据

	void SetIoOperation(CPacketStream::CIoOperationType nType);
	CPacketStream::CIoOperationType GetIoOperator();
	int GetLeaveSize(); //获取剩余的大小
	void SetupRead(); //设置读操作 填充WSABUF结构
    void SetupWrite(); //设置写操作 填充WSABUF结构
	void SetupAccept(); //设置接受连接操作 填充WSABUF结构
	WSABUF *GetWSABUF() const; //
	void Initialize(); //初始化
	void SetOwnerList(CPacketList *pList); //设置属于哪个宿主
	void RemoveFromList();  //移出列表
	void RemoveData(int nSize); //移除多少数据
	void SetSocket(SOCKET h);
	void SetListenSocket(SOCKET h);
	SOCKET GetSocket();
	SOCKET GetListenSocket();
	char *GetData() const;
	void PrintStream(); //打印内存数据
public:
	OVERLAPPED m_Overlapped;
private:
    char *m_pBuff;
	CPacketAllocator *m_pAllocator; //内存分配器
	WSABUF m_WsaBuf;
	long   m_lRef;
	CIoOperationType m_IoType;
	CPacketList *m_pOwnerList;

	//网络状况有关参数
	SOCKET m_hSocket; //本身SOCKET
	SOCKET m_hListen; //监听SOCKET
};



#endif