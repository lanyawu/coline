#pragma once

#include <ComDef.h>

// GUID of our COM server
_declspec(selectany) GUID CLSID_TEST_SVR =  { 0xb001f01e, 0xef79, 0x4492, 
                                            { 0xbd, 0xcd, 0xd4, 0x72, 0x64, 0x54, 0x38, 0xb8 } };



// interface definition
// for sample COM server just replace the uuid of interface and its name
// and add new method prototypes here ..
// 
interface __declspec(uuid("93DD9CA1-1747-4ce2-8550-2405C40615AB")) ITestSvrInterface : public IUnknown
{
	STDMETHOD (Square) (long *pVal) PURE;
	STDMETHOD (Cube) (long *pVal)   PURE;
};

interface __declspec(uuid("93DAE42D-0D75-4750-A13E-10EC475C2F1F")) ITestSubSvrInterface : public IUnknown
{
	STDMETHOD (GetNameById) (int nId, char *szName) PURE;
	STDMETHOD (SetNameById) (int nId, char *szName) PURE;
};