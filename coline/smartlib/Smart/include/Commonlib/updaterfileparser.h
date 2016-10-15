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
	char strRemoteFileName[MAX_PATH]; //Զ���ļ�·��
	char strLocalFileName[MAX_PATH];  //���ش��·��
	char strFileMd5[36];        //�ļ��ͣģ�ֵ
	DWORD dwFileId;   //�ļ�ID��
	DWORD dwFileSize; //�ļ���С
	DWORD dwVersion;  //�ļ��汾��
	DWORD dwOperator; //���� 0������ 1��ʾɾ��
}UPDATE_FILE_ITEM, *LPUPDATE_FILE_ITEM;

//�����ļ�����
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
	DWORD m_dwTotalFileSize; //���ļ���С
	DWORD m_dwVersion;  //�ܰ汾��
	std::string m_strRunCommand; //��������
	std::list<std::string> m_strComments;    //��������
};

#endif