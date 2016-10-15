#pragma once

#include <map>
#include <string>
#include <CommonLib/GraphicPlus.h>
#include <CommonLib/GdiPlusImage.h>
#include <UILib/UIContainer.h>
#include <UILib/uiresource.h>
#include <UILib/UIMenu.h>

#define MAX_NODE_NAME_SIZE 32 //�ڵ�������󳤶�

#define TREE_NODE_VERT_DISTANCE      4   //�ڵ���������
#define TREE_NODE_HORZ_DISTANCE      10  //�ڵ��ĺ�����
#define LEAF_NODE_IMAGE_WIDTH_BIG    40  //Ҷ�ӽڵ��ͼ���� ��ͼ��
#define LEAF_NODE_IMAGE_HEIGHT_BIG   40  //Ҷ�ӽڵ��ͼ��߶� ��ͼ��
#define LEAF_NODE_IMAGE_WIDTH_SMALL  20 //Ҷ�ӿ�� Сͼ��
#define LEAF_NODE_IMAGE_HEIGHT_SMALL 20 //Ҷ�Ӹ߶� Сͼ��

#define GROUP_NODE_IMAGE_WIDTH  16  //����ڵ��ͼ����
#define GROUP_NODE_IMAGE_HEIGHT 16  //����ڵ��ͼ��߶�
#define GROUP_NODE_TEXT_HEIGHT  16 //�ڵ����ָ߶�

#define CHECK_STATUS_IMAGE_WIDTH  8
#define CHECK_STATUS_IMAGE_HEIGHT 8

//ѡ��״̬
#define CHECK_STATUS_NORMAL  0
#define CHECK_STATUS_CHECKED 1
#define CHECK_STATUS_GRAY    2
//״̬ͼ���С
#define GROUP_NODE_IMAGE_STATUS_W     12
#define GROUP_NODE_IMAGE_STATUS_H     6

#define MAX_RECT_LINKS_COUNT 6

class CTreeNodeItem;

class IUITreeViewApp
{
public:
	virtual DWORD GetDefaultImage(BOOL bGray) = 0; //��ȡĬ�ϵ�ͼ��
	virtual DWORD GetDefaultGrpImageId() = 0; //����ڵ�
	virtual DWORD GetCheckStatusImageId() = 0; //
	virtual int  GetStatusImage(int &w, int &h) = 0; //��ȡ״̬ͼ��
	virtual BOOL FreeNodeExtData(TreeNodeType nodeType, void **pData) = 0; //�ͷŽڵ���չ����
	virtual void InvalidateItem(RECT &rc) = 0; //��������
	virtual CPaintManagerUI *GetPaintManager() = 0;
	virtual DWORD GetTreeViewTop() = 0; //��ȡ��������
	virtual DWORD GetLeafNodeWidth() = 0;
	virtual DWORD GetLeafNodeHeight() = 0;
	virtual const char *GetNodeKey(TreeNodeType nodeType, const void *pData) = 0;
	virtual void  OnDeleteNode(CTreeNodeItem *pNode) = 0; //ɾ���ڵ�
	virtual BOOL GroupNodeSelState() const = 0;//��ڵ��Ƿ����ѡ��״̬
	virtual BOOL ShowCheckStatus() const = 0; //�Ƿ����ѡ��״̬
	virtual BOOL ShowExtractData() = 0; //�Ƿ������չ����
	virtual BOOL ShowCustomPicture() = 0; //��ʾ����ͼƬ
	virtual BOOL ShowPersonLabel() = 0; //�Ƿ���ʾ����ǩ��
};


//�ڵ����� ���ݽڵ㰲ȫ
class CTreeNodeItem
{
public:
	CTreeNodeItem(CTreeNodeItem *pParent, CTreeNodeType NodeType, IUITreeViewApp *pTreeView);
	virtual ~CTreeNodeItem();
public:
	void *GetData();
	void Draw(HDC hdc, DWORD &dwTop,  int &dwScrollPos, DWORD dwLeft, const RECT &rc, 
		    BOOL bNodeRoot, const BOOL &bIsShowCount); //�ڴ˿��ڻ���
	void Clear(); //����ӽڵ�

	CTreeNodeItem *Selected(); //�Ƿ�ѡ�� �����ӽڵ�
	void SetSelect(BOOL bSelected); //�����Ƿ�ѡ��
	TCHAR *GetName();
	TCHAR *GetLabel();
	BYTE  GetStatus() const;
	CTreeNodeItem *XYInItem(int x, int y); //���������Ƿ��ڱ��ڵ��� �����ӽڵ�
	CTreeNodeItem *GetNextNode(); //��ȡ��һ���ڵ�
	CTreeNodeItem *GetFirstNode(); //��ȡ��һ���ڵ�
	CTreeNodeItem *GetChildNode(int iIndex);//��ȡ��iIndex���ڵ�
	void SetGraph(const char *szFileName); //���ýڵ�ͼ�� 
	void SetExtraImageId(const int nImageId);
	void SetNodeName(const char *szName); //���ýڵ�����
	void SetNodeName(const TCHAR *szName); //���ýڵ�����
	void SetNodeData(void *pData); //���ýڵ�����
	void SetLabel(const char *szLabel); //���ø���ǩ��
	void SetLabel(const TCHAR *szLabel); //���ø���ǩ��
	void SetExtraData(const TCHAR *szExtra); //������չ����
	void SetStatus(BYTE byteStatus); //����״̬
	void SetAllUserStatus(BYTE byteStatus); //���������û�״̬
	void SetNodeId(DWORD dwNodeId); //�ڵ�Ψһ��ʶ
	void SetCheckStatus(const int nStatus); //
	int  GetCheckStatus();
	CTreeNodeItem *AddChildNode(const DWORD dwId, const char *szName, CTreeNodeType NodeType,
		              const char *szLabel, void *pData); //����һ���½ڵ�
	CTreeNodeItem *AddChildNode(const DWORD dwId, const TCHAR *szName, CTreeNodeType NodeType, const TCHAR *szLabel,
		                void *pData, const TCHAR *szImageFileName, const TCHAR *szExtraData); //����һ���½ڵ�
	//����һ���ڵ�
	CTreeNodeItem *AdjustChildNode(const TCHAR *szName, CTreeNodeType NodeType, void *pData, BOOL bAdd, BOOL bRecursive);
	//
	BOOL DeleteNodeById(const int nId, CTreeNodeType tnType);
	BOOL DeleteNode(CTreeNodeItem *pNode); //ɾ��һ���ڵ�
	BOOL DeleteNode(int iIndex);//ɾ���ڵ�
	void Expanded(BOOL bRecursived = FALSE); //��չ�ڵ�
	void SelectAll(BOOL bRecursived = FALSE); //ȫѡ
	void UnSelected(BOOL bRecursived = FALSE); //��ѡ
	void DeleteSelected(BOOL bRecursived = FALSE); //ɾ��ѡ��Ľڵ�
	BOOL IsExpanded();
	RECT GetImageRect();
	RECT GetCheckRect();
	DWORD CalcHeight(DWORD &dwTop); //����߶�
	BOOL  CalcHeightToKey(const char *szKey, DWORD &dwTop); //���㵽ĳ���ڵ�ĸ߶�
	BOOL  AdjustScrollPos(int &dwTop, int &dwScrollPos); //����������λ��
	void SetFocus(BOOL bIsFocus);
	CTreeNodeType GetNodeType() const;
	DWORD GetNodeId();
	void Reduce(); //�����ڵ�
	DWORD GetChildCount(); //�ӽڵ����
	CTreeNodeItem *GetParent(); //��ȡ���ڵ�
	void ClearFocus(); //������� �����½���
	void ClearSelected(); //���ѡ�� �����½���
	void Sort(LPSKIN_COMPARENODE pCompare, BOOL bRecursived = FALSE); //����
	BOOL GetNodeByKey(const char *szKey, TCHAR *szName, int *nNameLen, void **pSelNode,
		         CTreeNodeType *tnType, void **pData);
	//
	BOOL GetNodeById(const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData);
	//�������ļ�
	void SaveToStream(std::ofstream &ofs, char *szPreChars, BYTE byteFileType);
	//�ӽڵ㷢���仯
	void OnNodeChange();
	//ͳ�����нڵ��µ��ӽڵ��������߽ڵ���
	void StatChildNode(DWORD &dwChildCount, DWORD &dwOnlineCount);
	//get selected user lsit
	BOOL GetSelectUserList(std::string &strUsers, BOOL bRecursive);
	//getuser list
	BOOL GetUserList(std::string &strUsers, BOOL bRecursive);
	//��ȡҶ�ӽڵ���
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
	void Invalidate(); //���½���
	//���ƴ˽ڵ�
	void DrawItem(HDC hDc, const RECT &rc, const BOOL &bIsShowCount, const BOOL bInvalidate = FALSE);
	void DrawItemBkground(HDC hdc, const RECT& rc);
	void QuickSort(LPSKIN_COMPARENODE pCompare, int nLow, int nHigh);
private:
	TCHAR m_szName[MAX_NODE_NAME_SIZE];
	TCHAR *m_szLabel;
	CStdString m_strExtra; //��չ����
	BYTE  m_byteStatus; //�û�״̬
	DWORD m_dwNodeId;  //�ڵ��ʶ
	CGraphicPlus m_pImage; //ͼ��
	CGraphicPlus m_pGrayImage; //�һ�ͼƬ
	int  m_nExtraImageId;
	void *m_pData; //��չ���ݣ����������ڲ������ͷŴ��ڴ�
	RECT m_rcItem; //ռ�õĻ��ƿ�
	RECT m_rcImage; //ͼ����ÿ�
	RECT m_rcCheck; //
	BOOL m_bSelected;
	BOOL m_bExpanded; //�Ƿ���չ
	BOOL m_bIsFocus;  //�Ƿ��ǻ�ý���״̬

	RECT m_rcLinks[MAX_RECT_LINKS_COUNT];
	int  m_nLinksCount;
	//�ӽڵ�
	CTreeNodeItem **m_Nodes; //�ڵ��б�
	CTreeNodeItem *m_pParent; //���ڵ�
	DWORD m_dwNodeCount; //�ڵ����
	DWORD m_dwCurrSeq;   //��ǰ
	CTreeNodeType m_NodeType;
	IUITreeViewApp *m_pTreeView;

	DWORD  m_dwLeafCount; //Ҷ�ӽڵ����
	DWORD  m_dwOnlineCount; //�����û�����
	//
	int m_nCheckStatus; //ѡ��״̬ 0-δѡ�� 1-ѡ�� 2-��ѡ
	//
	std::string m_strPersonImageFile;
};

//���οؼ�
class CUITreeView :public CContainerUI, public IUITreeViewApp
{
public:
	CUITreeView(void);
	~CUITreeView(void);
public:
	//IUITreeViewApp �麯��
	virtual DWORD GetDefaultImage(BOOL bGray) ; //��ȡĬ�ϵ�ͼ��
	virtual DWORD GetDefaultGrpImageId(); //����ڵ�
	virtual DWORD GetCheckStatusImageId();
	virtual int  GetStatusImage(int &w, int &h);
	virtual BOOL FreeNodeExtData(TreeNodeType nodeType, void **pData); //�ͷŽڵ���չ����
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

	//CControlUI �麯��
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
	//������нڵ�
	void Clear();

	//TreeView��غ���
	CTreeNodeItem *GetNode();
	CTreeNodeItem *GetSelected(); //��ȡ��ǰѡ��Ľڵ�
	CTreeNodeItem *SelectNode(CTreeNodeItem* pNode);
	CTreeNodeItem *GetFocusNode(); //��ȡ��ǰ����ڵ�
	void SetNodeIconType(CTreeNodeIconType IconType); //���ýڵ�ͼ������
	CTreeNodeIconType GetNodeIconType() const;
	//�����Ƿ���ʾͳ������
	void SetShowStatData(BOOL bIsShow);
	//�Ƿ���ʾ��չ����
	void SetShowExtraData(BOOL bIsShow); //
	//������ĳ���չ��
	void ScrollToNodeByKey(const char *szKey);
	//
	LPVOID SetExtraData(const char *szKey, const TCHAR *szExtraData, BOOL bMulti);
	//
	LPVOID SetImageFile(const char *szKey, const char *szImageFileName, BOOL bMulti);
	//
	LPVOID SetExtraImageFile(const char *szKey, const int nImageId, BOOL bMulti);
	
	//
	BOOL GetTreeNodeById(const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData);
	//�����ͷ����ݻص�
	void SetFreeNodeFun(LPSKIN_FREE_NODE_EXDATA pFun);
	//���ùؼ���
	void SetGetNodeKeyFun(LPSKIN_GET_TREE_NODE_KEY pFun);
	//����һ���ļ�
	void SaveToFile(char *szFileName, BYTE byteFileType = 0);
    void SetDefaultImage(const char *szFileName);
	//������ڵ�ѡ��״̬
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
	void AdjustScrollBarRect(); //����������λ��
	void AdjustScrollBarPos(int &dwScrollPos, BOOL bNext); //����������

	//menu
	void LoadAllMenu();
	void DestroyAllMenu();
	//
	void MoveToInitPos();
private:
	CTreeNodeItem *m_RootNode; //���ڵ㣬�˽ڵ㲻����
 
	//menu
	CStdString m_strGroupMenu;
	CStdString m_strLeafMenu; 
	CMenuUI *m_pGroupMenu;
	CMenuUI *m_pLeafMenu;

	//
	LPSKIN_FREE_NODE_EXDATA  m_pFreeNodeFun; //�ͷŽڵ����ݺ����ص�
	LPSKIN_GET_TREE_NODE_KEY m_pGetNodeKeyFun; //��ȡ�ڵ�ؼ���
	CTreeNodeIconType m_IconType; //ͼ�����ͣ� ��ͼ�껹��Сͼ�ꣿ
	DWORD m_dwLeafNodeWidth;  //Ҷ�ӽڵ���
	DWORD m_dwLeafNodeHeight; //Ҷ�ӽڵ�߶�
	RECT m_rcClient; //ʵ�ʻ�������
	DWORD m_dwHeight; //ʵ����Ҫ���Ƶĸ߶�
	DWORD m_dwScrollLine;//����ʱ�ƶ��ĸ߶�,��ڵ��Сͼ�����һ��,��ͼ������

	//��ǰѡ��Ľڵ�
	CTreeNodeItem *m_pFocusNode;  //��ǰѡ��Ľ���ڵ�
	CTreeNodeItem *m_pSelectNode; //��ǰѡ��Ľڵ�
	CTreeNodeItem *m_pDragExpandNode;   //��ǰ��ҷչ���Ľڵ�
	BOOL m_bShowStatData; //��ʾͳ������
	BOOL m_bGroupNodeSelState;//��ڵ��Ƿ���ʾѡ��״̬
	BOOL m_bShowCheckStatus; //
	BOOL m_bShowExtraData;   //
	BOOL m_bShowCustomPicture; //��ʾ����ͼƬ
	BOOL m_bShowPersonLabel; //�Ƿ���ʾ����ǩ��
	//
	DWORD m_dwGroupImageId; //����
	DWORD m_dwBigStatusImageId;  //æµͼ��
	DWORD m_dwSmallStatusImageId; //�뿪״̬ͼ��
	DWORD m_dwDefaultImageId; //Ĭ�Ͻڵ�ͼ��
	DWORD m_dwDefaultGrayImageId; //Ĭ�ϻ�ɫͼ��
	DWORD m_dwCheckStatusImageId; //

	//
	int m_iInitPos; //��ʼ��λ��
};

