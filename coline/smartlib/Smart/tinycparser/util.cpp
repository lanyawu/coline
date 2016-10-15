/****************************************************/
/* File: util.c                                     */
/* author: zhewei.fan                               */
/* 某些共用的字符串的处理函数                       */
/****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "util.h"

#pragma warning(disable:4996)

char * copy_string(const char *s) {
	char *t;

    if (!s) {
        return NULL;
    }
    t = new char[strlen(s) + 1];
    strcpy(t, s);
    return t;
}


/**
 * 删除字符串头尾的空格、制表符、回车符、换行符
 */
char * trim(char *str) {
    int len, i;
    char *tmp;
	
	if (str) {
		tmp = str;
	
		while (*tmp == ' ' || *tmp == '\t' || *tmp == '\r' || *tmp == '\n') {
			tmp++;
		}
	
		len = strlen(tmp) - 1;
		while (len > 0 && (tmp[len] == ' ' || tmp[len] == '\t' || *tmp == '\r' || *tmp == '\n')) {
			tmp[len] = '\0';
			len--;
		}
	
		len = strlen(tmp);
		for (i = 0; i < len; i++) {
			str[i] = tmp[i];
		}
		str[i] = '\0';
	}
    
    return str;
}

/**
 * 以sDdelim为分隔符分割sSource，类似于strtok，不同的为当出现两个连续的分隔符时，返回空字符串
 * 使用方式与strtok一样
 * @param char sSource[] 需要分割的字符串
 * @param char sDelim[] 分隔符号字符串
 * @return 字符串，返回为NULL时表示字符串分割取值完毕
 */
char * strtok_all(char *sSource, char *sDelim) {
    static char *sUseSource;
    char *sSubStr;
    static int iLen, iLenDiv, iBegin;
    int i, j;
    int flag = 0;

    if (sSource) {
        iLen = strlen(sSource);
        iLenDiv = strlen(sDelim);
        for (i = 0; i < iLen; i++) {
            flag = 0;
            for (j = 0; j < iLenDiv; j++) {
                if (*(sSource + i + j) == *(sDelim + j)) {
                    flag = 1;
                } else {
                    flag = 0;
                    break;
                }
            }
            if (flag == 1) {
                for (j = 0; j < iLenDiv; j++) {
                    *(sSource + i + j) = '\0';
                }
                i = i + iLenDiv - 1;
            }
        }
        sUseSource = sSource;
        sSubStr = sUseSource;
        iBegin = strlen(sSource);
    } else {
        if (iBegin + iLenDiv < iLen) {
            sSubStr = sUseSource + iBegin + iLenDiv;
        } else {
            sSubStr = NULL;
        }
        for (i = iBegin + iLenDiv; i < iLen; i++) {
            if (*(sUseSource + i) == '\0')  {
                iBegin = i;
                break;
            }
        }
        if (i == iLen) {
            iBegin = iLen;
        }
    }
    return sSubStr;
}

/**
 * 将字符串转换为大写
 */
void upper_case_str(char *pchStr) {
	int iLen, i;
	if (pchStr) {
		iLen = strlen(pchStr);
		for (i = 0; i < iLen; i++) {
			pchStr[i] = toupper(pchStr[i]);
		}
	}
}

/**
 * 字符串替换
 */
char * str_replace(char *pchSource, char *pchOld, char *pchNew) {
	char *chTmp;
	char *pTmp;

	int iMulti = strlen(pchOld) - strlen(pchNew) + 1;
	if (iMulti < 1)
		iMulti = 1;
	chTmp = new char[strlen(pchSource) * iMulti + 1];

	if (pchSource) {
		pTmp = strtok_all(pchSource, pchOld);
		strcpy(chTmp, pTmp);
	
		while (pTmp = strtok_all(NULL, pchOld)) {
			strcat(chTmp, pchNew);
			strcat(chTmp, pTmp);
		}
		pTmp = copy_string(chTmp);
		delete chTmp;
		return pTmp;
	}
	return NULL;
}

/**
 * 取制定文件流的大小
 */
long get_file_size(FILE *pFile) {
	long lFileSize = 0;
	
	if (pFile) {
		fseek(pFile, 0, SEEK_END);
		lFileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
	}
	return lFileSize;
}

/**
 * 取不带路径、后缀的文件名
 */
void get_filename_without_ext(char *pFullFilaName, char *pFileName) {
	int iPos1, iPos2, iLen, i;

	if (pFullFilaName) {
		iPos1 = 0;
		iLen = strlen(pFullFilaName);
		iPos2 = iLen;
	
		for (i = iLen; i > 0; i--) {
			if (pFullFilaName[i - 1] == '.') {
				iPos2 = i - 1;
			}
			if (pFullFilaName[i - 1] == '/') {
				iPos1 = i;
				break;
			}
		}
		strncpy(pFileName, pFullFilaName + iPos1, iPos2 - iPos1);
		pFileName[iPos2 - iPos1] = '\0';
		trim(pFileName);
	}
}

#pragma warning(default:4996)
