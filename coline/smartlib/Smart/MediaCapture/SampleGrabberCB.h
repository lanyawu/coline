#ifndef __SIMPLEGRABBERCB_H__
#define __SIMPLEGRABBERCB_H__

#include "MediaStream.h"
#include <qedit.h>

 static const GUID CLSID_CAudioTransFilter = {0x71927fe1, 0x6b24, 0x4d79, { 0xaa, 0x1b, 0x3c, 0xf0, 0x67, 0x26, 0x20, 0xb5}};

//实时视频采集Grabber
class CSampleGrabberCB: public ISampleGrabberCB
{
public:
	CSampleGrabberCB();
    ~CSampleGrabberCB();

    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    //查询接口
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);

    // 此函数与BufferCB功能相同，只需要实现一个即可
    STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample );

	//此回调在分离线程当中，并非运行在主应用线程
	//在win9x当中，在此回调函数中不充许调用系统API
	//每一帧图象
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


//音频的trans filter
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