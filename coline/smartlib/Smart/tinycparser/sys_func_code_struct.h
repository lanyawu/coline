/****************************************************/
/* File: sys_func_code_struct.h                     */
/* author: zhewei.fan                               */
/* CCodeStruct 内部函数子类定义                     */
/****************************************************/
#ifndef _SYS_FUNC_CODE_STRUCT_H_
#define _SYS_FUNC_CODE_STRUCT_H_

#include <math.h>
#include <string.h>
#include "formula_public.h"
#include "formula.h"
#include "formula_group.h"

#pragma warning(disable:4996)

/* 函数类型的CodeStruct */
class CCodeStruct_SysFunc : public CCodeStruct {
public:
	CCodeStruct_SysFunc() {
		this->pResValue = new CVarStruct();
		this->pCParamList = NULL;
	}
};

class CCodeStruct_Atof : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p = this->pCParamList->get_param()->deal_param();
		this->pResValue->set_value(atof(p->get_value()));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:atof()"; }
};

class CCodeStruct_Ceil : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p = this->pCParamList->get_param()->deal_param();
		this->pResValue->set_value(ceil(atof(p->get_value())));
		return this->pResValue;
	}

	string get_op_name() { return "sys_func:ceil()"; }
};

class CCodeStruct_Floor : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p = this->pCParamList->get_param()->deal_param();
		this->pResValue->set_value(floor(atof(p->get_value())));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:Floor()"; }
};

class CCodeStruct_Abs : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p = this->pCParamList->get_param()->deal_param();
		this->pResValue->set_value(fabs(atof(p->get_value())));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:abs()"; }
};

class CCodeStruct_Index : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		CVarStruct *p2 = this->pCParamList->pNext->get_param()->deal_param();

		this->pResValue->set_value(strstr(p1->get_value(), p2->get_value()));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:index()"; }
};

class CCodeStruct_Strtok : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		CVarStruct *p2 = this->pCParamList->pNext->get_param()->deal_param();

		this->pResValue->set_value(strtok(p1->get_value(), p2->get_value()));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:strtok()"; }
};

class CCodeStruct_Substr : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		CVarStruct *p2 = this->pCParamList->pNext->get_param()->deal_param();
		CVarStruct *p3 = this->pCParamList->pNext->pNext->get_param()->deal_param();

		int iLen = (int) atof(p3->get_value());
		char *tmp = new char[iLen + 1];
		strncpy(tmp, p1->get_value() + (int) atof(p2->get_value()), iLen);
		if (NULL != this->pResValue->get_value())
			delete this->pResValue->get_value();
		
		this->pResValue->set_value(tmp);
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:substr()"; }
};

class CCodeStruct_Strlen : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		
		this->pResValue->set_value((int) strlen(p1->get_value()));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:strlen()"; }
};

class CCodeStruct_Trim : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		
		this->pResValue->set_value(trim(p1->get_value()));
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:trim()"; }
};


class CCodeStruct_Split : public CCodeStruct_SysFunc {
private:
	CFormula *pFm;
public:
	CCodeStruct_Split(CFormula *pFm) {
		this->pResValue = new CVarStruct();
		this->pCParamList = NULL;
		this->pFm = pFm;
	}

	CVarStruct * proc_code_stuct(bool& bReturnFlag){
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		CVarStruct *p2 = this->pCParamList->pNext->get_param()->deal_param();

		CVarStruct *pCVarStruct = pFm->get_split_var();
		int i = 0;
		int iOldLen = pFm->get_split_len();
		char *pOldStr[MAX_SPLIT_VAL];
		char *pc;

		if (i > 0) {
			for ( ; i > 0; i--) {
				pOldStr[i - 1] = (pCVarStruct[i++]).get_value();
			}
		}

		(pCVarStruct[i++]).set_value(copy_string(p1->get_value()));
		(pCVarStruct[i++]).set_value(copy_string(strtok_all(p1->get_value(), p2->get_value())));
		while (NULL != (pc = strtok_all(NULL, p2->get_value()))) {
			(pCVarStruct[i++]).set_value(copy_string(pc));
			if (i >= MAX_SPLIT_VAL - 1) {
				printf("字符串\"%s\"以\"%s\"分隔后产生的子串的数目超过%d限制\n", (pCVarStruct[0]).get_value(), p2->get_value(),  MAX_SPLIT_VAL - 1);
				exit(1);
			}
		}
		pFm->set_split_len(i - 1);
		this->pResValue->set_value(i - 1);

		if (iOldLen > 0)
			for (i = 0; i < iOldLen; i++)
				if (NULL != pOldStr[i])
					free(pOldStr[i]);

		return this->pResValue;
	}
	string get_op_name() { return "sys_func:split()"; }
};


class CCodeStruct_Replace : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CVarStruct *p1 = this->pCParamList->get_param()->deal_param();
		CVarStruct *p2 = this->pCParamList->pNext->get_param()->deal_param();
		CVarStruct *p3 = this->pCParamList->pNext->pNext->get_param()->deal_param();

		char *tmp = str_replace(p1->get_value(), p2->get_value(), p3->get_value());

		if (NULL != this->pResValue->get_value())
			delete[] this->pResValue->get_value(); 
		this->pResValue->set_value(tmp);
		return this->pResValue;
	}
	string get_op_name() { return "sys_func:replace()"; }
};

class CCodeStruce_Printf : public CCodeStruct_SysFunc {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		CParamList * p = this->pCParamList;

		while (NULL != p) {
			CVarStruct *p1 = p->get_param()->deal_param();
			cout << p1->get_name() << ":" << p1->get_value() << endl;
			p = p->pNext;
		}
		return NULL;
	}
	string get_op_name() { return "sys_func:printf()"; }
};

#pragma warning(default:4996)

#endif
