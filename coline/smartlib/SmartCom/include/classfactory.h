// ClassFactory.h
//
#if !defined(AFX_CLASSFACTORY_H__369F4E09_64A7_11D4_B0B4_0050BABFC904__INCLUDED_)
#define AFX_CLASSFACTORY_H__369F4E09_64A7_11D4_B0B4_0050BABFC904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <ComBase.h>

// ���ڵ�ʵ���๤�����������ڶ��CreateObject���󣬷�����ͬ�Ķ���ָ��
//
template<class comObj>
class CSingleCreator
{
protected:
	CSingleCreator():m_pObj(0) {};

	comObj *CreateObject()
	{
		if(!m_pObj)
		{
			m_pObj = new comObj;
		}
		return m_pObj;
	}
	comObj * m_pObj;
};

// �����������ͨ���๤��������ÿһ��CreateObject���󶼷���һ���µĶ���ָ�롣
//
template<class comObj>
class CMultiCreator
{
protected:
	CMultiCreator():m_pObj(0) {};
	comObj *CreateObject()
	{
		return new comObj;
	}
	comObj * m_pObj;
};

// �๤��ʵ�֣�ȱʡ���๤����������MultiCreator�������ʵ����IClasFactory�ӿ�
// 
template <class comObj, class creatorClass  = CMultiCreator < comObj > >
class CClassFactory :  public CComBase<>,public InterfaceImpl<IClassFactory>,public creatorClass 
{
public:
	CClassFactory() {};
	virtual ~CClassFactory() {};

	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppv)
	{
		*ppv = NULL;
		if(IsEqualIID(riid,IID_IUnknown) || IsEqualIID(riid,IID_IClassFactory))
		{
			*ppv = (IClassFactory *) this;
			_AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj)
	{
		*ppvObj = NULL;
		if (pUnkOuter)
    		return CLASS_E_NOAGGREGATION;
		m_pObj = CreateObject();  // m_pObj ��creatorClass �ж���
		if (!m_pObj)
    		return E_OUTOFMEMORY;
		HRESULT hr = m_pObj->QueryInterface(riid, ppvObj);
		if(hr != S_OK)
		{
			delete m_pObj;
		}
		return hr;
	}

	STDMETHODIMP LockServer(BOOL) {	return S_OK; }  // δʵ��
};



#endif // !defined(AFX_CLASSFACTORY_H__369F4E09_64A7_11D4_B0B4_0050BABFC904__INCLUDED_)
