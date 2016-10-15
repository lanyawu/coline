#ifndef __UIRESOURCE_H___
#define __UIRESOURCE_H___

#include <map>
#include <string>
#include <xml/tinyxml.h>
#include <commonlib/types.h>
#include <Commonlib/stringutils.h>
#include <commonlib/systemutils.h>
#include <commonlib/GuardLock.h> 
#include <SmartSkin/SmartSkin.h>

#define STD_IMAGE__
#ifdef STD_IMAGE__
#include <commonlib/stdimage.h>
#define NEWIMAGE new CStdImage();
#else
#include <commonlib/graphicplus.h>
#define NEWIMAGE new CGraphicPlus();
#endif

const char FRAME_WINDOW_ROOT_NODE_NAME[] = "FrameWindow"; //窗口根节点
const char MENUS_ROOT_NODE_NAME[]  = "Menus";  //菜单根节点

//提示信息定义
#define  MBI_OK           0x00000001
#define  MBI_OKCANCEL     0x00000002
#define  MBI_SUCCESS      0x00000010
#define  MBI_QUESTION     0x00000020
#define  MBI_INFORMATION  0x00000040 
#define  MBI_ERROR        0x00000080

//
#define  ILF_VERTICAL     0 //
#define  ILF_HORIZONTAL   1 //

//
#define  ONLINE_STATUS_TYPE_OFFLINE  0
#define  ONLINE_STATUS_TYPE_HIDE     1
#define  ONLINE_STATUS_TYPE_LEAVE    2
#define  ONLINE_STATUS_TYPE_BUSY     3
#define  ONLINE_STATUS_TYPE_MOBILE   4
#define  ONLINE_STATUS_TYPE_ONLINE   255

#define MAX_IMAGE_NAME  16

typedef struct CUIImageItem
{
	char  szName[MAX_IMAGE_NAME];
	DWORD dwImageId;
	std::string m_strFileName;
	IImageInterface *pGraphic;
	DWORD dwListFmt; //图片排列方式
	DWORD dwTransColor; //透明颜色
	DWORD dwSrcTransColor; //源透色
	DWORD dwSubCount;   //包含图片个数
	DWORD dwImageType; //图片类型
	DWORD dwImageSize; //图片大小
}UI_IMAGE_ITEM, *LPUI_IMAGE_ITEM;

//客户端程序所有的控件类型
typedef enum ControlType
{
	CONTROL_TYPE_FIRST_ = 0,
	CONTROL_TYPE_LIST = 1,
	CONTROL_TYPE_CANVAS,
	CONTROL_TYPE_BUTTON,
	CONTROL_TYPE_OPTION,
	CONTROL_TYPE_TOOLBAR,
	CONTROL_TYPE_TABPAGE,
	CONTROL_TYPE_ACTIVEX,
	CONTROL_TYPE_TREEVIEW,
	CONTROL_TYPE_DROPDOWN,
	CONTROL_TYPE_FADEDLINE,
	CONTROL_TYPE_TASKPANEL,
	CONTROL_TYPE_STATUSBAR,
	CONTROL_TYPE_TABFOLDER,
	CONTROL_TYPE_TEXTPANEL,
	CONTROL_TYPE_RICHEDIT2,
	CONTROL_TYPE_SCINTILLAEDIT,
	CONTROL_TYPE_LISTHEADER,
	CONTROL_TYPE_LISTFOOTER,
	CONTROL_TYPE_TILELAYOUT,
	CONTROL_TYPE_TOOLBUTTON,
	CONTROL_TYPE_IMAGEPANEL,
	CONTROL_TYPE_LABELPANEL,
	CONTROL_TYPE_TOOLGRIPPER,
	CONTROL_TYPE_WHITECANVAS,
	CONTROL_TYPE_TITLESHADOW,
	CONTROL_TYPE_WINDOWCANVAS,
	CONTROL_TYPE_DIALOGCANVAS,
	CONTROL_TYPE_DIALOGLAYOUT,
	CONTROL_TYPE_PADDINGPANEL,
	CONTROL_TYPE_WARNINGPANEL,
	CONTROL_TYPE_GIFIMAGEPANEL,//gif图片
	CONTROL_TYPE_SEPARATORLINE,
	CONTROL_TYPE_CONTROLCANVAS,//control canvas画布
	CONTROL_TYPE_MULTILINEEDIT,
	CONTROL_TYPE_TOOLSEPARATOR,
	CONTROL_TYPE_VERTICALLAYOUT,
	CONTROL_TYPE_SINGLELINEEDIT,
	CONTROL_TYPE_SINGLELINEPICK,
	CONTROL_TYPE_NAVIGATORPANEL,
	CONTROL_TYPE_LISTHEADERITEM,
	CONTROL_TYPE_GREYTEXTHEADER,
	CONTROL_TYPE_LISTTEXTELEMENT,
	CONTROL_TYPE_NAVIGATORBUTTON,
	CONTROL_TYPE_TABFOLDERCANVAS,
	CONTROL_TYPE_INTERNETEXPLORER,
	CONTROL_TYPE_LISTHEADERSHADOW,
	CONTROL_TYPE_HORIZONTALLAYOUT,
	CONTROL_TYPE_LISTLABELELEMENT,
	CONTROL_TYPE_SEARCHTITLPANEL,
	CONTROL_TYPE_TOOLBARTITLEPANEL,
	CONTROL_TYPE_LISTEXPANDELEMENT,
	CONTROL_TYPE_IMAGEBUTTON,//位图按钮
	CONTROL_TYPE_IMAGECANVAS,//图片背景
	CONTROL_TYPE_IMAGETABFOLDER,//图片tabfoler
	CONTROL_TYPE_IMAGETABPAGE,//图片tab页
	CONTROL_TYPE_MENUBUTTON,//菜单按钮
	CONTROL_TYPE_PLUSMENUBUTTON, //扩展的菜单按钮
	CONTROL_TYPE_MENULIST,//菜单
	CONTROL_TYPE_MENUITEM,//菜单项
	CONTROL_TYPE_SLIDEFOLDER,//带滑块的页容器
	CONTROL_TYPE_SLIDEPAGE,//滑块页
	CONTROL_TYPE_GIFGRIDPANEL,//gif图片容器（带栅格）
	CONTROL_TYPE_TIPEDITUI,//单行，带提示文字的edit
	CONTROL_TYPE_CHECKBOX,//复选框
	CONTROL_TYPE_RADIOBOX,//radiobox
	CONTROL_TYPE_PROGRESSBAR,//进度条
	CONTROL_TYPE_FILEPROGRESSBAR,//文件进度条
	CONTROL_TYPE_AUTOSHORTCUT,   //自动排列快捷方式
	CONTROL_TYPE_DIVIDE,  //分割条
	CONTROL_TYPE_LAST_,
	CONTROL_TYPE_INVALID_
}CONTROLTYPE;

//图片列表透色
typedef enum
{
	TRANSCOLOR__FIRST = 0,
	TRANSCOLOR_PURPLE,//紫红
	TRANSCOLOR_BLUE,//蓝色
	TRANSCOLOR_GREY,
	TRANSCOLOR_WHITE, //
	TRANSCOLOR__LAST,
	TRANSCOLOR__INVALID,
}TRASPARENT_COLOR;

typedef struct CControlUIItem
{
	CONTROLTYPE nType;
	UINT nMode;
	int  nAttributeSize;
	BOOL bStretch;
	char *szAttribute;
}CONTROL_UI_ITEM;

 
typedef struct CControlNode
{
	CONTROL_UI_ITEM Data;
	CControlNode *pLeftNode; //子节点
	CControlNode *pRightNode; //兄弟节点
}CONTROLNODE, *LPCONTROLNODE;

 

typedef enum TreeNodeIconType {
	TREENODE_ICON_TYPE_BIG =1, //大图标
	TREENODE_ICON_TYPE_SMALL //小图标
}CTreeNodeIconType;

 

const char WNDNAME_MESSAGEBOX[] = "MessageBox";

class CUIResource
{
public:
	CUIResource(void);
	~CUIResource(void);
public:
	virtual BOOL LoadSkinDoc(const char *pData, DWORD dwSize); //载入XML数据
	virtual BOOL LoadImages(); //载入图片

	BOOL  ModifyColorStyle(int r, int g, int b);
	BOOL  MixBackGround(const char *szImageFile);
	BOOL  IsBackgroundImage(const int nImageId);
	BOOL  NeedMixBackground();
	BOOL  GetMixBackgroundImage(IImageInterface **pGraphic);
	//
	//
	DWORD GetMaxBtnImageId();
	DWORD GetMinBtnImageId();
	DWORD GetRestoreBtnImageId();
	DWORD GetCloseBtnImageId();
	DWORD GetFormBkgImageId();
	DWORD GetMsgBoxSuccImageId();
	DWORD GetMessageDlgBkgImageId();
	DWORD GetMsgBoxQuestionImageId();
	DWORD GetMsgBoxInfoImageId();
	DWORD GetMsgBoxErrorImageId();
	DWORD GetMenuCheckImageId();
	DWORD GetTreeGroupImage();

	void GetScrollBarImage(UINT &uPrior, UINT &uMid, UINT &uNext, BOOL bVert);

	//创建窗口节点
	LPCONTROLNODE CreateWindowNode(const char *szWindowName, SIZE &szMin, SIZE &szMax, SIZE &szCaption, UINT &uBkgImageId);
	//创建菜单节点
	virtual LPCONTROLNODE CreateMenuNodes(const char *szMenuName);
	//释放窗口、菜单节点
	virtual void ReleaseWindowNodes(const char *szMenuName);
	//XML节点操作相关
	void ClearAllUINodes();
	//删除所有图片资源
	void ClearImagesResource();
	//删除图片
	virtual BOOL DeleteImage(DWORD dwImageId);
	//获取图片数据
	virtual BOOL GetImageById(DWORD dwImageId, LPUI_IMAGE_ITEM *pImage);
	//
	virtual DWORD GetLinkImageIdByLink(LPCTSTR lpszLink);
	//static 
	static COLORREF StringToColor(const char *szValue);
	//根据扩展名获取图像类型
	static DWORD GetImageTypeByExt(const char *szExt);
	//设置皮肤路径
	void SetSkinPath(const char *szPath);
	void GetBlendColorValue(int &r, int &g, int &b);
	LPCONTROLNODE CreateNodeByXml(const char *szSkinXml);
	static void ReleaseChildNodes(LPCONTROLNODE lpNode);
protected:
	void AddChildNodes(LPCONTROLNODE pNode, TiXmlElement *pXmlNode);
	void AddLeftChildNode(LPCONTROLNODE pNode, TiXmlElement *pXmlNode);
	void AddRightChildNode(LPCONTROLNODE, TiXmlElement *pXmlNode);
	LPCONTROLNODE CreateNode(TiXmlElement *pXmlNode, LPCONTROLNODE pParentNode, BOOL bChild);
	LPCONTROLNODE CreateNodeStruct(TiXmlElement *pXmlNode);
	TiXmlElement *FindWindowNodeByName(const char *szWindowName);
	BOOL AddChildWindow(TiXmlElement *pXmlNode);
	TiXmlElement *GetMenuRootNode();
	TiXmlElement *GetMenuNodeByName(const char *szMenuName);

	
	static void ReleaseLeftChild(LPCONTROLNODE lpParent);
	static void ReleaseRightChild(LPCONTROLNODE lpParent);
	static void ReleaseCurrent(LPCONTROLNODE lpNode);
	//初始化源图
	void InitSrcImage();
	void AddBkgSrcImage(const int nImageId); //加入源图
	void AddSrcImage(const int nImageId); //
private:
	void InitControlTable(); //初始化控制对应表	
	void RefreshBackground();
	void RefreshImage();
	void RefreshBkgImage();
	LPCONTROLNODE CreateWindowBkgForm(LPCONTROLNODE *pCaptionNode, const char *szFormName);
protected:
	TiXmlDocument *m_pSkinDoc; //皮肤XML文档
	std::string m_strSkinPath; //皮肤路径
	std::map<std::string, CONTROLTYPE> m_ControlTable; //皮肤数据中控件名称与程序中控件类型对照表
	std::map<CAnsiString_, LPCONTROLNODE> m_WindowList; //窗口列表
	std::map<CAnsiString_, SIZE> m_WindowMinSize; 
	std::map<CAnsiString_, SIZE> m_WindowMaxSize;
	std::map<CAnsiString_, SIZE> m_WindowCaptionSize; 
	std::map<CAnsiString_, UINT> m_WindowBkgImages; //
	std::map<DWORD, LPUI_IMAGE_ITEM> m_ImageList; //图片列表
	std::map<DWORD, std::string> m_SrcImages; //源图
	std::map<DWORD, int> m_SrcBkgImages; //background images
	std::map<DWORD, LPUI_IMAGE_ITEM> m_LinkImageList; //外链图片列表
	CGuardLock m_LinkLock;
	DWORD m_dwCurrLinkImageId; //
	int m_crRValue, m_crGValue, m_crBValue; 

	//
	COLORREF m_clrBase;

	//default image id list
	//标题栏图标相关
	DWORD m_dwFormBkgImageId;
	DWORD m_dwMaxBtnImageId;
	DWORD m_dwMinBtnImageId;
	DWORD m_dwRestoreBtnImageId;
	DWORD m_dwCloseBtnImageId;
	//
	DWORD m_dwMsgDlgBkgImageId;
	DWORD m_dwMsgBoxSuccImageId;
	DWORD m_dwMsgBoxQuestionImageId;
	DWORD m_dwMsgBoxInfoImageId;
	DWORD m_dwMsgBoxErrorImageId;
	//滚动条图标相关
	DWORD m_dwUpImageId;
	DWORD m_dwVertBtnImageId;
	DWORD m_dwDownImageId;
	DWORD m_dwLeftImageId;
	DWORD m_dwHorzBtnImageId;
	DWORD m_dwRightImageId;
	//菜单相关
	DWORD m_dwMenuCheckImageId;
};

#endif
