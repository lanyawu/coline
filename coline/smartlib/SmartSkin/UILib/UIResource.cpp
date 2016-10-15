#include <commonlib/stringutils.h>
#include <UILib/UIResource.h>
#include <UILib/uimanager.h>

#pragma warning(disable:4996) 

//
class CSkinNodeOperator
{
public:
	static LPCONTROLNODE CreateNode(LPCONTROLNODE pParent, CONTROLTYPE nType, UINT nMode, BOOL bStretch, 
		             const char *szAttr, BOOL IsChild); //加入一个节点
	static void FreeNode(LPCONTROLNODE pParent); //释放内存节点
};

//节点操作相关
LPCONTROLNODE CSkinNodeOperator::CreateNode(LPCONTROLNODE pParent, CONTROLTYPE nType, UINT nMode, BOOL bStretch, 
		            const char *szAttr, BOOL IsChild)
{
	LPCONTROLNODE pTemp = new CONTROLNODE();
	memset(pTemp, 0, sizeof(CONTROLNODE));
	pTemp->Data.nType = nType;
	pTemp->Data.nMode = nMode;
	pTemp->Data.bStretch = bStretch;
	if (szAttr)
	{
		pTemp->Data.nAttributeSize = (int) strlen(szAttr) + 1;
		pTemp->Data.szAttribute = new char[pTemp->Data.nAttributeSize];
		memset(pTemp->Data.szAttribute, 0, pTemp->Data.nAttributeSize);
		strcpy(pTemp->Data.szAttribute, szAttr);
	}
	if (pParent)
	{
		if (IsChild)
			pParent->pLeftNode = pTemp;
		else
			pParent->pRightNode = pTemp;
	}
	return pTemp;
}

void CSkinNodeOperator::FreeNode(LPCONTROLNODE pParent)
{
	//释放内存节点
}


CUIResource::CUIResource(void):
			 m_dwMaxBtnImageId(0),
			 m_dwMinBtnImageId(0),
			 m_dwRestoreBtnImageId(0),
			 m_dwCloseBtnImageId(0),
			 m_dwFormBkgImageId(0),
			 m_dwMsgDlgBkgImageId(0),
			 m_dwMsgBoxSuccImageId(0),
			 m_dwMsgBoxQuestionImageId(0),
			 m_dwMsgBoxInfoImageId(0),
			 m_dwMsgBoxErrorImageId(0),
			 m_dwMenuCheckImageId(0),
			 m_dwUpImageId(0),
			 m_dwVertBtnImageId(0),
			 m_dwDownImageId(0),
			 m_dwLeftImageId(0),
			 m_dwCurrLinkImageId(0x6F000000),
			 m_dwHorzBtnImageId(0),
			 m_dwRightImageId(0),
			 m_clrBase(0),
             m_pSkinDoc(NULL)
{
	InitControlTable();
}

CUIResource::~CUIResource(void)
{
	if (m_pSkinDoc)
		delete m_pSkinDoc;
	m_pSkinDoc = NULL;
	ClearAllUINodes();
	ClearImagesResource();
}

//初始化控制对应表
void CUIResource::InitControlTable()
{
	//control type and control name table
	m_ControlTable[std::string("Canvas")] = CONTROL_TYPE_CANVAS;
	m_ControlTable[std::string("DialogLayout")] = CONTROL_TYPE_DIALOGLAYOUT;
	m_ControlTable[std::string("DropDownBox")] = CONTROL_TYPE_DROPDOWN;
	m_ControlTable[std::string("HorizontalLayout")] = CONTROL_TYPE_HORIZONTALLAYOUT;
	m_ControlTable[std::string("StatusBar")] = CONTROL_TYPE_STATUSBAR;
	m_ControlTable[std::string("TabFolder")] = CONTROL_TYPE_TABFOLDER;
	m_ControlTable[std::string("TabPage")] = CONTROL_TYPE_TABPAGE;
	m_ControlTable[std::string("ToolBar")] = CONTROL_TYPE_TOOLBAR;
	m_ControlTable[std::string("TileLayout")] = CONTROL_TYPE_TILELAYOUT;
	m_ControlTable[std::string("VerticalLayout")] = CONTROL_TYPE_VERTICALLAYOUT;
	m_ControlTable[std::string("ControlCanvas")] = CONTROL_TYPE_CONTROLCANVAS;
	m_ControlTable[std::string("DialogCanvas")] = CONTROL_TYPE_DIALOGCANVAS;
	m_ControlTable[std::string("List")] = CONTROL_TYPE_LIST;
	m_ControlTable[std::string("ListFooter")] = CONTROL_TYPE_LISTFOOTER;
	m_ControlTable[std::string("ListHeader")] = CONTROL_TYPE_LISTHEADER;
	m_ControlTable[std::string("NavigatorPanel")] = CONTROL_TYPE_NAVIGATORPANEL;
	m_ControlTable[std::string("SearchTitlePanel")] = CONTROL_TYPE_SEARCHTITLPANEL;
	m_ControlTable[std::string("TabFolderCanvas")] = CONTROL_TYPE_TABFOLDERCANVAS;
	m_ControlTable[std::string("TaskPanel")] = CONTROL_TYPE_TASKPANEL;
	m_ControlTable[std::string("WhiteCanvas")] = CONTROL_TYPE_WHITECANVAS;
	m_ControlTable[std::string("WindowCanvas")] = CONTROL_TYPE_WINDOWCANVAS;
	m_ControlTable[std::string("ActiveXCtrl")] = CONTROL_TYPE_ACTIVEX;
	m_ControlTable[std::string("Button")] = CONTROL_TYPE_BUTTON;
	m_ControlTable[std::string("FadedLine")] = CONTROL_TYPE_FADEDLINE;
	m_ControlTable[std::string("GreyTextHeader")] = CONTROL_TYPE_GREYTEXTHEADER;
	m_ControlTable[std::string("ImagePanel")] = CONTROL_TYPE_IMAGEPANEL;
	m_ControlTable[std::string("LabelPanel")] = CONTROL_TYPE_LABELPANEL;
	m_ControlTable[std::string("ListHeaderItem")] = CONTROL_TYPE_LISTHEADERITEM;
	m_ControlTable[std::string("ListHeaderShadow")] = CONTROL_TYPE_LISTHEADERSHADOW;
	m_ControlTable[std::string("MultiLineEdit")] = CONTROL_TYPE_MULTILINEEDIT;
	m_ControlTable[std::string("OptionBox")] = CONTROL_TYPE_OPTION;
	m_ControlTable[std::string("PaddingPanel")] = CONTROL_TYPE_PADDINGPANEL;
	m_ControlTable[std::string("RichEdit2")] = CONTROL_TYPE_RICHEDIT2;
	m_ControlTable[std::string("SeparatorLine")] = CONTROL_TYPE_SEPARATORLINE;
	m_ControlTable[std::string("SingleLineEdit")] = CONTROL_TYPE_SINGLELINEEDIT;
	m_ControlTable[std::string("TitleShadow")] = CONTROL_TYPE_TITLESHADOW;
	m_ControlTable[std::string("PlusMenuButton")] = CONTROL_TYPE_PLUSMENUBUTTON;
	m_ControlTable[std::string("ToolButton")] = CONTROL_TYPE_TOOLBUTTON;
	m_ControlTable[std::string("ToolGripper")] = CONTROL_TYPE_TOOLGRIPPER;
	m_ControlTable[std::string("ToolSeparator")] = CONTROL_TYPE_TOOLSEPARATOR;
	m_ControlTable[std::string("ToolbarTitlePanel")] = CONTROL_TYPE_TOOLBARTITLEPANEL;
	m_ControlTable[std::string("ListExpandElement")] = CONTROL_TYPE_LISTEXPANDELEMENT;
	m_ControlTable[std::string("ListLabelElement")] = CONTROL_TYPE_LISTLABELELEMENT;
	m_ControlTable[std::string("ListTextElement")] = CONTROL_TYPE_LISTTEXTELEMENT;
	m_ControlTable[std::string("NavigatorButton")] = CONTROL_TYPE_NAVIGATORBUTTON;
	m_ControlTable[std::string("TextPanel")] = CONTROL_TYPE_TEXTPANEL;
	m_ControlTable[std::string("WarningPanel")] = CONTROL_TYPE_WARNINGPANEL;
	m_ControlTable[std::string("ImageButton")] = CONTROL_TYPE_IMAGEBUTTON;
	m_ControlTable[std::string("ImageCanvas")] = CONTROL_TYPE_IMAGECANVAS;
	m_ControlTable[std::string("ImageTabFolder")] = CONTROL_TYPE_IMAGETABFOLDER;
	m_ControlTable[std::string("ImageTabPage")] = CONTROL_TYPE_IMAGETABPAGE;
	m_ControlTable[std::string("MenuButton")] = CONTROL_TYPE_MENUBUTTON;
	m_ControlTable[std::string("MenuList")] = CONTROL_TYPE_MENULIST;
	m_ControlTable[std::string("MenuItem")] = CONTROL_TYPE_MENUITEM;
	m_ControlTable[std::string("SlideFolder")] = CONTROL_TYPE_SLIDEFOLDER;
	m_ControlTable[std::string("SlidePage")] = CONTROL_TYPE_SLIDEPAGE;
	m_ControlTable[std::string("TreeView")] = CONTROL_TYPE_TREEVIEW;
	m_ControlTable[std::string("GifGridPanel")] = CONTROL_TYPE_GIFGRIDPANEL;
	m_ControlTable[std::string("GifImagePanel")] = CONTROL_TYPE_GIFIMAGEPANEL;
	m_ControlTable[std::string("TipEdit")] = CONTROL_TYPE_TIPEDITUI;
	m_ControlTable[std::string("InternetExplorer")] = CONTROL_TYPE_INTERNETEXPLORER;
	m_ControlTable[std::string("CheckBox")] = CONTROL_TYPE_CHECKBOX;
	m_ControlTable[std::string("RadioBox")] = CONTROL_TYPE_RADIOBOX;
	m_ControlTable[std::string("ProgressBar")] = CONTROL_TYPE_PROGRESSBAR;
	m_ControlTable[std::string("FileProgressBar")] = CONTROL_TYPE_FILEPROGRESSBAR;
	m_ControlTable[std::string("ScintillaEdit")] = CONTROL_TYPE_SCINTILLAEDIT;
	m_ControlTable[std::string("AutoShortCut")] = CONTROL_TYPE_AUTOSHORTCUT;
	m_ControlTable[std::string("DividePanel")] = CONTROL_TYPE_DIVIDE;
}

DWORD CUIResource::GetLinkImageIdByLink(LPCTSTR lpszLink)
{
	return 0;
}

//删除所有图片资源
void CUIResource::ClearImagesResource()
{
	std::map<DWORD, LPUI_IMAGE_ITEM>::iterator it;
	for (it = m_ImageList.begin(); it != m_ImageList.end(); it ++)
	{
		if (it->second->pGraphic)
			delete it->second->pGraphic;
		delete it->second;
	}
	m_ImageList.clear();
 
	m_SrcImages.clear();
	m_SrcBkgImages.clear();
}

void CUIResource::ClearAllUINodes()
{
	std::map<CAnsiString_, LPCONTROLNODE>::iterator it;
	for (it = m_WindowList.begin(); it != m_WindowList.end(); ++it)
	{
		ReleaseChildNodes(it->second);
		it->second = NULL;
	}
}

void CUIResource::ReleaseChildNodes(LPCONTROLNODE lpNode)
{
	if (lpNode)
	{
		ReleaseLeftChild(lpNode);
		ReleaseRightChild(lpNode);
		ReleaseCurrent(lpNode);
	}
}

void CUIResource::ReleaseLeftChild(LPCONTROLNODE lpParent)
{
	LPCONTROLNODE lpLeftChild = lpParent->pLeftNode;
	if (lpLeftChild)		
		ReleaseChildNodes(lpLeftChild);
}

void CUIResource::ReleaseRightChild(LPCONTROLNODE lpParent)
{
	LPCONTROLNODE lpRightChild = lpParent->pRightNode;
	if (lpRightChild)
		ReleaseChildNodes(lpRightChild);
}

void CUIResource::ReleaseCurrent(LPCONTROLNODE lpNode)
{
	delete lpNode;
}

void CUIResource::AddChildNodes(LPCONTROLNODE pNode, TiXmlElement *pXmlNode)
{
	if ((pNode != NULL) && (pXmlNode != NULL))
	{
		AddLeftChildNode(pNode, pXmlNode);
		AddRightChildNode(pNode, pXmlNode);
	}
}

void CUIResource::AddLeftChildNode(LPCONTROLNODE pNode, TiXmlElement *pXmlNode)
{
	TiXmlElement * pCurrent = pXmlNode->FirstChildElement();
	if (pCurrent)
	{
		AddChildNodes(CreateNode(pCurrent, pNode, TRUE), pCurrent);
	}
}

void CUIResource::AddRightChildNode(LPCONTROLNODE pNode, TiXmlElement *pXmlNode)
{
	TiXmlElement *pCurrent = pXmlNode->NextSiblingElement();
	if (pCurrent != NULL)
	{
		AddChildNodes(CreateNode(pCurrent, pNode, FALSE), pCurrent);
	}
}

LPCONTROLNODE CUIResource::CreateNodeByXml(const char *szSkinXml)
{
	TiXmlDocument xmldoc;
	if (szSkinXml && (xmldoc.Load(szSkinXml, ::strlen(szSkinXml))))
	{
		TiXmlElement *pFirst = xmldoc.FirstChildElement();
		LPCONTROLNODE pNode = CreateNode(pFirst, NULL, FALSE);
		AddChildNodes(pNode, pFirst);
		return pNode;
	}
	return NULL;
}

LPCONTROLNODE CUIResource::CreateNode(TiXmlElement *pXmlNode, LPCONTROLNODE lpParent, BOOL bChild )
{
	//fetch the attribute string.
	//the first attribute is required which specifies the type
	//of the current node, and the others are optional
	//e.g.
	//...
	//  <Control/Container xis:type="SOMETYPE" ...
	//...
	TiXmlAttribute* pType = pXmlNode->FirstAttribute(); 
	if (pType == NULL)
	{
		return NULL;
	}
	const char *name;
	const char *value;
	value = pType->Value();
	//next find the CONTROLTYPE according to the type string
	CONTROLTYPE ct = m_ControlTable[std::string(value)];
 
	if (ct <= CONTROL_TYPE_FIRST_ || ct >= CONTROL_TYPE_LAST_ )
	{
		return NULL;
	}
	//get the attribute list
	//e.g. "name="aCanvas",pos="0 0 0 0""
	std::string szAttrList;
	TiXmlAttribute* pAttr = pType->Next();
	while(pAttr)
	{
		if (!szAttrList.empty())
			szAttrList += ',';

		//注意！某些属性值是用汉字描述的，在源数据流中以utf8格式
		//存储，在使用之前，utf8数据流必须被转换成asc流，否则会
		//出现乱码的情况（CControlNodeOperator::CreateNode的第五
		//个参数期望asc字符串）。
		//目前的属性名称只用英文，属性值用英文和汉字（且一个属性只
		//用一种文字表示），程序处理相对
		//简单，以后若要用其他语言也可以扩展
		name = pAttr->Name();
		szAttrList += name;
		szAttrList += "=\"";

		//the attribute list should contain less than 512 characters
		int nAttrSize = ::strlen(pAttr->Value()) * 2 + 1;
		char *attrValue = new char[nAttrSize];
		CStringConversion::UTF8ToString(pAttr->Value(), attrValue, nAttrSize - 1);
		szAttrList += attrValue;
		szAttrList += "\"";
		delete []attrValue;

		pAttr = pAttr->Next();
	}
	return CSkinNodeOperator::CreateNode(lpParent, ct, 0, FALSE, szAttrList.c_str(), bChild);
}

//载入XML数据
BOOL CUIResource::LoadSkinDoc(const char *pData, DWORD dwSize)
{
	BOOL bSucc = FALSE;
	if (m_pSkinDoc)
		delete m_pSkinDoc;
	m_pSkinDoc = new TiXmlDocument();
	if (m_pSkinDoc->Load(pData, dwSize))
	{
		//find frame window
		TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
		if (pXmlRoot)
		{ 
			TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
			for (;pFrame != NULL;)
			{
				if (stricmp(pFrame->Value(), FRAME_WINDOW_ROOT_NODE_NAME) == 0)
					break;
				pFrame = pFrame->NextSiblingElement();
			}
			if (pFrame)
			{
				const char *szValue = pFrame->Attribute("min");
				if (szValue)
				{
					m_dwMinBtnImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMinBtnImageId);
				}
				szValue = pFrame->Attribute("max");
				if (szValue)
				{
					m_dwMaxBtnImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMaxBtnImageId);
				}
				szValue = pFrame->Attribute("close");
				if (szValue)
				{
					m_dwCloseBtnImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwCloseBtnImageId);
				}
				szValue = pFrame->Attribute("restore");
				if (szValue)
				{
					m_dwRestoreBtnImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwRestoreBtnImageId);
				}
				szValue = pFrame->Attribute("background");
				if (szValue)
				{
					m_dwFormBkgImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwFormBkgImageId);
				}
				szValue = pFrame->Attribute("vscrollup");
				if (szValue)
				{
					m_dwUpImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwUpImageId);
				}
				szValue = pFrame->Attribute("vscrollbtn");
				if (szValue)
				{
					m_dwVertBtnImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwVertBtnImageId);
				}
				szValue = pFrame->Attribute("vscrolldown");
				if (szValue)
				{
					m_dwDownImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwDownImageId);
				}
				szValue = pFrame->Attribute("hscrollleft");
				if (szValue)
				{
					m_dwLeftImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwLeftImageId);
				}
				szValue = pFrame->Attribute("hscrollbtn");
				if (szValue)
				{
					m_dwHorzBtnImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwHorzBtnImageId);
				}
				szValue = pFrame->Attribute("hscrollright");
				if (szValue)
				{
					m_dwRightImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwRightImageId);
				}
				szValue = pFrame->Attribute("msgdlgbkg");
				if (szValue)
				{
					m_dwMsgDlgBkgImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMsgDlgBkgImageId);
				}
				szValue = pFrame->Attribute("msgdlgerror");
				if (szValue)
				{
					m_dwMsgBoxErrorImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMsgBoxErrorImageId);
				}
				szValue = pFrame->Attribute("msgdlginfo");
				if (szValue)
				{
					m_dwMsgBoxInfoImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMsgBoxInfoImageId);
				}
				szValue = pFrame->Attribute("msgdlgsucc");
				if (szValue)
				{
					m_dwMsgBoxSuccImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMsgBoxSuccImageId);
				}
				szValue = pFrame->Attribute("msgdlgquest");
				if (szValue)
				{
					m_dwMsgBoxQuestionImageId = ::atoi(szValue);
					AddBkgSrcImage(m_dwMsgBoxSuccImageId);
				}
				szValue = pFrame->Attribute("basecolor");
				if (szValue)
					m_clrBase = StringToColor(szValue); 
				bSucc = TRUE;
			}
			//get menu node	 
			pFrame = pXmlRoot->FirstChildElement();
			for ( ;pFrame != NULL;)
			{
				if (stricmp(pFrame->Value(), MENUS_ROOT_NODE_NAME) == 0)
					break;
				pFrame = pFrame->NextSiblingElement();
			}
			if (pFrame)
			{
				const char *szValue = pFrame->Attribute("menucheck");
				if (szValue)
					m_dwMenuCheckImageId = ::atoi(szValue);
			}
		} //end if (pXmlRoot)
	} //end if (m_pSkinDoc->Load(pData, dwSize)
	return bSucc;
}

BOOL CUIResource::ModifyColorStyle(int r, int g, int b)
{
	//return TRUE;
	if ((r != 0) || (g != 0) || (b != 0))
	{
		if (GetRValue(m_clrBase) == 0)
			m_crRValue = 0;
		else
			m_crRValue = r;
		if (GetGValue(m_clrBase) == 0)
			m_crGValue = 0;
		else
			m_crGValue = g;
		if (GetBValue(m_clrBase) == 0)
			m_crBValue = 0;
		else
			m_crBValue = b; 
	} else
	{
		m_crRValue = 0;
		m_crGValue = 0;
		m_crBValue = 0;
	}
	//RefreshImage();
	CPaintManagerUI::ShiftSystemTextColor(m_crRValue, m_crGValue, m_crBValue);
	//RefreshBackground();
	return TRUE;
}

void CUIResource::RefreshImage()
{
	std::map<DWORD, std::string>::iterator it;
	for (it = m_SrcImages.begin(); it != m_SrcImages.end(); it ++)
	{
		LPUI_IMAGE_ITEM pItem;
		if (GetImageById(it->first, &pItem))
		{
			//
			if (pItem->pGraphic)
			{
				pItem->pGraphic->LoadFromFile(it->second.c_str(), FALSE);
				if ((m_crRValue != 0) || (m_crGValue != 0) || (m_crBValue != 0))
				{
					if (pItem->dwSrcTransColor != 0)
					{
						COLORREF clr;
						
						clr = CPaintManagerUI::GetTransparentColor(pItem->dwSrcTransColor);
						BYTE r, g, b;
						r = GetRValue(clr);
						g = GetGValue(clr);
						b = GetBValue(clr);
						BYTE r1, g1, b1;
						r1 = (BYTE) max(0, min(255, (int)(r + m_crRValue)));
						g1 = (BYTE) max(0, min(255, (int)(g + m_crGValue)));
						b1 = (BYTE) max(0, min(255, (int)(b + m_crBValue)));
						pItem->dwTransColor = RGB(r1, g1, b1);
						//color.rgbRed = (BYTE)max(0,min(255,(int)(color.rgbRed + r)));
					    //color.rgbGreen = (BYTE)max(0,min(255,(int)(color.rgbGreen + g)));
					    //color.rgbBlue = (BYTE)max(0,min(255,(int)(color.rgbBlue + b)));
					}
					pItem->pGraphic->FillColorToImage(m_crRValue, m_crGValue, m_crBValue);
				}
			} //end if (Item.pGraphic)
		} //end if (GetImageById(it->first..
	} //end for (it = 
}

void CUIResource::RefreshBkgImage()
{
	std::map<DWORD, int>::iterator it;
	for (it = m_SrcBkgImages.begin(); it != m_SrcBkgImages.end(); it ++)
	{
		LPUI_IMAGE_ITEM pItem;
		if (GetImageById(it->first, &pItem))
		{
			//
			if (pItem->pGraphic)
			{
				std::map<DWORD, std::string>::iterator itSrc = m_SrcImages.find(it->first);
				if (itSrc != m_SrcImages.end())
				{
					pItem->pGraphic->LoadFromFile(itSrc->second.c_str(), FALSE); 
					if ((m_crRValue != 0) || (m_crGValue != 0) || (m_crBValue != 0))
					{
						if (pItem->dwSrcTransColor != 0)
						{
							COLORREF clr;
							
							clr = CPaintManagerUI::GetTransparentColor(pItem->dwSrcTransColor);
							BYTE r, g, b;
							r = GetRValue(clr);
							g = GetGValue(clr);
							b = GetBValue(clr);
							BYTE r1, g1, b1;
							r1 = (BYTE) max(0, min(255, (int)(r + m_crRValue)));
							g1 = (BYTE) max(0, min(255, (int)(g + m_crGValue)));
							b1 = (BYTE) max(0, min(255, (int)(b + m_crBValue)));
							pItem->dwTransColor = RGB(r1, g1, b1);
							//color.rgbRed = (BYTE)max(0,min(255,(int)(color.rgbRed + r)));
						    //color.rgbGreen = (BYTE)max(0,min(255,(int)(color.rgbGreen + g)));
						    //color.rgbBlue = (BYTE)max(0,min(255,(int)(color.rgbBlue + b)));
						}
						pItem->pGraphic->FillColorToImage(m_crRValue, m_crGValue, m_crBValue);
					} //end if ((m_crRValue
				} //end if (itSrc != ...
			} //end if (Item.pGraphic)
		} //end if (GetImageById(it->first..
	} //end for (it = 
}
	
void CUIResource::RefreshBackground()
{
	std::map<DWORD, int>::iterator it;
	for (it = m_SrcBkgImages.begin(); it != m_SrcBkgImages.end(); it ++)
	{
		LPUI_IMAGE_ITEM pItem;
		if (GetImageById(it->first, &pItem))
		{
			//
			if (pItem->pGraphic)
			{
				std::map<DWORD, std::string>::iterator itSrc = m_SrcImages.find(it->first);
				if (itSrc != m_SrcImages.end())
				{
					pItem->pGraphic->LoadFromFile(itSrc->second.c_str(), FALSE);
					/*if (!m_MixGraphic.IsEmpty())
						pItem->pGraphic->MixImage(&m_MixGraphic, TRUE);*/
				} //end if (itSrc != ...
			} //end if (Item.pGraphic)
		} //end if (GetImageById(it->first..
	} //end for (it = 
}

BOOL CUIResource::MixBackGround(const char *szImageFile)
{
	BOOL bSucc = FALSE;
	if (szImageFile)
	{
		
	} else
	{
	    //m_MixGraphic.ClearImage();
		bSucc = TRUE;
	}
	RefreshBackground();
	return bSucc;
}

BOOL CUIResource::NeedMixBackground()
{
	return FALSE;
}

//载入图片
BOOL CUIResource::LoadImages()
{
	return FALSE;
}


//协议描述
//<UIRoot .....
//	<FrameWindow ...
//		<Container xsi::type="ImageCanvas" ...
//			<Container xsi::type="VerticalLayout"
//				<Container xsi::type="caption"...
//					.....
//				</Container>
//			</Container>
//		</Container>
//		<Container xsi::type="any_container_type" name="LogonWindow"...
//			.....
//		</Container>
//		<Container xsi::type="any_container_type" name="MainWindow" ...
//			.....
//		</Container>
//		......

//	</FrameWindow>
//</UIRoot>
//创建窗口节点
LPCONTROLNODE CUIResource::CreateWindowBkgForm(LPCONTROLNODE *pCaptionNode, const char *szFormName)
{
	if ((!m_pSkinDoc) || (!szFormName))
		return NULL;
	//find frame window
	TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
	if (!pXmlRoot)
		return NULL;

	TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (stricmp(pFrame->Value(), FRAME_WINDOW_ROOT_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}
	if (!pFrame)
		return NULL;

	//创建根节点-画布,即显示的时候从此节点开始
	TiXmlElement *pCanvas = pFrame->FirstChildElement();
	for (; pCanvas != NULL;)
	{
		const char *szType = pCanvas->Attribute("xsi:type");
		if (szType != NULL)
		{
			if ((::stricmp(szType, "ImageCanvas") == 0) || ::stricmp(szType, "WhiteCanvas") == 0)
			{
				const char * name = pCanvas->Attribute("name");
				if ((name != NULL) && (stricmp(szFormName, name) == 0))
					break;
			}
		}
		pCanvas = pCanvas->NextSiblingElement();
	}

	if (!pCanvas)
		return NULL;
	//画布下的唯一容器
	if (pCanvas->Attribute("image") != NULL)
	{
		AddBkgSrcImage(atoi(pCanvas->Attribute("image")));
	}
	TiXmlElement* pVert = pCanvas->FirstChildElement();
	TiXmlElement* pCaption = NULL;
	if (pVert)
	{ 
		//标题节点
		pCaption = pVert->FirstChildElement();
		if (pCaption)
		{
			const char *capName = pCaption->Attribute("name"); 
		}
	}
	//创建根节点
	LPCONTROLNODE pRootNode = CreateNode(pCanvas, NULL, FALSE);
	if (pRootNode)
	{
		if (pVert)
		{
			LPCONTROLNODE pVertNode = CreateNode(pVert, pRootNode, TRUE);
			if (pVertNode)
			{
				//创建标题节点及其子节点
				if (pCaption)
				{
					*pCaptionNode = CreateNode(pCaption, pVertNode, TRUE);
					AddLeftChildNode(*pCaptionNode, pCaption);
				} //end if (pCaption)
			} //end if (pVertNode)
		} //end if (pRootNode)
	}
	return pRootNode;
}

//加入源图
void CUIResource::AddBkgSrcImage(const int nImageId)
{
	std::map<DWORD, int>::iterator it = m_SrcBkgImages.find(nImageId);
	if (it == m_SrcBkgImages.end())
	{
		m_SrcBkgImages.insert(std::pair<DWORD, int>(nImageId, nImageId)); 
	} //end if (it == ...
}

void CUIResource::AddSrcImage(const int nImageId)
{
	std::map<DWORD, std::string>::iterator it = m_SrcImages.find(nImageId);
	if (it == m_SrcImages.end())
	{ 
		LPUI_IMAGE_ITEM pItem;
		if (GetImageById(nImageId, &pItem))
		{ 
			m_SrcImages.insert(std::pair<DWORD, std::string>(nImageId, pItem->m_strFileName));
		} //end if (GetImageById(...
	} //end if (it == ...
}

//初始化源图
void CUIResource::InitSrcImage()
{
	if (!m_pSkinDoc)
		return ;
	//find frame window
	TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
	if (!pXmlRoot)
		return;

	TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (strcmp(pFrame->Value(), FRAME_WINDOW_ROOT_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}
	if (pFrame)
	{
		TiXmlElement *pBkg = pFrame->FirstChildElement();
		for (; pBkg != NULL; )
		{
			if (::stricmp(pBkg->Attribute("xsi:type"), "ImageCanvas") == 0)
			{
				int id = ::atoi(pBkg->Attribute("image"));
				if (id > 0)
				{
					AddSrcImage(id);
				} //end if (id > 0)
			} //end if (::stricmp(pBkg->Attribute(...
			pBkg = pBkg->NextSiblingElement();
		} //end for (; pBkg != NULL; )
	} //end if (pFrame)
}

LPCONTROLNODE CUIResource::CreateWindowNode(const char *szWindowName, SIZE &szMin, SIZE &szMax, SIZE &szCaption, UINT &uBkgImageId)
{
	std::map<CAnsiString_, LPCONTROLNODE>::iterator it = m_WindowList.find(szWindowName);
	if (it != m_WindowList.end())
	{
		std::map<CAnsiString_, SIZE>::iterator itSize = m_WindowMinSize.find(szWindowName);
		if (itSize != m_WindowMinSize.end())
			szMin = itSize->second;
		itSize = m_WindowMaxSize.find(szWindowName);
		if (itSize != m_WindowMaxSize.end())
			szMax = itSize->second;
		itSize = m_WindowCaptionSize.find(szWindowName);
		if (itSize != m_WindowCaptionSize.end())
			szCaption = itSize->second;
		std::map<CAnsiString_, UINT>::iterator itBkgImage = m_WindowBkgImages.find(szWindowName);
		if (itBkgImage != m_WindowBkgImages.end())
			uBkgImageId = itBkgImage->second;
		return it->second;
	}


	TiXmlElement *pClientRoot = FindWindowNodeByName(szWindowName);
 
	BOOL bRet = FALSE;

	LPCONTROLNODE pRootNode = NULL;
	//构建客户区域节点结构
	if (pClientRoot)
	{
		LPCONTROLNODE pCaptionNode = NULL;
		pRootNode = CreateWindowBkgForm(&pCaptionNode, pClientRoot->Attribute("style"));
		if (pClientRoot->Attribute("MinSize"))
		{
			char *p = NULL;
			//szMin.cx = 
			szMin.cx = strtol(pClientRoot->Attribute("MinSize"), &p, 10);
			if (p)
				szMin.cy = strtol(p + 1, &p, 10);
			m_WindowMinSize[szWindowName] = szMin;
		}
		if (pClientRoot->Attribute("MaxSize"))
		{
			char *p = NULL;
			szMax.cx = strtol(pClientRoot->Attribute("MaxSize"), &p, 10);
			if (p)
				szMax.cy = strtol(p + 1, &p, 10);
		    m_WindowMaxSize[szWindowName] = szMax;
		}
		if (pClientRoot->Attribute("CaptionSize"))
		{
			char *p = NULL; 
			szCaption.cx = strtol(pClientRoot->Attribute("CaptionSize"), &p, 10);
			if (p)
				szCaption.cy = strtol(p + 1, &p, 10);
			m_WindowCaptionSize[szWindowName] = szCaption;
		}
		if (pClientRoot->Attribute("windowbackimage"))
		{
			uBkgImageId = ::atoi(pClientRoot->Attribute("windowbackimage"));
			m_WindowBkgImages[szWindowName] = uBkgImageId;
		}
		LPCONTROLNODE pClientRootNode;
		if (pCaptionNode)
		    pClientRootNode = CreateNode(pClientRoot, pCaptionNode, FALSE);
		else
			pClientRootNode = CreateNode(pClientRoot, pRootNode, TRUE);
		AddLeftChildNode(pClientRootNode, pClientRoot);
		bRet = TRUE;
	} //end if (pCaptionNode...
 
 

	if (bRet)
	{
		m_WindowList[szWindowName] = pRootNode;		
		return pRootNode;
	} else
	{
		ReleaseChildNodes(pRootNode);
		return NULL;
	} //end if (bRet)
}


BOOL CUIResource::AddChildWindow(TiXmlElement *pXmlNode)
{
	//find frame window
	if (!m_pSkinDoc)
		return FALSE;
	TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
	if (!pXmlRoot)
		return FALSE;

	TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (stricmp(pFrame->Value(), FRAME_WINDOW_ROOT_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}

	if (pFrame)
	{
		pFrame->InsertEndChild(*pXmlNode);
		return TRUE; 
	} else
		return FALSE;
}

TiXmlElement *CUIResource::FindWindowNodeByName(const char *szWindowName)
{
	//find frame window
	if (!m_pSkinDoc)
		return NULL;
	TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
	if (!pXmlRoot)
		return NULL;

	TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (stricmp(pFrame->Value(), FRAME_WINDOW_ROOT_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}
	if (!pFrame)
		return NULL;

	//FrameWindow 节点
	TiXmlElement *pEle = pFrame->FirstChildElement();
 
	for (; pEle != NULL;)
	{
		const char * name = pEle->Attribute("name");
		if ((name != NULL) && (stricmp(szWindowName, name) == 0))
			break;

		pEle = pEle->NextSiblingElement();
	}
	return pEle;
}

//创建菜单节点
LPCONTROLNODE CUIResource::CreateMenuNodes(const char *szMenuName)
{
	std::map<CAnsiString_, LPCONTROLNODE>::iterator it = m_WindowList.find(szMenuName);
	if (it != m_WindowList.end())
	{
		return it->second;
	} 
		
	TiXmlElement *pMenu = GetMenuNodeByName(szMenuName);
	if (!pMenu)
		return NULL;

	BOOL bRet = FALSE;
	//创建根节点
	LPCONTROLNODE pRootNode = CreateNode(pMenu, NULL, FALSE);
	if (pRootNode)
	{
		AddLeftChildNode(pRootNode, pMenu);
		bRet = TRUE;
	}

	if (bRet)
	{
		m_WindowList[szMenuName] = pRootNode;
		return pRootNode;
	} else
	{
		ReleaseChildNodes(pRootNode);
		return NULL;
	}
}

TiXmlElement *CUIResource::GetMenuRootNode()
{
	if (m_pSkinDoc)
	{
		TiXmlElement* pSkinRoot = m_pSkinDoc->RootElement();
		if (!pSkinRoot)
			return NULL;
		TiXmlElement *pFrame = pSkinRoot->FirstChildElement();
		for (;pFrame != NULL;)
		{
			if (stricmp(pFrame->Value(), MENUS_ROOT_NODE_NAME) == 0)
				break;
			pFrame = pFrame->NextSiblingElement();
		}
		return pFrame;
	}
	return NULL;
}

TiXmlElement *CUIResource::GetMenuNodeByName(const char *szMenuName)
{
	//find root element	
	TiXmlElement *pFrame = GetMenuRootNode();
	if (!pFrame)
		return NULL;
	//根据uiName寻找菜单
	TiXmlElement * pMenu = pFrame->FirstChildElement();
	if (!pMenu)
		return NULL;
	for (; pMenu != NULL; pMenu = pMenu->NextSiblingElement())
	{
		const char* name = pMenu->Attribute("name");
		if ((name != NULL) && (::stricmp(szMenuName, name) == 0))
			break;
	}
	//find the menu specified by uiName
	return pMenu;
}

//释放窗口、菜单节点
void CUIResource::ReleaseWindowNodes(const char *szMenuName)
{
	std::map<CAnsiString_, LPCONTROLNODE>::iterator it = m_WindowList.find(szMenuName);
	if (it != m_WindowList.end())
	{
		LPCONTROLNODE pRoot = it->second;
		m_WindowList.erase(it);
		CSkinNodeOperator::FreeNode(pRoot);
	}
}

 

BOOL CUIResource::IsBackgroundImage(const int nImageId)
{
	std::map<DWORD, int>::iterator it = m_SrcBkgImages.find(nImageId);
	if (it != m_SrcBkgImages.end())
		return TRUE;
	return FALSE;
}

//删除图片
BOOL CUIResource::DeleteImage(DWORD dwImageId)
{
	return FALSE;
}

//获取图片数据
BOOL CUIResource::GetImageById(DWORD dwImageId, LPUI_IMAGE_ITEM *pImage)
{
	std::map<DWORD, LPUI_IMAGE_ITEM>::iterator it = m_ImageList.find(dwImageId);
	if( it != m_ImageList.end() )
	{	
		if (!it->second->pGraphic)
		{
			it->second->pGraphic = NEWIMAGE;
			COLORREF clr = clr = CPaintManagerUI::GetTransparentColor(it->second->dwSrcTransColor);; 
			it->second->pGraphic->SetImageMask(clr);
			it->second->pGraphic->LoadFromFile(it->second->m_strFileName.c_str(), FALSE);
		}
		*pImage = it->second;  
		return TRUE;
	}
	{
		CGuardLock::COwnerLock guard(m_LinkLock); 
		it = m_LinkImageList.find(dwImageId);
		if (it != m_LinkImageList.end())
		{
			*pImage = it->second;
			return TRUE;
		}
	}
	return FALSE;
}

//static 
COLORREF CUIResource::StringToColor(const char *szValue)
{
	COLORREF clr = 0;
	if (*szValue == '#')
	{
		szValue++;
		clr = ::strtol(szValue, const_cast<char **>(&szValue), 16);		 
	}
	return clr;
}

//根据扩展名获取图像类型
DWORD CUIResource::GetImageTypeByExt(const char *szExt)
{
	DWORD dwImageType = 0;	
	return dwImageType;
}

void CUIResource::GetBlendColorValue(int &r, int &g, int &b)
{
	r = m_crRValue;
	g = m_crGValue;
	b = m_crBValue;
}

//设置皮肤路径
void CUIResource::SetSkinPath(const char *szPath)
{
	if (szPath)
	{
		m_strSkinPath = szPath;
		if (szPath[strlen(szPath) - 1] != '\\')
			m_strSkinPath.append("\\");
	} else
		m_strSkinPath.clear();
}

DWORD CUIResource::GetMaxBtnImageId()
{
	return m_dwMaxBtnImageId;
}

DWORD CUIResource::GetMinBtnImageId()
{
	return m_dwMinBtnImageId;
}

DWORD CUIResource::GetRestoreBtnImageId()
{
	return m_dwRestoreBtnImageId;
}

DWORD CUIResource::GetCloseBtnImageId()
{
	return m_dwCloseBtnImageId;
}

DWORD CUIResource::GetFormBkgImageId()
{
	return m_dwFormBkgImageId;
}

DWORD CUIResource::GetMsgBoxSuccImageId()
{
	return m_dwMsgBoxSuccImageId;
}

DWORD CUIResource::GetMsgBoxQuestionImageId()
{
	return m_dwMsgBoxQuestionImageId;
}

DWORD CUIResource::GetMsgBoxInfoImageId()
{
	return m_dwMsgBoxInfoImageId;
}

DWORD CUIResource::GetMsgBoxErrorImageId()
{
	return m_dwMsgBoxErrorImageId;
}

DWORD CUIResource::GetMessageDlgBkgImageId()
{
	return m_dwMsgDlgBkgImageId;
}

DWORD CUIResource::GetMenuCheckImageId()
{
	return m_dwMenuCheckImageId;
}

void CUIResource::GetScrollBarImage(UINT &uPrior, UINT &uMid, UINT &uNext, BOOL bVert)
{
	if (bVert)
	{
		uPrior = m_dwUpImageId;
		uMid = m_dwVertBtnImageId;
		uNext = m_dwDownImageId;
	} else
	{
		uPrior = m_dwLeftImageId;
		uMid = m_dwHorzBtnImageId;
		uNext = m_dwRightImageId;
	}
}

#pragma warning(default:4996)
