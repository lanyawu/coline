#ifndef __BASENETUTILS_H___
#define __BASENETUTILS_H___

#include <commonlib/types.h>

//�������纯��
//IPͷ���ṹ
typedef struct _iphdr
{
	BYTE  byteVer;        //4λ�ײ�����+4λIP�汾��
	BYTE  byteTos;        //8λ��������TOS
	WORD  wTotalLen;      //16λ�ܳ��ȣ��ֽڣ�
	WORD  wIdentify;      //16λ��ʶ
	WORD  wFragAndFlags;  //3λ��־λ
	BYTE  byteTTL;        //8λ����ʱ�� TTL
	BYTE  byteProto;      //8λЭ�� (TCP, UDP ������)
	WORD  wCheckSum;      //16λIP�ײ�У���
	DWORD dwSourceIp;     //32λԴIP��ַ
	DWORD dwDestIp;       //32λĿ��IP��ַ 
}IP_HEADER, *LPIP_HEADER; 

typedef struct _tcphdr
{
	WORD  wSourcePort; //16λԴ�˿�
	WORD  wDestPort;   //16λĿ�Ķ˿�
	DWORD dwSeq;       //32λ���к�
	DWORD dwAckSeq;    //32λȷ�Ϻ�
	BYTE  byteLenRes;  //4λ�ײ�����/6λ������
	BYTE  byteFlag;    //6λ��־λ
	WORD  wWinSize;    //16λ���ڴ�С
	WORD  wChekcSum;   //16λУ���
	WORD  wURP;        //16λ��������ƫ����
}TCP_HEADER, *LPTCP_HEADER;

//UDP ͷ������
typedef struct _udphdr
{
	WORD wSourcePort;   //16λԴ�˿�
	WORD wDestPort;     //16λĿ�Ķ˿�
	WORD wLen;          //16λ����
	WORD wCheckSum;     //16λУ���
}UDP_HEADER, *LPUDP_HEADER;

//ICMPͷ��
typedef struct _icmphdr
{
	BYTE  byteType;     //8λ����
	BYTE  byteCode;     //8λ����
    WORD  wCheckSum;    //16λУ��� 
	WORD  wId;          //ʶ��ţ�һ���ý��̺���Ϊʶ��ţ�
	WORD  wSeq;         //�������к� 
	DWORD dwTimeStamp;  //ʱ���
}ICMP_HEADER, *LPICMP_HEADER;

 
class CBaseNetUtils
{
public:
	static BOOL SpyNetworkFlux(); //�����������״̬
 
};

#endif