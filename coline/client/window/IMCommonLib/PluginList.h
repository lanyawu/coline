#ifndef __AIPLUGINLIST_H____
#define __AIPLUGINLIST_H____

#include <string>
#include <vector>
#include <Commonlib/types.h>
 

#pragma warning(disable:4996)
 

typedef struct CPluginItem
{
	int nType;
	std::string strPluginGuid;
	std::string strInterfaceGuid;
	std::string strPluginName;
	std::string strPluginDesc;
}AI_PLUGIN_ITEM, *LPAI_PLUGIN_ITEM;

//
class CPluginList
{
public:
	CPluginList(void);
	~CPluginList(void);
public:
	BOOL LoadPluginsFromReg(const char *szRegDir);
	BOOL GetPluginListByType(const int nType, std::vector<LPAI_PLUGIN_ITEM> &Plugins);
private:
	void Clear();
private:
	std::vector<LPAI_PLUGIN_ITEM> m_PluginList;
};
 
#pragma warning(default:4996)

#endif
