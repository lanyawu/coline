#ifndef __ORDERWINMSGLIST_H____
#define __ORDERWINMSGLIST_H____
#include <map>
#include <vector>
#include <string>
#include <Core/CoreInterface.h>

class COrderWinMsgItem
{
public:
	COrderWinMsgItem();
	~COrderWinMsgItem();
public:
	void Clear();
	BOOL AddOrderEvent(ICoreEvent *pCore);
	BOOL DoWinMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
private:
	std::vector<ICoreEvent *> m_OrderList;
};

class COrderWinMsgList
{
public:
	COrderWinMsgList(void);
	~COrderWinMsgList(void);
public:
	void Clear();
	BOOL AddOrderEvent(UINT uMsg, ICoreEvent *pCore);
	BOOL DoWinMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes);
private:
	std::map<UINT, COrderWinMsgItem *> m_OrderList;
};

#endif
