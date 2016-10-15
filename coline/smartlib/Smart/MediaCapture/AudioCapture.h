#ifndef __AUDIOCAPTURE_H__
#define __AUDIOCAPTURE_H__

#include <dsound.h>
#include <mmsystem.h>

#define NOTIFY_POSITION     1024
#define CAPTURE_BUFFER_SIZE 8192

class COutputStream
{
public:
	virtual ~COutputStream() {};
public:
	virtual void OnReceiveStream(char *pBuff, DWORD nBuffSize) = 0;
};

class CAudioCapture
{
public:
	CAudioCapture(void);
public:
	~CAudioCapture(void);
public:
	BOOL Start(COutputStream *pStream, WAVEFORMATEX *pFormat = NULL, DWORD nNotifySize = 0, 
		   CLSID *pDeviceID = NULL);
	void Stop();
private:
	BOOL InitSoundCapture(CLSID *pSoundDevice = NULL); //初始化声音采集
	BOOL CreateSoundBuffer(WAVEFORMATEX *pFormat);
	static DWORD WINAPI ReadData(void *Param); //读取数据线程
	void ReadSoundData();
private:
	//DirectX interface
	LPDIRECTSOUNDCAPTURE m_pCapture;
	LPDIRECTSOUNDCAPTUREBUFFER m_pCaptureBuffer;
	LPDIRECTSOUNDNOTIFY m_pNotify;
	DWORD m_dwCaptureBufferSize;
	DWORD m_dwNextReadOffset;
	char *m_pSoundBuff;
	DWORD m_dwSoundBuffSize;
	HANDLE m_hEvent;
	HANDLE m_hThread;
	COutputStream *m_pOutStream;
	BOOL m_bWorking;
};

#endif