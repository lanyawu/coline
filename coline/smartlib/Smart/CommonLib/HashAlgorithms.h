#ifndef __HASHALGORITHMS_H___
#define __HASHALGORITHMS_H___

#include <string>
#include <Commonlib/types.h>

class COMMONLIB_API CHashAlgorithms
{
public:
	/**
	* �ӷ�hash
	* @param key �ַ���
	* @param prime һ������
	* @return hash���
	*/
	static int addHash(char *szKey, int nPrime);
	/**
	* ��תhash
	* @param key �����ַ���
	* @param prime ����
	* @return hashֵ
	*/
    static int rotatingHash(char *szKey, int nPrime);
	/**
	* һ��һ��hash
	* @param key �����ַ���
	* @return ���hashֵ
	*/
	static int oneByOneHash(char *szKey);
	/**
	* Bernstein's hash
	* @param key �����ֽ�����
	* @param level ��ʼhash����
	* @return ���hash
	*/
	static int bernstein(char *szKey);
	/**
	* Universal Hashing
	*/
	static int universal(char *szKey, int mask, int *nTab);
	/**
	* Zobrist Hashing
	*/ 
	static int zobrist(char *szKey,int mask, int **nTab);
	/**
	* 32λ��FNV�㷨
	* @param data ����
	* @return intֵ
	*/
    static int FNVHash(BYTE *byteData, int nLen);
    /**
     * �Ľ���32λFNV�㷨1
     * @param data ����
     * @return intֵ
     */
    static int FNVHash1(BYTE *byteData, int nLen);
    /**
     * �Ľ���32λFNV�㷨1
     * @param data �ַ���
     * @return intֵ
     */
    static int FNVHash1(char *szKey);
    /**
     * Thomas Wang���㷨������hash
     */ 
    static int intHash(int nKey);
    /**
     * RS�㷨hash
     * @param str �ַ���
     */
    static int RSHash(char *szKey);
 
    /**
     * JS�㷨
     */
    static int JSHash(char *szKey);
    /**
     * PJW�㷨
     */
    static int PJWHash(char *szKey);
    /**
     * ELF�㷨
     */
    static int ELFHash(char *szKey);
    /**
     * BKDR�㷨
     */
    static int BKDRHash(char *szKey);
    /**
     * SDBM�㷨
     */
    static int SDBMHash(char *szKey);
    /**
     * DJB�㷨
     */
	static int DJBHash(char *szKey);
    /**
     * DEK�㷨
     */
    static int DEKHash(char *szKey);
    /**
     * AP�㷨
     */
    static int APHash(char *szKey);
};

#endif
