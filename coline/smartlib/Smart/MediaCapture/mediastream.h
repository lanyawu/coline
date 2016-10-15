#ifndef __MEDIASTREAM_H__
#define __MEDIASTREAM_H__


#include <streams.h>
#include <commonlib/packetseq.h>
#include <commonlib/types.h>

const DWORD ALPHA_LOGO_X = 5; //logo的X坐标
const DWORD ALPHA_LOGO_Y = 5; //logo的Y坐标
const DWORD ALPHA_LOGO_BX = 56;
const DWORD ALPHA_LOGO_BY = 32;
const float ALPHA_LOGO_ALPHA_VALUE = 0.8f; //LOGO的透明度

#define MEDIA_HAS_VIDEO 0x01
#define MEDIA_HAS_AUDIO 0x02
#define MAX_WAVE_EXTRA_DATA_SIZE  64
#define MAX_VIDEO_EXTRA_DATA_SIZE 128

//存储数据用的结构
typedef struct
{
	CPacketSeq nSeq;
	WORD nRef;
	WORD nOverTime; //是否已经过期
	DWORD nSize;     //数据长度
	BYTE  *pData;     //数据
}CMediaBuffer;

typedef unsigned __int64 REFTIMETYPE; //帧的图像戳

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
    BYTE  nFramePerSec; //每秒多少帧
	BYTE  nEncoderCode; //编码器ID
	WORD nMediaContext; //媒体内容， 1位表示有图像 2位表示有声音
	DWORD nChannelID; //频道ID
	DWORD nKey;       //密钥
	DWORD nBitRate; //比特率 kpbs
    CWAVEFORMATEX WaveInfo; //音频相关
    BG_BITMAPINFOHEADER HeaderInfo;  //视频相关
	BYTE  pWaveExtraData[MAX_WAVE_EXTRA_DATA_SIZE];
	DWORD nWaveExtraSize;
	BYTE  pVideoExtraData[MAX_VIDEO_EXTRA_DATA_SIZE];
	DWORD nVideoExtraSize;
}CMediaHeaderInfo;

//编码器类型
typedef enum
{	
	ENCODER_NULL,
	ENCODER_AVS,
	ENCODER_H264,
	ENCODER_GPIKE
}CEncoderType;

//流媒体数据类
class CPacketStream
{
public:
	virtual ~CPacketStream() {};
    virtual CMediaBuffer * GetNextBuffer(CPacketSeq &nPktSeq) = 0; //获取下一个封包
	virtual void ReleaseMediaBuffer(CMediaBuffer *buff) = 0; //释放一个使用完毕的一个包
    virtual bool GetStreamHeaderData(BYTE *Header, DWORD &nSize) = 0; //获得头部数据
	virtual void EnvolopBuffer(REFTIMETYPE fTime, BYTE *buff, DWORD nSize, bool IsVideo = true) = 0; //加入一个数据包
	virtual void SetMediaHeaderInfo(CMediaHeaderInfo *Header) = 0; //设置媒体头部信息
};


//流媒体编码器
class CMediaEncoder
{
public:
	virtual ~CMediaEncoder() {};
	//视频数据，fSimpleTime帧时间
	virtual void OnRecvVideoBuff(REFTIMETYPE fSimpleTime, char *buff, DWORD nSize) = 0;
	//音频数据
	virtual void OnRecvAudioBuff(REFTIMETYPE fSimpleTime, char *buff, DWORD nSize) = 0;
	//视频相关定义数据，包括帧数及BITMAP信息
	virtual void SetVideoHeaderInfo(VIDEOINFOHEADER *Header) = 0;
    //相应的音频信息设置
	virtual void SetAudioHeaderInfo(CWAVEFORMATEX *pWaveFormat) = 0;

	virtual void SetEncodeQuality(DWORD nQuality) = 0; //编码质量
	virtual bool InitEncoder() = 0; //初始化编码器
	virtual bool LoadEncoderLogo(char *FileName) = 0; //装载Logo图标
};

//源数据流 
class CChannelSourceStream
{
public:
	virtual ~CChannelSourceStream() {};
	virtual  DWORD ReadData(char *pBuff, DWORD &nSize, bool &bIsReset) = 0;
	virtual  void ChuckInvalidPacket() = 0; //丢弃一个无效的包
};

//播放器解码基类 采用推模式，即解码后处理的数据直接送入播放器
class CMediaDecoder
{
public:
	virtual ~CMediaDecoder() {};
	//加入编码过的数据
	virtual bool AddMediaData(char *buff, DWORD nSize) = 0;
	//流媒体数据头
	virtual void AddMediaHeaderInfo(CMediaHeaderInfo *Info) = 0;
    //获取循环内存的可写指针和大小
	virtual bool GetCycleBuffer(char **p1, DWORD &nSize1, char **p2, DWORD &nSize2) = 0;
	//设置源数据流
	virtual void SetSourceStream(CChannelSourceStream *pStream) = 0;
	//Detach源数据
	virtual void DetachSourceStream() = 0;
	//读取头部信息
	virtual bool ReadMediaHeaderInfo(CMediaHeaderInfo *Info) = 0;
	//读取一帧图像
	virtual bool ReadFrameBuff(char *buff, REFTIMETYPE &fTime) = 0;
	//读取音频数据
	virtual bool ReadAudioBuff(char *buff, DWORD &nSize, REFTIMETYPE &fTime) = 0;

	virtual void StartWork() = 0;
	virtual void StopWork() = 0;
};

//播放器基类
class CMediaPlayerInterface
{
public:
	virtual ~CMediaPlayerInterface() {};
	//开始播放
	virtual void Play() = 0;
	//停止
	virtual void Stop() = 0;
	//暂停
	virtual void Pause() = 0;
	//拖动
	virtual double Seek(double fPos) = 0;
	//音频数据在解码器中直接推送？
	//获取播放器的播放窗口
	virtual HWND GetPlayHWND() = 0; 
	//设置解码器
	virtual void AddMediaHeaderInfo(CMediaHeaderInfo *Info) = 0;
	//获取当前的解码器
	virtual CMediaDecoder *GetDecoder() = 0;
};

//源类型类
class CMediaSource
{
public:
	virtual ~CMediaSource() {};
	//设备初始化
	virtual BOOL InitSource(char *DeviceName) = 0;
	//开始播放 hPreview 预览窗口
	virtual BOOL Play(HWND hPreview) = 0;
	//停止播放
	virtual BOOL Stop() = 0;
	//设置编码器
	virtual void SetMediaEncoder(CMediaEncoder *Encoder) = 0;
	//设置媒体源数据质量
	virtual void SetSourceQuality(HWND hApp) = 0;
	//调整大小
	virtual void AdjustRect() = 0;
};

#endif