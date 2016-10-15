#include <commonlib/Debuglog.h>
#include <commonlib/StringUtils.h>
#include "ASFCapture.h"
#include "dmo.h"

CASFCapture::CASFCapture(void):
             m_pGraph(NULL),
			 m_pMediaCtrl(NULL),
			 m_pMediaEvent(NULL),
			 m_pVideoWin(NULL),
			 m_pBasicAudio(NULL),
			 m_pBasicVideo(NULL),
			 m_pMediaSeeking(NULL),
			 m_pVideoRenderer(NULL),
			 m_pAudioRenderer(NULL),
			 m_pReader(NULL),
			 m_pFileSource(NULL),
			 m_pGrabber(NULL),
			 m_pCapGraph(NULL),
			 m_AudioFilter(NULL),
			 m_bInit(false),
			 m_bPlay(false)
{

}

CASFCapture::~CASFCapture(void)
{
	Stop();
	CloseGraph();
}

BOOL CASFCapture::InitSource(char *pDeviceName)
{
	Stop();
	CloseGraph();
	return InitGraph(pDeviceName);
}

BOOL CASFCapture::Play(HWND hPreview)
{
	if (!m_bPlay && m_bInit)
	{
		HRESULT hr;
		m_hPreview = hPreview;

		// Setup the video window
		hr = m_pVideoWin->put_Owner((OAHWND)m_hPreview);
		if (FAILED(hr))
			return FALSE;
		hr = m_pVideoWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		if (FAILED(hr))
			return FALSE;
		hr = InitVideoWindow(1, 1);
		if (FAILED(hr))
			return FALSE;

		hr = m_pMediaCtrl->Run();
		if (FAILED(hr))
			return FALSE;
		m_bPlay = TRUE;
		return m_bPlay;
	}
	return FALSE;
}

void CASFCapture::AdjustRect()
{
	InitVideoWindow(1, 1, false);
}

HRESULT CASFCapture::InitVideoWindow(int nMultiplier, int nDivider, bool IsDefault)
{
    LONG lHeight, lWidth;
    HRESULT hr = S_OK;
    RECT rect;

    if (!m_pBasicVideo)
        return S_OK;

    // Read the default video size
    hr = m_pBasicVideo->GetVideoSize(&lWidth, &lHeight);
    if (hr == E_NOINTERFACE)
        return S_OK;


	if (IsDefault)
	{
		// Account for requests of normal, half, or double size
		lWidth  = lWidth  * nMultiplier / nDivider;
		lHeight = lHeight * nMultiplier / nDivider;
	    
		int nTitleHeight  = GetSystemMetrics(SM_CYCAPTION);
		int nBorderWidth  = GetSystemMetrics(SM_CXBORDER);
		int nBorderHeight = GetSystemMetrics(SM_CYBORDER);


		SetWindowPos(m_hPreview, NULL, 0, 0, lWidth + 2 * nBorderWidth,
					 lHeight + nTitleHeight + 2*nBorderHeight,
					 SWP_NOMOVE | SWP_NOOWNERZORDER);
	}
	
    GetClientRect(m_hPreview, &rect);  
	m_pBasicVideo->put_SourceWidth(rect.right);
    hr = m_pVideoWin->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom);

    return hr;
}

BOOL CASFCapture::Stop()
{
	m_bPlay = false;		
	HRESULT hr;
	if (m_pMediaCtrl)
	{
		hr = m_pMediaCtrl->Stop();
		if (SUCCEEDED(hr))
		{
			LONGLONG pos;
			hr = m_pMediaSeeking->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                       NULL, AM_SEEKING_NoPositioning);
		}
		return SUCCEEDED(hr);
	}
	return false;
}

void CASFCapture::SetMediaEncoder(CMediaEncoder *Encoder)
{
	m_pEncoder = Encoder;
	m_BufferCB.SetEncoder(m_pEncoder);
}

void CASFCapture::SetSourceQuality(HWND hApp)
{
	//不做操作
}

void CASFCapture::GetMediaType()
{
	//获取参数
	HRESULT hr;
    AM_MEDIA_TYPE mt;
    hr = m_pGrabber->GetConnectedMediaType( &mt );
    if (FAILED(hr))
    {
		PRINTDEBUGLOG(dtInfo, "Could not read the connected media type");
        return;
    }
    
    VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mt.pbFormat;
	//复制当前的位图信息
	if (m_pEncoder)
		m_pEncoder->SetVideoHeaderInfo(vih);
    FreeMediaType(mt);
}

BOOL CASFCapture::InitGraph(char *pFileName)
{
	WCHAR wFile[MAX_PATH] = {0};
    HRESULT hr;

    if (!pFileName)
        return FALSE;


    //转换字符串
	CStringConversion::StringToWideChar(pFileName, wFile, MAX_PATH - 1);
    do
	{
        hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                             IID_IGraphBuilder, (void **)&m_pGraph);
		if (FAILED(hr))
			break;

		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
		     IID_ICaptureGraphBuilder2, (void**)&m_pCapGraph);
		if (FAILED(hr))
			break;

		hr = m_pCapGraph->SetFiltergraph(m_pGraph);
		if (FAILED(hr))
			break;

        hr = m_pGraph->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEvent);
		if (FAILED(hr))
			break;
		
		hr = CoCreateInstance(CLSID_WMAsfReader, NULL, CLSCTX_INPROC_SERVER,
                     IID_IBaseFilter, (void **) &m_pReader);
		if (FAILED(hr))
			break;
		hr = m_pGraph->AddFilter(m_pReader, L"ASF Reader");
		if (FAILED(hr))
			break;
		// Set its source filename
		hr = m_pReader->QueryInterface(IID_IFileSourceFilter, (void **) &m_pFileSource);
		if (FAILED(hr))
			break;
		// Attempt to load this file
		hr = m_pFileSource->Load(wFile, NULL);
		if (FAILED(hr))
			break;
									  
		hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER,
			                  IID_IBaseFilter, (void **)&m_pVideoRenderer);
		if (FAILED(hr))
			break;

		hr = m_pGraph->AddFilter(m_pVideoRenderer, L"Video Renderer");
		if (FAILED(hr))
			break;

		hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
			                  IID_IBaseFilter, (void **)&m_pAudioRenderer);
		if (FAILED(hr))
			break;

		hr = m_pGraph->AddFilter(m_pAudioRenderer, L"Audio Renderer");
		if (FAILED(hr))
			break;

							   
		//抓取数据
        //设备采集回调
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                     IID_ISampleGrabber, (void **)&m_pGrabber);
		if (FAILED(hr)) 
			break;
					//获取Filter
		hr = m_pGrabber->QueryInterface(IID_IBaseFilter, (void **)&m_pCapFilter);
		if (FAILED(hr)) 
			break;

		//设置视频格式
	    // 强制连接到视频 24位 bitmap格式
		CMediaType VideoType;
		VideoType.SetType( &MEDIATYPE_Video );
		VideoType.SetSubtype( &MEDIASUBTYPE_RGB24 );
		hr = m_pGrabber->SetMediaType(&VideoType); 
		if (FAILED(hr)) 
			break;

		hr = m_pGraph->AddFilter(m_pCapFilter, L"Grabber" ); 
		if (FAILED(hr)) 
			break;
	   
		//构建视频链路
		hr = m_pCapGraph->RenderStream(NULL, &MEDIATYPE_Video, m_pReader, m_pCapFilter, m_pVideoRenderer);
		if (FAILED(hr))
			break;

		//插入音频采集Filter
		m_AudioFilter = new CAudioTransFilter(NULL, &hr);
		if (FAILED(hr)) 
			break;
        
		if (m_AudioFilter)
		   ((CAudioTransFilter *)m_AudioFilter)->SetMediaEncoder(m_pEncoder);

		hr = m_pGraph->AddFilter(m_AudioFilter, L"Trans Filter");
		if (FAILED(hr)) 
			break;

		//构建音频链路
		hr = m_pCapGraph->RenderStream(NULL, &MEDIATYPE_Audio, m_pReader, m_AudioFilter, m_pAudioRenderer);
		if (FAILED(hr))
			break;

        // QueryInterface for DirectShow interfaces
        hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaCtrl);
		if (FAILED(hr))
			break;
        hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&m_pMediaSeeking);
		if (FAILED(hr))
			break;

        hr = m_pGraph->QueryInterface(IID_IVideoWindow, (void **)&m_pVideoWin);
		if (FAILED(hr))
			break;
        hr = m_pGraph->QueryInterface(IID_IBasicVideo,  (void **)&m_pBasicVideo);
		if (FAILED(hr))
			break;
				
		hr = m_pGraph->QueryInterface(IID_IBasicAudio, (void **)&m_pBasicAudio);
        if (FAILED(hr))
			break;

		hr = m_pGrabber->SetBufferSamples( FALSE );
  		hr = m_pGrabber->SetOneShot( FALSE );
		hr = m_pGrabber->SetCallback( &m_BufferCB, 1 );
        if (FAILED(hr)) break;	
		
		GetMediaType(); //获取一次MediaType
		m_bInit = TRUE;
		return m_bInit;
    } while(0);

	return FALSE;
}


void CASFCapture::CloseGraph()
{
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
	    
	HRESULT hr;

    if(m_pVideoWin)
    {
        hr = m_pVideoWin->put_Visible(OAFALSE);
        hr = m_pVideoWin->put_Owner(NULL);
    }

    // Disable event callbacks
    if (m_pMediaEvent)
        hr = m_pMediaEvent->SetNotifyWindow((OAHWND)NULL, 0, 0);


    // Release and zero DirectShow interfaces
	SAFE_RELEASE(m_pVideoRenderer);
	SAFE_RELEASE(m_pAudioRenderer);
	SAFE_RELEASE(m_pFileSource);
    SAFE_RELEASE(m_pReader);
	SAFE_RELEASE(m_pGrabber);

    SAFE_RELEASE(m_pMediaEvent);
    SAFE_RELEASE(m_pMediaSeeking);
    SAFE_RELEASE(m_pMediaCtrl);
    SAFE_RELEASE(m_pBasicAudio);
    SAFE_RELEASE(m_pBasicVideo);
    SAFE_RELEASE(m_pVideoWin);
    SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pCapGraph);
	m_bInit = false;
}

