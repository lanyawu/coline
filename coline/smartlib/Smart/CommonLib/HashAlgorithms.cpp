#include "HashAlgorithms.h"

/**
* 加法hash
* @param key 字符串
* @param prime 一个质数
* @return hash结果
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
* 旋转hash
* @param key 输入字符串
* @param prime 质数
* @return hash值
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

// 替代：
// 使用：hash = (hash ^ (hash>>10) ^ (hash>>20)) & mask;
// 替代：hash %= prime;


/**
* MASK值，随便找一个值，最好是质数
*/
static int M_MASK = 0x8765fed1;
/**
* 一次一个hash
* @param key 输入字符串
* @return 输出hash值
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
* @param key 输入字节数组
* @param level 初始hash常量
* @return 结果hash
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
// 见Bob Jenkins(3).c文件

// 32位FNV算法
static int M_SHIFT = 0;
/**
* 32位的FNV算法
* @param data 数组
* @return int值
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
 * 改进的32位FNV算法1
 * @param data 数组
 * @return int值
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
 * 改进的32位FNV算法1
 * @param data 字符串
 * @return int值
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
 * Thomas Wang的算法，整数hash
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
 * RS算法hash
 * @param str 字符串
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
 * JS算法
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
 * PJW算法
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
 * ELF算法
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
 * BKDR算法
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
 * SDBM算法
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
 * DJB算法
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
 * DEK算法
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
 * AP算法
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

   
 
