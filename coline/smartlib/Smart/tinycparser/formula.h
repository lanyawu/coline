#ifndef _FORMULA_H_
#define _FORMULA_H_

#include <string>
#include <vector>
#include "formula_public.h"
#include "user_func.h"

using namespace std;

#define MAX_SPLIT_VAL 100 /* 每个字符串分割后允许的最大项数 */

class CFormula {
private:
	string sOriFormula;			/* 原始公式代码 */
	int iID;

	CUserFuncList *pFuncList;					/* 自定义函数列表 */
	CCodeStructList *pCCodeStructList;             /*执行代码四元组指针*/
	CVarStructList *pVarList;        					/*运算变量存储区*/
	CVarStructList *pOutVarList;            			/*输出变量列表 */

	int iUsed;

	CVarStruct varStructs[MAX_SPLIT_VAL];/* 使用split分隔的字符串列表，分隔后字符串以$开头带数字的名称方式访问 */
	char *vSplitStr[MAX_SPLIT_VAL];	/* 使用split分隔的字符串列表，分隔后字符串以$开头带数字的名称方式访问 */
	int iLen;	/* 当前split分隔的字符串的子串数量 */


	CCodeStructList * format_code_list(CCodeStructList * pHeader);
	void format_para_list(CParamList * p);
public:
	int (* load_var)(CVarStruct *pCVarStruct);	/* 加载外部输入变量的函数指针 */

	CFormula(int iFormulaID) {
		this->iID = iFormulaID;
		this->sOriFormula = "";
		this->pFuncList = NULL;
		this->pCCodeStructList = NULL;
		this->pVarList = NULL;
		this->pOutVarList = NULL;
		this->iLen = 0;
		this->load_var = NULL;
	}

	CFormula(int iFormulaID, string sSource, int (* load_var)(CVarStruct *pCVarStruct)) {
		this->iID = iFormulaID;
		this->sOriFormula = sSource;
		this->pFuncList = NULL;
		this->pCCodeStructList = NULL;
		this->pVarList = NULL;
		this->pOutVarList = NULL;
		this->iLen = 0;
		this->load_var = load_var;
	}
	~CFormula() {
		if (NULL != pFuncList)
			delete pFuncList;
		if (NULL != pCCodeStructList)
			delete pCCodeStructList;
		if (NULL != pVarList)
			delete pVarList;
		if (NULL != pOutVarList)
			delete pOutVarList;
		while (iLen > 0)
			delete[] vSplitStr[(iLen--) - 1];
	}

	void format_formula();
	string get_ori_str() { return this->sOriFormula; }
	
	char ** get_split_str() { return this->vSplitStr; }

	CVarStruct *get_split_var() { return this->varStructs; }

	int get_split_len() { return this->iLen; }
	void set_split_len(int iLen) { this->iLen = iLen; }

	void set_load_var_func(int (* load_var)(CVarStruct *pCVarStruct));

	CVarStructList * getOutVarList() {
		return pOutVarList;
	}

	int get_id () {
		return iID;
	}

	CCodeStructList * get_c_s_list() {
		return this->pCCodeStructList;
	}

	void set_c_s_list(CCodeStructList *p) { this->pCCodeStructList = p; }

	CVarStruct * get_sysvar(string sVarName, string sTokenString, int iFlag);
	bool is_sysvar(CVarStruct* pVar);

	CUseFunc * add_user_func(const string sFuncName);

	CUseFunc * get_user_func(const string sFuncName);

	void show_formula();

	void process();
};



#endif
