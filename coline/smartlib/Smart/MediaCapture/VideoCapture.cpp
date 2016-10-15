#include <commonlib/Debuglog.h>
#include "VideoCapture.h"



CVideoCapture::CVideoCapture(void):
               m_Header(NULL),
			   m_bCapture(false),
			   m_bInit(false)
{
}

CVideoCapture::~CVideoCapture(void)
{
	ClearGraphs();
	delete m_Header;
}
BOOL CVideoCapture::InitSource(char *DeviceName)
{
	if (m_bInit)
		return TRUE;
	HRESULT hr = InitStillGraph();
	if (SUCCEEDED(hr))
		return TRUE;
	else
		return FALSE;
}

BOOL CVideoCapture::Play(HWND hPreview)
{
	if (!m_bInit)
	{
		HRESULT hr = InitStillGraph();
		if (FAILED(hr))
			return FALSE;
	}
	m_hPreview = hPreview;
    StartCapture();
	return TRUE;
}

BOOL CVideoCapture::Stop()
{
	StopCapture();
	return TRUE;
}

HRESULT CVideoCapture::InitStillGraph()
{
    HRESULT hr;

    //创建实例
    hr = m_pGraph.CoCreateInstance( CLSID_FilterGraph );
    if( !m_pGraph )
    {
		PRINTDEBUGLOG(dtInfo, "Could not create filter graph");
        return E_FAIL;
    }

    // 获取设备
    GetDefaultCapDevice( &m_pCap );
    if( !m_pCap )
    {
		PRINTDEBUGLOG(dtInfo, "No video capture device was detected on your system.\r\n\r\n");
        return E_FAIL;
    }

    // 加入cap设备到filter
    hr = m_pGraph->AddFilter( m_pCap, L"Cap" );
    if( FAILED( hr ) )
    {
		PRINTDEBUGLOG(dtInfo, "Could not put capture device in graph");
        return E_FAIL;
    }

    // 创建一个Grabber
    hr = m_pGrabber.CoCreateInstance( CLSID_SampleGrabber );
    if( !m_pGrabber )
    {
		PRINTDEBUGLOG(dtInfo, "Could not create SampleGrabber (is qedit.dll registered?)");
        return hr;
    }

    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabBase( m_pGrabber );

    // 强制连接到视频 24位 bitmap格式
    CMediaType VideoType;
    VideoType.SetType( &MEDIATYPE_Video );
    VideoType.SetSubtype( &MEDIASUBTYPE_RGB24 );
    hr = m_pGrabber->SetMediaType( &VideoType ); // 
    if( FAILED( hr ) )
    {
		PRINTDEBUGLOG(dtInfo, "Could not set media type");
        return hr;
    }

    // 把Grabber加入到Filter当中
    hr = m_pGraph->AddFilter(pGrabBase, L"Grabber" );
    if( FAILED( hr ) )
    {
		PRINTDEBUGLOG(dtInfo, "Could not put sample grabber in graph");
        return hr;
    }

    // 创建Graph
    hr = m_pCGB2.CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC);
    if (FAILED( hr ))
    {
		PRINTDEBUGLOG(dtInfo, "Can't get a ICaptureGraphBuilder2 reference");
        return hr;
    }

    hr = m_pCGB2->SetFiltergraph( m_pGraph );
    if (FAILED( hr ))
    {
		PRINTDEBUGLOG(dtInfo, "SetGraph failed");
        return hr;
    }

	// If there is a VP pin present on the video device, then put the 
    // renderer on CLSID_NullRenderer
    hr = BuildPreview();
    if( FAILED( hr ) )
    {
		PRINTDEBUGLOG(dtInfo, "Can't build the graph") ;
        return hr;
    }

	//获取配置接口
    hr = m_pCGB2->FindInterface(&PIN_CATEGORY_CAPTURE,
                                     &MEDIATYPE_Interleaved,
                                     m_pCap, IID_IAMStreamConfig, (void **)&m_Config);
	if (FAILED(hr))
	{
		hr = m_pCGB2->FindInterface(&PIN_CATEGORY_CAPTURE,
                                          &MEDIATYPE_Video, m_pCap,
                                          IID_IAMStreamConfig, (void **)&m_Config);
		if (FAILED(hr))
		{
		    PRINTDEBUGLOG(dtInfo, "Get Config Interface failed");
		}
	}
	pGrabBase.Release();
	m_bInit = TRUE;
    return S_OK;
}

HRESULT CVideoCapture::BuildPreview()
{
    CComPtr<IBaseFilter> pRenderer;
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabBase( m_pGrabber );
    CComPtr<IPin> pVPPin;
    HRESULT hr = m_pCGB2->FindPin(
                        m_pCap, 
                        PINDIR_OUTPUT, 
                        &PIN_CATEGORY_VIDEOPORT, 
                        NULL, 
                        FALSE, 
                        0, 
                        &pVPPin);
	if (hr == S_OK)
	{
		hr = pRenderer.CoCreateInstance(CLSID_NullRenderer);    
		if (S_OK != hr)
		{
			PRINTDEBUGLOG(dtInfo, "Unable to make a NULL renderer");
			return S_OK;
		}
		hr = m_pGraph->AddFilter(pRenderer, L"NULL renderer");
		if (FAILED (hr))
		{
			PRINTDEBUGLOG(dtInfo, "Can't add the filter to graph");
			return hr;
		}

		hr = m_pCGB2->RenderStream(
								&PIN_CATEGORY_PREVIEW,
								&MEDIATYPE_Interleaved, 
								m_pCap, 
								pGrabBase, 
								pRenderer);
	}
    if (FAILED (hr))
    {
        // try to render preview pin
        hr = m_pCGB2->RenderStream( 
                                &PIN_CATEGORY_PREVIEW, 
                                &MEDIATYPE_Video,
                                m_pCap, 
                                pGrabBase, 
                                pRenderer);

        // try to render capture pin
        if( FAILED( hr ) )
        {
            hr = m_pCGB2->RenderStream( 
                                    &PIN_CATEGORY_CAPTURE, 
                                    &MEDIATYPE_Video,
                                    m_pCap, 
                                    pGrabBase, 
                                    pRenderer);
        }
    }	
	GetMediaType();
    // don't buffer the samples as they pass through
    //
    hr = m_pGrabber->SetBufferSamples( FALSE );

    // only grab one at a time, stop stream after
    // grabbing one sample
    //
    hr = m_pGrabber->SetOneShot( FALSE );

    // set the callback, so we can grab the one sample
    //
    hr = m_pGrabber->SetCallback( &m_SampleGrabberCb, 1 );
    // ask for the connection media type so we know how big
    // it is, so we can write out bitmaps
	pRenderer.Release();
	pVPPin.Release();
	pGrabBase.Release();
    return S_OK;
}

void CVideoCapture::GetMediaType()
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

void CVideoCapture::ShowCapPropertyPage(HWND hApp)
{
	if (m_pCap)
	{
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
        HRESULT hr;
		hr = m_pCap->QueryInterface(IID_ISpecifyPropertyPages,
			(void **)&pSpec);
		if(hr == S_OK)
		{
			hr = pSpec->GetPages(&cauuid);

			hr = OleCreatePropertyFrame(hApp, 30, 30, NULL, 1,
				(IUnknown **)&m_Config, cauuid.cElems,
				(GUID *)cauuid.pElems, 0, 0, NULL);

			CoTaskMemFree(cauuid.pElems);
			pSpec->Release();
			pSpec = NULL;
		}
	}
}

void CVideoCapture::ClearGraphs()
{  
	//预览窗口
    if( m_pGraph )
    {

        CComQIPtr< IMediaControl, &IID_IMediaControl > pControl = m_pGraph;
		//停止运行
        if( pControl ) 
		{
            pControl->Stop( );
			pControl.Release();
		}

		//清除预览窗口
        CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pGraph;
        if( pWindow )
        {
            pWindow->put_Visible( OAFALSE );
            pWindow->put_Owner( NULL );
			pWindow.Release();
        }

		ULONG l;
		l = m_Config->Release();
		m_Config = NULL;
        m_pGrabber.Release( );   		
		m_pCap.Release();  
		m_pCGB2.Release();        
		m_pGraph.Release( );
		
    }

 	delete m_Header;
	m_Header = NULL;
}

void CVideoCapture::GetDefaultCapDevice( IBaseFilter ** ppCap)
{
    HRESULT hr;

    ASSERT(ppCap);
    if (!ppCap)
        return;

    *ppCap = NULL;

    // create an enumerator
    //
    CComPtr< ICreateDevEnum > pCreateDevEnum;
    pCreateDevEnum.CoCreateInstance( CLSID_SystemDeviceEnum );

    ASSERT(pCreateDevEnum);
    if( !pCreateDevEnum )
        return;

    // enumerate video capture devices
    //
    CComPtr< IEnumMoniker > pEm;
    pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );

    ASSERT(pEm);
    if( !pEm )
        return;

    pEm->Reset( );
 
    // go through and find first video capture device
    //
    while( 1 )
    {
        ULONG ulFetched = 0;
        CComPtr< IMoniker > pM;

        hr = pEm->Next( 1, &pM, &ulFetched );
        if( hr != S_OK )
            break;

        // get the property bag interface from the moniker
        //
        CComPtr< IPropertyBag > pBag;
        hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
        if( hr != S_OK )
            continue;

        // ask for the english-readable name
        //
        CComVariant var;
        var.vt = VT_BSTR;
        hr = pBag->Read(L"FriendlyName", &var, NULL);
        if( hr != S_OK )
            continue;

        hr = pM->BindToObject( 0, 0, IID_IBaseFilter, (void**) ppCap );
        if( *ppCap )
            break;
    }

    return;
}

void CVideoCapture::StartCapture()
{
    if (!m_bCapture)
	{
		m_bCapture = true;
		
        GetMediaType();
        // find the video window and stuff it in our window
		CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pGraph;
		if( !pWindow )
		{
			PRINTDEBUGLOG(dtInfo, "Could not get video window interface");
			return ;
		}
        
		// 预览
		if (m_hPreview)
		{
			RECT rc;
			::GetWindowRect( m_hPreview, &rc );
            HRESULT hr;
			hr = pWindow->put_Owner( (OAHWND) m_hPreview );
			hr = pWindow->put_Left( 0 );
			hr = pWindow->put_Top( 0 );
			hr = pWindow->put_Width( rc.right - rc.left );
			hr = pWindow->put_Height( rc.bottom - rc.top );
			hr = pWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS );
			hr = pWindow->put_Visible( OATRUE );
		}
		pWindow.Release();  
		IMediaControl *pMC = NULL;
		HRESULT hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
		if(SUCCEEDED(hr))
		{
			hr = pMC->Run();
			if(FAILED(hr))
			{
				// stop parts that ran
				pMC->Stop();
			}
			pMC->Release();
		}
		if(FAILED(hr))
		{
			PRINTDEBUGLOG(dtInfo, "Error %x: Cannot run preview graph");
		}
	}
}

void CVideoCapture::StopCapture()
{
	if (m_bCapture)
	{
		m_bCapture = false;
		IMediaControl *pMC = NULL;
		HRESULT hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&pMC);
		if(SUCCEEDED(hr))
		{
			hr = pMC->Stop();
			pMC->Release();
		}
		if(FAILED(hr))
		{
			PRINTDEBUGLOG(dtInfo, "Error %x: Cannot stop preview graph");
		}
	}
}

BOOL CVideoCapture::SetFramePerSec(DWORD nFrame)
{
	BOOL bPreview = m_bCapture;
	BOOL b = FALSE;
	StopCapture();
	if (m_Config)
	{
		AM_MEDIA_TYPE *pmt;
		if (m_Config->GetFormat(&pmt) == NOERROR)
		{
		    if(pmt->formattype == FORMAT_VideoInfo)
            {
                VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
				pvi->AvgTimePerFrame = 10000000 / nFrame;
                HRESULT hr = m_Config->SetFormat(pmt);
				if (SUCCEEDED(hr))
					b = TRUE;
            }
            DeleteMediaType(pmt);
		} 
	}
	if (bPreview)
		StartCapture();
	return b;
}

void CVideoCapture::SetSourceQuality(HWND hApp)
{
	BOOL bPreview = m_bCapture;
	BOOL b = FALSE;
	StopCapture();
	if (m_Config)
	{
		ISpecifyPropertyPages *pSpec;
		CAUUID cauuid;
        HRESULT hr;
		hr = m_Config->QueryInterface(IID_ISpecifyPropertyPages,
			(void **)&pSpec);
		if(hr == S_OK)
		{
			hr = pSpec->GetPages(&cauuid);

			hr = OleCreatePropertyFrame(hApp, 30, 30, NULL, 1,
				(IUnknown **)&m_Config, cauuid.cElems,
				(GUID *)cauuid.pElems, 0, 0, NULL);

			CoTaskMemFree(cauuid.pElems);
			ULONG l = pSpec->Release();
			pSpec = NULL;
		}
	}
	if (bPreview)
	{
		StartCapture();
	}
}