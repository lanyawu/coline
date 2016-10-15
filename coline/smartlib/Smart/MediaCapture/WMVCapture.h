#ifndef __WMVCAPTURE_H__
#define __WMVCAPTURE_H__

#include "MediaStream.h"
#include "SampleGrabberCB.h"

class CWMVCapture  :public CMediaSource
{
public:
	CWMVCapture(void);
public:
	~CWMVCapture(void);
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
	void AdjustRect() {};
private:
	bool InitGraph(char *DeviceName);
	void CloseGraph();
	void GetMediaType();
	void SetOutputStream();
private:
    BOOL m_bPlay; //是否正在播放
	BOOL m_bInit; //是否已经初始化
	HWND m_hPreview; //预览窗口
	CMediaEncoder *m_pEncoder;
	CSampleGrabberCB m_BufferCB;  //采集的Grabber
    
	IMediaDet *m_pDet;
	ISampleGrabber *m_pGrabber;
	IBaseFilter *m_pFilter;
	IFilterGraph *m_pGraph;

	//控制相关
    IMediaControl *m_pMediaCtrl;
    IMediaEventEx *m_pMediaEvent;
    IVideoWindow  *m_pVideoWin;
    IBasicAudio   *m_pBasicAudio;
    IBasicVideo   *m_pBasicVideo;
    IMediaSeeking *m_pMediaSeeking;
    IMediaPosition *m_pMediaPos;
};

#endif