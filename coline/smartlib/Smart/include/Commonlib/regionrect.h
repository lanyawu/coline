#ifndef __REGIONRECT_H____
#define __REGIONRECT_H____

#include <commonlib/types.h>
#include <vector>

#define MAXCHANGES_BUF      2000
#define SCREEN_SCREEN       11
#define BLIT                12
#define SOLIDFILL           13
#define BLEND               14
#define TRANS               15
#define PLG                 17
#define TEXTOUT             18

typedef BOOL (WINAPI* pEnumDisplayDevices)(PVOID, DWORD, PVOID, DWORD);
typedef LONG (WINAPI* pChangeDisplaySettingsExA)(LPCSTR, LPDEVMODEA, HWND, DWORD, LPVOID);
//λͼ��Ϣ
typedef struct CDesktopBitmapInfo
{
	BOOL bTrueColor; //�Ƿ����
	BITMAPINFO bmInfo;
	RGBQUAD clMap[256];  //color map
}DESKTOP_BITMAP_INFO, *LPDESKTOP_BITMAP_INFO;

//�ı�������¼
typedef struct ChangesItem
{
	ULONG type;  //screen_to_screen, blit, newcache,oldcache
	RECT rc;	
	POINT pt;
}CHANGES_ITEM, *LPCHANGES_ITEM;

//�ı��BUFFER
typedef struct ChangesBuffer
{
	DWORD dwCounter;
	CHANGES_ITEM Data[MAXCHANGES_BUF];
}CHANGERS_BUFFER, *LPCHANGERS_BUFFER;

class COMMONLIB_API CRegionRect
{
public:
	//constructor 
	CRegionRect(void);
	CRegionRect(int x1, int y1, int x2, int y2);
	CRegionRect(const CRegionRect &rc);
	~CRegionRect(void);
public:
	//
	void SetRegionRect(int x1, int y1, int x2, int y2);
	void SetRegionRect(const RECT &rc);
	//�ཻ����
	CRegionRect Intersect(const CRegionRect &rc) const;
	CRegionRect Intersect(const RECT &rc) const;
	//��������
	CRegionRect Union(const CRegionRect &rc) const;
	//ƫ��λ��
	CRegionRect Translate(const int x, const int y) const;
    //
	RECT GetRect() const;
	//�Ƿ����
	BOOL Equal(const CRegionRect &rc);
	//������
	BOOL Enclose_By(const CRegionRect &rc);
	//�Ƿ�Ϊ��
	BOOL Is_Empty();
	//reset
	void Reset(int x1, int y1, int x2, int y2);
	//���
	void Clear();
	//��ȡ����
	int  GetLeft() const;
	int  GetTop() const;
	int  GetRight() const;
	int  GetBottom() const;
	//���
	inline int Width() const { return (m_nRight - m_nLeft); };
	inline int Height() const { return (m_nBottom - m_nTop); };
private:
	int m_nLeft;
	int m_nTop;
	int m_nRight;
	int m_nBottom;
};

typedef std::vector<CRegionRect> CRectVectors;

#endif
