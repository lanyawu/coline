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
	void AdjustRect() {};
private:
	bool InitGraph(char *DeviceName);
	void CloseGraph();
	void GetMediaType();
	void SetOutputStream();
private:
    BOOL m_bPlay; //�Ƿ����ڲ���
	BOOL m_bInit; //�Ƿ��Ѿ���ʼ��
	HWND m_hPreview; //Ԥ������
	CMediaEncoder *m_pEncoder;
	CSampleGrabberCB m_BufferCB;  //�ɼ���Grabber
    
	IMediaDet *m_pDet;
	ISampleGrabber *m_pGrabber;
	IBaseFilter *m_pFilter;
	IFilterGraph *m_pGraph;

	//�������
    IMediaControl *m_pMediaCtrl;
    IMediaEventEx *m_pMediaEvent;
    IVideoWindow  *m_pVideoWin;
    IBasicAudio   *m_pBasicAudio;
    IBasicVideo   *m_pBasicVideo;
    IMediaSeeking *m_pMediaSeeking;
    IMediaPosition *m_pMediaPos;
};

#endif