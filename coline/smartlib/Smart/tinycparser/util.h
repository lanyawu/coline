#ifndef _F_UTIL_H_
#define _F_UTIL_H_

#ifndef NULL
#define NULL 0
#endif
#include <stdio.h>

extern char * copy_string(const char *);

extern char * trim(char *str);

extern char * strtok_all(char *sSource, char *sDelim);

extern void upper_case_str(char *pchStr);

extern char * str_replace(char *pchSource, char *pchOld, char *pchNew);

extern long get_file_size(FILE *pFile);

extern void get_filename_without_ext(char *pFullFilaName, char *pFileName);

#endif

