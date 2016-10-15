#ifndef __SMARTUTILSIMPL_H___
#define __SMARTUTILSIMPL_H___
#include <ComBase.h>
#include <SmartUtils/SmartUtilsInterface.h>

class CSmartUtilsImpl : public CComBase<>, public InterfaceImpl<ISmartUtils>
{
public:
	CSmartUtilsImpl(void);
	virtual ~CSmartUtilsImpl(void);
public:
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);
private:

};

#endif
