#pragma once

#include <CommonLib/SqliteDBOP.h>

class CSkinMove
{
public:
	CSkinMove(const char *szSkinFileName, const char *szKey);
	~CSkinMove(void);
private:
	BOOL StreamToFile(const char *szFileName, const char *szStream, const DWORD dwSize);
	BOOL LoadFromFile(const char *szFileName, char **szStream, DWORD &dwSize);
public:
	BOOL ImportFromPath(const char *szPath);
	BOOL ExportToPath(const char *szPath);
private:
	CSqliteDBOP m_DBOP;
};
