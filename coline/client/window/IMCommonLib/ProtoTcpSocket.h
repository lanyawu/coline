#ifndef __PROTOTCPSOCKET_H_____
#define __PROTOTCPSOCKET_H_____
#include <Commonlib/types.h>
#include <netlib/AsyncNetIoSocket.h>
#include <commonlib/classes.h>

class IProtoSocketNotify
{
public:
	virtual ~IProtoSocketNotify() {};

	virtual BOOL OnRecvProtocol(const char *szBuf, const int nBufSize) = 0;
	virtual void OnSocketClose(CAsyncNetIoSocket *pSocket, const int nErrorNo) = 0;
	virtual void OnSocketConnect(CAsyncNetIoSocket *pSocket, const int nErrorNo) = 0;
};

class CProtoTcpSocket :public CAsyncNetIoSocket
{
public:
	CProtoTcpSocket(void);
	~CProtoTcpSocket(void);
public:
	virtual void OnReceive(int nErrorCode);
	BOOL SendRawData(const char *pBuf, const int nBufSize);
    BOOL IsConnected();
protected:
	virtual BOOL OnRecvProtocol(const char *szBuf, const int nBufSize);
private:
	BOOL DoFullProtocol();
	BOOL ParserStream();
private:
	int  m_nLeaveSize;
	CMemoryStream m_Stream;
	char *m_pBuffer;
	int m_nBufSize;
	int m_nTotalSize;
};

class CProtoInterfaceSocket :public CProtoTcpSocket
{
public:
	CProtoInterfaceSocket(IProtoSocketNotify *pNotify);
public:
	virtual BOOL OnRecvProtocol(const char *szBuf, const int nBufSize);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode); 
private:
	IProtoSocketNotify *m_pNotify;
};

#endif
