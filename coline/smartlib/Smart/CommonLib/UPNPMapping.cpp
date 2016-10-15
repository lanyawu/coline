#include <stdio.h>
#include <CommonLib/UPNPMapping.h>
#include <CommonLib/DebugLog.h>
#include <miniupnp/upnpcommands.h>
#include <miniupnp/upnperrors.h>

#pragma warning(disable:4996)

CUPNPMapping::CUPNPMapping(void):
              m_hThread(NULL)
{
	m_nOldUDPPort = 0;
	m_nOldTCPPort = 0;
	m_pURLs = NULL;
	m_pIGDData = NULL;
	m_bAbortDiscovery = false;
}

CUPNPMapping::~CUPNPMapping(void)
{
	if (m_hThread)
	{
		WaitForSingleObject(m_hThread, 5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	if (m_pURLs != NULL)
	{
		FreeUPNPUrls(m_pURLs);
		delete m_pURLs;
		m_pURLs = NULL;
	}
	if (m_pIGDData)
	{
		delete m_pIGDData;
		m_pIGDData = NULL;
	}
}

bool CUPNPMapping::IsReady()
{
	 if (!m_bAbortDiscovery)
		 return true;
	 else
		 return false;
}

void CUPNPMapping::StopAsyncFind()
{
	//Í£Ö¹Ïß³ÌËÑË÷
}

void CUPNPMapping::DeletePorts()
{ 
	m_nOldUDPPort = m_nUDPPort; 
	m_nUDPPort = 0;
	m_nOldTCPPort = m_nTCPPort;
	m_nTCPPort = 0;
	DeletePorts(false); 
}

void CUPNPMapping::DeletePorts(bool bSkipLock)
{
	if ((m_pURLs == NULL) || (m_pIGDData == NULL))
		return ;
	if (m_nOldTCPPort != 0)
	{
		char achPort[10] = {0};
		sprintf(achPort, "%u", m_nOldTCPPort);
		if (UPNP_DeletePortMapping(m_pURLs->controlURL, m_pIGDData->servicetype, achPort, "TCP") == UPNPCOMMAND_SUCCESS)
		{
			m_nOldTCPPort = 0;
		}
	}
    
	if (m_nOldUDPPort != 0)
	{
		char achPort[10] = {0};
		sprintf(achPort, "%u", m_nOldUDPPort);
		if (UPNP_DeletePortMapping(m_pURLs->controlURL, m_pIGDData->servicetype, achPort, "UDP") == UPNPCOMMAND_SUCCESS)
		{
			m_nOldUDPPort = 0;
		}

	}

}

void CUPNPMapping::StartDiscovery(WORD nTCPPort, WORD nUDPPort)
{
 
	m_nOldUDPPort = m_nUDPPort;
	m_nUDPPort = nUDPPort;
	m_nOldTCPPort = m_nTCPPort;
	m_nTCPPort = nTCPPort;
	m_bUPnPPortsForwarded = TRIS_UNKNOWN;

	if (m_pURLs != NULL)
	{
		FreeUPNPUrls(m_pURLs);
		delete m_pURLs;
		m_pURLs = NULL;
	}
	if (m_pIGDData)
	{
		delete m_pIGDData;
		m_pIGDData = NULL;
	}
	if (m_bAbortDiscovery)
		return;
	m_hThread = ::CreateThread(NULL, 0, DiscoveryThread, this, 0, NULL);
}

 

DWORD WINAPI CUPNPMapping::DiscoveryThread(LPVOID lpParam)
{
	CUPNPMapping *pThis = (CUPNPMapping *)lpParam;

	if (pThis->m_bAbortDiscovery) 
		return 0;

	UPNPDev* structDeviceList = upnpDiscover(2000, NULL, NULL);
	if (structDeviceList == NULL){
		pThis->m_bUPnPPortsForwarded = TRIS_FALSE;
		//pThis->SendResultMessage();
		return 0;
	}

	if (pThis->m_bAbortDiscovery)
	{ 
		freeUPNPDevlist(structDeviceList);
		return 0;
	}

 
	for(UPNPDev* pDevice = structDeviceList; pDevice != NULL; pDevice = pDevice->pNext)
	{
		//DebugLog(_T("Desc: %S, st: %S"), pDevice->descURL, pDevice->st);
	}
	pThis->m_pURLs = new UPNPUrls;
	ZeroMemory(pThis->m_pURLs, sizeof(UPNPUrls));
	pThis->m_pIGDData = new IGDdatas;
	ZeroMemory(pThis->m_pIGDData, sizeof(IGDdatas));
	char achLanIP[16];
	achLanIP[0] = 0;

	int iResult = UPNP_GetValidIGD(structDeviceList, pThis->m_pURLs, pThis->m_pIGDData, achLanIP, sizeof(achLanIP));
	switch (iResult){
		case 1:
			//DebugLog(_T("Found valid IGD : %S"), m_pOwner->m_pURLs->controlURL);
			break;
		case 2:
			//DebugLog(_T("Found a (not connected?) IGD : %S - Trying to continue anyway"), m_pOwner->m_pURLs->controlURL);
			break;
		case 3:
			//DebugLog(_T("UPnP device found. Is it an IGD ? : %S - Trying to continue anyway"), m_pOwner->m_pURLs->controlURL);
			break;
		default:
			;//DebugLog(_T("Found device (igd ?) : %S - Trying to continue anyway"), m_pOwner->m_pURLs->controlURL);
	}
	freeUPNPDevlist(structDeviceList);
	//DebugLog(_T("Our LAN IP: %S"), achLanIP);

	if (pThis->m_bAbortDiscovery) // requesting to abort ASAP?
		return 0;

	// do we still have old mappings? Remove them first
	pThis->DeletePorts(true);
	
	BOOL bSucceeded = pThis->OpenPort(pThis->m_nTCPPort, true, achLanIP);
	if (bSucceeded && pThis->m_nUDPPort != 0)
		bSucceeded = pThis->OpenPort(pThis->m_nUDPPort, false, achLanIP);

	if (!pThis->m_bAbortDiscovery){ // dont send a result on a abort request
		pThis->m_bUPnPPortsForwarded = bSucceeded ? TRIS_TRUE : TRIS_FALSE;
		//pThis->SendResultMessage();
	}
	return 0;
}

BOOL CUPNPMapping::OpenPort(WORD nPort, bool bTCP, char* pachLANIP)
{
	const char achTCP[] = "TCP";
	const char achUDP[] = "UDP";
	const char achDescTCP[] = "ITMsg_TCP";
	const char achDescUDP[] = "ITMsg_UDP";
	char achPort[10] = {0};
	sprintf(achPort, "%u", nPort);
	
	if (m_bAbortDiscovery)
		return FALSE;

	int nResult;
	if (bTCP)
		nResult = UPNP_AddPortMapping(m_pURLs->controlURL, m_pIGDData->servicetype,
		                achPort, achPort, pachLANIP, achDescTCP, achTCP);
	else
		nResult = UPNP_AddPortMapping(m_pURLs->controlURL, m_pIGDData->servicetype,		
		               achPort, achPort, pachLANIP, achDescUDP, achUDP);

	if (nResult != UPNPCOMMAND_SUCCESS)
	{
		return FALSE;
	}

	if (m_bAbortDiscovery)
		return FALSE;

	// make sure it really worked
	char achOutIP[20];
	achOutIP[0] = 0;
	if (bTCP)
		nResult = UPNP_GetSpecificPortMappingEntry(m_pURLs->controlURL, m_pIGDData->servicetype,
            		achPort, achTCP, achOutIP, achPort);
	else
		nResult = UPNP_GetSpecificPortMappingEntry(m_pURLs->controlURL, m_pIGDData->servicetype, 
		              achPort, achUDP, achOutIP, achPort);

	if (nResult == UPNPCOMMAND_SUCCESS && achOutIP[0] != 0)
	{
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}

#pragma warning(default:4996)