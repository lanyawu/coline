#include <commonlib/debuglog.h>
#include <commonlib/Region2D.h>

CRegion2D::CRegion2D(void):
           m_hRgn(NULL)
{
}

CRegion2D::~CRegion2D(void)
{
	Clear();
}

CRegion2D::CRegion2D(int x1, int y1, int x2, int y2)
{
	m_hRgn = ::CreateRectRgn(x1, y1, x2, y2);
}

CRegion2D::CRegion2D(const CRegionRect &rc)
{
	m_hRgn = ::CreateRectRgnIndirect(&rc.GetRect());
}

CRegion2D::CRegion2D(const CRegion2D &rc)
{
	m_hRgn = ::CreateRectRgn(0, 0, 1, 1);
	if (::CombineRgn(m_hRgn, rc.m_hRgn, NULL, RGN_COPY) == NULLREGION)
		Clear();
}

BOOL CRegion2D::Intersect(const CRegion2D &rc)
{   
	if (!m_hRgn)
		return FALSE;
	if (::CombineRgn(m_hRgn, m_hRgn, rc.m_hRgn, RGN_AND) == NULLREGION)
	{
		Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CRegion2D::Intersect(const CRegionRect &rc)
{
	if (!m_hRgn)
		return FALSE;
	CRegion2D hRgn(rc);
	return Intersect(hRgn);	
}

BOOL CRegion2D::Union(const RECT &rc)
{
	if (!m_hRgn)
	{
		m_hRgn = ::CreateRectRgnIndirect(&rc);
		return (m_hRgn != NULL);
	} else
	{
		CRegion2D hRgn(rc.left, rc.top, rc.right, rc.bottom);
		return Union(hRgn);
	}
}

BOOL CRegion2D::Union(const HRGN &hRgn)
{
	if (!hRgn)
		return FALSE;
	if (!m_hRgn)
	{
		m_hRgn = ::CreateRectRgn(0, 0, 1, 1);
	    if (::CombineRgn(m_hRgn, hRgn, NULL, RGN_COPY) == NULLREGION)
		{
			PRINTDEBUGLOG(dtInfo, "combine region faile, error:%d", ::GetLastError());
			Clear();
			return FALSE;
		}
	} else if (::CombineRgn(m_hRgn, m_hRgn, hRgn, RGN_OR) == NULLREGION)
	{
		Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CRegion2D::Union(const CRegion2D &rc) 
{
	if (!m_hRgn)
	{
		m_hRgn = ::CreateRectRgn(0, 0, 1, 1);
	    if (::CombineRgn(m_hRgn, rc.m_hRgn, NULL, RGN_COPY) == NULLREGION)
		{
			PRINTDEBUGLOG(dtInfo, "combine region faile, error:%d", ::GetLastError());
			Clear();
			return FALSE;
		}
	} else if (::CombineRgn(m_hRgn, m_hRgn, rc.m_hRgn, RGN_OR) == NULLREGION)
	{
		Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CRegion2D::Union(const CRegionRect &rc)
{
	if (!m_hRgn)
	{
		m_hRgn = ::CreateRectRgnIndirect(&(rc.GetRect()));
		return (m_hRgn != NULL);
	} else
	{
		CRegion2D hRgn(rc);
		return Union(hRgn);
	}
}

BOOL CRegion2D::Subtract(const CRegion2D &rc) 
{
	if (::CombineRgn(m_hRgn, m_hRgn, rc.m_hRgn, RGN_DIFF) == NULLREGION)
	{
		Clear();
		return FALSE;
	}
	return TRUE;
}

BOOL CRegion2D::Subtract(const CRegionRect &rc)
{
	CRegion2D hRgn(rc);
	return Subtract(hRgn);
}

CRegion2D & CRegion2D::operator = (const CRegion2D &rc)
{
	if (::CombineRgn(m_hRgn, rc.m_hRgn, NULL, RGN_COPY) == NULLREGION)
	{
		Clear();
	}
	return *this;
}

//
void CRegion2D::Reset(int x1, int y1, int x2, int y2)
{
	Clear();
	m_hRgn = ::CreateRectRgn(x1, y1, x2, y2);
}

void CRegion2D::Translate(const int x, const int y)
{
	if (::OffsetRgn(m_hRgn, x, y) == ERROR)
	{
		Clear();
	}
}

BOOL CRegion2D::GetRects(CRectVectors &rects, BOOL bLeft2Right, BOOL bTop2Down) 
{
	DWORD dwBufSize = ::GetRegionData(m_hRgn, 0, NULL);
	if (dwBufSize == 0) //错误的
		Clear();
	if (IsEmpty())
		return FALSE;
	BYTE *pBuf = new BYTE[dwBufSize];
	if (::GetRegionData(m_hRgn, dwBufSize, (LPRGNDATA)pBuf) != dwBufSize)
	{
		Clear();
		delete []pBuf;
		return FALSE;
	}

	//数据
	LPRGNDATA pRgnData = (LPRGNDATA)pBuf;
	DWORD dwCount = pRgnData->rdh.nCount;
	if (dwCount == 0)
		return FALSE;
	LONG lCurrentY = INT_MIN;
	LONG lStart, lEnd;
	if (bTop2Down)
	{
		lStart = 0;
		lEnd = -1;
		rects.reserve(dwCount);
		for (DWORD i = 0; i < dwCount; i ++)
		{
			RECT rc = ((RECT *)&pRgnData->Buffer[0])[i];
			if (rc.top == lCurrentY)
			{
				lEnd = i;
			} else //else if (rc.top...
			{
				if (bLeft2Right)
				{
					for (LONG j = lStart; j <= lEnd; j ++)
					{
						RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
						CRegionRect rr(r.left, r.top, r.right, r.bottom);
						rects.push_back(rr);
					}
				} else // else if (bLeft2Right)
				{
					for (LONG j = lEnd; j >= lStart; j --)
					{
						RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
						CRegionRect rr(r.left, r.top, r.right, r.bottom);
						rects.push_back(rr);
					}
				} //end else if (bLeft2Right)
				lStart = i;
				lEnd = i;
				lCurrentY = rc.top;
			} //end else if (rc.top == ...
		} //end for (DWORD i = ...
        if (bLeft2Right)
		{
			for (LONG j = lStart; j <= lEnd; j ++)
			{
				RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
				CRegionRect rr(r.left, r.top, r.right, r.bottom);
				rects.push_back(rr);
			}
		} else
		{
			for (LONG j = lEnd; j >= lStart; j --)
			{
				RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
				CRegionRect rr(r.left, r.top, r.right, r.bottom);
				rects.push_back(rr);
			}
		}
	} else // else if (bTop2Down)
	{
		lStart = dwCount;
		lEnd = dwCount - 1;
		rects.reserve(dwCount);
		for (DWORD i = dwCount - 1; i >= 0; i --)
		{
			RECT rc = ((RECT *)&pRgnData->Buffer[0])[i];
			if (rc.top == lCurrentY)
			{
				lStart = i;
			} else
			{
				if (bLeft2Right)
				{
					for (LONG j = lStart; j <= lEnd; j ++)
					{
						RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
						CRegionRect rr(r.left, r.top, r.right, r.bottom);
						rects.push_back(rr);
					}
				} else
				{
					for (LONG j = lEnd; j >= lStart; j --)
					{
						RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
						CRegionRect rr(r.left, r.top, r.right, r.bottom);
						rects.push_back(rr);
					}
				} //end else if(bLeft2Right)...
                lEnd = i;
				lStart = i;
				lCurrentY = rc.top;
			} // end else if (rc.top...
		} //
		if (bLeft2Right)
		{
			for (LONG j = lStart; j <= lEnd; j ++)
			{
				RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
				CRegionRect rr(r.left, r.top, r.right, r.bottom);
				rects.push_back(rr);
			}
		} else
		{
			for (LONG j = lEnd; j >= lStart; j --)
			{
				RECT r = ((RECT *)&pRgnData->Buffer[0])[j];
				CRegionRect rr(r.left, r.top, r.right, r.bottom);
				rects.push_back(rr);
			}
		}
	} // end if else (bTop2Down)
	delete []pBuf;
	return TRUE;
 }


CRegionRect CRegion2D::GetBoundingRect() const
{
	RECT rc;
	CRegionRect r;
	if (::GetRgnBox(m_hRgn, &rc))
	{
		r.Reset(rc.left, rc.top, rc.right, rc.bottom);
	}
	return r;
}

RECT CRegion2D::GetBoundRect() const
{
	RECT rc = {0};
	if (m_hRgn != NULL)
	{
		if (::GetRgnBox(m_hRgn, &rc) == ERROR)
		{
			PRINTDEBUGLOG(dtInfo, "GetRgnBox Failed, Error:%d", ::GetLastError());
		}
	} else
	{
		PRINTDEBUGLOG(dtInfo, "Region Handle is NULL");
	}
	return rc;
}

//初始化RGN
BOOL CRegion2D::CheckRgn(const BOOL bCreate)
{
	if ((!m_hRgn) && bCreate)
	{
		m_hRgn = ::CreateRectRgn(0, 0, 1, 1);
	}
	return (m_hRgn != NULL);
}

void CRegion2D::Clear()
{
	if (m_hRgn)
	{
		::DeleteObject(m_hRgn);
	}
	m_hRgn = NULL;
}

BOOL CRegion2D::Equal(const CRegion2D &rc) const
{
	return  ::EqualRgn(m_hRgn, rc.m_hRgn);
}

BOOL CRegion2D::IsEmpty() const
{
	RECT rc;
	return (::GetRgnBox(m_hRgn, &rc) == 0);
}

HRGN CRegion2D::GetHandle() const
{
	return m_hRgn;
}
