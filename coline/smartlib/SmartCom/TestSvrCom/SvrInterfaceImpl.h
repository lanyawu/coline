#ifndef __SVRINTERFACEIMPL_H___
#define __SVRINTERFACEIMPL_H___

#include <ComBase.h>
#include "SvrInterface.h"
#include <string>
#include <map>
// sample use of the framework 
// this class implements a single interface ImyInterface ...
//
class CTestSubSvrInterfaceImpl: public CComBase<>, public InterfaceImpl<ITestSubSvrInterface>
{
public:
	CTestSubSvrInterfaceImpl();
	virtual ~CTestSubSvrInterfaceImpl();
public:
	// we however need to write code for queryinterface 
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);   
	STDMETHOD (GetNameById) (int nId, char *szName);
	STDMETHOD (SetNameById) (int nId, char *szName);
private:
	std::map<int, std::string> m_Names;
};

class CTestSvrInterfaceImpl : public CComBase<> , public InterfaceImpl<ITestSvrInterface> 
{
private:
	CTestSubSvrInterfaceImpl m_SubSvr;
public:
	CTestSvrInterfaceImpl();
	virtual ~CTestSvrInterfaceImpl();

	// we however need to write code for queryinterface 
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);

	// ImyInterface methods
	STDMETHOD(Square)(long *pVal);
	STDMETHOD(Cube)(long *pVal);

};


#endif