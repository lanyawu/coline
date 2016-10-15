#ifndef _EXT_CODE_STRUCT_H_
#define _EXT_CODE_STRUCT_H_

#include "user_func.h"
#include "formula_public.h"
#include "formula.h"

/* 以下为代码四元组的子类 */
/* if CodeStruct 的类 */
class CCodeStruct_IF : public CCodeStruct {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag); 

	string get_op_name() {return "if";};
};

/* while CodeStruct 的类 */
class CCodeStruct_WHILE : public CCodeStruct {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag);

	string get_op_name() {return "while";}
};

/* 调用自定义函数 CodeStruct */
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

/* 附值语句的codestruct类 */
class CCodeStruct_Asign : public CCodeStruct {
public:
	CVarStruct * proc_code_stuct(bool& bReturnFlag);
	string get_op_name(){return "=";}
};

/* 数值计算的codestruct类 */
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

/* 比较计算的codestruct类 */
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

/* and， or 等的逻辑处理codestruct类 */
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

/* 变量类型的CodeStruct */
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

/* renturn类型的CodeStruct */
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
