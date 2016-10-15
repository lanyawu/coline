#ifndef __TREE_CALLBACK_FUN_H___
#define __TREE_CALLBACK_FUN_H___


const char *CALLBACK GetTreeNodeKey(CTreeNodeType tnType, const void *pData)
{
	if (pData)
	{
		return ((LPORG_TREE_NODE_DATA)(pData))->szUserName;
	}
	return NULL;
}
 
BOOL CALLBACK FreeTreeNodeData(CTreeNodeType nodeType, void **pData)
{
	if (pData)
	{
		LPORG_TREE_NODE_DATA pItem = (LPORG_TREE_NODE_DATA)(*pData);
		if (pItem)
		{
			if (pItem->szDisplayName)
				delete []pItem->szDisplayName;
			delete pItem;
		}
		*pData = NULL;
		return TRUE;
	}
	return FALSE;
}

#endif
