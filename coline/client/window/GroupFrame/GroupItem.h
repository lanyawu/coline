#ifndef ______GROUP_ITEM_H_______
#define ______GROUP_ITEM_H_______

#include "../IMCommonLib/InterfaceUserList.h"

class CGroupItem
{
public:
	CGroupItem(const char *szGrpId, HWND hWnd);
	~CGroupItem(void);
public:
	HWND GetHWND();
	void SetOwnerHWND(HWND hWnd);
	BOOL DrawUsersToUI();
	void SetDispName(const char *szDspName);
	void SetCreator(const char *szCreator);
	BOOL IsGroupFrame(const char *szGrpId);
	BOOL GetUserList(IUserList *pUsers);
	void SetNewUserList(IUserList *pUsers);
	const char *GetGroupId();
	const char *GetCreator();
	const char *GetDispName();
	BOOL AddGroupUser(const char *szUserId, const char *szDspName);
	BOOL DeleteGroupUser(const char *szUserId);
	//
	BOOL m_bInitAckMenu; 
private:
	HWND m_hWnd;
	std::string m_strGrpId;
	std::string m_strDspName;
	std::string m_strCreator;
	CInterfaceUserList m_UserList; 
};

#endif
