#ifndef __SMARTSKIN_H___
#define __SMARTSKIN_H___

#include <windows.h>
//#include <UILib/uiresource.h>
//事件定义
#define EVENT_TYPE_UNKNOWN      0  //未知事件
#define EVENT_TYPE_CLICK        1  //click事件
#define EVENT_TYPE_INIT         2  //初始化事件
#define EVENT_TYPE_LINK         3  //Link事件
#define EVENT_TYPE_ITEMSELECT   4  //条目选择事件
#define EVENT_TYPE_KEYDOWN      5  //键盘事件
#define EVENT_TYPE_CHANGED      6  //状态改变事件
#define EVENT_TYPE_EDITCHANGED  7  //edit change事件

//RICHEDIT 回调事件通知
#define RICHEDIT_EVENT_SENDFILE        1  //发送文件 
#define RICHEDIT_EVENT_GETFILEBYTAG    2  //根据TAG获取文件名
#define RICHEDIT_EVENT_GETCUSTOMPIC    3  //根据TAG获取自定义图像文件名
#define RICHEDIT_EVENT_GETTIPPIC       4  //获取Tip Picture 文件名
#define RICHEDIT_EVENT_CUSTOMLINKCLICK 5  //点击链接

#ifndef MAX_TEXT_FACENAME_SIZE
   #define  MAX_TEXT_FACENAME_SIZE      32
#endif

typedef enum TreeNodeType {
	TREENODE_TYPE_GROUP = 1, //分组节点
	TREENODE_TYPE_LEAF   //叶子节点
}CTreeNodeType;



typedef struct __CharFontStyle
{
	int nFontSize;
	int nFontStyle;
	int cfColor;
	TCHAR szFaceName[MAX_TEXT_FACENAME_SIZE];
}CCharFontStyle, *LPCCharFontStyle;


//释放节点
typedef BOOL (CALLBACK *LPSKIN_FREE_NODE_EXDATA)(CTreeNodeType nodeType, void **pData);

//获取节点关键词
typedef const char *(CALLBACK *LPSKIN_GET_TREE_NODE_KEY)(CTreeNodeType nodeType, const void *pData);
//外链回调
typedef DWORD (CALLBACK *LPSKIN_GET_IMAGE_ID_BY_LINK)(LPCTSTR pstrLink, LPVOID pOverlapped);
//事件回调函数
typedef BOOL (CALLBACK *LPSKIN_WINDOW_EVENT_CALLBACK)(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName, 
	                                             POINT *ptMouse, WPARAM wParam, LPARAM lParam, void *pOverlapped);

//windows消息回调
typedef BOOL (CALLBACK *LPSKIN_WINDOW_MESSAGE_CALLBACK)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes, 
	                                             void *pOverlapped);
//树节点排序
typedef int (CALLBACK *LPSKIN_COMPARENODE)(CTreeNodeType tnType1, int nStatus1, void  *pData1, 
	                                  CTreeNodeType tnType2, int nStatus2, void *pData2);

//RichEdit 事件回调
typedef BOOL (CALLBACK *LPSKIN_RICHEDIT_EVENT_CALLBACK)(HWND hOwner, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped);

//检测配环境
BOOL  CALLBACK SkinCheckRunOption();
//运行
DWORD CALLBACK SkinApplicationRun();
//
BOOL CALLBACK SkinShowHintWindow();
//
BOOL  CALLBACK SkinReInitApplicationRun();

BOOL  CALLBACK SkinBlendSkinStyle(int r, int g, int b);

BOOL  CALLBACK SkinMixSkinBackGround(const char *szImageFile);

BOOL  CALLBACK SkinCloseWindow(HWND hWindow);
//
BOOL  CALLBACK SkinSetDockDesktop(HWND hWindow, BOOL bDock, COLORREF crKey, BYTE Alpha);
//
BOOL  CALLBACK SkinSetWindowTransparent(HWND hWindow, COLORREF crKey, BYTE Alpha, int FLAG);
//
BOOL  CALLBACK SkinSetCanActived(HWND hWindow, BOOL bCanActived);
//
DWORD CALLBACK SkinAddLinkImage(const char *szFileName, int nSubCount, COLORREF clrParent);
//
void  CALLBACK SkinSetLinkImageCallBack(LPSKIN_GET_IMAGE_ID_BY_LINK pCallBack, LPVOID pOverlapped);
//
BOOL  CALLBACK SkinSetWindowMinSize(HWND hWindow, int cx, int cy);
//
BOOL  CALLBACK SkinSetWindowMaxSize(HWND hWindow, int cx, int cy);
//从XML文件初始化皮肤
DWORD CALLBACK SkinCreateFromFile(const char *szXmlFileName);
//载入插件皮肤
BOOL  CALLBACK SkinAddPluginXML(const char *szXmlString);

BOOL  CALLBACK SkinSetControlFocus(HWND hWindow, const TCHAR *szCtrlName, BOOL bFocus);
//加入子控件
BOOL  CALLBACK SkinAddChildControl(HWND hWindow, const TCHAR *szParentCtrlName,
	                           const char *szSkinXml, TCHAR *szFlag, int *nFlagSize, 
							   const int nIdx);
//移除子控件
BOOL  CALLBACK SkinRemoveChildControl(HWND hWindow, const TCHAR *szParentCtrlName, const TCHAR *szFlag);

//从数据流初始化皮肤
BOOL  CALLBACK SkinCreateFromStream(const char *szXmlStream, const DWORD dwStreamSize, const char *szSkinPath);
//创建一个窗体
HWND CALLBACK SkinCreateWindowByName(const char *szWindowName, const TCHAR *szCaption, HWND hParent, DWORD dwStyle, DWORD dwExStyle, 
								 const RECT *prc, BOOL bForceCreate, LPSKIN_WINDOW_EVENT_CALLBACK pCallBack, 
								 LPSKIN_WINDOW_MESSAGE_CALLBACK pMsgCallBack, void *pOverlapped);
BOOL CALLBACK SkinOrderWindowMessage(HWND hWindow, UINT uMsg);
BOOL CALLBACK SkinNotifyEvent(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szEventName, WPARAM wParam, LPARAM lParam);
//弹出一个模态窗口
DWORD CALLBACK SkinMessageBox(HWND hParent, const TCHAR *szContent, TCHAR *szCaption, DWORD dwStyle);
//弹出模态窗口
DWORD CALLBACK SkinShowModal(HWND hWnd);

//
void CALLBACK SkinSetModalValue(HWND hWnd, int nValue);
//
BOOL CALLBACK SkinGetControlRect(HWND hWindow, const TCHAR *szCtrlName, RECT *rc);
BOOL CALLBACK SkinSetControlAttr(HWND hWindow, const TCHAR *szControlName, const TCHAR *szAttrName,
	                          const TCHAR *szValue);
BOOL CALLBACK SkinGetControlAttr(HWND hWindow, const TCHAR *szControlName, const TCHAR *szAttrName, 
	                          TCHAR *szValue, int *nMaxValueSize);
//获取一个控制指针
void * CALLBACK SkinGetControlByName(HWND hWindow, const TCHAR *szControlName);
//设置控件Text
BOOL  CALLBACK SkinSetControlTextByName(HWND hWindow, const TCHAR *szControlName, const TCHAR *szText);
//获取控件Text
BOOL  CALLBACK SkinGetControlTextByName(HWND hWindow, const TCHAR *szControlName, TCHAR *szText, int *nSize);
//设置控制的可见性
BOOL  CALLBACK SkinSetControlVisible(HWND hWindow, const TCHAR *szCtrlName, BOOL bVisible);
BOOL  CALLBACK SkinGetControlVisible(HWND hWindow, const TCHAR *szCtrlName);

//设置控制的Enable
BOOL  CALLBACK SkinSetControlEnable(HWND hWindow, const TCHAR *szCtrlName, BOOL bEnable);
BOOL  CALLBACK SkinGetControlEnable(HWND hWindow, const TCHAR *szCtrlName);
//update
BOOL  CALLBACK SkinUpdateControlUI(HWND hWindow, const TCHAR *szCtrlName);

//optionui 相关
BOOL  CALLBACK SkinSetOptionData(HWND hWindow, const TCHAR *szCtrlName, const int nData);
//radio控件相关
BOOL  CALLBACK SkinSetRadioChecked(HWND hWindow, const TCHAR *szCtrlName, BOOL bChecked);
BOOL  CALLBACK SkinGetRadioChecked(HWND hWindow, const TCHAR *szCtrlName);

//checkbox 控件相关
BOOL  CALLBACK SkinSetCheckBoxStatus(HWND hWindow, const TCHAR *szCtrlName, const int nStatus);
int   CALLBACK SkinGetCheckBoxStatus(HWND hWindow, const TCHAR *szCtrlName);

//Edit 相关
BOOL  CALLBACK SkinSetEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly);
BOOL  CALLBACK SkinSetMultiEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly);
//RichEdit 相关应用函数
BOOL  CALLBACK SkinAddRichChatText(HWND hWindow, const TCHAR *szControlName, const char *szId, const DWORD dwUserId, const char *szUserName, 
							   const char *szTime, const char *szText, const CCharFontStyle *cfStyle,
							   const int nNickColor, BOOL bIsUTF8, BOOL bAck);
BOOL  CALLBACK SkinGetCurrentRichEditFont(HWND hWindow, const TCHAR *szCtrlName, CCharFontStyle *cfStyle);
BOOL  CALLBACK SkinSetRichEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly);
BOOL  CALLBACK SkinGetRichEditText(HWND hWindow, const TCHAR *szCtrlName, DWORD dwStyle, char **pBuf, int *nSize);
char * CALLBACK SkinGetRichEditOleText(HWND hWindow, const TCHAR *szCtrlName, DWORD dwStyle);
BOOL  CALLBACK SkinInsertImageToRichEdit(HWND hWindow, const TCHAR *szCtrlName, const char *szFileName, 
	                 const char *szTag, const int nPos);
BOOL  CALLBACK SkinCancelCustomLink(HWND hWindow, const TCHAR *szCtrlName, DWORD dwLinkFlag);
BOOL  CALLBACK SkinRichEditCommand(HWND hWindow, const TCHAR *szCtrlName, const char *szCommand, LPVOID lpParams);
BOOL  CALLBACK SkinSetRichEditCallBack(HWND hWindow, const TCHAR *szCtrlName, LPSKIN_RICHEDIT_EVENT_CALLBACK pCallBack, LPVOID lpOverlapped);
BOOL  CALLBACK SkinReplaceImageInRichEdit(HWND hWindow, const TCHAR *szCtrlName, const char *szFileName, const char *szTag);
BOOL  CALLBACK SkinSetRichEditText(HWND hWindow, const TCHAR *szCtrlName, const char *szText, DWORD dwStyle);
BOOL  CALLBACK SkinSetRichEditAutoDetectLink(HWND hWindow, const TCHAR *szCtrlName, BOOL bAutoDetect);
BOOL  CALLBACK SkinRichEditInsertTip(HWND hWindow, const TCHAR *szCtrlName, CCharFontStyle *cfStyle,
	                             DWORD dwOffset, const TCHAR *szText); 
BOOL  CALLBACK SkinRichEditAddFileLink(HWND hWindow, const TCHAR *szCtrlName, CCharFontStyle *cfStyle,
	                             DWORD dwOffset, const char *szTip, const char *szFileName);
BOOL  CALLBACK SkinGetEditReadOnly(HWND hWindow, const TCHAR *szCtrlName);
BOOL  CALLBACK SkinGetMultiEditReadOnly(HWND hWindow, const TCHAR *szCtrlName);
BOOL  CALLBACK SkinGetRichEditReadOnly(HWND hWindow, const TCHAR *szCtrlName);
BOOL  CALLBACK SkinGetRichEditOleFlag(HWND hWindow, const TCHAR *szCtrlName, char **pOleFlags);
BOOL  CALLBACK SkinREInsertOlePicture(HWND hWindow, const TCHAR *szCtrlName, const char *szPicFileName);
BOOL  CALLBACK SkinGetRESelectImageFile(HWND hWindow, const TCHAR *szCtrlName, char *szFileName, int *nSize);
int   CALLBACK SkinGetRESelectStyle(HWND hWindow, const TCHAR *szCtrlName);
BOOL  CALLBACK SkinGetREChatId(HWND hWindow, const TCHAR *szCtrlName, char *szId);
BOOL  CALLBACK SkinREClearChatMsg(HWND hWindow, const TCHAR *szCtrlName, const char *szId);
//GifImage 相关
BOOL  CALLBACK SkinLoadGifImage(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szImageFileName, BOOL bTransParent, 
						   DWORD dwTransClr, BOOL bAnimate);

//TreeView相关
//设置释放节点内存的回调函数
BOOL CALLBACK SkinSetFreeNodeDataCallBack(HWND hWindow, const TCHAR *szCtrlName, LPSKIN_FREE_NODE_EXDATA pCallback);
//展开树节点
BOOL CALLBACK  SkinExpandTree(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bExpanded, BOOL bRecursive);
//全选
BOOL CALLBACK  SkinTreeSelectAll(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bRecursive);
//反选
BOOL CALLBACK  SkinTreeUnSelected(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bRecursive);
//删除选择
BOOL CALLBACK  SkinTreeDelSelected(HWND hWindow, const TCHAR *szCtrlName, BOOL bRecursive);
//
BOOL CALLBACK  SkinTreeGetSelectedUsers(HWND hWindow, const TCHAR *szCtrlName, char *szUsers, int *nSize);
//
BOOL CALLBACK  SkinTreeScrollToNodeByKey(HWND hWindow, const TCHAR *szCtrlName, const char *szKey);
//
int  CALLBACK  SkinGetTreeNodeStatus(void *pNode);
//清除所有树节点
BOOL CALLBACK  SkinTreeViewClear(HWND hWindow, const TCHAR *szCtrlName);
//获取在线用户统计数据
BOOL CALLBACK  SkinTVGetOnlineCount(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, DWORD *dwTotalCount, DWORD *dwOnlineCount);
//删除节点
BOOL CALLBACK  SkinTVDelNodeByID(HWND hWindow, const TCHAR *szCtrlName, int nIId, CTreeNodeType tnType);
//
BOOL CALLBACK  SkinShowTreeExtraData(HWND hWindow, const TCHAR *szCtrlName, BOOL bShow);
//
LPVOID CALLBACK SkinUpdateTreeNodeExtraData(HWND hWindow, const TCHAR *szCtrlName, const char *szKey, const TCHAR *szExtraData, BOOL bMulti);
//
LPVOID CALLBACK SkinUpdateTreeNodeImageFile(HWND hWindow, const TCHAR *szCtrlName, const char *szKey,
	const char *szImageFile, BOOL bMulti);
//
LPVOID CALLBACK SkinUpdateTreeNodeExtraImageFile(HWND hWindow, const TCHAR *szCtrlName, 
	                           const char *szKey, const int nImageId, BOOL bMulti);
//
LPVOID CALLBACK SkinAdjustTreeNode(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, const TCHAR *szName, 
	                     CTreeNodeType tnType, void *pData, BOOL bAdd, BOOL bRecursive);
//
BOOL CALLBACK  SkinSortTreeNode(HWND hWindow, const TCHAR *szCtrlName, void *pNode, LPSKIN_COMPARENODE pCompare,
	                            BOOL bRecursive, BOOL bParent);
//设置Icon的类型
BOOL CALLBACK  SkinSetTreeIconType(HWND hWindow, const TCHAR *szCtrlName, BYTE byteIconType);
//设置组节点是否可选
BOOL CALLBACK  SkinSetTreeGroupNodeIsSelect(HWND hWindow, const TCHAR *szCtrlName, BOOL bIsSelected);
//装载默认图标
BOOL CALLBACK  SkinLoadTreeDefaultImage(HWND hWindow, const TCHAR *szCtrlName, const char *szImageFileName);
//加入一个树节点
LPVOID CALLBACK SkinAddTreeChildNode(HWND hWindow, const TCHAR *szCtrlName, const DWORD dwId, void *pParentNode, const TCHAR *szText, CTreeNodeType tnTyp,
							   void *pData, const TCHAR *szLabel, const TCHAR *szImageFileName, const TCHAR *szExtraData);
//获取当前选择的树节点
BOOL CALLBACK SkinGetSelectTreeNode(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData);
//获取当前节点状态,1--展开 0--收缩
BOOL CALLBACK SkinGetNodeIsExpanded(HWND hWindow, void *pNode, BOOL *bExpanded);

//
BOOL CALLBACK SkinGetNodeByKey(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData);
//
BOOL CALLBACK SkinGetTreeNodeById(HWND hWindow, const TCHAR *szCtrlName, const DWORD dwId, 
	CTreeNodeType tnType, void **pNode, void **pData);
//获取节点下用户列表
BOOL CALLBACK SkinGetNodeChildUserList(HWND hWindow, void *pNode, char *szUserList, int *nSize, BOOL bRecursive);

//update status
LPVOID CALLBACK SkinUpdateUserStatusToNode(HWND hWindow, const TCHAR *szCtrlName, const char *szUserName, 
	                                 const char *szStatus, BOOL bMulti);
//update label
LPVOID CALLBACK SkinUpdateUserLabelToNode(HWND hWindow, const TCHAR *szCtrlName, const char *szUserName,
	                                 const char *szUTF8Label,  BOOL bMulti);
//
BOOL CALLBACK SkinSetGetKeyFun(HWND hWindow, const TCHAR *szCtrlName, LPSKIN_GET_TREE_NODE_KEY pCallBack);
//
BOOL CALLBACK SkinSetTreeViewStatusOffline(HWND hWindow, const TCHAR *szCtrlName);

//导航相关
BOOL CALLBACK  SkinNavigate2URL(HWND hWindow, const TCHAR *szCtrlName, const char *szUrl);

//
BOOL CALLBACK  SkinAddEmotion(HWND hWindow, const TCHAR *szCtrlName, const char *szFileName,
	                         const char *szEmotionTag, const char *szEmotionShortCut, const char *szEmotionComment);
//
BOOL CALLBACK  SkinGetSelEmotion(HWND hWindow, const TCHAR *szCtrlName, char *szGifName, int *nNamSize,
	                char *szTag, int *nTagSize);
//
int  CALLBACK  SkinGetDisplayEmotionCount(HWND hWindow, const TCHAR *szCtrlName);
//
BOOL CALLBACK  SkinClearAllEmotion(HWND hWindow, const TCHAR *szCtrlName);
//Tab Control 相关
//导航到某页
BOOL CALLBACK  SkinTabNavigate(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
BOOL CALLBACK  SkinTabSelectItem(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szPageName);
BOOL CALLBACK  SkinTabGetSelItemName(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szSelTabName, int *nSize);
BOOL CALLBACK  SkinTabGetChildByClass(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szClassName,
	TCHAR *szTabName, int *nSize);
//DropDown 相关
//获取某项dropdown 数据
void *CALLBACK  SkinGetDropdownItemData(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
//设置某项dropdown 数据
BOOL CALLBACK   SkinSetDropdownItemData(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, void *pData);
//获取dropdown 项 string
BOOL CALLBACK  SkinGetDropdownItemString(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, TCHAR *szText, int *nSize);
//设置dropdown 项 string
int CALLBACK  SkinSetDropdownItemString(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, const TCHAR *szText, void *pData);
//获取Dropdown当前选择项
int  CALLBACK  SkinGetDropdownSelectIndex(HWND hWindow, const TCHAR *szCtrlName);
//删除Dropdown 某项
BOOL CALLBACK  SkinDeleteDropdownItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
//选择dropdown某项
BOOL CALLBACK  SkinSelectDropdownItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
//获取dropdown item 个数
int  CALLBACK  SkinGetDropdownItemCount(HWND hWindow, const TCHAR *szCtrlName);

//listbox
int  CALLBACK  SkinInsertListItem(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szDspName, void *pData, int idx);
int  CALLBACK  SkinAppendListItem(HWND hWindow, const TCHAR *szCtrlName, const char *szDspText, void *pData);
int  CALLBACK  SkinAppendListSubItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, 
	                                  const int nSubIdx, const char *szDspText);
int  CALLBACK  SkinGetListSelItem(HWND hWindow, const TCHAR *szCtrlName);
BOOL CALLBACK  SkinGetListItemInfo(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szDspName, void **pData, int idx);
BOOL CALLBACK  SkinDeleteListItem(HWND hWindow, const TCHAR *szCtrlName, int idx);
int  CALLBACK  SkinGetListCount(HWND hWindow, const TCHAR *szCtrlName);
BOOL CALLBACK  SkinListKeyDownEvent(HWND hWindow, const TCHAR *szCtrlName, WORD wKey);
BOOL CALLBACK  SkinRemoveListItem(HWND hWindow, const TCHAR *szCtrlName, int idx);
BOOL CALLBACK  SkinSetListSelItem(HWND hWindow, const TCHAR *szCtrlName, int idx);
//scintilla 相关
//set key word
BOOL CALLBACK SkinSetScintKeyWord(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szKeyWord);
//set item style
BOOL CALLBACK SkinSetScintStyle(HWND hWindow, const TCHAR *szCtrlName, int nStyle, COLORREF clrFore, 
	       COLORREF clrBack, int nSize, TCHAR *szFace);

//memnu 相关
BOOL CALLBACK SkinCreateMenu(HWND hWindow, const TCHAR *szMenuName);
//
BOOL CALLBACK SkinMenuAppendItem(HWND hWindow, const TCHAR *szMenuName, int nParentId, const TCHAR *szMenuCaption, int nMenuId);
//
BOOL CALLBACK SkinMenuGetItemCaption(HWND hWindow, const TCHAR *szMenuName, int nMenuId, TCHAR *szwCaption, int *nSize);
//
BOOL CALLBACK SkinPopTrackMenu(HWND hWindow, const TCHAR *szMenuName, UINT uFlags, const int X, const int Y);
//
BOOL CALLBACK SkinDestroyMenu(HWND hWindow, const TCHAR *szMenuName);
//
BOOL CALLBACK SkinGrayMenu(HWND hWindow, const TCHAR *szMenuName, UINT uMenuID, BOOL bGray);
//
BOOL CALLBACK SkinSetMenuChecked(HWND hWindow, const TCHAR *szMenuName, UINT uMenuID, BOOL bChecked);
//
BOOL CALLBACK SkinSetMenuItemAttr(HWND hWindow, const TCHAR *szMenuName, UINT uMenuId, const TCHAR *szAttrName, const TCHAR *szValue);
//加入一个快捷方式
BOOL CALLBACK SkinAddAutoShortCut(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szCaption, const TCHAR *szFileName, const int nImageId,
	const TCHAR *szTip, TCHAR *szFlag);

//debug function
BOOL CALLBACK SkinFillRectToFile(COLORREF clr, const char *szFileName);
//释放皮肤资源
void CALLBACK SkinDestroyResource();
//
BOOL CALLBACK SkinTransImage(const char *szSrcImageName, const char *szDstImageName,
	                         const int nWidth, const int nHeight);
#endif