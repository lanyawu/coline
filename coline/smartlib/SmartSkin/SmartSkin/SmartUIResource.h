#ifndef __SMARTUIRESOURCE_H___
#define __SMARTUIRESOURCE_H___

#include <UIlib/UILib.h>

const TCHAR SMART_TOOLTIP_WINDOW_NAME[] = L"HintWindow";
const char SMART_TOOLTIP_WINDOW_NODE_NAME[] = "HintWindow";
const COLORREF STD_TRANSPARENT_CLR = RGB(255, 0, 255);

class CSmartUIResource :public CUIResource
{
public:
	CSmartUIResource(void);
	~CSmartUIResource(void);
public:
    static CSmartUIResource *Instance();
	static void FreeInstance();
public:
	virtual BOOL LoadImages(); //载入图片
    virtual BOOL LoadSkinDoc(const char *pData, DWORD dwSize); //载入XML数据

	DWORD GetHintWindowBkgImageId();
	//
	BOOL AddPluginSkin(const char *szXmlString);
	//
	int AddLinkGraphic(const char *szFileName, int nSubCount, COLORREF clrParent);

	virtual DWORD GetLinkImageIdByLink(LPCTSTR lpszLink);
	//
	void SetLinkCallBack(LPSKIN_GET_IMAGE_ID_BY_LINK pLinkCallBack, LPVOID pOverlapped);
public:
	//装载一个图片数据
	BOOL LoadImageData(const DWORD dwImageFlag, const char *szImagePath, char **pImageData, 
		             DWORD &dwImageSize, DWORD &dwImageType, std::string &strFileName);
private:
	BOOL DefLoadImage(const DWORD dwImageFlag, const char *szImagePath, 
		        char **pImageData, DWORD &dwImageSize, DWORD &dwImageType);
	BOOL ReadImagesFromNode(TiXmlElement *pItem);
	//
	BOOL AddPluginToFrameWindow(TiXmlDocument &xmlDoc);
	//
	BOOL AddPluginToMenu(TiXmlDocument &xmlDoc);

private:
	//常用窗口图像ID
	static CSmartUIResource *m_pResource;
	DWORD m_dwHintWinBkgImageId;
	LPSKIN_GET_IMAGE_ID_BY_LINK m_pLinkCallBack;
	LPVOID m_pOverlapped;
};

#endif