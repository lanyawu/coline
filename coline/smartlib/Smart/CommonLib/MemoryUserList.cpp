#include <commonlib/MemoryUserList.h>

#pragma warning(disable:4996)

CMemoryUserList::CMemoryUserList(void)
{
}

CMemoryUserList::~CMemoryUserList(void)
{
}

//���ֲ���
int CMemoryUserList::GetUserPos(int nId, int& nResultState)
{
	int nLo, nHi, nMid, nPos, nCurId;

	nLo = 0;
	nHi = GetCount() - 1;
	nResultState = -2;
	nPos = -1;
	while (nLo <= nHi)
	{
		nMid = (nLo + nHi) / 2;
		nPos = nMid;
		nCurId = GetUserIDByIndex(nMid);

		if (nId > nCurId)
		{
			nLo = nMid + 1;
			nResultState = 1;
		}
		else if (nId < nCurId)
		{
			nHi = nMid - 1;
			nResultState = -1;
		}
		else
		{
			nResultState = 0;
			break;
		}
	}
	return nPos;
}

//����id�Ƿ����
int CMemoryUserList::InternalFindUser(int nId)
{
	int nPos, nResultState;

	nPos = GetUserPos(nId, nResultState);
	if (nResultState == 0)
		return nPos;
	else
		return -1;
}

//����id����λ��
int CMemoryUserList::FindInsertPos(int nId)
{
	int nPos, nResultState;

	nPos = GetUserPos(nId, nResultState);
	
	if (nResultState == 0 || nResultState == 1) 
		nPos = nPos + 1;
	else if (nResultState == -2)
		nPos = 0;

	return nPos;
}


//   ===  class CStringMemoryUserList
CStringMemoryUserList::CStringMemoryUserList()
{
}

CStringMemoryUserList::~CStringMemoryUserList()
{
}

int CStringMemoryUserList::GetUserPos(const char *szName, int& nResultState)
{
	int nLo, nHi, nMid, nPos;

	nLo = 0;
	nHi = GetCount() - 1;
	nResultState = -2;
	nPos = -1;
	int nCompare;
	while (nLo <= nHi)
	{
		nMid = (nLo + nHi) / 2;
		nPos = nMid;
		nCompare = ::stricmp(szName, GetUserIDByIndex(nMid));
		if (nCompare > 0)
		{
			nLo = nMid + 1;
			nResultState = 1;
		} else if (nCompare < 0)
		{
			nHi = nMid - 1;
			nResultState = -1;
		} else
		{
			nResultState = 0;
			break;
		}
	}
	return nPos;
}

int CStringMemoryUserList::InternalFindUser(const char *szName)
{
	int nPos, nResultState;

	nPos = GetUserPos(szName, nResultState);
	if (nResultState == 0)
		return nPos;
	else
		return -1;
}

//���Ҳ���λ��
int CStringMemoryUserList::FindInsertPos(const char *szName)
{
	int nPos, nResultState;

	nPos = GetUserPos(szName, nResultState);
	
	if (nResultState == 0 || nResultState == 1) 
		nPos = nPos + 1;
	else if (nResultState == -2)
		nPos = 0;

	return nPos;
}

//////////////////////////////////////////////////////////////////////////////
// class CPointerDataMap
CPointerDataMap::CPointerDataMap(void)
{
	m_pDatas = new  CPointerDataShm();
	memset(m_pDatas, 0, sizeof(CPointerDataShm));
	InitAllocStack();
}

CPointerDataMap::~CPointerDataMap(void)
{
	delete m_pDatas;
}


void CPointerDataMap::InitAllocStack()
{
	for (int i = 0; i < MAX_DATA_COUNT; i++)
		m_pDatas->nAllocStack[i] = i;
	m_pDatas->nStackTos = 0;
	m_pDatas->dwDataCount = 0;
}


int CPointerDataMap::AllocUserSlot()
{
	int nResult;
	int &nTos = m_pDatas->nStackTos;

	nResult = m_pDatas->nAllocStack[nTos];
	m_pDatas->nAllocStack[nTos] = -1;
	nTos++;

	return nResult;
}

void CPointerDataMap::FreeUserSlot(int nIndex)
{
	int &nTos = m_pDatas->nStackTos;

	nTos--;
	m_pDatas->nAllocStack[nTos] = nIndex;
}

CPointerDataItem &CPointerDataMap::operator [](int nIdx)
{
	return (*m_pDatas)[nIdx];
}

bool CPointerDataMap::InsertData(DWORD dwKey, void *pData)
{
	int i, nPos;

	if (GetCount() >= MAX_DATA_COUNT) return false;

	//�ж��Ƿ����������б���
	i = InternalFindUser(dwKey);
	//�������
	if (i == -1)
	{
		//ȡ�ò���λ��
		nPos = FindInsertPos(dwKey);
		//�����û���Ϣ��λ
		memmove(&(*m_pDatas).nDataIdx[nPos+1], &(*m_pDatas).nDataIdx[nPos], (GetCount() - nPos) * sizeof(int));
		(*m_pDatas).nDataIdx[nPos] = AllocUserSlot();
		//�����û���Ϣ
		(*m_pDatas)[nPos].dwKey = dwKey;
		(*m_pDatas)[nPos].pData = pData;
		//�����û�����
		(*m_pDatas).dwDataCount++;
		return true;
	}
	return false;
}

int CPointerDataMap::FindDataPos(DWORD dwKey)
{
	return InternalFindUser(dwKey);
}

bool CPointerDataMap::EraseData(DWORD dwKey)
{
	int nPos = InternalFindUser(dwKey);
	if ((nPos >= 0) && ( nPos < (int)GetCount()))
	{
		//�ͷ��û���Ϣ��λ
		FreeUserSlot((*m_pDatas).nDataIdx[nPos]);
		memmove(&(*m_pDatas).nDataIdx[nPos], &(*m_pDatas).nDataIdx[nPos + 1], (GetCount() - nPos - 1) * sizeof(int));
		//�����û�����
		(*m_pDatas).dwDataCount--;
		return true;
	} else
		return false;
}

void CPointerDataMap::Delete(int nIdx)
{
	if ((nIdx >= 0) && ( nIdx < (int)GetCount()))
	{
		//�ͷ��û���Ϣ��λ
		FreeUserSlot((*m_pDatas).nDataIdx[nIdx]);
		memmove(&(*m_pDatas).nDataIdx[nIdx], &(*m_pDatas).nDataIdx[nIdx + 1], (GetCount() - nIdx - 1) * sizeof(int));
		//�����û�����
		(*m_pDatas).dwDataCount--;
	}
}

void CPointerDataMap::Clear()
{
	InitAllocStack();
}

#pragma warning(default:4996)
