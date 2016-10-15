#ifndef __CHECKVERSION_H______
#define __CHECKVERSION_H______

#include <commonlib/types.h>

BOOL CALLBACK CheckMainPlugins(const char *szAppPath); 

//����Ƿ���Ҫ���¸����ļ�
BOOL CALLBACK CheckUpdateLocalFiles(char *szRunFileName, int *nFileSize);
//�����ļ�
BOOL CALLBACK CopyUpdateFiles(const char *szAppFileName, const char *szTempPath);
//szVerFile -- ��ǰ�汾�ļ�����
//szUrl -- Զ�̰汾�ļ���ַ
//szTempPath -- ���ص���ʱĿ¼
BOOL CALLBACK UpdateFilesFromSvr(const char *szVerFile, const char *szUrl, const char *szTempPath);
#endif
