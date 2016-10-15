#include <commonlib/debuglog.h>
#include <Netlib/asock.h>
#include <NetLib/BaseNetUtils.h>
#include <mstcpip.h>

#define PACKET_SIZE  4096
#define MAX_ADDR_LEN  16

#pragma warning(disable:4996)

//监控网络运行状态
BOOL CBaseNetUtils::SpyNetworkFlux() 
{
	CASocket::ASocketInit();
	sockaddr_in srcAddr, dstAddr;
	char lpBuf[PACKET_SIZE];
	char szSourceIp[MAX_ADDR_LEN];
	char szDestIp[MAX_ADDR_LEN];
	SOCKET hSocket = ::socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (hSocket == SOCKET_ERROR)
		return FALSE;
	 
    //获取本机IP地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
	char szName[256] = {0};
	PHOSTENT pHost;
	if (::gethostname(szName, 255) == 0)
	{
		pHost = ::gethostbyname(szName);
		if (pHost != NULL)
		{
			memcpy(&(addr.sin_addr.S_un.S_addr), (struct in_addr *)(*pHost->h_addr_list), sizeof((struct in_addr *)(*pHost->h_addr_list)));
		}
	}
	addr.sin_family = AF_INET;
	if (::bind(hSocket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		::closesocket(hSocket);
		return FALSE;
	}
    //设置SOCK_RAW为SIO_RCVALL，以便接收所有的IP包
    int nOn = 1;
	DWORD dwNum;
	if (::WSAIoctl(hSocket, SIO_RCVALL, &nOn, sizeof(nOn), NULL, 0, &dwNum, NULL, NULL) == SOCKET_ERROR)
	{
		::closesocket(hSocket);
		return FALSE;
	}
    
	struct sockaddr_in fromAddr;
    int nFromLen;
	int nRcvSize;
    int nTTL;
	int nIpHeaderLen; //IP
    //侦听IP报文
	while(TRUE)
	{
		memset(lpBuf, 0, PACKET_SIZE);
		memset(&fromAddr, 0, sizeof(fromAddr));
		nFromLen = sizeof(fromAddr);
		nRcvSize = ::recvfrom(hSocket, lpBuf, PACKET_SIZE, 0, (struct sockaddr *)&fromAddr, &nFromLen);
		if (nRcvSize == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEMSGSIZE)
				continue;
		}
		LPIP_HEADER pIphdr = (LPIP_HEADER)lpBuf;
		//源地址
		srcAddr.sin_addr.S_un.S_addr = pIphdr->dwSourceIp;
		strncpy(szSourceIp, inet_ntoa(srcAddr.sin_addr), MAX_ADDR_LEN);
		//目的地址
		dstAddr.sin_addr.S_un.S_addr = pIphdr->dwDestIp;
		strncpy(szDestIp, inet_ntoa(dstAddr.sin_addr), MAX_ADDR_LEN);
        //
		nTTL = pIphdr->byteTTL;
		//计算IP头部长度
		nIpHeaderLen = 4 * (pIphdr->byteVer & 0x0F);
		switch(pIphdr->byteProto)
		{
		case IPPROTO_ICMP:
			{
				PRINTDEBUGLOG(dtInfo, "ICMP Packet, FromIP=%s, DestIP=%s, TTL=%d", szSourceIp, szDestIp, nTTL);
				break;
			}
		case IPPROTO_IGMP:
			{
				PRINTDEBUGLOG(dtInfo, "IGMP Packet, FromIP=%s, DestIP=%s, TTL=%d", szSourceIp, szDestIp, nTTL);
				break;
			}
		case IPPROTO_TCP:
			{
				//FILE *fp = fopen("f:\\a.p", "a+b");
				LPTCP_HEADER pTcpHdr = (LPTCP_HEADER)(lpBuf + nIpHeaderLen);
				int nTcpHdrLen = pTcpHdr->byteLenRes >> 4;
				int nOffset = nIpHeaderLen + sizeof(TCP_HEADER) + nTcpHdrLen;
				//char *szText = new char[nRcvSize - nOffset + 1];
				//memset(szText, 0, nRcvSize - nOffset + 1);
				//memcpy(szText, lpBuf + nOffset, nRcvSize - nOffset);
				PRINTDEBUGLOG(dtInfo, "TCP Packet, FromIP=%s, FromPort=%d, DestIP=%s, DestPort=%d, TTL=%d. Tcp Header Size=%d ", 
					szSourceIp, htons(pTcpHdr->wSourcePort), szDestIp, htons(pTcpHdr->wDestPort), nTTL, nTcpHdrLen );
				//fwrite(lpBuf, nRcvSize, 1, fp);
				//fclose(fp);
				//delete []szText;
				break;
			}
		case IPPROTO_UDP:
			{
				LPUDP_HEADER pUdpHdr = (LPUDP_HEADER)(lpBuf + nIpHeaderLen);
				int nOffset = nIpHeaderLen + sizeof(UDP_HEADER);
				char *szText = new char[nRcvSize - nOffset + 1];
				memset(szText, 0, nRcvSize - nOffset + 1);
				memcpy(szText, lpBuf + nOffset, nRcvSize - nOffset);
				PRINTDEBUGLOG(dtInfo, "UDP Packet, FromIP=%s FromPort=%d DestIP=%s DestPort=%d, TTL=%d Text=%s", szSourceIp, htons(pUdpHdr->wSourcePort),
					szDestIp, htons(pUdpHdr->wDestPort), nTTL, szText);
				delete []szText;
				break;
			}
		default:
			PRINTDEBUGLOG(dtInfo, "unknown packet from %s, TTL = %d", inet_ntoa(fromAddr.sin_addr), nTTL);
			break;
		}
	}//end while
	::closesocket(hSocket);
}

#pragma warning(default:4996)
