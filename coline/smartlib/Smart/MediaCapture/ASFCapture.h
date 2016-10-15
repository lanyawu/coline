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
	//���ô�С
	void AdjustRect();
private:
	BOOL InitGraph(char *pFileName);
	void CloseGraph();
	void GetMediaType();
	HRESULT InitVideoWindow(int nMultiplier, int nDivider, bool IsDefault = true);
private:
	// DirectShow �ӿ�
	IGraphBuilder *m_pGraph;
	IMediaControl *m_pMediaCtrl;
	IMediaEventEx *m_pMediaEvent;
	IVideoWindow  *m_pVideoWin;
	IBasicAudio   *m_pBasicAudio;
	IBasicVideo   *m_pBasicVideo;
	IMediaSeeking *m_pMediaSeeking;


	// ����Դ �ӿ�
	IBaseFilter       *m_pReader;
	IFileSourceFilter *m_pFileSource;

	
	//Grabber
	ISampleGrabber *m_pGrabber;
	ICaptureGraphBuilder2 *m_pCapGraph; 
	IBaseFilter *m_pCapFilter;	
	IBaseFilter       *m_pVideoRenderer;
	IBaseFilter       *m_pAudioRenderer;
	//����
	HWND m_hPreview; //Ԥ������
	CSampleGrabberCB m_BufferCB;  //�ɼ���Grabber
    IBaseFilter *m_AudioFilter; //�ɼ���Ƶ��Filter
	CMediaEncoder *m_pEncoder;
	BOOL m_bPlay; //�Ƿ����ڲ���
	BOOL m_bInit;
};												      

#endif