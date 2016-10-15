#include <commonlib/RegionRect.h>

CRegionRect::CRegionRect(void):
             m_nLeft(0),
			 m_nTop(0),
			 m_nRight(0),
			 m_nBottom(0)
{
}

CRegionRect::~CRegionRect(void)
{
}

 
CRegionRect::CRegionRect(int x1, int y1, int x2, int y2):
             m_nLeft(x1),
		     m_nTop(y1),
			 m_nRight(x2),
			 m_nBottom(y2)
{
}

CRegionRect::CRegionRect(const CRegionRect &rc):
             m_nLeft(rc.m_nLeft),
			 m_nTop(rc.m_nTop),
			 m_nRight(rc.m_nRight),
			 m_nBottom(rc.m_nBottom)
{
}
void CRegionRect::SetRegionRect(int x1, int y1, int x2, int y2)
{
	m_nLeft = x1;
	m_nTop = y1;
    m_nRight = x2;
	m_nBottom = y2;
}

void CRegionRect::SetRegionRect(const RECT &rc)
{
	m_nLeft = rc.left;
	m_nTop = rc.top;
	m_nRight = rc.right;
	m_nBottom = rc.bottom;
}

//相交区域
CRegionRect CRegionRect::Intersect(const CRegionRect &rc) const
{
	CRegionRect rcResult;
	rcResult.m_nLeft = max(m_nLeft, rc.m_nLeft);
	rcResult.m_nTop = max(m_nTop, rc.m_nTop);
	rcResult.m_nRight = min(m_nRight, rc.m_nRight);
	rcResult.m_nBottom = min(m_nBottom, rc.m_nBottom);
	return rcResult;
}

//相交区域
CRegionRect CRegionRect::Intersect(const RECT &rc) const
{
	CRegionRect rcResult;
	rcResult.m_nLeft = max(m_nLeft, rc.left);
	rcResult.m_nTop = max(m_nTop, rc.top);
	rcResult.m_nRight = min(m_nRight, rc.right);
	rcResult.m_nBottom = min(m_nBottom, rc.bottom);
	return rcResult;
}

//联合区域
CRegionRect CRegionRect::Union(const CRegionRect &rc) const
{
	CRegionRect rcResult;
	rcResult.m_nLeft = min(m_nLeft, rc.m_nLeft);
	rcResult.m_nTop = min(m_nTop, rc.m_nTop);
	rcResult.m_nRight = max(m_nRight, rc.m_nRight);
	rcResult.m_nBottom = max(m_nBottom, rc.m_nBottom);
	return rcResult;
}

//偏移位置
CRegionRect CRegionRect::Translate(const int x, const int y) const
{
	CRegionRect rcResult;
	rcResult.m_nLeft = m_nLeft + x;
	rcResult.m_nTop = m_nTop + y;
	rcResult.m_nRight = m_nRight + x;
	rcResult.m_nBottom = m_nBottom + y;
	return rcResult;
}

RECT CRegionRect::GetRect() const
{
	RECT r;
	r.left = m_nLeft;
	r.top = m_nTop;
	r.right = m_nRight;
	r.bottom = m_nBottom;
	return r;
}

//是否相等
BOOL CRegionRect::Equal(const CRegionRect &rc)
{
	return ((m_nLeft == rc.m_nLeft) && (m_nTop == rc.m_nTop)
		&& (m_nRight == rc.m_nRight) && (m_nBottom == rc.m_nBottom));
}

//被包含
BOOL CRegionRect::Enclose_By(const CRegionRect &rc)
{
	return ((m_nLeft >= rc.m_nLeft) && (m_nTop >= rc.m_nTop)
		&& (m_nRight <= rc.m_nRight) && (m_nBottom <= rc.m_nBottom));
}

//reset
void CRegionRect::Reset(int x1, int y1, int x2, int y2)
{
	m_nLeft = x1;
	m_nTop = y1;
	m_nRight = x2;
	m_nBottom = y2;
}

//获取数据
int CRegionRect::GetLeft() const
{
	return m_nLeft;
}

int CRegionRect::GetTop() const
{
	return m_nTop;
}

int CRegionRect::GetRight() const
{
	return m_nRight;
}

int CRegionRect::GetBottom() const
{
	return m_nBottom;
}

//是否为空
BOOL CRegionRect::Is_Empty()
{
	return ((Width() <= 0) || (Height() <= 0));
}

//清除
void CRegionRect::Clear()
{
	m_nLeft = 0;
	m_nTop = 0;
	m_nRight = 0;
	m_nBottom = 0;
}