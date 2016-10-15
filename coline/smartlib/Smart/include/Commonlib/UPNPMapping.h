#ifndef __UPNPMAPPING_H__
#define __UPNPMAPPING_H__

#include <CommonLib/GuardLock.h>
#include <miniupnp/miniupnpc.h>


enum TRISTATE{
	TRIS_FALSE,
	TRIS_UNKNOWN,
	TRIS_TRUE
};
//UPNP¶Ë¿ÚÓ³Éä
class COMMONLIB_API CUPNPMapping
{
public:
	CUPNPMapping(void);
	~CUPNPMapping(void);
public:
	void	StartDiscovery(WORD nTCPPort, WORD nUDPPort);
	void	StopAsyncFind();
	void	DeletePorts();
	bool	IsReady();

protected:
	void DeletePorts(bool bSkipLock);
private:
	static DWORD WINAPI DiscoveryThread(LPVOID lpParam);
	BOOL OpenPort(WORD nPort, bool bTCP, char* pachLANIP);
private:
	WORD	m_nOldUDPPort;
	WORD	m_nOldTCPPort;

	WORD    m_nTCPPort;
	WORD    m_nUDPPort;
	WORD    m_bUPnPPortsForwarded; 
	UPNPUrls*	m_pURLs;
	IGDdatas*	m_pIGDData;
	HANDLE		m_hThread;
	
	static  CGuardLock m_Lock;
	volatile bool	m_bAbortDiscovery;
};

#endif