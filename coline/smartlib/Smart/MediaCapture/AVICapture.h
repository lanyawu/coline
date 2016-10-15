#ifndef __AVICAPTURE_H__
#define __AVICAPTURE_H__

#include "MediaStream.h"
#include "SampleGrabberCB.h"

class CAVICapture :public CMediaSource
{
public:
	CAVICapture(void);
public:
	~CAVICapture(void);
public:
	//设备初始化
	BOOL InitSource(char *DeviceName);
	//开始播放 hPreview 预览窗口
	BOOL Play(HWND hPreview);
	//停止播放
	BOOL Stop();
	//设置编码器
	void SetMediaEncoder(CMediaEncoder *Encoder);
	//设置媒体源数据质量
	void SetSourceQuality(HWND hApp);
	void AdjustRect();
private:
	BOOL InitGraph(char *DeviceName);
	void CloseGraph();
	void GetMediaType();
    HRESULT InitVideoWindow(int nMultiplier, int nDivider, bool IsDefault = true);
private:
    BOOL m_bPlay; //是否正在播放
	BOOL m_bInit; //是否已经初始化
	HWND m_hPreview; //预览窗口

	CSampleGrabberCB m_BufferCB;  //采集的Grabber
    IBaseFilter *m_AudioFilter; //采集音频的Filter

	ICaptureGraphBuilder2 *m_pCapGB;
	IBaseFilter *m_Filter;
	IFileSourceFilter *m_pSrc;
	IBaseFilter *m_pAviReader;
    IBaseFilter *m_pVideoRenderer;
	IBaseFilter *m_pAudioRenderer;

	ISampleGrabber *m_pGrabber;
	CMediaEncoder *m_pEncoder;
    IGraphBuilder *m_pGrapBuilder;
    IMediaControl *m_pMediaCtrl;
    IMediaSeeking *m_pMediaSeeking;
    IVideoWindow  *m_pVideoWin;
    IBasicAudio   *m_pBasicAudio;
    IBasicVideo   *m_pBasicVideo;
};

#endif