#ifndef __SIMPLEGRABBERCB_H__
#define __SIMPLEGRABBERCB_H__

#include "MediaStream.h"
#include <qedit.h>

 static const GUID CLSID_CAudioTransFilter = {0x71927fe1, 0x6b24, 0x4d79, { 0xaa, 0x1b, 0x3c, 0xf0, 0x67, 0x26, 0x20, 0xb5}};

//ʵʱ��Ƶ�ɼ�Grabber
class CSampleGrabberCB: public ISampleGrabberCB
{
public:
	CSampleGrabberCB();
    ~CSampleGrabberCB();

    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    //��ѯ�ӿ�
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);

    // �˺�����BufferCB������ͬ��ֻ��Ҫʵ��һ������
    STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample );

	//�˻ص��ڷ����̵߳��У�������������Ӧ���߳�
	//��win9x���У��ڴ˻ص������в��������ϵͳAPI
	//ÿһ֡ͼ��
     STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize);

	 void SetEncoder(CMediaEncoder *pEncoder)
	 {
		 m_pEncoder = pEncoder;
	 }
private:
	BYTE *m_pBuffer;
	DWORD m_nBuffSize;
	double m_SampleTime;
	CMediaEncoder *m_pEncoder;
};


//��Ƶ��trans filter
class CAudioTransFilter : public CTransInPlaceFilter
{
public:
    CAudioTransFilter(LPUNKNOWN punk, HRESULT *phr );
    ~CAudioTransFilter( );

    HRESULT Transform(IMediaSample *pSample);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT SetMediaType(PIN_DIRECTION pindir, const CMediaType *pMediaType);

public:
	void SetMediaEncoder(CMediaEncoder *Encoder) { m_pEncoder = Encoder; };
    

private:
	CMediaEncoder *m_pEncoder;
	char *m_pBuff;
	DWORD m_nBufSize;
	DWORD m_nSampleSize;
	double m_fSampleTime;
};

#endif