#ifndef __MEDIASTREAM_H__
#define __MEDIASTREAM_H__


#include <streams.h>
#include <commonlib/packetseq.h>
#include <commonlib/types.h>

const DWORD ALPHA_LOGO_X = 5; //logo��X����
const DWORD ALPHA_LOGO_Y = 5; //logo��Y����
const DWORD ALPHA_LOGO_BX = 56;
const DWORD ALPHA_LOGO_BY = 32;
const float ALPHA_LOGO_ALPHA_VALUE = 0.8f; //LOGO��͸����

#define MEDIA_HAS_VIDEO 0x01
#define MEDIA_HAS_AUDIO 0x02
#define MAX_WAVE_EXTRA_DATA_SIZE  64
#define MAX_VIDEO_EXTRA_DATA_SIZE 128

//�洢�����õĽṹ
typedef struct
{
	CPacketSeq nSeq;
	WORD nRef;
	WORD nOverTime; //�Ƿ��Ѿ�����
	DWORD nSize;     //���ݳ���
	BYTE  *pData;     //����
}CMediaBuffer;

typedef unsigned __int64 REFTIMETYPE; //֡��ͼ���

typedef struct {
    DWORD biSize;
    int  biWidth;
    int  biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    int  biXPelsPerMeter;
    int  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
}BG_BITMAPINFOHEADER, *LPBG_BITMAPINFOHEADER;

typedef struct
{
    WORD  wFormatTag;         /* format type */
    WORD  nChannels;          /* number of channels (i.e. mono, stereo...) */
    WORD  nSamplesPerSec;     /* sample rate */
    DWORD  nAvgBytesPerSec;    /* for buffer estimation */
    WORD  nBlockAlign;        /* block size of data */
    WORD  wBitsPerSample;     /* number of bits per sample of mono data */
    WORD  cbSize;             /* the count in bytes of the size of */
	WORD  nReserved;
}CWAVEFORMATEX, *PCWAVEFORMATEX;

typedef struct
{
    BYTE  nFramePerSec; //ÿ�����֡
	BYTE  nEncoderCode; //������ID
	WORD nMediaContext; //ý�����ݣ� 1λ��ʾ��ͼ�� 2λ��ʾ������
	DWORD nChannelID; //Ƶ��ID
	DWORD nKey;       //��Կ
	DWORD nBitRate; //������ kpbs
    CWAVEFORMATEX WaveInfo; //��Ƶ���
    BG_BITMAPINFOHEADER HeaderInfo;  //��Ƶ���
	BYTE  pWaveExtraData[MAX_WAVE_EXTRA_DATA_SIZE];
	DWORD nWaveExtraSize;
	BYTE  pVideoExtraData[MAX_VIDEO_EXTRA_DATA_SIZE];
	DWORD nVideoExtraSize;
}CMediaHeaderInfo;

//����������
typedef enum
{	
	ENCODER_NULL,
	ENCODER_AVS,
	ENCODER_H264,
	ENCODER_GPIKE
}CEncoderType;

//��ý��������
class CPacketStream
{
public:
	virtual ~CPacketStream() {};
    virtual CMediaBuffer * GetNextBuffer(CPacketSeq &nPktSeq) = 0; //��ȡ��һ�����
	virtual void ReleaseMediaBuffer(CMediaBuffer *buff) = 0; //�ͷ�һ��ʹ����ϵ�һ����
    virtual bool GetStreamHeaderData(BYTE *Header, DWORD &nSize) = 0; //���ͷ������
	virtual void EnvolopBuffer(REFTIMETYPE fTime, BYTE *buff, DWORD nSize, bool IsVideo = true) = 0; //����һ�����ݰ�
	virtual void SetMediaHeaderInfo(CMediaHeaderInfo *Header) = 0; //����ý��ͷ����Ϣ
};


//��ý�������
class CMediaEncoder
{
public:
	virtual ~CMediaEncoder() {};
	//��Ƶ���ݣ�fSimpleTime֡ʱ��
	virtual void OnRecvVideoBuff(REFTIMETYPE fSimpleTime, char *buff, DWORD nSize) = 0;
	//��Ƶ����
	virtual void OnRecvAudioBuff(REFTIMETYPE fSimpleTime, char *buff, DWORD nSize) = 0;
	//��Ƶ��ض������ݣ�����֡����BITMAP��Ϣ
	virtual void SetVideoHeaderInfo(VIDEOINFOHEADER *Header) = 0;
    //��Ӧ����Ƶ��Ϣ����
	virtual void SetAudioHeaderInfo(CWAVEFORMATEX *pWaveFormat) = 0;

	virtual void SetEncodeQuality(DWORD nQuality) = 0; //��������
	virtual bool InitEncoder() = 0; //��ʼ��������
	virtual bool LoadEncoderLogo(char *FileName) = 0; //װ��Logoͼ��
};

//Դ������ 
class CChannelSourceStream
{
public:
	virtual ~CChannelSourceStream() {};
	virtual  DWORD ReadData(char *pBuff, DWORD &nSize, bool &bIsReset) = 0;
	virtual  void ChuckInvalidPacket() = 0; //����һ����Ч�İ�
};

//������������� ������ģʽ����������������ֱ�����벥����
class CMediaDecoder
{
public:
	virtual ~CMediaDecoder() {};
	//��������������
	virtual bool AddMediaData(char *buff, DWORD nSize) = 0;
	//��ý������ͷ
	virtual void AddMediaHeaderInfo(CMediaHeaderInfo *Info) = 0;
    //��ȡѭ���ڴ�Ŀ�дָ��ʹ�С
	virtual bool GetCycleBuffer(char **p1, DWORD &nSize1, char **p2, DWORD &nSize2) = 0;
	//����Դ������
	virtual void SetSourceStream(CChannelSourceStream *pStream) = 0;
	//DetachԴ����
	virtual void DetachSourceStream() = 0;
	//��ȡͷ����Ϣ
	virtual bool ReadMediaHeaderInfo(CMediaHeaderInfo *Info) = 0;
	//��ȡһ֡ͼ��
	virtual bool ReadFrameBuff(char *buff, REFTIMETYPE &fTime) = 0;
	//��ȡ��Ƶ����
	virtual bool ReadAudioBuff(char *buff, DWORD &nSize, REFTIMETYPE &fTime) = 0;

	virtual void StartWork() = 0;
	virtual void StopWork() = 0;
};

//����������
class CMediaPlayerInterface
{
public:
	virtual ~CMediaPlayerInterface() {};
	//��ʼ����
	virtual void Play() = 0;
	//ֹͣ
	virtual void Stop() = 0;
	//��ͣ
	virtual void Pause() = 0;
	//�϶�
	virtual double Seek(double fPos) = 0;
	//��Ƶ�����ڽ�������ֱ�����ͣ�
	//��ȡ�������Ĳ��Ŵ���
	virtual HWND GetPlayHWND() = 0; 
	//���ý�����
	virtual void AddMediaHeaderInfo(CMediaHeaderInfo *Info) = 0;
	//��ȡ��ǰ�Ľ�����
	virtual CMediaDecoder *GetDecoder() = 0;
};

//Դ������
class CMediaSource
{
public:
	virtual ~CMediaSource() {};
	//�豸��ʼ��
	virtual BOOL InitSource(char *DeviceName) = 0;
	//��ʼ���� hPreview Ԥ������
	virtual BOOL Play(HWND hPreview) = 0;
	//ֹͣ����
	virtual BOOL Stop() = 0;
	//���ñ�����
	virtual void SetMediaEncoder(CMediaEncoder *Encoder) = 0;
	//����ý��Դ��������
	virtual void SetSourceQuality(HWND hApp) = 0;
	//������С
	virtual void AdjustRect() = 0;
};

#endif