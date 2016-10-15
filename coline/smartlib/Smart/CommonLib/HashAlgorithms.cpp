#include "HashAlgorithms.h"

/**
* �ӷ�hash
* @param key �ַ���
* @param prime һ������
* @return hash���
*/
int CHashAlgorithms::addHash(char *szKey, int nPrime)
{
	if (!szKey)
		return 0;
	int hash, i;
	int nSize = (int) ::strlen(szKey);
	for (hash = nSize, i = 0; i < nSize; i++)
		hash += szKey[i];
	return (hash % nPrime);
}

/**
* ��תhash
* @param key �����ַ���
* @param prime ����
* @return hashֵ
*/
int CHashAlgorithms::rotatingHash(char *szKey, int nPrime)
{
	if (!szKey)
		return 0;
	int hash, i;
	int nSize = (int) ::strlen(szKey);
	for (hash = nSize, i = 0; i < nSize; ++i)
		hash = (hash << 4) ^ (hash >> 28) ^ szKey[i];
	return (hash % nPrime);
}

// �����
// ʹ�ã�hash = (hash ^ (hash>>10) ^ (hash>>20)) & mask;
// �����hash %= prime;


/**
* MASKֵ�������һ��ֵ�����������
*/
static int M_MASK = 0x8765fed1;
/**
* һ��һ��hash
* @param key �����ַ���
* @return ���hashֵ
*/
int CHashAlgorithms::oneByOneHash(char *szKey)
{
	if (!szKey)
		return 0;
	int hash, i;
	int nSize = (int) ::strlen(szKey);
	for (hash = 0, i = 0; i < nSize; ++i)
    {
		hash += szKey[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return (hash & M_MASK);
}

/**
* Bernstein's hash
* @param key �����ֽ�����
* @param level ��ʼhash����
* @return ���hash
*/
int CHashAlgorithms::bernstein(char *szKey)
{
	if (!szKey)
		return 0;
	int hash = 0;
	int i;
	int nSize = (int) ::strlen(szKey);
	for (i = 0; i < nSize; ++i) 
		hash = 33 * hash + szKey[i];
	return hash;
}
 

/**
* Universal Hashing
*/
int CHashAlgorithms::universal(char *szKey, int mask, int *nTab)
{
	int hash = (int) ::strlen(szKey), i, len = (int) ::strlen(szKey);
	for (i = 0; i < (len << 3); i += 8)
	{
		char k = szKey[i >> 3];
		if ((k & 0x01) == 0) 
			hash ^= nTab[i + 0];
		if ((k & 0x02) == 0) 
			hash ^= nTab[i + 1];
		if ((k & 0x04) == 0) 
			hash ^= nTab[i + 2];
		if ((k & 0x08) == 0) 
			hash ^= nTab[i + 3];
		if ((k & 0x10) == 0) 
			hash ^= nTab[i + 4];
		if ((k & 0x20) == 0) 
			hash ^= nTab[i + 5];
		if ((k & 0x40) == 0) 
			hash ^= nTab[i + 6];
		if ((k & 0x80) == 0) 
			hash ^= nTab[i + 7];
	}
	return (hash & mask);
}

/**
* Zobrist Hashing
*/ 
int CHashAlgorithms::zobrist(char *szKey, int mask, int **nTab)
{
	int hash, i;
	int nSize = (int) ::strlen(szKey);
	for (hash = nSize, i = 0; i < nSize; ++ i)
		hash ^= nTab[i][szKey[i]];
	return (hash & mask);
}

// LOOKUP3 
// ��Bob Jenkins(3).c�ļ�

// 32λFNV�㷨
static int M_SHIFT = 0;
/**
* 32λ��FNV�㷨
* @param data ����
* @return intֵ
*/
int CHashAlgorithms::FNVHash(BYTE *byteData, int nLen)
{
	int hash = (int)2166136261L;
	int i;
	for (i = 0; i < nLen; i ++)
		hash = (hash * 16777619) ^ byteData[i];
	if (M_SHIFT == 0)
		return hash;
	return (hash ^ (hash >> M_SHIFT)) & M_MASK;
}

/**
 * �Ľ���32λFNV�㷨1
 * @param data ����
 * @return intֵ
 */
int CHashAlgorithms::FNVHash1(BYTE *byteData, int nLen)
{
	int p = 16777619;
	int hash = (int)2166136261L;
	int i;
	for (i = 0; i < nLen; i ++)
		hash = (hash ^ byteData[i]) * p;
	hash += hash << 13;
	hash ^= hash >> 7;
	hash += hash << 3;
	hash ^= hash >> 17;
	hash += hash << 5;
	return hash;
}

/**
 * �Ľ���32λFNV�㷨1
 * @param data �ַ���
 * @return intֵ
 */
int CHashAlgorithms::FNVHash1(char *szKey)
{
	if (!szKey)
		return 0;
	int p = 16777619;
	int hash = (int)2166136261L;
	int nSize = (int) ::strlen(szKey);
	for (int i = 0; i < nSize; i ++)
		hash = (hash ^ szKey[i]) * p;
	hash += hash << 13;
	hash ^= hash >> 7;
	hash += hash << 3;
	hash ^= hash >> 17;
	hash += hash << 5;
	return hash;
}

/**
 * Thomas Wang���㷨������hash
 */ 
int CHashAlgorithms::intHash(int nKey)
{
	nKey += ~(nKey << 15);
	nKey ^= (nKey >> 10);
	nKey += (nKey << 3);
	nKey ^= (nKey >> 6);
	nKey += ~(nKey << 11);
	nKey ^= (nKey >> 16);
	return nKey;
}

/**
 * RS�㷨hash
 * @param str �ַ���
 */
int CHashAlgorithms::RSHash(char *szKey)
{
	if (!szKey)
		return 0;
	int b = 378551;
	int a = 63689;
	int hash = 0;
	int nSize = (int) ::strlen(szKey);
	for(int i = 0; i < nSize; i ++)
	{
		hash = hash * a + szKey[i];
		a = a * b;
	}
	return (hash & 0x7FFFFFFF);
}
  

/**
 * JS�㷨
 */
int CHashAlgorithms::JSHash(char *szKey)
{
	if (!szKey)
		return 0;
	int hash = 1315423911;
	int nSize = (int) ::strlen(szKey);
	for ( int i = 0; i < nSize; i ++)
	{
		hash ^= ((hash << 5) + szKey[i] + (hash >> 2));
	}
	return (hash & 0x7FFFFFFF);
}
   

/**
 * PJW�㷨
 */
int CHashAlgorithms::PJWHash(char *szKey)
{
	int BitsInUnsignedInt = 32;
	int ThreeQuarters     = (BitsInUnsignedInt * 3) / 4;
	int OneEighth         = BitsInUnsignedInt / 8;
	int HighBits          = 0xFFFFFFFF << (BitsInUnsignedInt - OneEighth);
	int hash              = 0;
	int test              = 0;
	int nSize = (int) ::strlen(szKey);
	for (int i = 0; i < nSize; i++)
	{
		hash = (hash << OneEighth) + szKey[i];
		if((test = hash & HighBits) != 0)
		{
			hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}
	return (hash & 0x7FFFFFFF);
}

/**
 * ELF�㷨
 */
int CHashAlgorithms::ELFHash(char *szKey)
{
	if (!szKey)
		return 0;
	int hash = 0;
	int x    = 0;
	int nSize = (int) ::strlen(szKey);
	for(int i = 0; i < nSize; i++)
	{
		hash = (hash << 4) + szKey[i];
		if((x = (int)(hash & 0xF0000000L)) != 0)
		{
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}
	return (hash & 0x7FFFFFFF);
}


/**
 * BKDR�㷨
 */
int CHashAlgorithms::BKDRHash(char *szKey)
{
	if (!szKey)
		return 0;
	int seed = 131; // 31 131 1313 13131 131313 etc..
	int hash = 0;
	int nSize = (int) ::strlen(szKey);
	for(int i = 0; i < nSize; i ++)
	{
		hash = (hash * seed) + szKey[i];
	}
	return (hash & 0x7FFFFFFF);
}


/**
 * SDBM�㷨
 */
int CHashAlgorithms::SDBMHash(char *szKey)
{
	if (!szKey)
		return 0;
	int hash = 0;
	int nSize = (int) ::strlen(szKey);
	for (int i = 0; i < nSize; i++)
	{
		hash = szKey[i] + (hash << 6) + (hash << 16) - hash;
	}
	return (hash & 0x7FFFFFFF);
}

 

/**
 * DJB�㷨
 */
int CHashAlgorithms::DJBHash(char *szKey)
{
	if (!szKey)
		return 0;
	int hash = 5381;
	int nSize = (int) ::strlen(szKey);
	for (int i = 0; i < nSize; i ++)
	{
		hash = ((hash << 5) + hash) + szKey[i];
	}
	return (hash & 0x7FFFFFFF);
}
    

/**
 * DEK�㷨
 */
int CHashAlgorithms::DEKHash(char *szKey)
{
	if (!szKey)
		return 0;
	int nSize = (int) ::strlen(szKey);
	int hash = nSize;
	for(int i = 0; i < nSize; i++)
	{
		hash = ((hash << 5) ^ (hash >> 27)) ^ szKey[i];
	}
	return (hash & 0x7FFFFFFF);
}

/**
 * AP�㷨
 */
int CHashAlgorithms::APHash(char *szKey)
{
	if (!szKey)
		return 0;
	int hash = 0;
	int nSize = ::strlen(szKey);
	for(int i = 0; i < nSize; i ++)
	{
		hash ^= ((i & 1) == 0) ? ( (hash << 7) ^ szKey[i] ^ (hash >> 3)) :
                                   (~((hash << 11) ^ szKey[i] ^ (hash >> 5)));
	}
	return hash;
}

   
 
