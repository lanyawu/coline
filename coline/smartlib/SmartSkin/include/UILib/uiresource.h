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

const char FRAME_WINDOW_ROOT_NODE_NAME[] = "FrameWindow"; //���ڸ��ڵ�
const char MENUS_ROOT_NODE_NAME[]  = "Menus";  //�˵����ڵ�

//��ʾ��Ϣ����
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
	DWORD dwListFmt; //ͼƬ���з�ʽ
	DWORD dwTransColor; //͸����ɫ
	DWORD dwSrcTransColor; //Դ͸ɫ
	DWORD dwSubCount;   //����ͼƬ����
	DWORD dwImageType; //ͼƬ����
	DWORD dwImageSize; //ͼƬ��С
}UI_IMAGE_ITEM, *LPUI_IMAGE_ITEM;

//�ͻ��˳������еĿؼ�����
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
	CONTROL_TYPE_GIFIMAGEPANEL,//gifͼƬ
	CONTROL_TYPE_SEPARATORLINE,
	CONTROL_TYPE_CONTROLCANVAS,//control canvas����
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
	CONTROL_TYPE_IMAGEBUTTON,//λͼ��ť
	CONTROL_TYPE_IMAGECANVAS,//ͼƬ����
	CONTROL_TYPE_IMAGETABFOLDER,//ͼƬtabfoler
	CONTROL_TYPE_IMAGETABPAGE,//ͼƬtabҳ
	CONTROL_TYPE_MENUBUTTON,//�˵���ť
	CONTROL_TYPE_PLUSMENUBUTTON, //��չ�Ĳ˵���ť
	CONTROL_TYPE_MENULIST,//�˵�
	CONTROL_TYPE_MENUITEM,//�˵���
	CONTROL_TYPE_SLIDEFOLDER,//�������ҳ����
	CONTROL_TYPE_SLIDEPAGE,//����ҳ
	CONTROL_TYPE_GIFGRIDPANEL,//gifͼƬ��������դ��
	CONTROL_TYPE_TIPEDITUI,//���У�����ʾ���ֵ�edit
	CONTROL_TYPE_CHECKBOX,//��ѡ��
	CONTROL_TYPE_RADIOBOX,//radiobox
	CONTROL_TYPE_PROGRESSBAR,//������
	CONTROL_TYPE_FILEPROGRESSBAR,//�ļ�������
	CONTROL_TYPE_AUTOSHORTCUT,   //�Զ����п�ݷ�ʽ
	CONTROL_TYPE_DIVIDE,  //�ָ���
	CONTROL_TYPE_LAST_,
	CONTROL_TYPE_INVALID_
}CONTROLTYPE;

//ͼƬ�б�͸ɫ
typedef enum
{
	TRANSCOLOR__FIRST = 0,
	TRANSCOLOR_PURPLE,//�Ϻ�
	TRANSCOLOR_BLUE,//��ɫ
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
	CControlNode *pLeftNode; //�ӽڵ�
	CControlNode *pRightNode; //�ֵܽڵ�
}CONTROLNODE, *LPCONTROLNODE;

 

typedef enum TreeNodeIconType {
	TREENODE_ICON_TYPE_BIG =1, //��ͼ��
	TREENODE_ICON_TYPE_SMALL //Сͼ��
}CTreeNodeIconType;

 

const char WNDNAME_MESSAGEBOX[] = "MessageBox";

class CUIResource
{
public:
	CUIResource(void);
	~CUIResource(void);
public:
	virtual BOOL LoadSkinDoc(const char *pData, DWORD dwSize); //����XML����
	virtual BOOL LoadImages(); //����ͼƬ

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

	//�������ڽڵ�
	LPCONTROLNODE CreateWindowNode(const char *szWindowName, SIZE &szMin, SIZE &szMax, SIZE &szCaption, UINT &uBkgImageId);
	//�����˵��ڵ�
	virtual LPCONTROLNODE CreateMenuNodes(const char *szMenuName);
	//�ͷŴ��ڡ��˵��ڵ�
	virtual void ReleaseWindowNodes(const char *szMenuName);
	//XML�ڵ�������
	void ClearAllUINodes();
	//ɾ������ͼƬ��Դ
	void ClearImagesResource();
	//ɾ��ͼƬ
	virtual BOOL DeleteImage(DWORD dwImageId);
	//��ȡͼƬ����
	virtual BOOL GetImageById(DWORD dwImageId, LPUI_IMAGE_ITEM *pImage);
	//
	virtual DWORD GetLinkImageIdByLink(LPCTSTR lpszLink);
	//static 
	static COLORREF StringToColor(const char *szValue);
	//������չ����ȡͼ������
	static DWORD GetImageTypeByExt(const char *szExt);
	//����Ƥ��·��
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
	//��ʼ��Դͼ
	void InitSrcImage();
	void AddBkgSrcImage(const int nImageId); //����Դͼ
	void AddSrcImage(const int nImageId); //
private:
	void InitControlTable(); //��ʼ�����ƶ�Ӧ��	
	void RefreshBackground();
	void RefreshImage();
	void RefreshBkgImage();
	LPCONTROLNODE CreateWindowBkgForm(LPCONTROLNODE *pCaptionNode, const char *szFormName);
protected:
	TiXmlDocument *m_pSkinDoc; //Ƥ��XML�ĵ�
	std::string m_strSkinPath; //Ƥ��·��
	std::map<std::string, CONTROLTYPE> m_ControlTable; //Ƥ�������пؼ�����������пؼ����Ͷ��ձ�
	std::map<CAnsiString_, LPCONTROLNODE> m_WindowList; //�����б�
	std::map<CAnsiString_, SIZE> m_WindowMinSize; 
	std::map<CAnsiString_, SIZE> m_WindowMaxSize;
	std::map<CAnsiString_, SIZE> m_WindowCaptionSize; 
	std::map<CAnsiString_, UINT> m_WindowBkgImages; //
	std::map<DWORD, LPUI_IMAGE_ITEM> m_ImageList; //ͼƬ�б�
	std::map<DWORD, std::string> m_SrcImages; //Դͼ
	std::map<DWORD, int> m_SrcBkgImages; //background images
	std::map<DWORD, LPUI_IMAGE_ITEM> m_LinkImageList; //����ͼƬ�б�
	CGuardLock m_LinkLock;
	DWORD m_dwCurrLinkImageId; //
	int m_crRValue, m_crGValue, m_crBValue; 

	//
	COLORREF m_clrBase;

	//default image id list
	//������ͼ�����
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
	//������ͼ�����
	DWORD m_dwUpImageId;
	DWORD m_dwVertBtnImageId;
	DWORD m_dwDownImageId;
	DWORD m_dwLeftImageId;
	DWORD m_dwHorzBtnImageId;
	DWORD m_dwRightImageId;
	//�˵����
	DWORD m_dwMenuCheckImageId;
};

#endif
