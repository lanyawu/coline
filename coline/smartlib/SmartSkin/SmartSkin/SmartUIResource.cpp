#include <commonlib/debuglog.h>
#include <commonlib/systemutils.h>
#include <commonlib/stringutils.h>
#include <fstream>
#include "SmartUIResource.h"
 
const char *IMAGES_NODE_NAME = "Images";
//布局
const char *IMAGE_LAYOUT_TYPE_VERT = "vertical";
const char *IMAGE_LAYOUT_TYPE_HORZ = "horizontal";

#pragma warning(disable:4996)

CSmartUIResource *CSmartUIResource::m_pResource = NULL;

CSmartUIResource::CSmartUIResource(void):
                  m_dwHintWinBkgImageId(0),
				  m_pLinkCallBack(NULL)
{
}

CSmartUIResource::~CSmartUIResource(void)
{
}

CSmartUIResource *CSmartUIResource::Instance()
{
	if (!m_pResource)
		m_pResource = new CSmartUIResource();
	return m_pResource;
}

void CSmartUIResource::FreeInstance()
{
	if (m_pResource)
	{
		delete m_pResource;
		m_pResource = NULL;
	}
}

//
void CSmartUIResource::SetLinkCallBack(LPSKIN_GET_IMAGE_ID_BY_LINK pLinkCallBack, LPVOID pOverlapped)
{
	m_pLinkCallBack = pLinkCallBack;
	m_pOverlapped = pOverlapped;
}

DWORD CSmartUIResource::GetLinkImageIdByLink(LPCTSTR lpszLink)
{
	if (m_pLinkCallBack)
		return m_pLinkCallBack(lpszLink, m_pOverlapped);
	return 0;
}

//装载一个图片数据
BOOL CSmartUIResource::LoadImageData(const DWORD dwImageFlag, const char *szImagePath, char **pImageData, 
	             DWORD &dwImageSize, DWORD &dwImageType, std::string &strFileName)
{
	if (szImagePath && (szImagePath[0] != '\0'))
	{
		if ((m_strSkinPath.size() > 0) && (szImagePath[0] == '.') && (szImagePath[1] == '\\'))
		{
			const char *p = szImagePath + 2;
			strFileName = m_strSkinPath;
			if (*p)
				strFileName += p; 

			return DefLoadImage(dwImageFlag, strFileName.c_str(), pImageData, dwImageSize, dwImageType);
		} else
			return DefLoadImage(dwImageFlag, szImagePath, pImageData, dwImageSize, dwImageType);
	} else
		return DefLoadImage(dwImageFlag, NULL, pImageData, dwImageSize, dwImageType);
}

BOOL CSmartUIResource::DefLoadImage(const DWORD dwImageFlag, const char *szImagePath, 
	        char **pImageData, DWORD &dwImageSize, DWORD &dwImageType)
{
	BOOL bSucc = FALSE;
	if (szImagePath && CSystemUtils::FileIsExists(szImagePath))
	{
		char szExt[MAX_PATH] = {0};
		CSystemUtils::ExtractFileExtName(szImagePath, szExt, MAX_PATH - 1);
		dwImageType = GetImageTypeByExt(szExt);
		TCHAR szwFileName[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szImagePath, szwFileName, MAX_PATH - 1);
		ifstream ifs(szwFileName, std::ios::binary);
		if (ifs.is_open())
		{
			ifs.seekg(0, std::ios::end);
			dwImageSize = (DWORD) ifs.tellg();
			if (dwImageSize > 0)
			{
				*pImageData = new char[dwImageSize];
				ifs.seekg(0, std::ios::beg);
				ifs.read(*pImageData, dwImageSize);
				bSucc = (ifs.gcount() == dwImageSize);
			}
			ifs.close();
		}
	}
	return bSucc;
}

DWORD CSmartUIResource::GetHintWindowBkgImageId()
{
	return m_dwHintWinBkgImageId;
}

//载入XML数据
BOOL CSmartUIResource::LoadSkinDoc(const char *pData, DWORD dwSize)
{
	if (CUIResource::LoadSkinDoc(pData, dwSize))
	{
		TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
		if(!pXmlRoot)
			return FALSE;
		//find Images
		TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
		for (;pFrame != NULL;)
		{
			if (stricmp(pFrame->Value(), SMART_TOOLTIP_WINDOW_NODE_NAME) == 0)
				break;
			pFrame = pFrame->NextSiblingElement();
		}
		if (pFrame)
		{
			const char *szValue = pFrame->Attribute("background");
			if (szValue)
				m_dwHintWinBkgImageId = ::atoi(szValue);
			TiXmlElement *pCanvas = pFrame->FirstChildElement();
			if (pCanvas)
			{
				//创建根节点
				LPCONTROLNODE pRootNode = CreateNode(pCanvas, NULL, FALSE);
				if (pRootNode)
				{
					AddLeftChildNode(pRootNode, pCanvas);
					m_WindowList[SMART_TOOLTIP_WINDOW_NODE_NAME] = pRootNode;		
				    return TRUE;
				} //end if (pRootNode)
			} //end if (pCanvas)
		} //end if (pFrame)
	}  //end if (CUIResource::..
	return FALSE;
}

BOOL CSmartUIResource::AddPluginToFrameWindow(TiXmlDocument &xmlDoc)
{
	//find frame window
	TiXmlElement *pXmlRoot = xmlDoc.RootElement();
	if (!pXmlRoot)
		return NULL;

	TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (strcmp(pFrame->Value(), FRAME_WINDOW_ROOT_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}
	if (pFrame)
	{
		//
		TiXmlElement *pPlugWindow = pFrame->FirstChildElement();
		while (pPlugWindow)
		{
			const char *szType = pPlugWindow->Attribute("xsi:type");
			if (szType && ::stricmp(szType, "ImageCanvas") == 0)
			{
				AddChildWindow(pPlugWindow);
				int id = ::atoi(pPlugWindow->Attribute("image"));
				if (id > 0)
				{
					AddBkgSrcImage(id);
					AddSrcImage(id);
				} //end if (id > 0)
			} else if (szType && ::stricmp(szType, "WhiteCanvas") == 0)
			{
				AddChildWindow(pPlugWindow);
			} else
			{
				const char *szWindowName = pPlugWindow->Attribute("name");
				TiXmlElement *pSrcWindow = FindWindowNodeByName(szWindowName);
				if (pSrcWindow)
				{
					TiXmlElement *pLevel = pPlugWindow->FirstChildElement();
					TiXmlElement *pSrcLevel = pSrcWindow->FirstChildElement();
					TiXmlElement *pPushLevel = pSrcLevel;
					while (pLevel && pSrcLevel)
					{
						const char *pType = pLevel->Attribute("xsi:type");
						const char *pSeq = pLevel->Attribute("seq");					
						if (pType && pSeq)
						{
							int nSeq = ::atoi(pSeq);
							
							while (pSrcLevel)
							{							
								const char *pSrcType = pSrcLevel->Attribute("xsi:type");
								if (pSrcType && (::strcmp(pType, pSrcType) == 0))
									nSeq --;
								if (nSeq <= 0)
									break;
								pSrcLevel = pSrcLevel->NextSiblingElement();
							} //end while (pSrcLevel)
							if (pSrcLevel)
							{
								const char *pIsPlugNode = pLevel->Attribute("plugin");
								if (pIsPlugNode && (::stricmp(pIsPlugNode, "true") == 0))
								{
									pPushLevel->InsertBeforeChild(pSrcLevel, *pLevel);
									break;
								} else
								{
									pPushLevel = pSrcLevel;
									pSrcLevel = pSrcLevel->FirstChildElement();
									pLevel = pLevel->FirstChildElement();
									if (!pSrcLevel)
									{
										const char *pIsPlugNode = pLevel->Attribute("plugin");
										if (pIsPlugNode && (::stricmp(pIsPlugNode, "true") == 0))
										{
											pPushLevel->InsertEndChild(*pLevel);
											break;
										} //end if (pIsPlugNode && (::...
									} //end if (!pSrcLevel)
								} //end else if (pIsPlugNode && (::stricmp(pIsPlugNode, "true") == 0))
							}  else
							{
								const char *pIsPlugNode = pLevel->Attribute("plugin");
								if (pIsPlugNode && (::stricmp(pIsPlugNode, "true") == 0))
								{
									pPushLevel->InsertEndChild(*pLevel);
									break;
								} //end if (pIsPlugNode...
							} //end else if (pSrcLevel)
						} else
						{
							PRINTDEBUGLOG(dtInfo, "Plugin Xml Failed, no Type or Seq Node");
							break;
						} //end else if (pType && pLevel)
					} //end while (pLevel && pSrcLevel)
				} else 
				{
					AddChildWindow(pPlugWindow);
				}//end else if (pSrcWindow)
			} //end else if (szType && ::stricmp(szType, "ImageCanvas") == 0)

			pPlugWindow = pPlugWindow->NextSiblingElement();
		} //end while (pPlugWindow)
		
		return TRUE;		
	} //end if (pFrame)
	return FALSE;
}

//
BOOL CSmartUIResource::AddPluginToMenu(TiXmlDocument &xmlDoc)
{
	//find root element
	TiXmlElement* pSkinRoot = xmlDoc.RootElement();
	if (!pSkinRoot)
		return NULL;
	TiXmlElement *pFrame = pSkinRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (strcmp(pFrame->Value(), MENUS_ROOT_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}

	if (pFrame) 
	{
		TiXmlElement *pCurrMenu = pFrame->FirstChildElement();
		while (pCurrMenu)
		{
			const char *szMenuName = pCurrMenu->Attribute("name");
			if (szMenuName)
			{
				TiXmlElement *pSrcMenu = GetMenuNodeByName(szMenuName);
				if (pSrcMenu)
				{
					TiXmlElement *pMenuItem  = pCurrMenu->FirstChildElement();
					while (pMenuItem)
					{
						const char *szSeq = pMenuItem->Attribute("seq");
						if (szSeq)
						{
							int nSeq = ::atoi(szSeq);
							TiXmlElement *pSrcItem = pSrcMenu->FirstChildElement();
							nSeq --;
							while (pSrcItem && (nSeq > 0))
							{								
								pSrcItem = pSrcItem->NextSiblingElement();
								nSeq --;
							}
							if (pSrcItem)
							{
								pSrcMenu->InsertBeforeChild(pSrcItem, *pMenuItem);
							} else
							{
								pSrcMenu->InsertEndChild(*pMenuItem);
							}
						}
						pMenuItem = pMenuItem->NextSiblingElement();
					}
				} else
				{
					TiXmlElement *pSrcMenuRoot = GetMenuRootNode();
					if (pSrcMenuRoot)
						pSrcMenuRoot->InsertEndChild(*pCurrMenu);
				}
				pCurrMenu = pCurrMenu->NextSiblingElement();
			}
		}
	} //end if (pFrame)

	return FALSE;
}

//
BOOL CSmartUIResource::AddPluginSkin(const char *szXmlString)
{
	if (szXmlString)
	{
		TiXmlDocument xmlDoc;
		if (xmlDoc.Load(szXmlString, (int) ::strlen(szXmlString)))
		{
			//read images
			TiXmlElement *pXmlRoot = xmlDoc.RootElement();
			if(!pXmlRoot)
				return FALSE;
			//find Images
			TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
			for (;pFrame != NULL;)
			{
				if (strcmp(pFrame->Value(), IMAGES_NODE_NAME) == 0)
					break;
				pFrame = pFrame->NextSiblingElement();
			}
			if (pFrame)
			{
				TiXmlElement *pItem = pFrame->FirstChildElement();
				ReadImagesFromNode(pItem);
			}

			//FrameWindow
			AddPluginToFrameWindow(xmlDoc); 

			//menu
			AddPluginToMenu(xmlDoc);
		}
		//m_pSkinDoc->SaveFile("F:\\a.xml");
		return TRUE;
	}
	return FALSE;	
}

BOOL CSmartUIResource::LoadImages()
{
	if (!m_pSkinDoc)
		return FALSE; 
	
	TiXmlElement *pXmlRoot = m_pSkinDoc->RootElement();
	if (!pXmlRoot)
		return FALSE;
	//find Images
	TiXmlElement *pFrame = pXmlRoot->FirstChildElement();
	for (;pFrame != NULL;)
	{
		if (strcmp(pFrame->Value(), IMAGES_NODE_NAME) == 0)
			break;
		pFrame = pFrame->NextSiblingElement();
	}
	if (!pFrame)
		return FALSE;
	TiXmlElement *pItem = pFrame->FirstChildElement();
	BOOL bSucc = ReadImagesFromNode(pItem);
	//初始化源图
	InitSrcImage();
	return bSucc;
}

//
int CSmartUIResource::AddLinkGraphic(const char *szFileName, int nSubCount, COLORREF clrParent)
{
	LPUI_IMAGE_ITEM pImage = new UI_IMAGE_ITEM();
	memset(pImage, 0, sizeof(UI_IMAGE_ITEM));
	pImage->pGraphic = NEWIMAGE;
	pImage->pGraphic->SetImageMask(clrParent);
	if (pImage->pGraphic->LoadFromFile(szFileName, FALSE))
	{
		CGuardLock::COwnerLock guard(m_LinkLock);
		m_dwCurrLinkImageId ++;
		pImage->dwImageId = m_dwCurrLinkImageId;
		pImage->m_strFileName = szFileName;
		pImage->dwSubCount = nSubCount;
		pImage->dwSrcTransColor = clrParent;
		m_LinkImageList.insert(std::pair<DWORD, LPUI_IMAGE_ITEM>(m_dwCurrLinkImageId, pImage));
		return m_dwCurrLinkImageId;
	}
	return 0;
}

//
BOOL CSmartUIResource::ReadImagesFromNode(TiXmlElement *pItem)
{
	const char *pValue;
	char *pImageData;
	DWORD dwImageSize;
	DWORD dwImageType;
	while (pItem)
	{
		pValue = pItem->Attribute("id");
		if (pValue)
		{
			LPUI_IMAGE_ITEM pImage = new UI_IMAGE_ITEM();
			memset(pImage, 0, sizeof(UI_IMAGE_ITEM));
			pImage->dwImageId = atoi(pValue);
			if (pImage->dwImageId > 0)
			{
				std::map<DWORD, LPUI_IMAGE_ITEM>::iterator it = m_ImageList.find(pImage->dwImageId);
				if (it == m_ImageList.end())
				{
					pValue = pItem->Attribute("name");
					if (pValue)
					{
						strncpy(pImage->szName, pValue, MAX_IMAGE_NAME - 1);
					}
					pValue = pItem->Attribute("subcount");
					if (pValue)
						pImage->dwSubCount = atoi(pValue);
					else
						pImage->dwSubCount = 1;
					pValue = pItem->Attribute("layout");
					if (pValue)
					{
						if (strcmp(IMAGE_LAYOUT_TYPE_HORZ, pValue) == 0)
							pImage->dwListFmt = ILF_HORIZONTAL;
					}
					pValue = pItem->Attribute("transcolor");
					if (pValue)
					{
						pImage->dwSrcTransColor = StringToColor(pValue);
						pImage->dwTransColor = pImage->dwSrcTransColor;
					}
					pValue = pItem->Attribute("colorshift");
					if (pValue && ::stricmp(pValue, "true") == 0)
					{
						AddBkgSrcImage(pImage->dwImageId);
					}
					pValue = pItem->Attribute("path");
					if (LoadImageData(pImage->dwImageId, pValue, &pImageData, dwImageSize, dwImageType, pImage->m_strFileName))
					{
						if (pImage->m_strFileName.empty())
						{
							pImage->pGraphic = new CGraphicPlus();
							pImage->pGraphic->SetImageMask(pImage->dwSrcTransColor);
							if (pImage->pGraphic->LoadFromBuff(pImageData, dwImageSize, dwImageType))
							{
								pImage->dwImageType = dwImageType;
								pImage->dwImageSize = dwImageSize; 
								 
							} else
							{
								delete pImage->pGraphic;
							}
						}
						m_ImageList.insert(std::pair<DWORD, LPUI_IMAGE_ITEM>(pImage->dwImageId, pImage));
						AddSrcImage(pImage->dwImageId);
						if (pImageData)
							delete []pImageData; 
					} else
						delete pImage;
				} else
				{
					PRINTDEBUGLOG(dtInfo, "Image id(%d) is exists", pImage->dwImageId);
					delete pImage;
				}
			} else
				delete pImage;
		}  else
		{
			PRINTDEBUGLOG(dtInfo, "image node(id) is not exists");
		}
		pItem = pItem->NextSiblingElement();
	}
	return TRUE;
}

#pragma warning(default:4996)
