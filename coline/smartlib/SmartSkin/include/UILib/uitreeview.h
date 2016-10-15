#pragma once

#include <map>
#include <string>
#include <CommonLib/GraphicPlus.h>
#include <CommonLib/GdiPlusImage.h>
#include <UILib/UIContainer.h>
#include <UILib/uiresource.h>
#include <UILib/UIMenu.h>

#define MAX_NODE_NAME_SIZE 32 //节点名称最大长度

#define TREE_NODE_VERT_DISTANCE      4   //节点间的纵向间隔
#define TREE_NODE_HORZ_DISTANCE      10  //节点间的横向间隔
#define LEAF_NODE_IMAGE_WIDTH_BIG    40  //叶子节点的图像宽度 大图标
#define LEAF_NODE_IMAGE_HEIGHT_BIG   40  //叶子节点的图像高度 大图标
#define LEAF_NODE_IMAGE_WIDTH_SMALL  20 //叶子宽度 小图标
#define LEAF_NODE_IMAGE_HEIGHT_SMALL 20 //叶子高度 小图标

#define GROUP_NODE_IMAGE_WIDTH  16  //分组节点的图像宽度
#define GROUP_NODE_IMAGE_HEIGHT 16  //分组节点的图像高度
#define GROUP_NODE_TEXT_HEIGHT  16 //节点文字高度

#define CHECK_STATUS_IMAGE_WIDTH  8
#define CHECK_STATUS_IMAGE_HEIGHT 8

//选中状态
#define CHECK_STATUS_NORMAL  0
#define CHECK_STATUS_CHECKED 1
#define CHECK_STATUS_GRAY    2
//状态图标大小
#define GROUP_NODE_IMAGE_STATUS_W     12
#define GROUP_NODE_IMAGE_STATUS_H     6

#define MAX_RECT_LINKS_COUNT 6

class CTreeNodeItem;

class IUITreeViewApp
{
public:
	virtual DWORD GetDefaultImage(BOOL bGray) = 0; //获取默认的图像
	virtual DWORD GetDefaultGrpImageId() = 0; //分组节点
	virtual DWORD GetCheckStatusImageId() = 0; //
	virtual int  GetStatusImage(int &w, int &h) = 0; //获取状态图标
	virtual BOOL FreeNodeExtData(TreeNodeType nodeType, void **pData) = 0; //释放节点扩展数据
	virtual void InvalidateItem(RECT &rc) = 0; //更新区域
	virtual CPaintManagerUI *GetPaintManager() = 0;
	virtual DWORD GetTreeViewTop() = 0; //获取顶点坐标
	virtual DWORD GetLeafNodeWidth() = 0;
	virtual DWORD GetLeafNodeHeight() = 0;
	virtual const char *GetNodeKey(TreeNodeType nodeType, const void *pData) = 0;
	virtual void  OnDeleteNode(CTreeNodeItem *pNode) = 0; //删除节点
	virtual BOOL GroupNodeSelState() const = 0;//组节点是否绘制选中状态
	virtual BOOL ShowCheckStatus() const = 0; //是否绘制选择状态
	virtual BOOL ShowExtractData() = 0; //是否绘制扩展数据
	virtual BOOL ShowCustomPicture() = 0; //显示个性图片
	virtual BOOL ShowPersonLabel() = 0; //是否显示个性签名
};


//节点数据 数据节点安全
class CTreeNodeItem
{
public:
	CTreeNodeItem(CTreeNodeItem *pParent, CTreeNodeType NodeType, IUITreeViewApp *pTreeView);
	virtual ~CTreeNodeItem();
public:
	void *GetData();
	void Draw(HDC hdc, DWORD &dwTop,  int &dwScrollPos, DWORD dwLeft, const RECT &rc, 
		    BOOL bNodeRoot, const BOOL &bIsShowCount); //在此框内绘制
	void Clear(); //清除子节点

	CTreeNodeItem *Selected(); //是否被选择 包括子节点
	void SetSelect(BOOL bSelected); //设置是否被选择
	TCHAR *GetName();
	TCHAR *GetLabel();
	BYTE  GetStatus() const;
	CTreeNodeItem *XYInItem(int x, int y); //这个座标点是否在本节点内 包括子节点
	CTreeNodeItem *GetNextNode(); //获取下一个节点
	CTreeNodeItem *GetFirstNode(); //获取第一个节点
	CTreeNodeItem *GetChildNode(int iIndex);//获取第iIndex个节点
	void SetGraph(const char *szFileName); //设置节点图标 
	void SetExtraImageId(const int nImageId);
	void SetNodeName(const char *szName); //设置节点名称
	void SetNodeName(const TCHAR *szName); //设置节点名称
	void SetNodeData(void *pData); //设置节点数据
	void SetLabel(const char *szLabel); //设置个性签名
	void SetLabel(const TCHAR *szLabel); //设置个性签名
	void SetExtraData(const TCHAR *szExtra); //设置扩展数据
	void SetStatus(BYTE byteStatus); //设置状态
	void SetAllUserStatus(BYTE byteStatus); //设置所有用户状态
	void SetNodeId(DWORD dwNodeId); //节点唯一标识
	void SetCheckStatus(const int nStatus); //
	int  GetCheckStatus();
	CTreeNodeItem *AddChildNode(const DWORD dwId, const char *szName, CTreeNodeType NodeType,
		              const char *szLabel, void *pData); //加入一个新节点
	CTreeNodeItem *AddChildNode(const DWORD dwId, const TCHAR *szName, CTreeNodeType NodeType, const TCHAR *szLabel,
		                void *pData, const TCHAR *szImageFileName, const TCHAR *szExtraData); //加入一个新节点
	//增减一个节点
	CTreeNodeItem *AdjustChildNode(const TCHAR *szName, CTreeNodeType NodeType, void *pData, BOOL bAdd, BOOL bRecursive);
	//
	BOOL DeleteNodeById(const int nId, CTreeNodeType tnType);
	BOOL DeleteNode(CTreeNodeItem *pNode); //删除一个节点
	BOOL DeleteNode(int iIndex);//删除节点
	void Expanded(BOOL bRecursived = FALSE); //扩展节点
	void SelectAll(BOOL bRecursived = FALSE); //全选
	void UnSelected(BOOL bRecursived = FALSE); //反选
	void DeleteSelected(BOOL bRecursived = FALSE); //删除选择的节点
	BOOL IsExpanded();
	RECT GetImageRect();
	RECT GetCheckRect();
	DWORD CalcHeight(DWORD &dwTop); //计算高度
	BOOL  CalcHeightToKey(const char *szKey, DWORD &dwTop); //计算到某个节点的高度
	BOOL  AdjustScrollPos(int &dwTop, int &dwScrollPos); //调整滚动条位置
	void SetFocus(BOOL bIsFocus);
	CTreeNodeType GetNodeType() const;
	DWORD GetNodeId();
	void Reduce(); //收缩节点
	DWORD GetChildCount(); //子节点个数
	CTreeNodeItem *GetParent(); //获取父节点
	void ClearFocus(); //清除焦点 不更新界面
	void ClearSelected(); //清除选择 不更新界面
	void Sort(LPSKIN_COMPARENODE pCompare, BOOL bRecursived = FALSE); //排序
	BOOL GetNodeByKey(const char *szKey, TCHAR *szName, int *nNameLen, void **pSelNode,
		         CTreeNodeType *tnType, void **pData);
	//
	BOOL GetNodeById(const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData);
	//导出到文件
	void SaveToStream(std::ofstream &ofs, char *szPreChars, BYTE byteFileType);
	//子节点发生变化
	void OnNodeChange();
	//统计所有节点下的子节点数和在线节点数
	void StatChildNode(DWORD &dwChildCount, DWORD &dwOnlineCount);
	//get selected user lsit
	BOOL GetSelectUserList(std::string &strUsers, BOOL bRecursive);
	//getuser list
	BOOL GetUserList(std::string &strUsers, BOOL bRecursive);
	//获取叶子节点数
	DWORD GetLeafCount();
	//
	LPVOID UpdateUserStatusToNode_(const char *szUserName, const char *szStatus, BOOL bMulti);
	//
	LPVOID UpdateUserLabelToNode_(const char *szUserName,  const char *szUTF8Label,  BOOL bMulti);
	//
	LPVOID UpdateImageFile(const char *szKey, const char *szImageFile, BOOL bMulti);
	//
	LPVOID UpdateExtraImageFile(const char *szKey, const int nExtraImageId, BOOL bMulti);
	//
	LPVOID UpdateExtraData(const char *szKey, const TCHAR *szExtra, BOOL bMulti);
private:
	void Invalidate(); //更新界面
	//绘制此节点
	void DrawItem(HDC hDc, const RECT &rc, const BOOL &bIsShowCount, const BOOL bInvalidate = FALSE);
	void DrawItemBkground(HDC hdc, const RECT& rc);
	void QuickSort(LPSKIN_COMPARENODE pCompare, int nLow, int nHigh);
private:
	TCHAR m_szName[MAX_NODE_NAME_SIZE];
	TCHAR *m_szLabel;
	CStdString m_strExtra; //扩展数据
	BYTE  m_byteStatus; //用户状态
	DWORD m_dwNodeId;  //节点标识
	CGraphicPlus m_pImage; //图标
	CGraphicPlus m_pGrayImage; //灰化图片
	int  m_nExtraImageId;
	void *m_pData; //扩展数据，本函数体内不负责释放此内存
	RECT m_rcItem; //占用的绘制框
	RECT m_rcImage; //图像点用框
	RECT m_rcCheck; //
	BOOL m_bSelected;
	BOOL m_bExpanded; //是否扩展
	BOOL m_bIsFocus;  //是否是获得焦点状态

	RECT m_rcLinks[MAX_RECT_LINKS_COUNT];
	int  m_nLinksCount;
	//子节点
	CTreeNodeItem **m_Nodes; //节点列表
	CTreeNodeItem *m_pParent; //父节点
	DWORD m_dwNodeCount; //节点个数
	DWORD m_dwCurrSeq;   //当前
	CTreeNodeType m_NodeType;
	IUITreeViewApp *m_pTreeView;

	DWORD  m_dwLeafCount; //叶子节点个数
	DWORD  m_dwOnlineCount; //在线用户个数
	//
	int m_nCheckStatus; //选择状态 0-未选中 1-选中 2-灰选
	//
	std::string m_strPersonImageFile;
};

//树形控件
class CUITreeView :public CContainerUI, public IUITreeViewApp
{
public:
	CUITreeView(void);
	~CUITreeView(void);
public:
	//IUITreeViewApp 虚函数
	virtual DWORD GetDefaultImage(BOOL bGray) ; //获取默认的图像
	virtual DWORD GetDefaultGrpImageId(); //分组节点
	virtual DWORD GetCheckStatusImageId();
	virtual int  GetStatusImage(int &w, int &h);
	virtual BOOL FreeNodeExtData(TreeNodeType nodeType, void **pData); //释放节点扩展数据
	virtual void InvalidateItem(RECT &rc);
	virtual CPaintManagerUI * GetPaintManager();
	virtual DWORD GetLeafNodeWidth();
	virtual DWORD GetLeafNodeHeight();
    virtual void  OnDeleteNode(CTreeNodeItem *pNode);
	virtual DWORD GetTreeViewTop();
	virtual BOOL ShowCheckStatus() const;
	virtual BOOL ShowExtractData();
	virtual BOOL ShowCustomPicture();
	virtual BOOL ShowPersonLabel();
	virtual const char *GetNodeKey(TreeNodeType nodeType, const void *pData);

	//CControlUI 虚函数
	LPCTSTR GetClass() const;
	void Notify(TNotifyUI &msg);
	void Event(TEventUI& event);
	void DoPaint(HDC hDC, const RECT & rcPaint);
    SIZE EstimateSize(SIZE szAvailable);
	void SetPos(RECT rc);
	void Init();
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	//
	BOOL GetUserCount(void *pParentNode, DWORD &dwTotalCount, DWORD &dwOnlineCount);
	//清除所有节点
	void Clear();

	//TreeView相关函数
	CTreeNodeItem *GetNode();
	CTreeNodeItem *GetSelected(); //获取当前选择的节点
	CTreeNodeItem *SelectNode(CTreeNodeItem* pNode);
	CTreeNodeItem *GetFocusNode(); //获取当前焦点节点
	void SetNodeIconType(CTreeNodeIconType IconType); //设置节点图标类型
	CTreeNodeIconType GetNodeIconType() const;
	//设置是否显示统计数据
	void SetShowStatData(BOOL bIsShow);
	//是否显示扩展数据
	void SetShowExtraData(BOOL bIsShow); //
	//滚动到某结点展现
	void ScrollToNodeByKey(const char *szKey);
	//
	LPVOID SetExtraData(const char *szKey, const TCHAR *szExtraData, BOOL bMulti);
	//
	LPVOID SetImageFile(const char *szKey, const char *szImageFileName, BOOL bMulti);
	//
	LPVOID SetExtraImageFile(const char *szKey, const int nImageId, BOOL bMulti);
	
	//
	BOOL GetTreeNodeById(const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData);
	//设置释放数据回调
	void SetFreeNodeFun(LPSKIN_FREE_NODE_EXDATA pFun);
	//设置关键词
	void SetGetNodeKeyFun(LPSKIN_GET_TREE_NODE_KEY pFun);
	//导出一个文件
	void SaveToFile(char *szFileName, BYTE byteFileType = 0);
    void SetDefaultImage(const char *szFileName);
	//绘制组节点选中状态
	void EnableGroupNodeSelState(BOOL bEnable);
	BOOL GroupNodeSelState() const;	
	BOOL SetTreeViewStatusOffline();
	BOOL GetNodeByKey(void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData);
	BOOL SortTree(void *pNode, LPSKIN_COMPARENODE pCompare,
	                            BOOL bRecursive, BOOL bParent);
	CTreeNodeItem *AdjustChildNode(void *pParentNode, const TCHAR *szName, CTreeNodeType NodeType, void *pData, 
		BOOL bAdd, BOOL bRecursive);
	BOOL DeleteNodeById(const int nId, CTreeNodeType tnType);
	//
	void SelectedAll(void *pNode, BOOL bRecursive);
	void UnSelected(void *pNode, BOOL bRecursive);
	void DeleteSelected(BOOL bRecursive);
	void GetSelectUsers(std::string &strUsers, BOOL bRecursive);
private:
	void AdjustScrollBarRect(); //调整滚动条位置
	void AdjustScrollBarPos(int &dwScrollPos, BOOL bNext); //调整滚动条

	//menu
	void LoadAllMenu();
	void DestroyAllMenu();
	//
	void MoveToInitPos();
private:
	CTreeNodeItem *m_RootNode; //根节点，此节点不绘制
 
	//menu
	CStdString m_strGroupMenu;
	CStdString m_strLeafMenu; 
	CMenuUI *m_pGroupMenu;
	CMenuUI *m_pLeafMenu;

	//
	LPSKIN_FREE_NODE_EXDATA  m_pFreeNodeFun; //释放节点数据函数回调
	LPSKIN_GET_TREE_NODE_KEY m_pGetNodeKeyFun; //获取节点关键词
	CTreeNodeIconType m_IconType; //图标类型， 大图标还是小图标？
	DWORD m_dwLeafNodeWidth;  //叶子节点宽度
	DWORD m_dwLeafNodeHeight; //叶子节点高度
	RECT m_rcClient; //实际绘制区域
	DWORD m_dwHeight; //实际需要绘制的高度
	DWORD m_dwScrollLine;//滚动时移动的高度,组节点和小图标滚动一行,大图标两行

	//当前选择的节点
	CTreeNodeItem *m_pFocusNode;  //当前选择的焦点节点
	CTreeNodeItem *m_pSelectNode; //当前选择的节点
	CTreeNodeItem *m_pDragExpandNode;   //当前拖曳展开的节点
	BOOL m_bShowStatData; //显示统计数据
	BOOL m_bGroupNodeSelState;//组节点是否显示选中状态
	BOOL m_bShowCheckStatus; //
	BOOL m_bShowExtraData;   //
	BOOL m_bShowCustomPicture; //显示个性图片
	BOOL m_bShowPersonLabel; //是否显示个性签名
	//
	DWORD m_dwGroupImageId; //分组
	DWORD m_dwBigStatusImageId;  //忙碌图标
	DWORD m_dwSmallStatusImageId; //离开状态图标
	DWORD m_dwDefaultImageId; //默认节点图标
	DWORD m_dwDefaultGrayImageId; //默认灰色图标
	DWORD m_dwCheckStatusImageId; //

	//
	int m_iInitPos; //初始化位置
};

