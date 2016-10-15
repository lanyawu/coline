#include "UserEventList.h"

//COrderEventList
COrderEventList::COrderEventList()
{
}

COrderEventList::~COrderEventList()
{
	Clear();
}

void COrderEventList::SetEventName(const char *szEventName)
{
	m_strEventName = szEventName;
}

void COrderEventList::SetEventType(const char *szEventType)
{
	m_strEventType = szEventType;
}

BOOL COrderEventList::DoEvent(HWND hWnd, WPARAM wParam, LPARAM lParam, HRESULT *hResult)
{
	std::vector<ICoreEvent *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		(*it)->DoCoreEvent(hWnd, m_strEventType.c_str(), m_strEventName.c_str(), wParam, lParam, hResult);
	}
	return TRUE;
}



BOOL COrderEventList::DoEvent(HWND hWnd, const char *szCtrlName, const char *szEventType, WPARAM wParam, 
		           LPARAM lParam, HRESULT *hResult)
{
	std::vector<ICoreEvent *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		(*it)->DoCoreEvent(hWnd, szEventType, szCtrlName, wParam, lParam, hResult);
	}
	return TRUE;
}

BOOL COrderEventList::DeleteOrderEvent(ICoreEvent *pOrder)
{
	if (pOrder)
	{
		std::vector<ICoreEvent *>::iterator it;
		BOOL bExists = FALSE;
		for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
		{
			if ((*it) == pOrder)
			{
				m_OrderList.erase(it);
				return TRUE;
			} //end if ((*it) == ...
		} //end for (it = ..
	} //end if (pOrder...
	return FALSE;
}

BOOL COrderEventList::AddOrderEvent(ICoreEvent *pOrder)
{
	if (pOrder)
	{
		std::vector<ICoreEvent *>::iterator it;
		BOOL bExists = FALSE;
		for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
		{
			if ((*it) == pOrder)
			{
				bExists = TRUE;
				break;
			}
		}
		if (!bExists)
		{
			pOrder->AddRef();
			m_OrderList.push_back(pOrder);
			return TRUE;
		}
	}
	return FALSE;
}

void COrderEventList::Clear()
{
	std::vector<ICoreEvent *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		(*it)->Release();
	}
	m_OrderList.clear();
}

//CUserEventList
CUserEventList::CUserEventList(void)
{
	//
}


CUserEventList::~CUserEventList(void)
{
	Clear();
}

void CUserEventList::Clear()
{
	std::map<CAnsiString_, COrderEventList *>::iterator it;
	for (it = m_EventList.begin(); it != m_EventList.end(); it ++)
		delete it->second;
	m_EventList.clear();
	for (it = m_WndEventList.begin(); it != m_WndEventList.end(); it ++)
		delete it->second;
	m_WndEventList.clear();
}

BOOL CUserEventList::DoEvent(HWND hWnd, const char *szWndName, const char *szCtrlName, const char *szEventType, WPARAM wParam, 
		           LPARAM lParam, HRESULT *hResult)
{
	BOOL bSucc = FALSE;
	if (szWndName && szCtrlName && szEventType)
	{
		CAnsiString_ strEvent = szWndName;
		strEvent += "_";
		strEvent += szCtrlName;
		strEvent += "_";
		strEvent += szEventType;
		std::map<CAnsiString_,  COrderEventList *>::iterator it = m_WndEventList.find(szWndName);
		if (it != m_WndEventList.end())
		{
			bSucc = it->second->DoEvent(hWnd, szCtrlName, szEventType, wParam, lParam, hResult);
		}
		it = m_EventList.find(strEvent);
		if (it != m_EventList.end())
		{
			bSucc = it->second->DoEvent(hWnd, wParam, lParam, hResult);
		}
	}
	return bSucc;
}

BOOL CUserEventList::DeleteAllWndEvent(ICoreEvent *pCore, const char *szWndName)
{
	if (szWndName && pCore)
	{
		std::map<CAnsiString_,  COrderEventList *>::iterator it = m_WndEventList.find(szWndName);
		if (it != m_WndEventList.end())
		{
			it->second->DeleteOrderEvent(pCore);
			return TRUE;
		} 
	}
	return FALSE;
}

BOOL CUserEventList::DeleteNormalEvent(ICoreEvent *pCore, const char *szWndName, const char *szCtrlName, const char *szEventType)
{
	if (szWndName && szCtrlName && szEventType && pCore)
	{
		CAnsiString_ strEvent = szWndName;
		strEvent += "_";
		strEvent += szCtrlName;
		strEvent += "_";
		strEvent += szEventType;
		std::map<CAnsiString_,  COrderEventList *>::iterator it = m_EventList.find(strEvent);
		if (it != m_EventList.end())
		{
			it->second->DeleteOrderEvent(pCore);
			return TRUE;
		} 
	}
	return FALSE;
}

BOOL CUserEventList::OrderAllWndEvent(ICoreEvent *pCore, const char *szWndName)
{
	if (szWndName && pCore)
	{
		std::map<CAnsiString_,  COrderEventList *>::iterator it = m_WndEventList.find(szWndName);
		if (it != m_WndEventList.end())
		{
			it->second->AddOrderEvent(pCore);
		} else
		{
			COrderEventList *pEvent = new COrderEventList();
			pEvent->AddOrderEvent(pCore);
			m_WndEventList.insert(std::pair<CAnsiString_, COrderEventList *>(szWndName, pEvent));
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CUserEventList::OrderNormalEvent(ICoreEvent *pCore, const char *szWndName, 
	           const char *szCtrlName, const char *szEventType)
{
	if (szWndName && szCtrlName && szEventType && pCore)
	{
		CAnsiString_ strEvent = szWndName;
		strEvent += "_";
		strEvent += szCtrlName;
		strEvent += "_";
		strEvent += szEventType;
		std::map<CAnsiString_,  COrderEventList *>::iterator it = m_EventList.find(strEvent);
		if (it != m_EventList.end())
		{
			it->second->AddOrderEvent(pCore);
		} else
		{
			COrderEventList *pEvent = new COrderEventList();
			pEvent->AddOrderEvent(pCore);
			pEvent->SetEventName(szCtrlName);
			pEvent->SetEventType(szEventType);
			m_EventList.insert(std::pair<CAnsiString_, COrderEventList *>(strEvent, pEvent));
		}
		return TRUE;
	}
	return FALSE;
}

//
BOOL CUserEventList::AddEvent(const char *szWndName, const char *szCtrlName, const char *szEventType, ICoreEvent *pCore)
{
	if (pCore)
	{
		if (szWndName)
		{
			if (szCtrlName && szEventType)
				OrderNormalEvent(pCore, szWndName, szCtrlName, szEventType);
			else
				OrderAllWndEvent(pCore, szWndName);
		} //end if (szWndName)
	} //end if (pCore)
	return FALSE;
}

BOOL CUserEventList::DeleteEvent(const char *szWndName, const char *szCtrlName, const char *szEventType, ICoreEvent *pCore)
{
	if (pCore)
	{
		if (szWndName)
		{
			if (szCtrlName && szEventType)
				DeleteNormalEvent(pCore, szWndName, szCtrlName, szEventType);
			else
				DeleteAllWndEvent(pCore, szWndName);
		} //end if (szWndName)
	} //end if (pCore)
	return FALSE;
}


//COrderProtocolList
COrderProtocolList::COrderProtocolList()
{
}

COrderProtocolList::~COrderProtocolList()
{
	std::vector<IProtocolParser *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		(*it)->Release();
	}
	m_OrderList.clear();
}

BOOL COrderProtocolList::AddOrderProtocol(IProtocolParser *pOrder)
{
	if (pOrder)
	{
		std::vector<IProtocolParser *>::iterator it;
		BOOL bExists = FALSE;
		for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
		{
			if ((*it) == pOrder)
			{
				bExists = TRUE;
				break;
			}
		}
		if (!bExists)
		{
			pOrder->AddRef();
			m_OrderList.push_back(pOrder);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL COrderProtocolList::DeleteOrderProtocol(IProtocolParser *pOrder)
{
	if (pOrder)
	{
		std::vector<IProtocolParser *>::iterator it;
		for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
		{
			if ((*it) == pOrder)
			{
				(*it)->Release();
				m_OrderList.erase(it);
				break;
			} //end if ((*it) == pOrder)
		} //end for (it = m_OrderList....
	} //end if (pOrder)
	return FALSE;
}

BOOL COrderProtocolList::DoPresenceChanged(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder)
{
	std::vector<IProtocolParser *>::iterator it;
	BOOL bSucc = FALSE;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		BOOL b = (SUCCEEDED((*it)->DoPresenceChange(szUserName, szPresence, szMemo, bOrder)));
		bSucc = bSucc || b;
	}
	return bSucc;
}

BOOL COrderProtocolList::DoProtocol(const char *pBuf, const int nBufSize)
{
	std::vector<IProtocolParser *>::iterator it;
	BOOL bSucc = FALSE;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		BOOL b = (SUCCEEDED((*it)->DoRecvProtocol((BYTE *)pBuf, nBufSize)));
		bSucc = bSucc || b;
	}
	return bSucc;
}

//COrderUserProtocol
COrderUserProtocol::COrderUserProtocol()
{
}

COrderUserProtocol::~COrderUserProtocol()
{
	Clear();
}

void COrderUserProtocol::Clear()
{
	std::map<CAnsiString_, COrderProtocolList *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)  //one protocol, name and type
		delete it->second;
	m_OrderList.clear();
	for (it = m_OrderName.begin(); it != m_OrderName.end(); it ++) //All Name Protocol
		delete it->second;
	m_OrderName.clear();
}

BOOL COrderUserProtocol::AddOrderProto(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType)
{
	if (pOrder)
	{
		if (szProtoName)
		{
			if (szProtoType)
				return OrderNameType(pOrder, szProtoName, szProtoType);
			else
				return OrderAllName(pOrder, szProtoName);
		} //end if (szProtoName)
	} //end if (pOrder)
	return FALSE;
}

BOOL COrderUserProtocol::DoPresenceChanged(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder)
{
	static char STATIC_PRESENCE_CHANGE[] = "sys_presence";
	CGuardLock::COwnerLock guard(m_Lock);
    BOOL bSucc = FALSE;
	std::map<CAnsiString_,  COrderProtocolList *>::iterator it = m_OrderList.find(STATIC_PRESENCE_CHANGE);
	if (it != m_OrderList.end())
	{
		bSucc = bSucc || (it->second->DoPresenceChanged(szUserName, szPresence, szMemo, bOrder));
	}
	return bSucc;
}

BOOL COrderUserProtocol::DoProtocol(const char *szProtoName, const char *szType, const char *pBuf, const int nBufSize)
{
	BOOL bSucc = FALSE;
	if (szProtoName && szType && pBuf)
	{
		CAnsiString_ strEvent = szProtoName;
		strEvent += "_";
		strEvent += szType; 
		CGuardLock::COwnerLock guard(m_Lock);
		std::map<CAnsiString_,  COrderProtocolList *>::iterator it = m_OrderName.find(szProtoName);
		if (it != m_OrderName.end())
		{
			bSucc = it->second->DoProtocol(pBuf, nBufSize);
		}
		it = m_OrderList.find(strEvent);
		if (it != m_OrderList.end())
		{
			bSucc = bSucc || (it->second->DoProtocol(pBuf, nBufSize));
		}
	}
	return bSucc;
}

BOOL COrderUserProtocol::OrderAllName(IProtocolParser *pOrder, const char *szProtoName)
{
	if (szProtoName && pOrder)
	{
		CGuardLock::COwnerLock guard(m_Lock);
		std::map<CAnsiString_,  COrderProtocolList *>::iterator it = m_OrderName.find(szProtoName);
		if (it != m_OrderName.end())
		{
			it->second->AddOrderProtocol(pOrder);
		} else
		{
			COrderProtocolList *pList = new COrderProtocolList();
			pList->AddOrderProtocol(pOrder);
			m_OrderName.insert(std::pair<CAnsiString_, COrderProtocolList *>(szProtoName, pList));
		}
		return TRUE;
	}
	return FALSE;
}

BOOL COrderUserProtocol::OrderNameType(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType)
{
	if (szProtoName && szProtoType && pOrder)
	{
		CAnsiString_ strEvent = szProtoName;
		strEvent += "_";
		strEvent += szProtoType; 
		CGuardLock::COwnerLock guard(m_Lock);
		std::map<CAnsiString_,  COrderProtocolList *>::iterator it = m_OrderList.find(strEvent);
		if (it != m_OrderList.end())
		{
			it->second->AddOrderProtocol(pOrder);
		} else
		{
			COrderProtocolList *pEvent = new COrderProtocolList();
			pEvent->AddOrderProtocol(pOrder); 
			m_OrderList.insert(std::pair<CAnsiString_, COrderProtocolList *>(strEvent, pEvent));
		}
		return TRUE;
	}
	return FALSE;
}

BOOL COrderUserProtocol::DeleteOrderProto(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType)
{
	if (pOrder)
	{
		if (szProtoName)
		{
			if (szProtoType)
				return DeleteNameType(pOrder, szProtoName, szProtoType);
			else
				return DeleteAllName(pOrder, szProtoName);
		} //end if (szProtoName)
	} //end if (pOrder)
	return FALSE;
}

//delete
BOOL COrderUserProtocol::DeleteAllName(IProtocolParser *pOrder, const char *szProtoName)
{
	if (szProtoName && pOrder)
	{
		CGuardLock::COwnerLock guard(m_Lock);
		std::map<CAnsiString_,  COrderProtocolList *>::iterator it = m_OrderName.find(szProtoName);
		if (it != m_OrderName.end())
		{
			it->second->DeleteOrderProtocol(pOrder);
			return TRUE;
		} 		
	}
	return FALSE;
}

BOOL COrderUserProtocol::DeleteNameType(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType)
{
	if (szProtoName && szProtoType && pOrder)
	{
		CAnsiString_ strEvent = szProtoName;
		strEvent += "_";
		strEvent += szProtoType; 
		CGuardLock::COwnerLock guard(m_Lock);
		std::map<CAnsiString_,  COrderProtocolList *>::iterator it = m_OrderList.find(strEvent);
		if (it != m_OrderList.end())
		{
			it->second->DeleteOrderProtocol(pOrder);
			return TRUE;
		}  		
	}
	return FALSE;
}
