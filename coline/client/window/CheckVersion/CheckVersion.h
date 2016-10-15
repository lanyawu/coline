#ifndef __CHECKVERSION_H______
#define __CHECKVERSION_H______

#include <commonlib/types.h>

BOOL CALLBACK CheckMainPlugins(const char *szAppPath); 

//检测是否需要更新复制文件
BOOL CALLBACK CheckUpdateLocalFiles(char *szRunFileName, int *nFileSize);
//复制文件
BOOL CALLBACK CopyUpdateFiles(const char *szAppFileName, const char *szTempPath);
//szVerFile -- 当前版本文件名称
//szUrl -- 远程版本文件地址
//szTempPath -- 下载的临时目录
BOOL CALLBACK UpdateFilesFromSvr(const char *szVerFile, const char *szUrl, const char *szTempPath);
#endif
