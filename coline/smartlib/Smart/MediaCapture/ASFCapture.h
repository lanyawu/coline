#ifndef __ASFCAPTURE_H__
#define __ASFCAPTURE_H__

#include "mediastream.h"
#include "SampleGrabberCB.h"

class CASFCapture :public CMediaSource
{
public:
	CASFCapture(void);
public:
	~CASFCapture(void);
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
	//设置大小
	void AdjustRect();
private:
	BOOL InitGraph(char *pFileName);
	void CloseGraph();
	void GetMediaType();
	HRESULT InitVideoWindow(int nMultiplier, int nDivider, bool IsDefault = true);
private:
	// DirectShow 接口
	IGraphBuilder *m_pGraph;
	IMediaControl *m_pMediaCtrl;
	IMediaEventEx *m_pMediaEvent;
	IVideoWindow  *m_pVideoWin;
	IBasicAudio   *m_pBasicAudio;
	IBasicVideo   *m_pBasicVideo;
	IMediaSeeking *m_pMediaSeeking;


	// 数据源 接口
	IBaseFilter       *m_pReader;
	IFileSourceFilter *m_pFileSource;

	
	//Grabber
	ISampleGrabber *m_pGrabber;
	ICaptureGraphBuilder2 *m_pCapGraph; 
	IBaseFilter *m_pCapFilter;	
	IBaseFilter       *m_pVideoRenderer;
	IBaseFilter       *m_pAudioRenderer;
	//变量
	HWND m_hPreview; //预览窗口
	CSampleGrabberCB m_BufferCB;  //采集的Grabber
    IBaseFilter *m_AudioFilter; //采集音频的Filter
	CMediaEncoder *m_pEncoder;
	BOOL m_bPlay; //是否正在播放
	BOOL m_bInit;
};												      

#endif