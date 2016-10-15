#include <commonlib/Debuglog.h>
#include <commonlib/StringUtils.h>
#include "WMVCapture.h"

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define WM_GRAPHWMVNOTIFY  WM_USER+13

CWMVCapture::CWMVCapture(void):
             m_bPlay(false),
			 m_hPreview(NULL),
			 m_bInit(false)
{
}

CWMVCapture::~CWMVCapture(void)
{
}

BOOL CWMVCapture::InitSource(char *DeviceName)
{
	return InitGraph(DeviceName);
}

BOOL CWMVCapture::Play(HWND hPreview)
{
	if (!m_bPlay)
	{
		m_hPreview = hPreview;
		//获取MediaType
        GetMediaType();
		HRESULT hr;
		do
		{
			hr = m_pMediaCtrl->Run();
			if (FAILED(hr)) break;
			m_bPlay = TRUE;
			SetFocus(hPreview);
			return TRUE;
		}while(0);
	}
	return FALSE;
}

void CWMVCapture::SetMediaEncoder(CMediaEncoder *Encoder)
{
	m_pEncoder = Encoder;
	m_BufferCB.SetEncoder(m_pEncoder);
}

BOOL CWMVCapture::Stop()
{
    HRESULT hr;
    
    if (m_pMediaCtrl && m_pMediaSeeking && m_bPlay)
	{
        LONGLONG pos = 0;
        hr = m_pMediaCtrl->Stop();
        m_bPlay = FALSE;

        hr = m_pMediaSeeking->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                 NULL, AM_SEEKING_NoPositioning);

        hr = m_pMediaCtrl->Pause();
		return SUCCEEDED(hr);
	}
	return FALSE;
}

void CWMVCapture::SetSourceQuality(HWND hApp)
{
	//不做处理
}

void CWMVCapture::GetMediaType()
{
	HRESULT hr;
	AM_MEDIA_TYPE Type;
	memset( &Type, 0, sizeof( Type ) );


	hr = m_pGrabber->GetConnectedMediaType(&Type);
	if( FAILED( hr ) ) return; 

	if( Type.majortype == MEDIATYPE_Video)
	{
		VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)Type.pbFormat;
		//复制当前的位图信息
		if (m_pEncoder)
			m_pEncoder->SetVideoHeaderInfo(vih);

	}
	

	FreeMediaType( Type );
}

void CWMVCapture::SetOutputStream()
{
	HRESULT hr;
    long Streams;
    hr = m_pDet->get_OutputStreams( &Streams );
    if( FAILED( hr )) return;

    for( int i = 0 ; i < Streams ; i++ )
    {
        bool b = false;

        AM_MEDIA_TYPE Type;
        memset( &Type, 0, sizeof( Type ) );

        hr = m_pDet->put_CurrentStream( i );
        if( FAILED( hr ) )  return;

        hr = m_pDet->get_StreamMediaType( &Type );
        if( FAILED( hr ) ) return; 

		if (Type.majortype == MEDIATYPE_Video)
		{
			b = true;
		}  


        FreeMediaType( Type );

        if( !b ) 
            continue;
        break;
    }
}

bool CWMVCapture::InitGraph(char *DeviceName)
{
	wchar_t wFile[MAX_PATH] = {0};
    HRESULT hr;
    
    if (!DeviceName)
        return false;

    // Convert filename to wide character string
	CStringConversion::StringToWideChar(DeviceName, wFile, MAX_PATH - 1);

    // Get the interface for DirectShow's GraphBuilder
    hr = CoCreateInstance( CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, 
                           IID_IMediaDet, (void**) &m_pDet );
	if (FAILED(hr)) return false;

    if(SUCCEEDED(hr))
    {
		do 
		{
			hr = m_pDet->put_Filename(wFile);
			if (FAILED(hr)) break;
		    SetOutputStream();
		    hr = m_pDet->EnterBitmapGrabMode(0.0f);
			if (FAILED(hr)) break;

			hr = m_pDet->GetSampleGrabber(&m_pGrabber);
			if (FAILED(hr)) break;
			
			//设置视频格式
			    // 强制连接到视频 24位 bitmap格式
			CMediaType VideoType;
			VideoType.SetType( &MEDIATYPE_Video );
			VideoType.SetSubtype( &MEDIASUBTYPE_RGB24 );
			VideoType.SetTemporalCompression(true);
			hr = m_pGrabber->SetMediaType(&VideoType); 
			if (FAILED(hr)) break;

			GetMediaType();
			//查询Filter
			hr = m_pGrabber->QueryInterface(IID_IBaseFilter, (void **)&m_pFilter);
			if (FAILED(hr)) break;

			hr = m_pGrabber->SetCallback(&m_BufferCB, 1);
			if (FAILED(hr)) break;

			hr = m_pGrabber->SetOneShot(FALSE);
			if (FAILED(hr)) break;

			hr = m_pGrabber->SetBufferSamples(FALSE);
			if (FAILED(hr)) break;

			FILTER_INFO fi;
			memset( &fi, 0, sizeof( fi ) );

			hr = m_pFilter->QueryFilterInfo( &fi );
			if( FAILED( hr )) break; 

			if( fi.pGraph ) 
				fi.pGraph->Release( );
			m_pGraph = fi.pGraph;

			// 查询directshow 接口
			hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaCtrl);
			if (FAILED(hr)) break;
			hr = m_pGraph->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEvent);
			if (FAILED(hr)) break;
			hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&m_pMediaSeeking);
			if (FAILED(hr)) break;
			hr = m_pGraph->QueryInterface(IID_IMediaPosition, (void **)&m_pMediaPos);
            if (FAILED(hr)) break;

			// 查询Video接口
			hr = m_pGraph->QueryInterface(IID_IVideoWindow, (void **)&m_pVideoWin);
			if (FAILED(hr)) break;
			hr = m_pGraph->QueryInterface(IID_IBasicVideo, (void **)&m_pBasicVideo);
			if (FAILED(hr)) break;

			// 查询音频接口
			hr = m_pGraph->QueryInterface(IID_IBasicAudio, (void **)&m_pBasicAudio);
			if (FAILED(hr)) break;
            
			REFERENCE_TIME Start = 0;
			REFERENCE_TIME Duration = 0;

			hr = m_pMediaSeeking->GetDuration( &Duration );
			if( FAILED( hr ))  break;

			hr = m_pMediaSeeking->SetPositions( &Start,  AM_SEEKING_AbsolutePositioning, 
										 &Duration, AM_SEEKING_AbsolutePositioning );
			if (FAILED(hr)) break;

			//到此为止，初始化正确
			return true;
		}while(0);
    }
    return false;
}

void CWMVCapture::CloseGraph()
{

}
