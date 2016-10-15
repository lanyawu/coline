#if !defined(AFX_BLUEBUILDER_H__20050505_A1C5_1D19_C2BA_0080AD509054__INCLUDED_)
#define AFX_BLUEBUILDER_H__20050505_A1C5_1D19_C2BA_0080AD509054__INCLUDED_

#pragma once

#include <UILib/UIResource.h>

class IDialogBuilderCallback
{
public:
   virtual CControlUI* CreateControl(LPCTSTR pstrClass) = 0;
   virtual CControlUI *CreateControlByType(CONTROLTYPE nType) = 0;
};

class CDialogBuilder
{
public:
   CControlUI* Create(LPCTSTR pstrXML, IDialogBuilderCallback* pCallback = NULL);
   CControlUI* CreateFromResource(UINT nRes, IDialogBuilderCallback* pCallback = NULL);
   CControlUI *CreateFromNode(LPCONTROLNODE pNode, CControlUI *pParent = NULL, IDialogBuilderCallback *pCallback = NULL);
private:
   CControlUI *CreateByNode(LPCONTROLNODE pNode, CControlUI *pParent = NULL);
   IDialogBuilderCallback* m_pCallback;
};


#endif // !defined(AFX_BLUEBUILDER_H__20050505_A1C5_1D19_C2BA_0080AD509054__INCLUDED_)

