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
//位图信息
typedef struct CDesktopBitmapInfo
{
	BOOL bTrueColor; //是否真彩
	BITMAPINFO bmInfo;
	RGBQUAD clMap[256];  //color map
}DESKTOP_BITMAP_INFO, *LPDESKTOP_BITMAP_INFO;

//改变的区域记录
typedef struct ChangesItem
{
	ULONG type;  //screen_to_screen, blit, newcache,oldcache
	RECT rc;	
	POINT pt;
}CHANGES_ITEM, *LPCHANGES_ITEM;

//改变的BUFFER
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
	//相交区域
	CRegionRect Intersect(const CRegionRect &rc) const;
	CRegionRect Intersect(const RECT &rc) const;
	//联合区域
	CRegionRect Union(const CRegionRect &rc) const;
	//偏移位置
	CRegionRect Translate(const int x, const int y) const;
    //
	RECT GetRect() const;
	//是否相等
	BOOL Equal(const CRegionRect &rc);
	//被包含
	BOOL Enclose_By(const CRegionRect &rc);
	//是否为空
	BOOL Is_Empty();
	//reset
	void Reset(int x1, int y1, int x2, int y2);
	//清除
	void Clear();
	//获取数据
	int  GetLeft() const;
	int  GetTop() const;
	int  GetRight() const;
	int  GetBottom() const;
	//宽度
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
