#ifndef __INTERFACE_ANSISTRING_H____
#define __INTERFACE_ANSISTRING_H____
#include <string>
#include <ComBase.h>
#include <Core/CoreInterface.h>

class CInterfaceAnsiString: public CComBase<>, 
	                        public InterfaceImpl<IAnsiString>
{
public:
	CInterfaceAnsiString(void);
	~CInterfaceAnsiString(void);
public:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv);

	//IAnsiString
	STDMETHOD (SetString)(const char *strInput);
	STDMETHOD (AppendString)(const char *strAppend);
	STDMETHOD (GetString)(char *szOutput, int *nSize);
	STDMETHOD_ (int,GetSize)();

	const char *GetData();
private:
	std::string m_strAnsi;
};

#endif
