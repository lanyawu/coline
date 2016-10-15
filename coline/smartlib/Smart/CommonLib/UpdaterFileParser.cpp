#include <Commonlib/UpdaterFileParser.h>
#include <CommonLib/StringUtils.h>
#include <xml/tinyxml.h>

#pragma warning(disable:4996)

CUpdaterFileParser::CUpdaterFileParser()
{
	m_dwTotalFileSize = 0;
}

CUpdaterFileParser::~CUpdaterFileParser(void)
{
	Clear();
}

void CUpdaterFileParser::Clear()
{
	//删除
	LPUPDATE_FILE_ITEM pItem;
	while(!m_FileList.empty())
	{
		pItem = m_FileList.back();
		delete pItem;
		m_FileList.pop_back();
	}
}

BOOL CUpdaterFileParser::GetNextFileItem(LPUPDATE_FILE_ITEM Item)
{
	if (m_FileList.empty())
		return FALSE;
	LPUPDATE_FILE_ITEM pItem = m_FileList.back();
	memset(Item, 0, sizeof(UPDATE_FILE_ITEM));
	Item->dwOperator = pItem->dwOperator;
	Item->dwFileSize = pItem->dwFileSize;
	Item->dwVersion = pItem->dwVersion;
	Item->dwFileId = pItem->dwFileId;
	strcpy(Item->strFileMd5, pItem->strFileMd5);
	strcpy(Item->strLocalFileName, pItem->strLocalFileName);
	strcpy(Item->strRemoteFileName, pItem->strRemoteFileName);
	m_FileList.pop_back();
	delete pItem;
	return TRUE;
}

DWORD CUpdaterFileParser::GetTotalFileSize()
{
	return m_dwTotalFileSize;
}

void CUpdaterFileParser::GetComment(std::list<std::string> &strComments) 
{
	while (!m_strComments.empty())
	{
		strComments.push_back(m_strComments.front());
		m_strComments.pop_front();		
	}
}

const char *CUpdaterFileParser::GetRunCommand()
{
	return m_strRunCommand.c_str();
}

DWORD CUpdaterFileParser::GetVersion()
{
	return m_dwVersion;
}


BOOL CUpdaterFileParser::ParserXML(const char *szFileName)
{
	Clear();
	TiXmlDocument doc(szFileName);
	if (!doc.LoadFile(TIXML_ENCODING_UTF8))
	{
		return FALSE;
	}

	TiXmlElement *pRoot = doc.RootElement();
	const char *pValue = pRoot->Value();
	if (strcmp(pValue, "AutoUpdate") == 0) //正确的升级文件
	{
		//解析module
		TiXmlElement *pModule = pRoot->FirstChildElement("Module");
		if (pModule)
		{
			TiXmlElement *pComment = pModule->FirstChildElement("comment");
			while (pComment)
			{
				std::string strUtf8 = pComment->GetText();
				char *szBuff = new char[strUtf8.size() * 2];
				memset(szBuff, 0, strUtf8.size() * 2);
				CStringConversion::UTF8ToString(strUtf8.c_str(), szBuff, (int) strUtf8.size() * 2);
				m_strComments.push_back(szBuff);
				delete []szBuff;
				pComment = pComment->NextSiblingElement("comment");
			}
			const char *pVersion = pModule->Attribute("version");
			if (pVersion)
				m_dwVersion = atoi(pVersion);
			else
				m_dwVersion = 0;
			
		} else
			return FALSE;
		//解析Files
		TiXmlElement *pFiles = pRoot->FirstChildElement("Files");
		if (pFiles)
		{
			const TiXmlElement *pChild = pFiles->FirstChildElement("File");
			const char *p;
			while (pChild)
			{
				LPUPDATE_FILE_ITEM pItem = new UPDATE_FILE_ITEM();
				memset(pItem, 0, sizeof(UPDATE_FILE_ITEM));
				p = pChild->Attribute("url");
				if (p)
					strncpy(pItem->strRemoteFileName, p, MAX_PATH - 1);
				p = pChild->Attribute("filename");
				if (p)
					strncpy(pItem->strLocalFileName, p, MAX_PATH - 1);
                p = pChild->Attribute("md5");
				if (p)
					strncpy(pItem->strFileMd5, p, 35);
				p = pChild->Attribute("operation");
				if ((p) && (strcmp(p, "delete") == 0))
					pItem->dwOperator = OPERATION_TYPE_DELETE;
				else
					pItem->dwOperator = OPERATION_TYPE_ADD;
				pChild->Attribute("filesize", (int *)&pItem->dwFileSize);
				m_dwTotalFileSize += pItem->dwFileSize;
				pChild->Attribute("version", (int *)&pItem->dwVersion);
				pChild->Attribute("id", (int *)&pItem->dwFileId);
				m_FileList.push_back(pItem);
				pChild = pChild->NextSiblingElement("File");
			}
		} else
			return FALSE;
		//解析run
		TiXmlElement *pRun = pRoot->FirstChildElement("run");
		if (pRun)
		{
			m_strRunCommand = pRun->Attribute("command");
		} else
			return FALSE;
	} else
		return FALSE;
	return TRUE;
}

#pragma warning(default:4996)
