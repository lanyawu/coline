#ifndef __BASENETUTILS_H___
#define __BASENETUTILS_H___

#include <commonlib/types.h>

//基础网络函数
//IP头部结构
typedef struct _iphdr
{
	BYTE  byteVer;        //4位首部长度+4位IP版本号
	BYTE  byteTos;        //8位服务类型TOS
	WORD  wTotalLen;      //16位总长度（字节）
	WORD  wIdentify;      //16位标识
	WORD  wFragAndFlags;  //3位标志位
	BYTE  byteTTL;        //8位生存时间 TTL
	BYTE  byteProto;      //8位协议 (TCP, UDP 或其他)
	WORD  wCheckSum;      //16位IP首部校验和
	DWORD dwSourceIp;     //32位源IP地址
	DWORD dwDestIp;       //32位目的IP地址 
}IP_HEADER, *LPIP_HEADER; 

typedef struct _tcphdr
{
	WORD  wSourcePort; //16位源端口
	WORD  wDestPort;   //16位目的端口
	DWORD dwSeq;       //32位序列号
	DWORD dwAckSeq;    //32位确认号
	BYTE  byteLenRes;  //4位首部长度/6位保留字
	BYTE  byteFlag;    //6位标志位
	WORD  wWinSize;    //16位窗口大小
	WORD  wChekcSum;   //16位校验和
	WORD  wURP;        //16位紧急数据偏移量
}TCP_HEADER, *LPTCP_HEADER;

//UDP 头部数据
typedef struct _udphdr
{
	WORD wSourcePort;   //16位源端口
	WORD wDestPort;     //16位目的端口
	WORD wLen;          //16位长度
	WORD wCheckSum;     //16位校验和
}UDP_HEADER, *LPUDP_HEADER;

//ICMP头部
typedef struct _icmphdr
{
	BYTE  byteType;     //8位类型
	BYTE  byteCode;     //8位代码
    WORD  wCheckSum;    //16位校验和 
	WORD  wId;          //识别号（一般用进程号作为识别号）
	WORD  wSeq;         //报文序列号 
	DWORD dwTimeStamp;  //时间戳
}ICMP_HEADER, *LPICMP_HEADER;

 
class CBaseNetUtils
{
public:
	static BOOL SpyNetworkFlux(); //监控网络运行状态
 
};

#endif