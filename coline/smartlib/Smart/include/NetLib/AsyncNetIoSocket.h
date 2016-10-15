#ifndef __ASYNCNETIOSOCKET_H___
#define __ASYNCNETIOSOCKET_H___
#include <netlib/asock.h>
#include <commonlib/guardlock.h>
#include <commonlib/classes.h>
#include <deque>
//异步IOsocket
class CAsyncNetIoSocket :public CASocket
{
public:
	CAsyncNetIoSocket(void);
	~CAsyncNetIoSocket(void);
public:
	//CASocket implementation
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnClose(int nErrorCode);

	//发送数据
	virtual BOOL Write(const char *pBuf, const int nBufSize);
	//导入未发送完毕的消息
	void ImportWrittingStream(CAsyncNetIoSocket *pSocket);
	LONG AddRef();
	LONG Release();
protected:
	BOOL AppendStream(const char *pBuf, const int nBufSize, BOOL bFront = FALSE);
	//处理协议
	virtual BOOL DoRcvStream(CMemoryStream &Stream);
	//发送剩余数据
	void SendLeaveStream();
protected:
	BOOL   m_bConnected;    //是否已经连接
	CGuardLock m_ListLock; //发送列表锁
	std::deque<CMemoryStream *> m_WrittingList; //待发送列表
private:
	volatile LONG m_lRef; //引用计数
	CMemoryStream m_RcvStream; //收到的数据
	volatile LONG  m_lWritting;     //是否正在写
};

#endif
