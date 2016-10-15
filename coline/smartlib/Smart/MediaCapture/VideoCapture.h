#ifndef __VIDEOCAPTURE_H__
#define __VIDEOCAPTURE_H__

#include <commonlib/types.h>
#include <atlbase.h>
#include <streams.h>
#include <qedit.h>


#include "MediaStream.h"
#include "SampleGrabberCb.h"

class CVideoCapture : public CMediaSource
{
public:
	CVideoCapture(void);
public:
	~CVideoCapture(void);

public:

	//=====CMediaSource 虚函数 ======
	BOOL InitSource(char *DeviceName);
	void SetMediaEncoder(CMediaEncoder *pEncoder)
	{
		m_pEncoder = pEncoder;
		m_SampleGrabberCb.SetEncoder(pEncoder);
	};
    //开始播放
    BOOL Play(HWND hPreview);
	//停止播放
	BOOL Stop();
	//属性设置
	void SetSourceQuality(HWND hApp);

	void AdjustRect() {};

    // =====CMediaSource 虚函数结束 ====
    
    //hPreview 预览窗口，传入0表示不预览
	HRESULT InitStillGraph();
	//清除
	void ClearGraphs();
	//获取当前的位图头数据
	VIDEOINFOHEADER * GetVideoInfoHeader() { return m_Header; };
	//设置帧数
    BOOL SetFramePerSec(DWORD nFrame);

	//开始采集
	void StartCapture();
	//停止采集
	void StopCapture();
	//显示属性页
	void ShowCapPropertyPage(HWND hApp);
private:
	void GetDefaultCapDevice( IBaseFilter ** ppCap);
	HRESULT BuildPreview();
	void GetMediaType();
private:
    // either the capture live graph, or the capture still graph
    CComPtr< IGraphBuilder > m_pGraph;
    CComPtr<ICaptureGraphBuilder2> m_pCGB2;
    // the sample grabber for grabbing stills
    CComPtr< ISampleGrabber > m_pGrabber;
	CComPtr< IBaseFilter > m_pCap;
	CSampleGrabberCB m_SampleGrabberCb;
	IAMStreamConfig *m_Config;
	CMediaEncoder *m_pEncoder;
	VIDEOINFOHEADER *m_Header; //位图结构
	HWND m_hPreview; //预览窗口
	BOOL m_bCapture;
	BOOL m_bInit; //已经初始化过
};

#endif
