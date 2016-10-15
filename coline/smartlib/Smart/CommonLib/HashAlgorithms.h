#ifndef __HASHALGORITHMS_H___
#define __HASHALGORITHMS_H___

#include <string>
#include <Commonlib/types.h>

class COMMONLIB_API CHashAlgorithms
{
public:
	/**
	* 加法hash
	* @param key 字符串
	* @param prime 一个质数
	* @return hash结果
	*/
	static int addHash(char *szKey, int nPrime);
	/**
	* 旋转hash
	* @param key 输入字符串
	* @param prime 质数
	* @return hash值
	*/
    static int rotatingHash(char *szKey, int nPrime);
	/**
	* 一次一个hash
	* @param key 输入字符串
	* @return 输出hash值
	*/
	static int oneByOneHash(char *szKey);
	/**
	* Bernstein's hash
	* @param key 输入字节数组
	* @param level 初始hash常量
	* @return 结果hash
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
	* 32位的FNV算法
	* @param data 数组
	* @return int值
	*/
    static int FNVHash(BYTE *byteData, int nLen);
    /**
     * 改进的32位FNV算法1
     * @param data 数组
     * @return int值
     */
    static int FNVHash1(BYTE *byteData, int nLen);
    /**
     * 改进的32位FNV算法1
     * @param data 字符串
     * @return int值
     */
    static int FNVHash1(char *szKey);
    /**
     * Thomas Wang的算法，整数hash
     */ 
    static int intHash(int nKey);
    /**
     * RS算法hash
     * @param str 字符串
     */
    static int RSHash(char *szKey);
 
    /**
     * JS算法
     */
    static int JSHash(char *szKey);
    /**
     * PJW算法
     */
    static int PJWHash(char *szKey);
    /**
     * ELF算法
     */
    static int ELFHash(char *szKey);
    /**
     * BKDR算法
     */
    static int BKDRHash(char *szKey);
    /**
     * SDBM算法
     */
    static int SDBMHash(char *szKey);
    /**
     * DJB算法
     */
	static int DJBHash(char *szKey);
    /**
     * DEK算法
     */
    static int DEKHash(char *szKey);
    /**
     * AP算法
     */
    static int APHash(char *szKey);
};

#endif
