#ifndef __SMARTWINDOW_H__
#define __SMARTWINDOW_H__

#include <string>
#include <map>
#include <UILib/UILib.h>
#include <commonlib/SystemUtils.h>
#include <SmartSkin/SmartSkin.h>
#include "SmartPaintManager.h"

const TCHAR STD_WNDCANVAS[] = _T("canvas");//窗口的背景画布
const TCHAR STD_CAPTION[] = _T("caption");//标题栏
const TCHAR STD_TITLE[] = _T("title");//标题文字
const TCHAR STD_LOGO[] = _T("logo"); //logo图标
const TCHAR STD_DRAG_AREA[] = _T("dragarea");//无实际功能的控件，用户可用此控件拖拽移动对话框
const TCHAR STD_BTN_MIN[] = _T("minbutton");//最小化、隐藏按钮
const TCHAR STD_BTN_MAX[] = _T("maxbutton");//最大化按钮
const TCHAR STD_BTN_RESTORE[] = _T("restorebutton");//恢复按钮
const TCHAR STD_BTN_CLOSE[] = _T("closebutton");//关闭按钮

using namespace UILib;

typedef void (CALLBACK *LPWINDOWDESTROY)(HWND hWnd);

class CSmartFrame
{
public:
	CSmartFrame(HWND hWnd, UINT uDragWidth = 4);
	~CSmartFrame();

	BOOL Init(UINT uDragWidth);

	CRect m_rcTopLeft;  //
	CRect m_rcLeft;
	CRect m_rcTopRight;
	CRect m_rcRight;
	CRect m_rcTop;
	CRect m_rcBottom;
	CRect m_rcBotLeft;
	CRect m_rcBotRight;
	CRect m_rcSizeBox;

	HWND m_hWnd;
	UINT m_uDragWidth;
};

class CSmartWindow: public CWindowWnd,
                	public INotifyUI
{
public:
	CSmartWindow(LPWINDOWDESTROY lpDestroy, const char *szWindowName, CWindowWnd *pParent = NULL);
	~CSmartWindow(void);
public:
	//INotifyUI overridable
	void Notify(TNotifyUI& msg);

	void SetParent(CWindowWnd *pParent);
	virtual void SetText(LPCTSTR szTitle);
	virtual std::string GetWindowName();
	//
	BOOL SetWindowMinSize_(int cx, int cy);
	BOOL SetWindowMaxSize_(int cx, int cy);
	void SetCaptionSize(int cx, int cy);
	BOOL OrderWindowMessage_(UINT uMsg);
	void Update();
	void SetBackGround(BYTE r, BYTE g, BYTE b);
	void SetModalValue(int nValue);
	void SetIsActive(BOOL bCanActived);
	//实现窗口停靠
	BOOL SetDockDesktop(BOOL bDock, COLORREF crKey, BYTE Alpha);
	//设置透明
	BOOL SetWindowTransparent(COLORREF crKey, BYTE Alpha, int FLAG);
	//常用
	UINT StdMsgBox(LPCTSTR lpContent, LPCTSTR lpCaption, UINT nStyle);
	CControlUI *FindControl(const TCHAR *szwName);
	CControlUI *FindControl(const char *szName);
	BOOL SetControlAttribute(const TCHAR *szControlName, const TCHAR *szAttrName, const TCHAR *szValue);
	BOOL GetControlAttribute(const TCHAR *szControlName, const TCHAR *szAttrName, 
	                          TCHAR *szValue, int *nMaxValueSize);
	BOOL SetControlText(const TCHAR *szControlName, const TCHAR *szText);
	BOOL GetControlText(const TCHAR *szControlName, TCHAR *szText, int *nSize);
	BOOL UpdateControl(const TCHAR *szCtrlName);
	void SetEventCallBack(LPSKIN_WINDOW_EVENT_CALLBACK pCallBack);
	void SetMsgCallBack(LPSKIN_WINDOW_MESSAGE_CALLBACK pCallBack);
	void SetOverlapped(void *pOverlapped);
	BOOL NotifyEvent(const TCHAR *szCtrlName, const TCHAR *szEventName, WPARAM wParam, LPARAM lParam);
	BOOL SetCtrlVisible(const TCHAR *szControlName, BOOL bVisible);
	BOOL GetControlVisible(const TCHAR *szCtrlName);
	BOOL GetControlEnable(const TCHAR *szCtrlName);
	BOOL SetControlFocus(const TCHAR *szCtrlName, BOOL bFocus);
	BOOL SetCtrlEnable(const TCHAR *szControlName, BOOL bEnable);
	BOOL AddControlToUI(const TCHAR *szParentCtrlName, const char *szSkinXml, TCHAR *szFlag, int *nMaxFlagSize,
		                const int nIdx);
	BOOL RemoveControlFromUI(const TCHAR *szParentCtrlName, const TCHAR *szCtrlFlag);
	//RichEdit 应用相关
	BOOL AddChatText(const TCHAR *szControlName, const char *szId, const DWORD dwUserId, const char *szUserName, 
		             const char *szTime, const char *szText, const CCharFontStyle *cfStyle, 
					 const int nNickColor, BOOL bIsUTF8, BOOL bAck);
	BOOL GetRESelectImageFile(const TCHAR *szCtrlName, char *szFileName, int *nSize);
	int  GetRESelectStyle(const TCHAR *szCtrlName);
	BOOL GetREChatId(const TCHAR *szCtrlName, char *szId);
	BOOL REClearChatMsg(const TCHAR *szCtrlName, const char *szId);
	BOOL GetRichEditOleFlag(const TCHAR *szCtrlName, char **pOleFlags);
	BOOL REInsertOlePicture(const TCHAR *szCtrlName, const char *szFileName);
	BOOL CancelCustomLink(const TCHAR *szCtrlName, DWORD dwFlag);
	//optionui相关
	BOOL SetOptionData_(const TCHAR *szCtrlName, const int nData);
	//radio 相关
	BOOL SetRadioCheck(const TCHAR *szCtrlName, BOOL bChecked);
	BOOL GetRadioCheck(const TCHAR *szCtrlName);
	//checkbox 相关
	BOOL SetCheckBoxStatus_(const TCHAR *szCtrlName, const int nStatus);
	int  GetCheckBoxStatus_(const TCHAR *szCtrlName);

	//Edit 相关
	BOOL SetEditReadOnlyValue(const TCHAR *szCtrlName, BOOL bReadOnly);
	BOOL SetMultiEditReadOnlyValue(const TCHAR *szCtrlName, BOOL bReadOnly);
	BOOL SetRichEditReadOnlyValue(const TCHAR *szCtrlName, BOOL bReadOnly);
	BOOL InsertImageToRichEdit_(const TCHAR *szCtrlName, const char *szFileName,
		                        const char *szTag, const int nPos);
	BOOL ReplaceImageInRichEdit_(const TCHAR *szCtrlName, const char *szFileName, const char *szTag);
	BOOL GetREText(const TCHAR *szCtrlName, DWORD dwStyle, char **pBuf, int &nSize);
	BOOL GetCurrentRichEditFont_(const TCHAR *szCtrlName, CCharFontStyle *cfStyle);
	char * GetOleText_(const TCHAR *szCtrlName, DWORD dwStyle);
	BOOL SetREText(const TCHAR *szCtrlName, const char *szText, DWORD dwStyle);
	BOOL RichEditCommand_(const TCHAR *szCtrlName, const char *szCommand, LPVOID lpParams);
	BOOL RichEditInsertTip_(const TCHAR *szCtrlName, CCharFontStyle *cfStyle,
		                   DWORD dwOffset, const TCHAR *szText);
	BOOL RichEditAddFileLink(const TCHAR *szCtrlName, CCharFontStyle *cfStyle, DWORD dwOffset,
		                    const char *szTip, const char *szFileName);
	BOOL SetEditAutoDetectLink(const TCHAR *szCtrlName, BOOL bAutoDetect);
	BOOL GetEditReadOnlyValue(const TCHAR *szCtrlName);
	BOOL SetRichEditCallBack_(const TCHAR *szCtrlName, LPSKIN_RICHEDIT_EVENT_CALLBACK pCallBack, LPVOID lpOverlapped);
	BOOL GetMultiEditReadOnlyValue(const TCHAR *szCtrlName);
	BOOL GetRichEditReadOnlyValue(const TCHAR *szCtrlName);
	BOOL GetControlRect_(const TCHAR *szCtrlName, RECT *rc);
	//GIF Image 相关
	BOOL SetGifImage(const TCHAR *szCtrlName, const TCHAR *szImageFileName, BOOL bTransParent, 
		     DWORD dwTransClr, BOOL bAnimate);
	//TreeView 相关
	//设置树节点回调函数
	BOOL SetFreeNodeDataCallBack_(const TCHAR *szCtrlName, LPSKIN_FREE_NODE_EXDATA pCallback);
	//展开树节点
	BOOL ExpandTreeView(const TCHAR *szCtrlName, void *pParentNode, BOOL bExpanded, BOOL bRecursive);
	//设置Icon的类型
	BOOL SetTreeViewIconType(const TCHAR *szCtrlName, BYTE byteIconType);
	//
	BOOL TVGetOnlineCount(const TCHAR *szCtrlName, void *pParentNode, DWORD *dwTotalCount, DWORD *dwOnlineCount);
	//
	BOOL TVDelNodeByID(const TCHAR *szCtrlName, const int nId, CTreeNodeType tnType);
	//设置所有用户离线
	BOOL SetTreeViewStatusOffline(const TCHAR *szCtrlName);
	//设置组节点是否可选
	BOOL SetTreeViewGroupNodeIsSelect(const TCHAR *szCtrlName, BOOL bIsSelected);
	//装载默认图标
	BOOL LoadTreeViewDefaultImage(const TCHAR *szCtrlName, const char *szImageFileName);
    //加入树节点
	void * AddTreeChildNode_(const TCHAR *szCtrlName, const DWORD dwId, void *pParentNode, const TCHAR *szText,  CTreeNodeType tnType,
		void *pData, const TCHAR *szLabel, const TCHAR *szImageFileName, const TCHAR *szExtraData);
	//
	BOOL GetTreeNodeById(const TCHAR *szCtrlName, const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData);
	//
	BOOL ShowTreeExtraData(const TCHAR *szCtrlName, BOOL bShow);
	//
	LPVOID UpdateTreeNodeExtraData(const TCHAR *szCtrlName, const char *szKey, const TCHAR *szExtraData, BOOL bMulti);
	//
	LPVOID UpdateTreeNodeImageFile(const TCHAR *szCtrlName, const char *szKey, const char *szImageFile, BOOL bMulti);
	//
	LPVOID UpdateTreeNodeExtraImageFile(const TCHAR *szCtrlName, const char *szKey, const int nImageId, BOOL bMulti);
	//
	LPVOID AdjustTreeNode(const TCHAR *szCtrlName, void *pParentNode, const TCHAR *szName, 
	                     CTreeNodeType tnType, void *pData, BOOL bAdd, BOOL bRecursive);
	//
	BOOL GetNodeIsExpanded_(void *pNode, BOOL *bExpanded);
	BOOL GetNodeByKey(const TCHAR *szCtrlName, void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData);
	//
	BOOL TreeScrollToNodeByKey(const TCHAR *szCtrlName, const char *szKey);
	//
	BOOL SortTreeNode(const TCHAR *szCtrlName, void *pNode, LPSKIN_COMPARENODE pCompare,
	                            BOOL bRecursive, BOOL bParent);
	BOOL GetNodeChildUserList_(void *pNode, char *szUserList, int *nSize, BOOL bRecursive);
	BOOL TreeViewSelectAll(const TCHAR *szCtrlName, void *pNode, BOOL bRecursive);
	BOOL TreeViewUnSelected(const TCHAR *szCtrlName, void *pNode, BOOL bRecursive);
	BOOL TreeViewDelSelected(const TCHAR *szCtrlName, BOOL bRecursive);
	BOOL TreeViewGetSelectedUsers(const TCHAR *szCtrlName, char *szUsers, int *nSize, BOOL bRecursive);
	//
	BOOL TreeViewClear(const TCHAR *szCtrlName);
	//获取当前选择的树节点
	BOOL GetSelectTreeNode_(const TCHAR *szCtrlName, TCHAR *szName, int *nNameLen, void **pSelNode, 
		                    CTreeNodeType *tnType, void **pData);
	LPVOID UpdateUserStatusToNode_(const TCHAR *szCtrlName, const char *szUserName, 
	                                 const char *szStatus, BOOL bMulti);
	LPVOID UpdateUserLabelToNode_(const TCHAR *szCtrlName, const char *szUserName,
	                                 const char *szUTF8Label,  BOOL bMulti);
	BOOL SetGetKeyFun(const TCHAR *szCtrlName, LPSKIN_GET_TREE_NODE_KEY pCallBack);
	//导航相关
	BOOL NavigateURL2(const TCHAR *szCtrlName, const char *szUrl);

	//Tab Control相关
	BOOL TabNavigate2(const TCHAR *szCtrlName, const int nIdx);
	BOOL TabSelectItem(const TCHAR *szCtrlName, const TCHAR *szPageName);
	BOOL TabGetSelItemName(const TCHAR *szCtrlName, TCHAR *szSelName, int *nSize);
	BOOL TabGetChildItemByClass(const TCHAR *szCtrlName, const TCHAR *szClassName, TCHAR *szName, int *nSize);
	//Dropdown 相关
	void * GetDropdownItemData_(const TCHAR *szCtrlName, const int nIdx);
	//设置dropdown 项 数据
	BOOL SetDropdownItemData_(const TCHAR *szCtrlName, const int nIdx, void *pData);
	//获取dropdown 项string
	BOOL GetDropdownItemString_(const TCHAR *szCtrlName, const int nIdx, TCHAR *szText, int *nSize);
	//设置dropdown 项 string
	int SetDropdownItemString_(const TCHAR *szCtrlName, const int nIdx, const TCHAR *szText, void *pData);
	//获取当前dropdown当前选择项
	int  GetDropdownSelIndex(const TCHAR *szCtrlName);
	//删除dropdown 某项
	BOOL DeleteDropdownItem_(const TCHAR *szCtrlName, const int nIdx);
	//选择某项
	BOOL SelectDropdownItem_(const TCHAR *szCtrlName, const int nIdx);
	//获取dropdown count
	int GetDropdownItemCount_(const TCHAR *szCtrlName);

	//list box
	int  InsertListItem(const TCHAR *szCtrlName, const TCHAR *szDspName, void *pData, int idx);
	int  AppendListItem(const TCHAR *szCtrlName, const char *szDspText, void *pData);
	int  AppendListSubItem(const TCHAR *szCtrlName, const int nIdx, const int nSubIdx, const char *szDspText);
	int  GetListSelItem(const TCHAR *szCtrlName);
	BOOL GetListItemInfo(const TCHAR *szCtrlName, TCHAR *szDspName, void **pData, int idx);
	int  GetListCount(const TCHAR *szCtrlName);
	BOOL DeleteListItem(const TCHAR *szCtrlName, const int nIdx);
	BOOL ListKeyDownEvent(const TCHAR *szCtrlName, WORD wKey);
	BOOL RemoveListItem(const TCHAR *szCtrlName, int idx);
	BOOL SetListSelItem(const TCHAR *szCtrlName, int idx);
	BOOL AddEmotion(const TCHAR *szCtrlName, const char *szFileName, const char *szEmotionTag, 
		       const char *szEmotionShortCut, const char *szEmotionComment);
	BOOL GetSelEmotion(const TCHAR *szCtrlName, char *szFileName, int *nNameSize, char *szTag, int *nTagSize);
	//
	int  GetDisplayEmotionCount(const TCHAR *szCtrlName);
	//
	BOOL ClearAllEmotion(const TCHAR *szCtrlName);
	//scintilla 
	BOOL SetScintKeyWord_(const TCHAR *szCtrlName, const TCHAR *szKeyWord);
	BOOL SetScintStyle_(const TCHAR *szCtrlName, int nStyle, COLORREF clrFore, 
	       COLORREF clrBack, int nSize, TCHAR *szFace);
	//menu 相关
	BOOL CreateSkinMenu_(const TCHAR *szMenuName);
	//
	BOOL MenuAppendItem(const TCHAR *szMenuName, UINT nParentId, const TCHAR *szMenuCaption, int nMenuId);
	//
	BOOL MenuGetItemCaption(const TCHAR *szMenuName, int nMenuId, TCHAR *szwCaption, int *nSize);
	//
	BOOL PopTrackSkinMenu_(const TCHAR *szMenuName, UINT uFlags, const int X, const int Y);
	//
	BOOL DestroySkinMenu_(const TCHAR *szMenuName);
	//
	BOOL GraySkinMenu_(const TCHAR *szMenuName, UINT uMenuID, BOOL bGray);
	//
	BOOL SetMenuChecked(const TCHAR *szMenuName, UINT uMenuID, BOOL bChecked);
	//
	BOOL SetMenuItemAttr(const TCHAR *szMenuName, UINT uMenuId, const TCHAR *szAttrName, const TCHAR *szValue);

	//
	BOOL AddAutoShortCut(const TCHAR *szCtrlName, const TCHAR *szCaption, 
	                 const TCHAR *szFileName, const int nImageId, const TCHAR *szTip, TCHAR *szFlag);
protected:
 	//CWindowWnd overridable
	LPCTSTR GetWindowClassName() const;
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnFinalMessage(HWND hWnd);
	UINT GetClassStyle() const;

	
	virtual BOOL Init();
	virtual POINT MinTrackSize();
	virtual POINT MaxTrackSize();
    virtual BOOL  OnCloseQuery();
protected:
	//message handlers
	virtual BOOL OnCreate(LRESULT &lRes);
	virtual BOOL OnNcHitTest(const POINT &pt, LRESULT &lRes);
	virtual BOOL OnWindowPosChanging(WINDOWPOS *pPos, LRESULT &lRes);
	virtual BOOL OnNCPaint(WPARAM wParam, LPARAM lParam, LRESULT &lRes);
	virtual BOOL OnSize(WPARAM wParam, LPARAM lParam, LRESULT &lRes);
	virtual BOOL OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam, LRESULT &lRes);
	virtual BOOL OnWindowPosChanged(WINDOWPOS *pPos, LRESULT &lRes);
	virtual BOOL OnDestroy(WPARAM wParam, LPARAM lParam, LRESULT &lRes);
	virtual BOOL OnKillFocus(WPARAM wParam, LPARAM lParam, LRESULT &lRes);
	virtual BOOL OnWMActive(WPARAM wParam, LPARAM lParam, LRESULT &lRes);
	//notification handlers
	virtual BOOL OnNotifyClickMinBtn(TNotifyUI &msg);
	virtual BOOL OnNotifyClickMaxBtn(TNotifyUI &msg);
	virtual BOOL OnNotifyClickRestoreBtn(TNotifyUI &msg);
	virtual BOOL OnNotifyClickCloseBtn(TNotifyUI &msg);

	virtual void SetWindowRgn(const RECT& rcWnd);

	//operations
	void SetBkgndImage(UINT uNewID, const StretchFixed &sf, BOOL bStretchChange);
	void HideCaption(BOOL bHide = TRUE);
	void HideMinBtn(BOOL bHide = TRUE);
	void HideMaxRestoreBtn(BOOL bHide = TRUE);
	void EnableResize(BOOL bEnable);
	void SetCtrlVisible(const CStdString &strCtrlName, BOOL bVisible = TRUE);
	void EnableCtrl(const CStdString &strCtrlName, BOOL bEnable);

private:
	void ResizeFrame();
	void LoadBkgndImage(UINT uImageId);
	void CombineRegion(HRGN hDst, int x, int y, int cxOffset, int cyOffset);
	static DWORD GetTypeByName(LPCTSTR pstrName);
protected:
	CSmartPaintManager m_paintMgr;
	CStdString m_szWndClassName;
	std::string m_strWindowName;
	CWindowWnd *m_pParent;
	CGraphicPlus m_graphBkgnd;
	CLabelPanelUI *m_pTitle;
	CImageButtonUI *m_pBtnMin;
	CImageButtonUI *m_pBtnMaxRestore;
	CHorizontalLayoutUI *m_pCaption;
private:
	void *m_pOverlapped;
	BOOL m_bCanActived;
	BOOL m_bDockDesktop;
	std::map<UINT, UINT> m_OrderMsgList; 
	LPSKIN_WINDOW_EVENT_CALLBACK m_pEventCallBack;
	LPSKIN_WINDOW_MESSAGE_CALLBACK m_pMsgCallBack;
	CRect m_rcLastWndPos;
	CSmartFrame *m_pFrame;
	CSize m_minSize;
	CSize m_maxSize;
	CSize m_captionSize;



	BOOL m_bResize;
	

	//
	LPWINDOWDESTROY m_pWinDestroy;
};

#endif
