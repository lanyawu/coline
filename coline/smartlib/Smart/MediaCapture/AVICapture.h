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
	//�豸��ʼ��
	BOOL InitSource(char *DeviceName);
	//��ʼ���� hPreview Ԥ������
	BOOL Play(HWND hPreview);
	//ֹͣ����
	BOOL Stop();
	//���ñ�����
	void SetMediaEncoder(CMediaEncoder *Encoder);
	//����ý��Դ��������
	void SetSourceQuality(HWND hApp);
	void AdjustRect();
private:
	BOOL InitGraph(char *DeviceName);
	void CloseGraph();
	void GetMediaType();
    HRESULT InitVideoWindow(int nMultiplier, int nDivider, bool IsDefault = true);
private:
    BOOL m_bPlay; //�Ƿ����ڲ���
	BOOL m_bInit; //�Ƿ��Ѿ���ʼ��
	HWND m_hPreview; //Ԥ������

	CSampleGrabberCB m_BufferCB;  //�ɼ���Grabber
    IBaseFilter *m_AudioFilter; //�ɼ���Ƶ��Filter

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