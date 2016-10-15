#include <string.h>
#include "parse.h"
#include "formula.h"

CVarStruct * CFormula::get_sysvar(string sVarName, string sTokenString, int iFlag) {
	CVarStructList *pVarList = NULL;
	int iVarType = 0;

	if (!sVarName.compare(0, OUT_VAR_HEADER.size(), OUT_VAR_HEADER)) {
		pVarList = this->pOutVarList;
		iVarType = 1 ;
	} else {
		pVarList = this->pVarList;
	}

	while (pVarList) {
		if (pVarList->get_var_struct()->get_name() == sTokenString)
			return pVarList->get_var_struct();
		pVarList = pVarList->pNext;
	}
	if (iFlag) {
		CVarStruct *pVarStruct;
		if (!sVarName.compare(0, IN_VAR_HEADER.size(), IN_VAR_HEADER)) {
			pVarStruct = new CInputVarStruct(sVarName, this->load_var);
		} else 
			pVarStruct = new CVarStruct(sVarName);
		pVarList = new CVarStructList(pVarStruct);

		if (iVarType) {
			pVarList->pNext = this->pOutVarList;
			this->pOutVarList = pVarList;
		} else {
			pVarList->pNext = this->pVarList;
			this->pVarList = pVarList;
		}

		return pVarStruct;
	}
	return NULL;
}

bool CFormula::is_sysvar(CVarStruct* pVar) {
	CVarStructList *pVarList = NULL;

	if (!pVar->get_name().compare(0, OUT_VAR_HEADER.size(), OUT_VAR_HEADER)) {
		pVarList = this->pOutVarList;
	} else {
		pVarList = this->pVarList;
	}
	while (pVarList) {
		if (pVarList->get_var_struct()->get_name() == pVar->get_name())
			return true;
		pVarList = pVarList->pNext;
	}
	return false;
}

void CFormula::format_para_list(CParamList * p) {
	if (NULL != p->get_param() && CodeK == p->get_param()->get_param_kind()) {
		p->set_param(format_code_list((CCodeStructList * ) p->get_param()));
	}
}

CCodeStructList * CFormula::format_code_list(CCodeStructList * pHeader) {
	CCodeStructList *pHead, *pTmp, *pPrev, *pNext;

	pTmp = pHead = pHeader;
	pPrev = pNext = NULL;

	while (NULL != pTmp) {
		CParamList *pParamList = pTmp->get_code_struct()->pCParamList;

		while (NULL != pParamList) {
			format_para_list(pParamList);
			pParamList = pParamList->pNext;
		}

		pNext = pTmp->pNext;
		if (0 == pTmp->get_code_struct()->get_op_name().compare("VAR_CODESTRUCT")) {
			pTmp->pNext = NULL;
			delete pTmp;
			pTmp = NULL;

			if (NULL == pPrev)
				pHead = pNext;
			else
				pPrev->pNext = pNext;
		}
		if (NULL != pTmp)
			pPrev = pTmp;
		pTmp = pNext;
	}
	return pHead;
}

void CFormula::format_formula() {
	this->pCCodeStructList = format_code_list(this->pCCodeStructList);
	CUserFuncList *p = this->pFuncList;
	while (NULL != p) {
		CUseFunc *pCUseFunc = p->get_use_func();
		pCUseFunc->pCCodeStructList = format_code_list(pCUseFunc->pCCodeStructList);
		p = p->get_next();
	}
}

/* 增加新的自定义函数 */
CUseFunc * CFormula::add_user_func(const string sFuncName) {
	CUserFuncList *p = new CUserFuncList(sFuncName);
	p->set_next(this->pFuncList);
	this->pFuncList = p;
	return p->get_use_func();
}

CUseFunc * CFormula::get_user_func(const string sFuncName) {
	CUserFuncList *p = this->pFuncList;
	while (NULL != p) {
		if (0 == sFuncName.compare(p->get_use_func()->get_func_name()))
			return p->get_use_func();
		p = p->get_next();
	}
	return NULL;
}

void CFormula::show_formula() {
	if (NULL != this->pFuncList)
		this->pFuncList->show();
	cout << endl << "sysvar:" << endl;
	if (NULL != this->pVarList)
		this->pVarList->show();
	cout << endl << "outvar:" << endl;
	if (NULL != this->pOutVarList)
		this->pOutVarList->show();
	cout << endl << "main_code_info" << endl;
	if (NULL != this->pCCodeStructList)
		this->pCCodeStructList->show();
	cout << endl;
	
}

void CFormula::process() {
	this->pCCodeStructList->process();
}

void CFormula::set_load_var_func(int (* load_var)(CVarStruct *pCVarStruct)) {
	this->load_var = load_var;
}
