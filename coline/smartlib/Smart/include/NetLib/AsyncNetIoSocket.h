#ifndef __ASYNCNETIOSOCKET_H___
#define __ASYNCNETIOSOCKET_H___
#include <netlib/asock.h>
#include <commonlib/guardlock.h>
#include <commonlib/classes.h>
#include <deque>
//�첽IOsocket
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

	//��������
	virtual BOOL Write(const char *pBuf, const int nBufSize);
	//����δ������ϵ���Ϣ
	void ImportWrittingStream(CAsyncNetIoSocket *pSocket);
	LONG AddRef();
	LONG Release();
protected:
	BOOL AppendStream(const char *pBuf, const int nBufSize, BOOL bFront = FALSE);
	//����Э��
	virtual BOOL DoRcvStream(CMemoryStream &Stream);
	//����ʣ������
	void SendLeaveStream();
protected:
	BOOL   m_bConnected;    //�Ƿ��Ѿ�����
	CGuardLock m_ListLock; //�����б���
	std::deque<CMemoryStream *> m_WrittingList; //�������б�
private:
	volatile LONG m_lRef; //���ü���
	CMemoryStream m_RcvStream; //�յ�������
	volatile LONG  m_lWritting;     //�Ƿ�����д
};

#endif
