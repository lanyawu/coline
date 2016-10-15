// CreateEmotion.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <Crypto/crypto.h>
#include <xml/tinyxml.h>

void TransByPath(const char *szPath)
{
	TCHAR szwFilePath[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szPath, szwFilePath, MAX_PATH - 1);
	WIN32_FIND_DATA fd = {0};
	lstrcat(szwFilePath, L"*.*");
	std::string strFileName;
	std::string strSaveAsFile;
	char szTmp[MAX_PATH];
	char szMd5[64] = {0};
	std::string strXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	strXml += "<EmotionList  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"Emotion.xsd\">";
	strXml += "<SystemEmotionList>";
	strFileName = szPath;
	strFileName += "new";
	CSystemUtils::ForceDirectories(strFileName.c_str());
	HANDLE h = ::FindFirstFile(szwFilePath, &fd);
	if (h)
	{
		do
		{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0) 
				continue;
			memset(szTmp, 0, MAX_PATH);
			CStringConversion::WideCharToString(fd.cFileName, szTmp, MAX_PATH - 1);
			strFileName = szPath;
			strFileName += szTmp;
			memset(szMd5, 0, 64);
			if (md5_encodefile(strFileName.c_str(), szMd5))
			{
				strXml += "<Emotion tag=\"";
				strXml += szMd5;
				strXml += "\" shortcut=\"";
				strXml += "\" comment=\"";
				strXml += "\" imagefile=\"";
				strXml += szMd5;
				strXml += ".gif\"/>";
				strSaveAsFile = szPath;
				strSaveAsFile += "new\\";
				strSaveAsFile += szMd5;
				strSaveAsFile += ".gif";
				CSystemUtils::CopyFilePlus(strFileName.c_str(), strSaveAsFile.c_str(), TRUE);
			}
		} while (::FindNextFile(h, &fd));
	}
	strXml += "</SystemEmotionList></EmotionList>";
	strFileName = szPath;
	strFileName += "New\\emotion.xml";
	TiXmlDocument xmldoc;
	if (xmldoc.Load(strXml.c_str(), strXml.size()))
	{
		xmldoc.SaveFile(strFileName.c_str());
	} 
}

int _tmain(int argc, _TCHAR* argv[])
{
	TransByPath("F:\\lanya\\workarea\\GoCom\\Bin\\Debug\\sysemotion\\");
	return 0;
}

