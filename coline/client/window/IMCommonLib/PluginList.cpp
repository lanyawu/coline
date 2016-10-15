#include <Commonlib/DebugLog.h>
#include <Core/CoreInterface.h>
#include "PluginList.h"
#include "../IMCommonLib/IMDllRegistar.h"

#pragma warning(disable:4996)

CPluginList::CPluginList(void)
{
}


CPluginList::~CPluginList(void)
{
	//
	Clear();
}

BOOL CPluginList::LoadPluginsFromReg(const char *szRegDir)
{
	char szPath[MAX_PATH] = {0};
	char szSubPath[MAX_PATH] = {0};
	char szValue[MAX_PATH] = {0};
	for (int i = 1; i <= MAX_PLUGIN_TYPE_ID; i ++)
	{
		memset(szPath, 0, MAX_PATH);
		sprintf(szPath, "%s\\%s", PLUGIN_REGISTER_DIR, PLUGIN_TYPE_NAMES[i]);
		std::vector<std::string> SubItems;
		CSystemUtils::ReadRegisterItems(HKEY_LOCAL_MACHINE, szPath, SubItems);
		std::vector<std::string>::iterator it;
		for (it = SubItems.begin(); it != SubItems.end(); it ++)
		{
			memset(szSubPath, 0, MAX_PATH);
			memset(szValue, 0, MAX_PATH);
			sprintf(szSubPath, "%s\\%s\\%s", PLUGIN_REGISTER_DIR, PLUGIN_TYPE_NAMES[i], it->c_str());
			if (CSystemUtils::ReadRegisterKey(HKEY_LOCAL_MACHINE, szSubPath, "guid", szValue, MAX_PATH - 1))
			{
				LPAI_PLUGIN_ITEM pItem = new AI_PLUGIN_ITEM();
				pItem->nType = i;
				pItem->strPluginGuid = szValue;
				pItem->strPluginName = it->c_str();
				memset(szValue, 0, MAX_PATH);
				if (CSystemUtils::ReadRegisterKey(HKEY_LOCAL_MACHINE, szSubPath, "desc", szValue, MAX_PATH - 1))
				{
					pItem->strPluginDesc = szValue;
				}
				//
				memset(szValue, 0, MAX_PATH);
				if (CSystemUtils::ReadRegisterKey(HKEY_LOCAL_MACHINE, szSubPath, "interface", szValue, MAX_PATH - 1))
				{
					pItem->strInterfaceGuid = szValue;
				}
				m_PluginList.push_back(pItem);
			} 
		}
	}
	return TRUE;
}

BOOL CPluginList::GetPluginListByType(const int nType, std::vector<LPAI_PLUGIN_ITEM> &Plugins)
{
	std::vector<LPAI_PLUGIN_ITEM>::iterator it;
	BOOL bSucc = FALSE;
	for (it = m_PluginList.begin(); it != m_PluginList.end(); it ++)
	{
		if ((*it)->nType == nType)
		{
			LPAI_PLUGIN_ITEM pItem = new AI_PLUGIN_ITEM();
			pItem->nType = (*it)->nType;
			pItem->strInterfaceGuid = (*it)->strInterfaceGuid;
			pItem->strPluginDesc = (*it)->strPluginDesc;
			pItem->strPluginGuid = (*it)->strPluginGuid;
			pItem->strPluginName = (*it)->strPluginName;
			Plugins.push_back(pItem);
			bSucc = TRUE;
		}
	}
	return bSucc;
}

void CPluginList::Clear()
{
	std::vector<LPAI_PLUGIN_ITEM>::iterator it;
	for (it = m_PluginList.begin(); it != m_PluginList.end(); it ++)
	{
		delete (*it);
	}
	m_PluginList.clear();
}

#pragma warning(default:4996)
