#ifndef __PINYINTRANS_H___
#define __PINYINTRANS_H___

#include <CommonLib/Types.h>

//拼音转换
class COMMONLIB_API CPinyinTrans
{
public:
	CPinyinTrans(void);
	~CPinyinTrans(void);
public:
	//根据文字获取简拼和全拼 
	static BOOL GetPinyinByText(WCHAR *szText, std::string *strSimplePinyin, std::string *strPinyin);
};

#endif