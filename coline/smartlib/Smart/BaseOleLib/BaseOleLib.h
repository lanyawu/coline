

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Tue May 24 23:23:32 2016
 */
/* Compiler settings for BaseOleLib.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0595 
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __BaseOleLib_h__
#define __BaseOleLib_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGifAnimate_FWD_DEFINED__
#define __IGifAnimate_FWD_DEFINED__
typedef interface IGifAnimate IGifAnimate;

#endif 	/* __IGifAnimate_FWD_DEFINED__ */


#ifndef __GifAnimate_FWD_DEFINED__
#define __GifAnimate_FWD_DEFINED__

#ifdef __cplusplus
typedef class GifAnimate GifAnimate;
#else
typedef struct GifAnimate GifAnimate;
#endif /* __cplusplus */

#endif 	/* __GifAnimate_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IGifAnimate_INTERFACE_DEFINED__
#define __IGifAnimate_INTERFACE_DEFINED__

/* interface IGifAnimate */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IGifAnimate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("557F9A93-6544-4AD1-A247-B53C5F958DB0")
    IGifAnimate : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LoadFile( 
            BSTR newVal,
            ULONG hParent) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LoadBitmap( 
            ULONG hBitmap,
            ULONG hParent) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IGifAnimateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGifAnimate * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGifAnimate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGifAnimate * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGifAnimate * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGifAnimate * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGifAnimate * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGifAnimate * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Play )( 
            IGifAnimate * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *LoadFile )( 
            IGifAnimate * This,
            BSTR newVal,
            ULONG hParent);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *LoadBitmap )( 
            IGifAnimate * This,
            ULONG hBitmap,
            ULONG hParent);
        
        END_INTERFACE
    } IGifAnimateVtbl;

    interface IGifAnimate
    {
        CONST_VTBL struct IGifAnimateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGifAnimate_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGifAnimate_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGifAnimate_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGifAnimate_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGifAnimate_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGifAnimate_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGifAnimate_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IGifAnimate_Play(This)	\
    ( (This)->lpVtbl -> Play(This) ) 

#define IGifAnimate_LoadFile(This,newVal,hParent)	\
    ( (This)->lpVtbl -> LoadFile(This,newVal,hParent) ) 

#define IGifAnimate_LoadBitmap(This,hBitmap,hParent)	\
    ( (This)->lpVtbl -> LoadBitmap(This,hBitmap,hParent) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGifAnimate_INTERFACE_DEFINED__ */



#ifndef __BaseOleLibLib_LIBRARY_DEFINED__
#define __BaseOleLibLib_LIBRARY_DEFINED__

/* library BaseOleLibLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_BaseOleLibLib;

EXTERN_C const CLSID CLSID_GifAnimate;

#ifdef __cplusplus

class DECLSPEC_UUID("23444ED6-2FFB-475A-B7FA-378A286AA613")
GifAnimate;
#endif
#endif /* __BaseOleLibLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


