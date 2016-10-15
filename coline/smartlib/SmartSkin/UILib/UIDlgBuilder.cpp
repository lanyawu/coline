#include "common.h"

#include <UILib/UIDlgBuilder.h>

#include <CommonLib/StringUtils.h>

#include <UILib/UIActiveX.h>
#include <UILib/UIAnim.h>
#include <UILib/UIButton.h>
#include <UILib/UICombo.h>
#include <UILib/UIDecoration.h>
#include <UILib/UIEdit.h>
#include <UILib/UILabel.h>
#include <UILib/UIList.h>
#include <UILib/UIMenu.h>
#include <UILib/UIPanel.h>
#include <UILib/UITab.h>
#include <UILib/UITool.h>
#include <UILib/UITreeView.h>
#include <UILib/UISlideFolder.h>
#include <UILib/UIRichEdit.h>
#include <UILib/UIGifImagePanel.h>
#include <UILib/UIGifGridPanel.h>
#include <UILib/UIProgressbar.h>
#include <UILib/UIScintillaEdit.h>

#pragma warning(disable:4996)

CControlUI* CDialogBuilder::Create(LPCTSTR pstrXML, IDialogBuilderCallback* pCallback /*= NULL*/)
{
	return NULL;
}

CControlUI* CDialogBuilder::CreateFromResource(UINT nRes, IDialogBuilderCallback* pCallback /*= NULL*/)
{
	return NULL;
}

CControlUI *CDialogBuilder::CreateFromNode(LPCONTROLNODE pNode, CControlUI *pParent, IDialogBuilderCallback *pCallback)
{
	m_pCallback = pCallback;
	if (!pNode) 
		return NULL;
	return CreateByNode(pNode, pParent);
}

CControlUI *CDialogBuilder::CreateByNode(LPCONTROLNODE pNode, CControlUI *pParent)
{
	LPCONTROLNODE pRightNode = pNode;
	CDialogLayoutUI* pStretched = NULL;
	IContainerUI* pContainer = NULL;
	CControlUI* pReturn = NULL;
	while (pRightNode)
	{
		CControlUI *pControl = NULL;

		//创建
        switch(pRightNode->Data.nType)
		{
			case CONTROL_TYPE_LIST:
				 pControl = new CListUI();
				 break;
			case CONTROL_TYPE_CANVAS:
				 pControl = new CCanvasUI();
				 break;
			case CONTROL_TYPE_BUTTON:
				 pControl = new CButtonUI();
				 break;
			case CONTROL_TYPE_CHECKBOX:
				pControl = new CCheckBoxUI();
				break;
			case CONTROL_TYPE_RADIOBOX:
				pControl = new CRadioBoxUI();
				break;
			case CONTROL_TYPE_TOOLBAR:
				 pControl = new CToolbarUI();
				 break;
			case CONTROL_TYPE_TABPAGE:
				 pControl = new CTabPageUI();
				 break;
			case CONTROL_TYPE_ACTIVEX:
				 pControl = new CActiveXUI();
				 break;
			case CONTROL_TYPE_DROPDOWN:
				 pControl = new CDropDownUI();
				 break;
			case CONTROL_TYPE_FADEDLINE:
				 pControl = new CFadedLineUI();
				 break;
			case CONTROL_TYPE_TASKPANEL:
				 pControl = new CTaskPanelUI();
				 break;
			case CONTROL_TYPE_STATUSBAR:
				 pControl = new CStatusbarUI();
				 break;
			case CONTROL_TYPE_TABFOLDER:
				 pControl = new CTabFolderUI();
				 break;
			case CONTROL_TYPE_TEXTPANEL:
				 pControl = new CTextPanelUI();
				 break;
			case CONTROL_TYPE_TREEVIEW:
				 pControl = new CUITreeView();
				 break;
			case CONTROL_TYPE_RICHEDIT2:
				 pControl = new CRichEditUI();
				 break;
			case CONTROL_TYPE_TILELAYOUT:
				 pControl = new CTileLayoutUI();
				 break;
			case CONTROL_TYPE_TOOLBUTTON:
			     pControl = new CToolButtonUI();
			     break;
			case CONTROL_TYPE_IMAGEPANEL:
			     pControl = new CImagePanelUI();
			     break;
			case CONTROL_TYPE_LABELPANEL:
			     pControl = new CLabelPanelUI();
			     break;
			case CONTROL_TYPE_TOOLGRIPPER:
			     pControl = new CToolGripperUI();
			     break;
			case CONTROL_TYPE_WHITECANVAS:
			     pControl = new CWhiteCanvasUI();
			     break;
			case CONTROL_TYPE_TITLESHADOW:
			     pControl = new CTitleShadowUI();
			     break;
			case CONTROL_TYPE_WINDOWCANVAS:
			     pControl = new CWindowCanvasUI();
			     break;
			case CONTROL_TYPE_DIALOGCANVAS:
			     pControl = new CDialogCanvasUI();
			     break;
			case CONTROL_TYPE_DIALOGLAYOUT:
			     pControl = new CDialogLayoutUI();
			     break;
			case CONTROL_TYPE_PADDINGPANEL:
			     pControl = new CPaddingPanelUI();
			     break;
			case CONTROL_TYPE_WARNINGPANEL:
			     pControl = new CWarningPanelUI();
			     break;
			case CONTROL_TYPE_GIFIMAGEPANEL:
				 pControl = new CGifImagePanelUI();
				 break;
			case CONTROL_TYPE_SEPARATORLINE:
			     pControl = new CSeparatorLineUI();
			     break;
			case CONTROL_TYPE_CONTROLCANVAS:
			     pControl = new CControlCanvasUI();
			     break;
			case CONTROL_TYPE_MULTILINEEDIT:
			     pControl = new CMultiLineEditUI();
			     break;
			case CONTROL_TYPE_TOOLSEPARATOR:
			     pControl = new CToolSeparatorUI();
			     break;
			case CONTROL_TYPE_VERTICALLAYOUT:
			     pControl = new CVerticalLayoutUI();
			     break;
			case CONTROL_TYPE_SINGLELINEEDIT:
			     pControl = new CSingleLineEditUI();
			     break;
			case CONTROL_TYPE_SINGLELINEPICK:
			     pControl = new CSingleLinePickUI();
			     break;
			case CONTROL_TYPE_GREYTEXTHEADER:
			     pControl = new CGreyTextHeaderUI();
			     break;
			case CONTROL_TYPE_TABFOLDERCANVAS:
			     pControl = new CTabFolderCanvasUI(); 
			     break;
			case CONTROL_TYPE_INTERNETEXPLORER:
				 pControl = new CInternetExplorerUI();
				 break;
			case CONTROL_TYPE_LISTHEADERSHADOW:
			     pControl = new CListHeaderShadowUI();
			     break;
			case CONTROL_TYPE_HORIZONTALLAYOUT:
			     pControl = new CHorizontalLayoutUI();
			     break;
			case CONTROL_TYPE_SEARCHTITLPANEL:
			     pControl = new CSearchTitlePanelUI();
			     break;
			case CONTROL_TYPE_TOOLBARTITLEPANEL:
			     pControl = new CToolbarTitlePanelUI();
			     break;
			case CONTROL_TYPE_IMAGEBUTTON:
				 pControl = new CImageButtonUI();
				 break;
			case CONTROL_TYPE_PLUSMENUBUTTON:
				 pControl = new CPlusMenuButtonUI();
				 break;
			case CONTROL_TYPE_IMAGECANVAS:
				 pControl = new CImageCanvasUI();
				 break;
			case CONTROL_TYPE_IMAGETABFOLDER:
				 pControl = new CImageTabFolderUI();
				 break;
			case CONTROL_TYPE_IMAGETABPAGE:
				 pControl = new CImageTabPageUI();
				 break;
			case CONTROL_TYPE_MENUBUTTON:
				 pControl = new CMenuButtonUI();
				 break;
			case CONTROL_TYPE_MENULIST:
				 pControl = new CMenuUI();
				 break;
			case CONTROL_TYPE_MENUITEM:
				 pControl = new CMenuItemUI();
				 break;
			case CONTROL_TYPE_SLIDEFOLDER:
				 pControl = new CSlideFolderUI();
				 break;
			case CONTROL_TYPE_SLIDEPAGE:
				 pControl = new CSlidePageUI();
				 break;
			case CONTROL_TYPE_GIFGRIDPANEL:
				 pControl = new CGifGridPanelUI();
				 break;
			case CONTROL_TYPE_TIPEDITUI:
				 pControl = new CTipEditUI();
				 break;
			case CONTROL_TYPE_PROGRESSBAR:
				 pControl = new CProgressBarUI();
				 break;
			case CONTROL_TYPE_FILEPROGRESSBAR:
				 pControl = new CFileProgressBarUI();
				 break;
			case CONTROL_TYPE_SCINTILLAEDIT:
				 pControl = new CScintillaEditUI();
				 break;
			case CONTROL_TYPE_AUTOSHORTCUT:
				 pControl = new CAutoShortCutVertList();
				 break;
			case CONTROL_TYPE_DIVIDE:
				 pControl = new CDivideLayoutUI();
				 break;
			case CONTROL_TYPE_NAVIGATORPANEL:
			case CONTROL_TYPE_NAVIGATORBUTTON:
			case CONTROL_TYPE_LISTTEXTELEMENT:
			case CONTROL_TYPE_LISTLABELELEMENT:
			case CONTROL_TYPE_LISTHEADER:
			case CONTROL_TYPE_LISTHEADERITEM:
			case CONTROL_TYPE_LISTFOOTER:
			default:
				TRACE( _T("不能识别的皮肤数据类型 %d"), pRightNode->Data.nType );
				ASSERT( FALSE );
				break;
		}

		//
		if (pControl == NULL && m_pCallback != NULL) 
		{
			pControl = m_pCallback->CreateControlByType(pRightNode->Data.nType);
		}
		ASSERT(pControl);
		if (pControl == NULL) 
			return NULL;

		// Add children
		if (pRightNode->pLeftNode)
		{
			CreateByNode(pRightNode->pLeftNode, pControl);
		}

		// Attach to parent
		if (pParent != NULL)
		{
			if (pContainer == NULL) 
				pContainer = static_cast<IContainerUI *>(pParent->GetInterface(_T("Container")));
			ASSERT(pContainer);
			if (pContainer == NULL) 
				return NULL;
			pContainer->Add(pControl, 9999999);
		}

		// Process attributes
		if (pRightNode->Data.szAttribute) 
		{
			int nSize = ::strlen(pRightNode->Data.szAttribute);
			TCHAR *szValue = new TCHAR[nSize + 1];
			memset(szValue, 0, sizeof(TCHAR) * (nSize + 1));
			CStringConversion::StringToWideChar(pRightNode->Data.szAttribute, szValue, nSize);
			pControl->ApplyAttributeList(szValue);
			delete []szValue;
		}
		
		// Very custom attributes
		if(pRightNode->Data.bStretch)
		{
			if (pStretched == NULL)
				pStretched = static_cast<CDialogLayoutUI *>(pParent->GetInterface(_T("DialogLayout")));
			ASSERT(pStretched);
			if (pStretched == NULL) 
				return NULL;
			pStretched->SetStretchMode(pControl, pRightNode->Data.nMode);
		}
		pRightNode = pRightNode->pRightNode;
		// Return first item
		if (pReturn == NULL)
		{
			pReturn = pControl;
		}
	} //end while (pRightNode)
	return pReturn;
}

#pragma warning(default:4996)

