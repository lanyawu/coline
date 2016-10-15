#include "SampleGrabberCB.h"

CSampleGrabberCB::CSampleGrabberCB():
                  m_pBuffer(NULL),
				  m_pEncoder(NULL),
				  m_nBuffSize(0)
{
}

CSampleGrabberCB::~CSampleGrabberCB()
{
	delete m_pBuffer;
	m_pBuffer = NULL;
}

STDMETHODIMP CSampleGrabberCB::QueryInterface(const IID &riid, void **ppv)
{
	    if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
        {
            *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
            return NOERROR;
        }    
        return E_NOINTERFACE;

}

STDMETHODIMP CSampleGrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
   return 0;
}


STDMETHODIMP CSampleGrabberCB::BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
{
	if (m_pEncoder)
	{
		if (!pBuffer)
		return E_POINTER;

		if( m_nBuffSize < lBufferSize )
		{
			delete [] m_pBuffer;
			m_pBuffer = NULL;
			m_nBuffSize = 0;
		}

		m_SampleTime = dblSampleTime;

		// 分配新的内存
		if (!m_pBuffer)
		{
			m_pBuffer = new BYTE[lBufferSize];
			m_nBuffSize = lBufferSize;
		}

		if( !m_pBuffer )
		{
			m_nBuffSize = 0;
			return E_OUTOFMEMORY;
		}
		memcpy(m_pBuffer, pBuffer, lBufferSize);
		//转成 1/10微秒基本单位 
		m_pEncoder->OnRecvVideoBuff(m_SampleTime * 10000000, (char *)m_pBuffer, lBufferSize);
	}
    return 0;
}


//音频Filter实现
CAudioTransFilter::CAudioTransFilter(LPUNKNOWN punk, HRESULT *phr):
                   CTransInPlaceFilter(TEXT("CAudioTransFilter"), punk, CLSID_CAudioTransFilter, phr),
				   m_pBuff(NULL),
				   m_nSampleSize(0),
				   m_nBufSize(0)


{

}

CAudioTransFilter::~CAudioTransFilter()
{
	delete m_pBuff;
	m_pBuff = NULL;
}

HRESULT CAudioTransFilter::SetMediaType( PIN_DIRECTION pindir, const CMediaType *pMediaType)
{
    CheckPointer(pMediaType, E_POINTER);

    CWAVEFORMATEX * pWave = (CWAVEFORMATEX*) pMediaType->pbFormat;
    CheckPointer(pWave, E_UNEXPECTED);
	if ((pMediaType->majortype == MEDIATYPE_Audio) && (pMediaType->formattype == FORMAT_WaveFormatEx))
	{
		//检测格式
		if (pWave->wFormatTag == WAVE_FORMAT_PCM)
		{
			int nSize = pMediaType->FormatLength();
			if (m_pEncoder)
				m_pEncoder->SetAudioHeaderInfo(pWave); //设置音频格式
		}
	}
    return CTransInPlaceFilter::SetMediaType( pindir, pMediaType );
}

HRESULT CAudioTransFilter::CheckInputType(const CMediaType *mtIn)
{
    CheckPointer(mtIn,E_POINTER);
    //检测参数
	if ((mtIn->majortype == MEDIATYPE_Audio) &&(mtIn->formattype == FORMAT_WaveFormatEx))
	{
		WAVEFORMATEX *pWave= (WAVEFORMATEX *)mtIn->pbFormat;
		if (pWave->wFormatTag == WAVE_FORMAT_PCM)
		    return S_OK;
	}
    return S_FALSE;
}

HRESULT CAudioTransFilter::Transform(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);
    
	REFERENCE_TIME nStart;
	REFERENCE_TIME nStop;
	pSample->GetTime(&nStart, &nStop);

    //拷贝数据
	m_nSampleSize = pSample->GetSize();
	if (!m_pBuff)
	{
		m_pBuff = new char[m_nSampleSize];
		m_nBufSize = m_nSampleSize;
	}
	if (m_nBufSize < m_nSampleSize)
	{
		if (m_pBuff)
			delete []m_pBuff;
		m_pBuff = new char[m_nSampleSize];
		m_nBufSize = m_nSampleSize;
	}
	if (m_pBuff)
	{
		BYTE *p = 0;
		HRESULT hr;
		hr = pSample->GetPointer(&p);
		if (SUCCEEDED(hr) && p)
		{
			memmove(m_pBuff, p, m_nSampleSize);
			m_pEncoder->OnRecvAudioBuff(nStart, m_pBuff, m_nSampleSize);
		}
	} else
	{
		m_nBufSize = 0;
		return E_OUTOFMEMORY;
	}
    return NOERROR;
}

