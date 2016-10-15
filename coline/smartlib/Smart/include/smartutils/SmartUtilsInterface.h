#ifndef __SMARTUTILS_INTERFACE_H____
#define __SMARTUTILS_INTERFACE_H____

#include <comdef.h>
 
// GUID of our COM server
_declspec(selectany) GUID CLSID_SMART_UTILS = { 0xc5341c3b, 0x748b, 0x451a,
                                              { 0x8a, 0xf2, 0x52, 0xf3, 0x6d, 0xae, 0x5d, 0xa9 }};

//interface
interface __declspec(uuid("A3293BC0-FA7E-4884-8244-23B7A8FBDC40")) ISmartStream :public IStream
{
	//STDMETHOD (GetSmart)(int aa) PURE;
};



#endif
