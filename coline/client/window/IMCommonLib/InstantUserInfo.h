#ifndef __INSTANTUSERINFO_H___
#define __INSTANTUSERINFO_H___

#include <ComBase.h>
#include <Core/CoreInterface.h>
#include <Commonlib/stringutils.h>
#include <string>
#include <map>

class CInstantUserInfo: public CComBase<>,
	                    public InterfaceImpl<IInstantUserInfo>
{
public:
	CInstantUserInfo(void);
	~CInstantUserInfo(void);

public:
	//IUnknow
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppv);

	//IInstantUserInfo
	//SET
	STDMETHOD (SetUserInfo)(const char *szParam, const char *szValue);
	STDMETHOD (SetUserStatus)(const char *szStatus);
	//GET
	STDMETHOD (GetUserInfo)(const char *szParam, IAnsiString *szValue);
	//
	STDMETHOD (GetUserStatus)(IAnsiString *szStatus);

	//
	BOOL AssignTo(IInstantUserInfo *pInfo);
	//
	LONG AddOrderRef();
	LONG ReleaseOrderRef();
	LONG GetOrderRef();
    void Clear();
private:
	std::map<CAnsiString_, std::string> m_Params;
	CAnsiString_ m_strStatus;
	volatile LONG m_nOrderRef;
};


#endif
