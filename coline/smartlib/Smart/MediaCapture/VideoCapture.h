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

	//=====CMediaSource �麯�� ======
	BOOL InitSource(char *DeviceName);
	void SetMediaEncoder(CMediaEncoder *pEncoder)
	{
		m_pEncoder = pEncoder;
		m_SampleGrabberCb.SetEncoder(pEncoder);
	};
    //��ʼ����
    BOOL Play(HWND hPreview);
	//ֹͣ����
	BOOL Stop();
	//��������
	void SetSourceQuality(HWND hApp);

	void AdjustRect() {};

    // =====CMediaSource �麯������ ====
    
    //hPreview Ԥ�����ڣ�����0��ʾ��Ԥ��
	HRESULT InitStillGraph();
	//���
	void ClearGraphs();
	//��ȡ��ǰ��λͼͷ����
	VIDEOINFOHEADER * GetVideoInfoHeader() { return m_Header; };
	//����֡��
    BOOL SetFramePerSec(DWORD nFrame);

	//��ʼ�ɼ�
	void StartCapture();
	//ֹͣ�ɼ�
	void StopCapture();
	//��ʾ����ҳ
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
	VIDEOINFOHEADER *m_Header; //λͼ�ṹ
	HWND m_hPreview; //Ԥ������
	BOOL m_bCapture;
	BOOL m_bInit; //�Ѿ���ʼ����
};

#endif
