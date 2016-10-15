#ifndef __SMARTSKIN_H___
#define __SMARTSKIN_H___

#include <windows.h>
//#include <UILib/uiresource.h>
//�¼�����
#define EVENT_TYPE_UNKNOWN      0  //δ֪�¼�
#define EVENT_TYPE_CLICK        1  //click�¼�
#define EVENT_TYPE_INIT         2  //��ʼ���¼�
#define EVENT_TYPE_LINK         3  //Link�¼�
#define EVENT_TYPE_ITEMSELECT   4  //��Ŀѡ���¼�
#define EVENT_TYPE_KEYDOWN      5  //�����¼�
#define EVENT_TYPE_CHANGED      6  //״̬�ı��¼�
#define EVENT_TYPE_EDITCHANGED  7  //edit change�¼�

//RICHEDIT �ص��¼�֪ͨ
#define RICHEDIT_EVENT_SENDFILE        1  //�����ļ� 
#define RICHEDIT_EVENT_GETFILEBYTAG    2  //����TAG��ȡ�ļ���
#define RICHEDIT_EVENT_GETCUSTOMPIC    3  //����TAG��ȡ�Զ���ͼ���ļ���
#define RICHEDIT_EVENT_GETTIPPIC       4  //��ȡTip Picture �ļ���
#define RICHEDIT_EVENT_CUSTOMLINKCLICK 5  //�������

#ifndef MAX_TEXT_FACENAME_SIZE
   #define  MAX_TEXT_FACENAME_SIZE      32
#endif

typedef enum TreeNodeType {
	TREENODE_TYPE_GROUP = 1, //����ڵ�
	TREENODE_TYPE_LEAF   //Ҷ�ӽڵ�
}CTreeNodeType;



typedef struct __CharFontStyle
{
	int nFontSize;
	int nFontStyle;
	int cfColor;
	TCHAR szFaceName[MAX_TEXT_FACENAME_SIZE];
}CCharFontStyle, *LPCCharFontStyle;


//�ͷŽڵ�
typedef BOOL (CALLBACK *LPSKIN_FREE_NODE_EXDATA)(CTreeNodeType nodeType, void **pData);

//��ȡ�ڵ�ؼ���
typedef const char *(CALLBACK *LPSKIN_GET_TREE_NODE_KEY)(CTreeNodeType nodeType, const void *pData);
//�����ص�
typedef DWORD (CALLBACK *LPSKIN_GET_IMAGE_ID_BY_LINK)(LPCTSTR pstrLink, LPVOID pOverlapped);
//�¼��ص�����
typedef BOOL (CALLBACK *LPSKIN_WINDOW_EVENT_CALLBACK)(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName, 
	                                             POINT *ptMouse, WPARAM wParam, LPARAM lParam, void *pOverlapped);

//windows��Ϣ�ص�
typedef BOOL (CALLBACK *LPSKIN_WINDOW_MESSAGE_CALLBACK)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes, 
	                                             void *pOverlapped);
//���ڵ�����
typedef int (CALLBACK *LPSKIN_COMPARENODE)(CTreeNodeType tnType1, int nStatus1, void  *pData1, 
	                                  CTreeNodeType tnType2, int nStatus2, void *pData2);

//RichEdit �¼��ص�
typedef BOOL (CALLBACK *LPSKIN_RICHEDIT_EVENT_CALLBACK)(HWND hOwner, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped);

//����价��
BOOL  CALLBACK SkinCheckRunOption();
//����
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
//��XML�ļ���ʼ��Ƥ��
DWORD CALLBACK SkinCreateFromFile(const char *szXmlFileName);
//������Ƥ��
BOOL  CALLBACK SkinAddPluginXML(const char *szXmlString);

BOOL  CALLBACK SkinSetControlFocus(HWND hWindow, const TCHAR *szCtrlName, BOOL bFocus);
//�����ӿؼ�
BOOL  CALLBACK SkinAddChildControl(HWND hWindow, const TCHAR *szParentCtrlName,
	                           const char *szSkinXml, TCHAR *szFlag, int *nFlagSize, 
							   const int nIdx);
//�Ƴ��ӿؼ�
BOOL  CALLBACK SkinRemoveChildControl(HWND hWindow, const TCHAR *szParentCtrlName, const TCHAR *szFlag);

//����������ʼ��Ƥ��
BOOL  CALLBACK SkinCreateFromStream(const char *szXmlStream, const DWORD dwStreamSize, const char *szSkinPath);
//����һ������
HWND CALLBACK SkinCreateWindowByName(const char *szWindowName, const TCHAR *szCaption, HWND hParent, DWORD dwStyle, DWORD dwExStyle, 
								 const RECT *prc, BOOL bForceCreate, LPSKIN_WINDOW_EVENT_CALLBACK pCallBack, 
								 LPSKIN_WINDOW_MESSAGE_CALLBACK pMsgCallBack, void *pOverlapped);
BOOL CALLBACK SkinOrderWindowMessage(HWND hWindow, UINT uMsg);
BOOL CALLBACK SkinNotifyEvent(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szEventName, WPARAM wParam, LPARAM lParam);
//����һ��ģ̬����
DWORD CALLBACK SkinMessageBox(HWND hParent, const TCHAR *szContent, TCHAR *szCaption, DWORD dwStyle);
//����ģ̬����
DWORD CALLBACK SkinShowModal(HWND hWnd);

//
void CALLBACK SkinSetModalValue(HWND hWnd, int nValue);
//
BOOL CALLBACK SkinGetControlRect(HWND hWindow, const TCHAR *szCtrlName, RECT *rc);
BOOL CALLBACK SkinSetControlAttr(HWND hWindow, const TCHAR *szControlName, const TCHAR *szAttrName,
	                          const TCHAR *szValue);
BOOL CALLBACK SkinGetControlAttr(HWND hWindow, const TCHAR *szControlName, const TCHAR *szAttrName, 
	                          TCHAR *szValue, int *nMaxValueSize);
//��ȡһ������ָ��
void * CALLBACK SkinGetControlByName(HWND hWindow, const TCHAR *szControlName);
//���ÿؼ�Text
BOOL  CALLBACK SkinSetControlTextByName(HWND hWindow, const TCHAR *szControlName, const TCHAR *szText);
//��ȡ�ؼ�Text
BOOL  CALLBACK SkinGetControlTextByName(HWND hWindow, const TCHAR *szControlName, TCHAR *szText, int *nSize);
//���ÿ��ƵĿɼ���
BOOL  CALLBACK SkinSetControlVisible(HWND hWindow, const TCHAR *szCtrlName, BOOL bVisible);
BOOL  CALLBACK SkinGetControlVisible(HWND hWindow, const TCHAR *szCtrlName);

//���ÿ��Ƶ�Enable
BOOL  CALLBACK SkinSetControlEnable(HWND hWindow, const TCHAR *szCtrlName, BOOL bEnable);
BOOL  CALLBACK SkinGetControlEnable(HWND hWindow, const TCHAR *szCtrlName);
//update
BOOL  CALLBACK SkinUpdateControlUI(HWND hWindow, const TCHAR *szCtrlName);

//optionui ���
BOOL  CALLBACK SkinSetOptionData(HWND hWindow, const TCHAR *szCtrlName, const int nData);
//radio�ؼ����
BOOL  CALLBACK SkinSetRadioChecked(HWND hWindow, const TCHAR *szCtrlName, BOOL bChecked);
BOOL  CALLBACK SkinGetRadioChecked(HWND hWindow, const TCHAR *szCtrlName);

//checkbox �ؼ����
BOOL  CALLBACK SkinSetCheckBoxStatus(HWND hWindow, const TCHAR *szCtrlName, const int nStatus);
int   CALLBACK SkinGetCheckBoxStatus(HWND hWindow, const TCHAR *szCtrlName);

//Edit ���
BOOL  CALLBACK SkinSetEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly);
BOOL  CALLBACK SkinSetMultiEditReadOnly(HWND hWindow, const TCHAR *szCtrlName, BOOL bReadOnly);
//RichEdit ���Ӧ�ú���
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
//GifImage ���
BOOL  CALLBACK SkinLoadGifImage(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szImageFileName, BOOL bTransParent, 
						   DWORD dwTransClr, BOOL bAnimate);

//TreeView���
//�����ͷŽڵ��ڴ�Ļص�����
BOOL CALLBACK SkinSetFreeNodeDataCallBack(HWND hWindow, const TCHAR *szCtrlName, LPSKIN_FREE_NODE_EXDATA pCallback);
//չ�����ڵ�
BOOL CALLBACK  SkinExpandTree(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bExpanded, BOOL bRecursive);
//ȫѡ
BOOL CALLBACK  SkinTreeSelectAll(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bRecursive);
//��ѡ
BOOL CALLBACK  SkinTreeUnSelected(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, BOOL bRecursive);
//ɾ��ѡ��
BOOL CALLBACK  SkinTreeDelSelected(HWND hWindow, const TCHAR *szCtrlName, BOOL bRecursive);
//
BOOL CALLBACK  SkinTreeGetSelectedUsers(HWND hWindow, const TCHAR *szCtrlName, char *szUsers, int *nSize);
//
BOOL CALLBACK  SkinTreeScrollToNodeByKey(HWND hWindow, const TCHAR *szCtrlName, const char *szKey);
//
int  CALLBACK  SkinGetTreeNodeStatus(void *pNode);
//����������ڵ�
BOOL CALLBACK  SkinTreeViewClear(HWND hWindow, const TCHAR *szCtrlName);
//��ȡ�����û�ͳ������
BOOL CALLBACK  SkinTVGetOnlineCount(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, DWORD *dwTotalCount, DWORD *dwOnlineCount);
//ɾ���ڵ�
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
//����Icon������
BOOL CALLBACK  SkinSetTreeIconType(HWND hWindow, const TCHAR *szCtrlName, BYTE byteIconType);
//������ڵ��Ƿ��ѡ
BOOL CALLBACK  SkinSetTreeGroupNodeIsSelect(HWND hWindow, const TCHAR *szCtrlName, BOOL bIsSelected);
//װ��Ĭ��ͼ��
BOOL CALLBACK  SkinLoadTreeDefaultImage(HWND hWindow, const TCHAR *szCtrlName, const char *szImageFileName);
//����һ�����ڵ�
LPVOID CALLBACK SkinAddTreeChildNode(HWND hWindow, const TCHAR *szCtrlName, const DWORD dwId, void *pParentNode, const TCHAR *szText, CTreeNodeType tnTyp,
							   void *pData, const TCHAR *szLabel, const TCHAR *szImageFileName, const TCHAR *szExtraData);
//��ȡ��ǰѡ������ڵ�
BOOL CALLBACK SkinGetSelectTreeNode(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData);
//��ȡ��ǰ�ڵ�״̬,1--չ�� 0--����
BOOL CALLBACK SkinGetNodeIsExpanded(HWND hWindow, void *pNode, BOOL *bExpanded);

//
BOOL CALLBACK SkinGetNodeByKey(HWND hWindow, const TCHAR *szCtrlName, void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData);
//
BOOL CALLBACK SkinGetTreeNodeById(HWND hWindow, const TCHAR *szCtrlName, const DWORD dwId, 
	CTreeNodeType tnType, void **pNode, void **pData);
//��ȡ�ڵ����û��б�
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

//�������
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
//Tab Control ���
//������ĳҳ
BOOL CALLBACK  SkinTabNavigate(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
BOOL CALLBACK  SkinTabSelectItem(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szPageName);
BOOL CALLBACK  SkinTabGetSelItemName(HWND hWindow, const TCHAR *szCtrlName, TCHAR *szSelTabName, int *nSize);
BOOL CALLBACK  SkinTabGetChildByClass(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szClassName,
	TCHAR *szTabName, int *nSize);
//DropDown ���
//��ȡĳ��dropdown ����
void *CALLBACK  SkinGetDropdownItemData(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
//����ĳ��dropdown ����
BOOL CALLBACK   SkinSetDropdownItemData(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, void *pData);
//��ȡdropdown �� string
BOOL CALLBACK  SkinGetDropdownItemString(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, TCHAR *szText, int *nSize);
//����dropdown �� string
int CALLBACK  SkinSetDropdownItemString(HWND hWindow, const TCHAR *szCtrlName, const int nIdx, const TCHAR *szText, void *pData);
//��ȡDropdown��ǰѡ����
int  CALLBACK  SkinGetDropdownSelectIndex(HWND hWindow, const TCHAR *szCtrlName);
//ɾ��Dropdown ĳ��
BOOL CALLBACK  SkinDeleteDropdownItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
//ѡ��dropdownĳ��
BOOL CALLBACK  SkinSelectDropdownItem(HWND hWindow, const TCHAR *szCtrlName, const int nIdx);
//��ȡdropdown item ����
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
//scintilla ���
//set key word
BOOL CALLBACK SkinSetScintKeyWord(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szKeyWord);
//set item style
BOOL CALLBACK SkinSetScintStyle(HWND hWindow, const TCHAR *szCtrlName, int nStyle, COLORREF clrFore, 
	       COLORREF clrBack, int nSize, TCHAR *szFace);

//memnu ���
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
//����һ����ݷ�ʽ
BOOL CALLBACK SkinAddAutoShortCut(HWND hWindow, const TCHAR *szCtrlName, const TCHAR *szCaption, const TCHAR *szFileName, const int nImageId,
	const TCHAR *szTip, TCHAR *szFlag);

//debug function
BOOL CALLBACK SkinFillRectToFile(COLORREF clr, const char *szFileName);
//�ͷ�Ƥ����Դ
void CALLBACK SkinDestroyResource();
//
BOOL CALLBACK SkinTransImage(const char *szSrcImageName, const char *szDstImageName,
	                         const int nWidth, const int nHeight);
#endif