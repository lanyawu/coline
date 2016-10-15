#include "OrderWinMsgList.h"

//COrderWinMsgItem
COrderWinMsgItem::COrderWinMsgItem()
{
	//
}

COrderWinMsgItem::~COrderWinMsgItem()
{
	Clear();
}

void COrderWinMsgItem::Clear()
{
	std::vector<ICoreEvent *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
		(*it)->Release();
	m_OrderList.clear();
}

BOOL COrderWinMsgItem::AddOrderEvent(ICoreEvent *pCore)
{
	BOOL bExists = FALSE;
	std::vector<ICoreEvent *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		if ((*it) = pCore)
		{
			bExists = TRUE;
			break;
		}
	}
	if (!bExists)
	{
		pCore->AddRef();
		m_OrderList.push_back(pCore);
	}
	return TRUE;
}

BOOL COrderWinMsgItem::DoWinMessage(HWND hWnd, UINT uMsg, WPARAM wParam, 
                                  LPARAM lParam, LRESULT *lRes)
{
	std::vector<ICoreEvent *>::iterator it;
	BOOL bDoing = FALSE;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		if (SUCCEEDED((*it)->DoWindowMessage(hWnd, uMsg, wParam, lParam, lRes)))
			bDoing = TRUE;
	}
	return bDoing;
}

//COrderWinMsgList
COrderWinMsgList::COrderWinMsgList(void)
{
}


COrderWinMsgList::~COrderWinMsgList(void)
{
	Clear();
}

void COrderWinMsgList::Clear()
{
	std::map<UINT, COrderWinMsgItem *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		delete it->second;
	}
	m_OrderList.clear();
}

BOOL COrderWinMsgList::AddOrderEvent(UINT uMsg, ICoreEvent *pCore)
{
	std::map<UINT, COrderWinMsgItem *>::iterator it = m_OrderList.find(uMsg);
	if (it != m_OrderList.end())
	{
		it->second->AddOrderEvent(pCore);
	} else
	{
		COrderWinMsgItem *pItem = new COrderWinMsgItem();
		pItem->AddOrderEvent(pCore);
		m_OrderList.insert(std::pair<UINT, COrderWinMsgItem *>(uMsg, pItem));
	}
	return TRUE;
}

BOOL COrderWinMsgList::DoWinMessage(HWND hWnd, UINT uMsg, WPARAM wParam,
	                               LPARAM lParam, LRESULT *lRes)
{
	std::map<UINT, COrderWinMsgItem *>::iterator it = m_OrderList.find(uMsg);
	if (it != m_OrderList.end())
	{
		return it->second->DoWinMessage(hWnd, uMsg, wParam, lParam, lRes);
	}
	return FALSE;
}
