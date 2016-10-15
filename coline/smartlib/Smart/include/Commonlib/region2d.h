#ifndef __REGION2D_H___
#define __REGION2D_H___

#include <commonlib/guardlock.h>
#include "RegionRect.h"

class COMMONLIB_API CRegion2D
{
public:
	CRegion2D(void);
	CRegion2D(int x1, int y1, int x2, int y2);
	CRegion2D(const CRegionRect &rc);
	CRegion2D(const CRegion2D &rc);
	~CRegion2D(void);
public:
	//��ʼ��RGB
	BOOL CheckRgn(const BOOL bCreate);
	//�ཻ����
	BOOL Intersect(const CRegion2D &rc);
	BOOL Intersect(const CRegionRect &rc);
	//�ಢ����
	BOOL Union(const CRegion2D &rc);
	BOOL Union(const RECT &rc);
	BOOL Union(const CRegionRect &rc);
	BOOL Union(const HRGN &hRgn);
	//�������
	BOOL Subtract(const CRegion2D &rc);
    BOOL Subtract(const CRegionRect &rc);

    CRegion2D & operator = (const CRegion2D &rc);

	//����
	void Reset(int x1, int y1, int x2, int y2);
	//ƫ��
	void Translate(const int x, const int y);
	BOOL GetRects(CRectVectors &rects, BOOL bLeft2Right, BOOL bTop2Down);
	CRegionRect GetBoundingRect() const;
	RECT GetBoundRect() const;
    void Clear();
	BOOL Equal(const CRegion2D &rc) const;
	BOOL IsEmpty() const;
	HRGN GetHandle() const;
private:
	HRGN m_hRgn;
	CGuardLock m_Lock;
};

#endif
