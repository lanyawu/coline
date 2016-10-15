#include "AudioCapture.h"

CAudioCapture::CAudioCapture(void):
               m_bWorking(false),
			   m_pCapture(NULL),
			   m_pCaptureBuffer(NULL),
			   m_pSoundBuff(NULL),
			   m_hThread(NULL),
			   m_dwCaptureBufferSize(0)
{
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CAudioCapture::~CAudioCapture(void)
{
	Stop();
	CloseHandle(m_hEvent);
}

void CAudioCapture::Stop()
{
	m_bWorking = FALSE;
	SetEvent(m_hEvent);
	if (m_hThread)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	if (m_pCaptureBuffer)
	{
		m_pCaptureBuffer->Release();
		m_pCaptureBuffer = NULL;
	}
	if (m_pCapture)
	{
		m_pCapture->Release();
		m_pCapture = NULL;
	}
	if (m_pSoundBuff)
	{
		delete []m_pSoundBuff;
		m_pSoundBuff = NULL;
	}
}

BOOL CAudioCapture::Start(COutputStream *pStream, WAVEFORMATEX *pFormat, DWORD nNotifySize, CLSID *pDeviceID)
{
	if (!m_bWorking)
	{
		m_pOutStream = pStream;
		WAVEFORMATEX WaveFormat;
		if (pFormat)
			memmove(&WaveFormat, pFormat, sizeof(WAVEFORMATEX));
		else //设置默认值
		{
			memset(&WaveFormat, 0, sizeof(WAVEFORMATEX));
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.cbSize = sizeof(WAVEFORMATEX);
			WaveFormat.nChannels = 1;
			WaveFormat.nSamplesPerSec = 22050;
			WaveFormat.wBitsPerSample = 8;
            WaveFormat.nBlockAlign = WaveFormat.nChannels * ( WaveFormat.wBitsPerSample / 8 );
            WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign * WaveFormat.nSamplesPerSec;
		}
		if (nNotifySize == 0)
		{
			nNotifySize = WaveFormat.nAvgBytesPerSec / 8;
			if (nNotifySize < NOTIFY_POSITION)
				nNotifySize = NOTIFY_POSITION;
            nNotifySize -= nNotifySize % WaveFormat.nBlockAlign;
		}
		int nNumNotify = WaveFormat.nAvgBytesPerSec * 2 / nNotifySize;
		if (nNumNotify < 2)
			nNumNotify = 2;
		m_dwCaptureBufferSize = nNumNotify * nNotifySize;

		if (m_pCaptureBuffer)
		{
			m_pCaptureBuffer->Release();
			m_pCaptureBuffer = NULL;
		}
		if (m_pCapture)
		{
			m_pCapture->Release();
			m_pCapture = NULL;
		}
		if (!InitSoundCapture(pDeviceID))
			return FALSE;
		if (!CreateSoundBuffer(&WaveFormat))
			return FALSE;
		//设置通知点
		HRESULT hr = m_pCaptureBuffer->QueryInterface(IID_IDirectSoundNotify, (void **)&m_pNotify);
		if (FAILED(hr) || (!m_pNotify))
			return FALSE;
		DSBPOSITIONNOTIFY pDSBNotify[16];
		for (int i = 0; i < 16; i ++)
		{
			pDSBNotify[i].dwOffset = nNotifySize * (i + 1) - 1;
			pDSBNotify[i].hEventNotify = m_hEvent;
		}
		hr = m_pNotify->SetNotificationPositions(16, pDSBNotify);
		if (SUCCEEDED(hr))
		{
			m_dwNextReadOffset = 0;
			if (m_pSoundBuff)
				delete []m_pSoundBuff;
			m_dwSoundBuffSize = nNotifySize * 2;
			m_pSoundBuff = new char[m_dwSoundBuffSize];
			DWORD Id;
			m_bWorking = TRUE;
			m_hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadData, (void *)this, 0, &Id);
			if (m_hThread != NULL)
				return TRUE;
		}
		
	}
	return FALSE;
}

DWORD WINAPI CAudioCapture::ReadData(void *Param)
{
	CAudioCapture *pInstance = (CAudioCapture *)Param;
	pInstance->ReadSoundData();
	return 0;
}

void CAudioCapture::ReadSoundData()
{
	DWORD dwReadSize, dwCapturePos, dwReadPos, dwLockSize, dwP1Size, dwP2Size;
    void *p1, *p2;
	HRESULT hr;
	while(m_bWorking)
	{
		::WaitForSingleObject(m_hEvent, INFINITE);
		if (!m_bWorking)
			break;
		//读取数据
        hr = m_pCaptureBuffer->GetCurrentPosition(&dwCapturePos, &dwReadPos);
		if (FAILED(hr))
			break;

        dwLockSize = dwReadPos - m_dwNextReadOffset;
        if( dwLockSize < 0 )
			dwLockSize += m_dwCaptureBufferSize;
        if( dwLockSize == 0)
			continue;
		if (dwLockSize > m_dwSoundBuffSize)
			dwLockSize = m_dwSoundBuffSize;
		hr = m_pCaptureBuffer->Lock(m_dwNextReadOffset, dwLockSize, &p1, &dwP1Size, &p2, &dwP2Size, 0);
		if (FAILED(hr))
			break;
		dwReadSize = 0;
        if (p1)
		{
			memmove(m_pSoundBuff, p1, dwP1Size);
		    dwReadSize = dwP1Size;
			m_dwNextReadOffset += dwP1Size;
			m_dwNextReadOffset %= m_dwCaptureBufferSize;
		}

		if (p2)
		{
			memmove(m_pSoundBuff + dwReadSize, p2, dwP2Size);
			dwReadSize += dwP2Size;
			m_dwNextReadOffset += dwP2Size;
			m_dwNextReadOffset %= m_dwCaptureBufferSize;
		}
		m_pCaptureBuffer->Unlock(p1, dwP1Size, p2, dwP2Size);
		if (m_pOutStream)
			m_pOutStream->OnReceiveStream(m_pSoundBuff, dwReadSize);
	}
}


BOOL CAudioCapture::InitSoundCapture(CLSID *pSoundDevice)
{
	HRESULT hr;
	hr = DirectSoundCaptureCreate(pSoundDevice, &m_pCapture, NULL);
	return SUCCEEDED(hr);
}

BOOL CAudioCapture::CreateSoundBuffer(WAVEFORMATEX *pFormat)
{
	HRESULT hr;
    DSCBUFFERDESC dscbd;

    // Create the capture buffer
    memset( &dscbd, 0, sizeof(dscbd) );
    dscbd.dwSize        = sizeof(dscbd);
    dscbd.dwBufferBytes = m_dwCaptureBufferSize;
    dscbd.lpwfxFormat   = pFormat; // Set the format during creatation

    hr = m_pCapture->CreateCaptureBuffer(&dscbd, &m_pCaptureBuffer, NULL); 
	return (SUCCEEDED(hr));
}