// skinman.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "SkinMove.h"
#include <string>
#include <commonlib/systemutils.h>

int _tmain(int argc, _TCHAR* argv[])
{
	int nCount = CSystemUtils::GetParamCount();
	if ((nCount != 3) && (nCount != 4))
	{
		printf("skinman [-I|-O] [SkinFileName] [SkinFilePath] [SkinFileKey] \n \
			   -I import skin from path \n \
			   -O export skin to path \n\
			   Example: \n \
			   skinman -I d:\\skin.db \"key\" d:\\skinpath\n");
		return 0;
	}
	std::string strFlag = CSystemUtils::GetParamStr(1);
	std::string strFileName = CSystemUtils::GetParamStr(2);
	std::string strSkinPath = CSystemUtils::GetParamStr(3);
	std::string strFileKey = "";
	if (nCount = 4)
		strFileKey = CSystemUtils::GetParamStr(4);
	if ((strcmp(strFlag.c_str(), "-I") == 0) || (strcmp(strFlag.c_str(), "-i") == 0))
	{
		CSkinMove Skin(strFileName.c_str(), strFileKey.c_str());
		printf("\nWait.");
		Skin.ImportFromPath(strSkinPath.c_str());
	} else if ((strcmp(strFlag.c_str(), "-O") == 0) || (strcmp(strFlag.c_str(), "-o") == 0))
	{
		CSkinMove Skin(strFileName.c_str(), strFileKey.c_str());
		printf("\nWait.");
		Skin.ExportToPath(strSkinPath.c_str());
	} else
	{
		printf("skinman [-I|-O] [SkinFileName] [SkinFilePath] [SkinFileKey] \n \
			   -I import skin from path \n \
			   -O export skin to path \n\
			   Example: \n \
			   skinman -I d:\\skin.db \"key\" d:\\skinpath\n");
	}
	printf("\nOK!success");
	return 0;
}

