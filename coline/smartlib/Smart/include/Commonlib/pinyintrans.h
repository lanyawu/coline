#ifndef __PINYINTRANS_H___
#define __PINYINTRANS_H___

#include <CommonLib/Types.h>

//ƴ��ת��
class COMMONLIB_API CPinyinTrans
{
public:
	CPinyinTrans(void);
	~CPinyinTrans(void);
public:
	//�������ֻ�ȡ��ƴ��ȫƴ 
	static BOOL GetPinyinByText(WCHAR *szText, std::string *strSimplePinyin, std::string *strPinyin);
};

#endif