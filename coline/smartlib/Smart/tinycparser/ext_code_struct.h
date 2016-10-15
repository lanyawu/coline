#ifndef _EXT_CODE_STRUCT_H_
#define _EXT_CODE_STRUCT_H_

#include "user_func.h"
#include "formula_public.h"
#include "formula.h"

/* ����Ϊ������Ԫ������� */
/* if CodeStruct ���� */
class CCodeStruct_IF : public CCodeStruct {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag); 

	string get_op_name() {return "if";};
};

/* while CodeStruct ���� */
class CCodeStruct_WHILE : public CCodeStruct {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag);

	string get_op_name() {return "while";}
};

/* �����Զ��庯�� CodeStruct */
class CCodeStruct_UFunc : public CCodeStruct {
private:
	CUseFunc *pCUseFunc;
public:
	CCodeStruct_UFunc(CUseFunc *pCUseFunc) {
		this->pCUseFunc = pCUseFunc;	
		this->pResValue = new CVarStruct();
	}
	~CCodeStruct_UFunc() {
		if (pCUseFunc)
			delete pCUseFunc;
	}
	CVarStruct * proc_code_stuct(bool& bReturnFlag);
	string get_op_name() {return pCUseFunc->get_func_name(); }
};

/* ��ֵ����codestruct�� */
class CCodeStruct_Asign : public CCodeStruct {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag);
	string get_op_name(){return "=";}
};

/* ��ֵ�����codestruct�� */
class CCodeStruct_Cal : public CCodeStruct {
private:
	TokenType op;
public:
	CCodeStruct_Cal(TokenType op) { 
		this->op = op; 
		this->pResValue = new CVarStruct();
	}
	CVarStruct * proc_code_stuct(bool& bReturnFlag); 
	string get_op_name(){return TokenTypeMap[op];}
};

/* �Ƚϼ����codestruct�� */
class CCodeStruct_Cmp : public CCodeStruct {
private:
	TokenType op;
public:
	CCodeStruct_Cmp(TokenType op) { 
		this->op = op;
		this->pResValue = new CVarStruct();
	}
	CVarStruct * proc_code_stuct(bool& bReturnFlag); 
	string get_op_name() {return TokenTypeMap[op];}
};

/* and�� or �ȵ��߼�����codestruct�� */
class CCodeStruct_Logic : public CCodeStruct {
private:
	TokenType op;
public:
	CCodeStruct_Logic(TokenType op) { 
		this->op = op; 
		this->pResValue = new CVarStruct();
	}
	CVarStruct * proc_code_stuct(bool& bReturnFlag); 
	string get_op_name() {return TokenTypeMap[op];}
};

/* �������͵�CodeStruct */
class CCodeStruct_Var : public CCodeStruct {
public:
	CCodeStruct_Var() {
		this->pResValue = NULL;
		this->pCParamList = NULL;
	}
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		return NULL; 
	}
	string get_op_name() {
		return "VAR_CODESTRUCT";
	}
};

/* renturn���͵�CodeStruct */
class CCodeStruct_Return : public CCodeStruct {
public:
	CCodeStruct_Return() {
		this->pResValue = NULL;
		this->pCParamList = NULL;
	}
	CVarStruct * proc_code_stuct(bool& bReturnFlag) {
		bReturnFlag = true;
		return pCParamList->get_param()->deal_param(); 
	}
	string get_op_name() {
		return "RETURN";
	}
};


#endif
