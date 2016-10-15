#ifndef __INTERFACEUSERLIST_H___
#define __INTERFACEUSERLIST_H___
#include <list>
#include <ComBase.h>
#include <core/coreinterface.h>

class CInterfaceUserList :public CComBase<>,
	                      public InterfaceImpl<IUserList>
{
public:
	CInterfaceUserList(void);
	~CInterfaceUserList(void);
public:
	//IUnknown
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);
	//
	STDMETHOD (AddUserInfo)(LPORG_TREE_NODE_DATA pData, BOOL bCopy);
	STDMETHOD (AddDeptInfo)(LPORG_TREE_NODE_DATA pData, BOOL bCopy);
	STDMETHOD (PopBackUserInfo)(LPORG_TREE_NODE_DATA *pData);
	STDMETHOD (PopBackDeptInfo)(LPORG_TREE_NODE_DATA *pData);
	STDMETHOD (PopFrontUserInfo)(LPORG_TREE_NODE_DATA *pData);
	STDMETHOD (PopFrontDeptInfo)(LPORG_TREE_NODE_DATA *pData);
	STDMETHOD_ (DWORD, GetUserCount)();
	STDMETHOD_ (DWORD, GetDeptCount)(); 
	STDMETHOD_ (BOOL, UserIsExists)(const char *szUserName);
	STDMETHOD  (SetDeptId)(const char *szPid);
	STDMETHOD  (GetDeptId)(IAnsiString *strPid);
	STDMETHOD  (CopyTo)(IUserList *pDst, BOOL bCopy);
	STDMETHOD  (PopsUserByName)(const char *szUserName, LPORG_TREE_NODE_DATA *pData);
	
    //
	void Clear();
	BOOL DeleteUserInfo(const char *szUserName);
	BOOL AddUserInfo(LPORG_TREE_NODE_DATA pData, BOOL bCopy, BOOL bChkExists); 
	BOOL CompareSub(IUserList *pCompare, IUserList *pAdd, IUserList *pSub); 
private:
	std::list<LPORG_TREE_NODE_DATA> m_UserList;
	std::list<LPORG_TREE_NODE_DATA> m_DeptList;
	std::string m_strPid;
};

#endif
