#ifndef __UPDATEFILEPARSER_H___
#define __UPDATEFILEPARSER_H___

#include <commonlib/types.h>
#include <string>
#include <list>
#include <vector>

#define OPERATION_TYPE_ADD    1
#define OPERATION_TYPE_DELETE 2

typedef struct CUpdateFileItem
{
	char strRemoteFileName[MAX_PATH]; //远程文件路径
	char strLocalFileName[MAX_PATH];  //本地存放路径
	char strFileMd5[36];        //文件ＭＤ５值
	DWORD dwFileId;   //文件ID号
	DWORD dwFileSize; //文件大小
	DWORD dwVersion;  //文件版本号
	DWORD dwOperator; //操作 0表增加 1表示删除
}UPDATE_FILE_ITEM, *LPUPDATE_FILE_ITEM;

//升级文件解析
class COMMONLIB_API CUpdaterFileParser
{
public:
	CUpdaterFileParser();
	~CUpdaterFileParser(void);
public:
	BOOL GetNextFileItem(LPUPDATE_FILE_ITEM Item);
	DWORD GetTotalFileSize();
	BOOL ParserXML(const char *szFileName);
	void GetComment(std::list<std::string> &strComments);
	const char *GetRunCommand();
	DWORD GetVersion();
private:
	void Clear();
private:
	std::vector<LPUPDATE_FILE_ITEM> m_FileList;
	DWORD m_dwTotalFileSize; //总文件大小
	DWORD m_dwVersion;  //总版本号
	std::string m_strRunCommand; //运行命令
	std::list<std::string> m_strComments;    //更新内容
};

#endif