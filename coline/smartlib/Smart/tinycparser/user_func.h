#ifndef _USER_FUNC_H_
#define _USER_FUNC_H_

#include "formula_public.h"

/* �Զ��庯�� */
class CUseFunc {
private:
	string sName;		/* ���������� */
	CVarStructList *pParaList;		/* ���������洢�� */
	/* CVarStruct *pRes;					��������ֵ */
	/* vector<VarStruct> vVarList;        		���������洢������ʱ�����ú����ľ�������ˣ���Ϊȫ�ֱ��� */
	/* ���������ĳ�ʼ�� */
	void init_func_param(CCodeStruct *pCCodeStruct) {
		CParamList *pCParamList = pCCodeStruct->pCParamList;
		CVarStructList *pFuncParaList = this->pParaList;
		
		while (NULL != pFuncParaList && NULL != pCParamList) {
			CVarStruct *pTarget = pFuncParaList->get_var_struct();
			CVarStruct *pSource = pCParamList->get_param()->deal_param();
			
			*pTarget = *pSource;
			
			pFuncParaList = pFuncParaList->pNext;
			pCParamList = pCParamList->pNext;
		}
	}
public:
	CCodeStructList *pCCodeStructList;		/* ��������ִ��Ԫ�� */

	CUseFunc(string sFuncName) {
		sName = sFuncName;
		pParaList = NULL;
		pCCodeStructList = NULL;
	}

	~CUseFunc() {
		if (this->pParaList)
			delete this->pParaList;
		if (this->pCCodeStructList)
			delete this->pCCodeStructList;
	}

	/* �Զ��庯��ִ��*/
	CVarStruct * process(CCodeStruct *pCCodeStruct) {
		init_func_param(pCCodeStruct);
		
		return pCCodeStructList->process();
	}

	string get_func_name() { return this->sName; }

	//CCodeStructList * get_codestruct_list() { return this->pCCodeStructList; }

	/* ȡ��������ָ�����ƵĲ��� */
	CVarStruct * get_func_param(const string sParamName) {
		CVarStructList *pParaList = this->pParaList;
		while (NULL != pParaList) {
			if (0 == sParamName.compare(pParaList->get_var_struct()->get_name()))
				return pParaList->get_var_struct();
			pParaList = pParaList->pNext;
		}
		return NULL;
	}
	
	/* ���ɱ������Ĳ��� */
	void add_func_para(const string sParaName) {
		CVarStructList *p = new CVarStructList(sParaName);
		p->pNext = pParaList;
		pParaList = p;
	}

	void show() {
		cout << "function:" << this->sName << endl;
		cout << "paramter:" << endl;
		if (NULL != this->pParaList)
			this->pParaList->show();
		cout << endl;
		cout << "detail:" << endl;
		if (NULL != pCCodeStructList)
			pCCodeStructList->show();
		cout << endl;

	}
};

class CUserFuncList {
private:
	CUseFunc *pCUserFunc;
	CUserFuncList *pNext;
public:
	CUserFuncList(const string sFuncName) {
		pCUserFunc = new CUseFunc(sFuncName);
		pNext = NULL;
	}

	~CUserFuncList() {
		if (NULL != pNext)
			delete pNext;
		if (NULL != pCUserFunc)
			delete pCUserFunc;
	}

	CUserFuncList * get_next() { return this->pNext; }

	void set_next(CUserFuncList *p) { this->pNext = p; }

	CUseFunc * get_use_func() { return this->pCUserFunc; }

	void show() {
		if (NULL != pCUserFunc)
			pCUserFunc->show();
		if (NULL != pNext)
			pNext->show();
	}
};

#endif
