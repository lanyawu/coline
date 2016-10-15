#include <commonlib/Debuglog.h>
#include <commonlib/StringUtils.h>
#include "AVICapture.h"


const CLSID CLSID_AviStreamReader = {0xD3588AB0, 0x0781, 0x11CE, 0xB0, 0x3A, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70};
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define WM_GRAPHNOTIFY  WM_USER+13

CAVICapture::CAVICapture(void):
             m_bPlay(false),
			 m_bInit(false),
	         m_pEncoder(NULL),
             m_pGrapBuilder(NULL),
             m_pMediaCtrl(NULL),
             m_pMediaSeeking(NULL),
             m_pVideoWin(NULL),
             m_pBasicAudio(NULL),
             m_pBasicVideo(NULL),
			 m_pGrabber(NULL),
			 m_Filter(NULL),
			 m_pCapGB(NULL),
			 m_pSrc(NULL),
			 m_pAviReader(NULL),
			 m_pVideoRenderer(NULL),
			 m_pAudioRenderer(NULL),
			 m_AudioFilter(NULL),
			 m_hPreview(NULL)
{

}

CAVICapture::~CAVICapture(void)
{
	Stop();
    CloseGraph();
}

BOOL CAVICapture::InitSource(char *DeviceName)
{
	return InitGraph(DeviceName);
}

BOOL CAVICapture::Play(HWND hPreview)
{
	if (!m_bPlay && m_bInit)
	{
		m_hPreview = hPreview;
		//获取MediaType
        GetMediaType();
		HRESULT hr;
		do
		{
			if (!m_pVideoWin) break;
            hr = m_pVideoWin->put_Owner((OAHWND)m_hPreview);
			if (FAILED(hr)) break;
            hr = m_pVideoWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			if (FAILED(hr)) break;

			InitVideoWindow(1, 1);
            if (FAILED(hr)) break; 
			ShowWindow(hPreview, SW_SHOWNORMAL);
			UpdateWindow(hPreview);
			SetForegroundWindow(hPreview);
        
			m_pMediaCtrl->Run();
			SetFocus(hPreview);

			m_bPlay = true;
			return true;
		}while(0);
	}
	return false;
}

void CAVICapture::AdjustRect()
{
	InitVideoWindow(1, 1, false);
}

HRESULT CAVICapture::InitVideoWindow(int nMultiplier, int nDivider, bool IsDefault)
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
					 lHeight + nTitleHeight + 2 * nBorderHeight,
					 SWP_NOMOVE | SWP_NOOWNERZORDER);
	}


    GetClientRect(m_hPreview, &rect);	
	m_pBasicVideo->put_SourceWidth(rect.right);
    hr = m_pVideoWin->SetWindowPosition(rect.left, rect.top, rect.right, rect.bottom);

    return hr;
}
void CAVICapture::SetMediaEncoder(CMediaEncoder *Encoder)
{
	m_pEncoder = Encoder;
	m_BufferCB.SetEncoder(m_pEncoder);
}

BOOL CAVICapture::Stop()
{
    HRESULT hr;
    
    if (m_pMediaCtrl  && m_bPlay)
	{
        LONGLONG pos = 0;
        hr = m_pMediaCtrl->Stop();
        
		if (SUCCEEDED(hr))
		{
			LONGLONG pos;
			hr = m_pMediaSeeking->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                       NULL, AM_SEEKING_NoPositioning);
		}  
		m_bPlay = FALSE;
		return SUCCEEDED(hr);
	}
	return false;
}

void CAVICapture::SetSourceQuality(HWND hApp)
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
	m_pGrabber->SetMediaType(&mt);
    FreeMediaType(mt);
}

void CAVICapture::GetMediaType()
{
	//获取参数
	HRESULT hr;
    AM_MEDIA_TYPE mt;
    hr = m_pGrabber->GetConnectedMediaType( &mt );
    if ( FAILED( hr) )
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

BOOL CAVICapture::InitGraph(char *DeviceName)
{
	wchar_t wFile[MAX_PATH] = {0};
    HRESULT hr;
    
    if (!DeviceName)
        return FALSE;

	CStringConversion::StringToWideChar(DeviceName, wFile, MAX_PATH - 1);
    // 
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                         IID_IGraphBuilder, (void **)&m_pGrapBuilder);
	if (FAILED(hr)) 
		return FALSE;
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
		     IID_ICaptureGraphBuilder2, (void**)&m_pCapGB);

    if(SUCCEEDED(hr))
    {
		do 
		{
			hr = m_pCapGB->SetFiltergraph(m_pGrapBuilder);
			if (FAILED(hr))
				break;

			// 装载文件
			hr = CoCreateInstance(CLSID_AviStreamReader, NULL, CLSCTX_INPROC_SERVER,
				                IID_IBaseFilter, (void **)&m_pAviReader);
			if (FAILED(hr))
				break;

			hr = m_pAviReader->QueryInterface(IID_IFileSourceFilter, (void **) &m_pSrc);
			if (FAILED(hr))
				break;
			
			hr = m_pSrc->Load(wFile, NULL);
			if (FAILED(hr))
				break;

			hr = m_pGrapBuilder->AddFilter(m_pAviReader, L"AVI Reader");
			if (FAILED(hr))
				break;

			//设备采集回调
			hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                         IID_ISampleGrabber, (void **)&m_pGrabber);
			if (FAILED(hr)) 
				break;
            
			//获取Filter
			hr = m_pGrabber->QueryInterface(IID_IBaseFilter, (void **)&m_Filter);
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

			hr = m_pGrapBuilder->AddFilter(m_Filter, L"Grabber" ); 
			if (FAILED(hr)) 
				break;

			hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER,
                         IID_IBaseFilter, (void **)&m_pVideoRenderer);
			if (FAILED(hr)) 
				break;
            
			hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
				         IID_IBaseFilter, (void **)&m_pAudioRenderer);
			if (FAILED(hr)) 
				break;

			hr = m_pGrapBuilder->AddFilter(m_pVideoRenderer, L"Video Renderer");
			if (FAILED(hr)) 
				break;

			hr = m_pGrapBuilder->AddFilter(m_pAudioRenderer, L"Audio Renderer");
			if (FAILED(hr)) 
				break;

			//PIN_CATEGORY_VIDEOPORT MEDIATYPE_Video
			hr = m_pCapGB->RenderStream(NULL, &MEDIATYPE_Video, m_pSrc, m_Filter, m_pVideoRenderer);
            if (FAILED(hr)) 
				break;

			//插入音频采集Filter
			m_AudioFilter = new CAudioTransFilter(NULL, &hr);
			if (FAILED(hr)) 
				break;
            
			if (m_AudioFilter)
			   ((CAudioTransFilter *)m_AudioFilter)->SetMediaEncoder(m_pEncoder);

			hr = m_pGrapBuilder->AddFilter(m_AudioFilter, L"Trans Filter");
			if (FAILED(hr)) 
				break;

			hr = m_pCapGB->RenderStream(NULL, &MEDIATYPE_Audio, m_pSrc, m_AudioFilter, m_pAudioRenderer);
			if (FAILED(hr)) 
				break;
 

			// 查询directshow 接口
			hr = m_pGrapBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaCtrl);
			if (FAILED(hr)) 
				break;
			hr = m_pGrapBuilder->QueryInterface(IID_IMediaSeeking, (void **)&m_pMediaSeeking);
			if (FAILED(hr)) 
				break;


			// 查询Video接口
			hr = m_pGrapBuilder->QueryInterface(IID_IVideoWindow, (void **)&m_pVideoWin);
			if (FAILED(hr)) 
				break;
			hr = m_pGrapBuilder->QueryInterface(IID_IBasicVideo, (void **)&m_pBasicVideo);
			if (FAILED(hr)) 
				break;

			// 查询音频接口
			hr = m_pGrapBuilder->QueryInterface(IID_IBasicAudio, (void **)&m_pBasicAudio);
			if (FAILED(hr)) 
				break;
            

			hr = m_pGrabber->SetBufferSamples( FALSE );
      		hr = m_pGrabber->SetOneShot( FALSE );
			hr = m_pGrabber->SetCallback( &m_BufferCB, 1 );
            if (FAILED(hr)) 
				break;
			
			//获取MediaType
			GetMediaType();

			//到此为止，初始化正确
			m_bInit = TRUE;
			return m_bInit;
		} while(0);
    }
    return FALSE;
}

void CAVICapture::CloseGraph()
{
	    
	HRESULT hr;

    if(m_pVideoWin)
    {
        hr = m_pVideoWin->put_Visible(OAFALSE);
        hr = m_pVideoWin->put_Owner(NULL);
    }

	SAFE_RELEASE(m_pVideoRenderer);
	SAFE_RELEASE(m_pAudioRenderer);	
	SAFE_RELEASE(m_pSrc);
	SAFE_RELEASE(m_pAviReader);
	SAFE_RELEASE(m_pGrabber);

	SAFE_RELEASE(m_pMediaSeeking);
    SAFE_RELEASE(m_pMediaCtrl);
    SAFE_RELEASE(m_pBasicAudio);
    SAFE_RELEASE(m_pBasicVideo);
    SAFE_RELEASE(m_pVideoWin);
    SAFE_RELEASE(m_pGrapBuilder);
	SAFE_RELEASE(m_pCapGB);
	m_bInit = FALSE;
}
