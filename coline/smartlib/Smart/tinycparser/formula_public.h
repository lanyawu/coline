#ifndef _FORMULA_PUBLIC_H_
#define _FORMULA_PUBLIC_H_

#include <iostream>
#include <sstream>
#include <vector>
#include "util.h"

using namespace std;

#define TRUE 1
#define FALSE 0


#define MAX_ID_NAME_LENGTH 33 /* 待解析变量的名称的最大长度 */

const string IN_VAR_HEADER	= "in_";		/* 输入变量头 */
const string OUT_VAR_HEADER = "out_";	/* 输出变量头 */

#pragma warning(disable:4996)

typedef enum {
	ENDSTR, ERROR, ID, NULLVAL,/* reserved words */
	IF, ELSEIF, ELSE, BREAK, CONTINUE, WHILE, RETURN,
	AND, OR,
	ASSIGN, EQ, LT, GT, GEQ, LEQ, UEQ,/* 附值比较 */
	PLUS, MINUS, MULTIPLY, DIVIDE, MOD,
	LBRACE/* { */, RBRACE/* } */, LPAREN/* ( */, RPAREN/* ) */, LBRACKET/* [ */, RBRACKET/* ] */,
	COMMA/* , */, SEMI/* ; */,
	FUNCTION, USERFUNC, FUNCDEF,/* 函数 */
	SPLITVAR, INPUTVAR, OUTPUTVAR, SYSVAR, CONSTSTRING, CONSTNUM
} TokenType;

/* CodeStruct的分类 */
typedef enum { StmtK, ExpK, ValueK } NodeKind;
/* VarStruct的分类 */
typedef enum { FloatK, StringK, UnknowK } VarKind;
/* ParamList的分类 */
typedef enum { CodeK, VarK } ParamKind;

const string TokenTypeMap[] = {
	"ENDSTR", "ERROR", "ID", "NULLVAL",
	"IF", "ELSEIF", "ELSE", "BREAK", "CONTINUE", "WHILE", "RETURN",
	"AND", " OR",
	"ASSIGN", "EQ", "LT", "GT", "GEQ", "LEQ", "UEQ",
	"PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MOD",
	"LBRACE/* { */", "RBRACE/* } */", "LPAREN/* ( */", "RPAREN/* ) */", "LBRACKET/* [ */", " RBRACKET/* ] */",
	"COMMA/* , */", "SEMI/* ; */",
	"FUNCTION", "USERFUNC", "FUNCDEF",
	"SPLITVAR", "INPUTVAR", "OUTPUTVAR", "SYSVAR", "CONSTSTRING", "CONSTNUM", "CONSTFLOAT"
};
const string NodeKindMap[] 	= {"StmtK", "ExpK", "ValueK"};
const string VarKindMap[] 	= {"FloatK", "StringK", "UnknowK"};
const string ParamKindMap[] = {"CodeK", "VarK"};

class CVarStruct {
private:
	VarKind iValueType;
	char * sValue;
	string sName;/* 临时变量没有变量名称 */
	int iFlag;		/* 0 初始状态，每次调用需先置值的标志 1 变更状态 */
public:
	int iLink; //当前变量的引用次数

	CVarStruct() {
		sName = "tmpVar";
		iValueType = UnknowK;
		sValue = NULL;
		iFlag = 0;
		iLink = 0;
	}
	CVarStruct(string sVarName) {
		sName = sVarName;
			
		this->iValueType = UnknowK;
		this->sValue = NULL;
		iFlag = 0;
		iLink = 0;
	}

	CVarStruct(string sVarName, const char * sValue) {
		sName = sVarName;

		this->iValueType = StringK;
		if (NULL == sValue)
			this->sValue = NULL;
		else {
			this->sValue = new char[strlen(sValue) + 1];
			strcpy(this->sValue, sValue);
		}
		
		iFlag = 0;
		iLink = 0;
	}

	CVarStruct(string sVarName, float fValue) {
		sName = sVarName;
		
		ostringstream s1;
		this->iValueType = FloatK;
		s1 << fValue;
		this->sValue = copy_string(s1.str().c_str());

		iFlag = 0;
		iLink = 0;
	}

	CVarStruct(const CVarStruct& source) {
		iValueType = source.iValueType;
		this->sValue = copy_string(source.sValue);
		iLink = 0;
	}


	VarKind get_var_kind() {
		return iValueType;
	}

	char * get_value() {
		return sValue;
	}

	string get_name() {
		return sName;
	}

	void set_value(double fValue) {
		if (NULL != this->sValue)
			delete[] this->sValue;
		ostringstream s1;
		this->iValueType = FloatK;
		s1 << fValue;
		this->sValue = copy_string(s1.str().c_str());
	}

	void set_value(int iValue) {
		if (NULL != this->sValue)
			delete[] this->sValue;
		ostringstream s1;
		this->iValueType = FloatK;
		s1 << iValue;
		this->sValue = copy_string(s1.str().c_str());
	}
	
	void set_value(char * sValue) {
		this->iValueType = StringK;
		this->sValue = sValue;
	}

	void set_val_type(VarKind iValueType) {
		this->iValueType = iValueType;
	}
	
	CVarStruct& operator =(const CVarStruct& source) {
		if (this == &source)
			return *this;
		iValueType = source.iValueType;
		if (NULL != this->sValue)
			delete[] this->sValue;
		this->sValue = copy_string(source.sValue);
		this->iLink = 0;

		return *this;
	}

	CVarStruct& copy(const CVarStruct& source) {
		if (this == &source)
			return *this;
		iValueType = source.iValueType;
		if (NULL != this->sValue)
			delete[] this->sValue;
		this->sValue = copy_string(source.sValue);
		this->iLink = 0;

		return *this;
	}

	~CVarStruct() {
		iValueType = UnknowK;
		if (NULL != this->sValue) {
			delete[] this->sValue;
			this->sValue = NULL;
		}
		iFlag = 0;
		iLink = 0;
	}

	void show() {
		cout << "<" << VarKindMap[this->iValueType] << ", " << this->sName;
		if (NULL != this->sValue)
			cout << ", " << this->sValue;
		else
			cout << ", NULL";
		cout << ">";
	}
};

class CInputVarStruct : public CVarStruct {
private:
	int (* load_var)(CVarStruct *pCVarStruct);
public:
	CInputVarStruct(string sVarName, int (* load_var)(CVarStruct *pCVarStruct)) : CVarStruct(sVarName) {
		this->load_var = load_var;
	}

	void load_self() {
		this->load_var(this);
	}
};

class CVarStructList {
private:
	CVarStruct * pCVarStruct;
public:
	CVarStructList *pNext;

	CVarStructList() {
		pCVarStruct = NULL;
		pNext = NULL;
	}

	CVarStructList(const string sVarName) {
		pCVarStruct = new CVarStruct(sVarName);
		pNext = NULL;
	}

	CVarStructList(CVarStruct * pCVarStruct) {
		this->pCVarStruct = pCVarStruct;
		pNext = NULL;
	}

	CVarStructList(CVarStruct * pCVarStruct, CVarStructList *pNext) {
		this->pCVarStruct = pCVarStruct;
		this->pNext = pNext;
	}

	~CVarStructList() {
		if (this->pNext) 
			delete this->pNext;
		if (this->pCVarStruct)
			if (0 == this->pCVarStruct->get_name().compare("tmpVar") && 0 == this->pCVarStruct->iLink)
				delete this->pCVarStruct;
			else
				this->pCVarStruct->iLink--;
	}

	void set_var_struct(CVarStruct * pCVarStruct) {
		this->pCVarStruct = pCVarStruct;
	}

	CVarStruct * get_var_struct() {
		return this->pCVarStruct;
	}

	void show() {
		if (NULL != this->pCVarStruct)
			pCVarStruct->show();
		if (NULL != pNext) {
			cout << ", ";
			pNext->show();
		}
	}

};

/* 参数类：*/
class CParam {
private:
	ParamKind paramK;
public:
	ParamKind get_param_kind() {
		return paramK;
	}
	void set_param_kind(ParamKind paramK) {
		this->paramK = paramK;
	}

	virtual CVarStruct * deal_param(){ return NULL; };

	virtual void show() { cout << "PARAMETER" << endl; }
};



class CVarParam : public CParam {
private:
	CVarStruct * pCVarStruct;
public:
	CVarParam(CVarStruct * pCVarStruct) {
		set_param_kind(VarK);
		this->pCVarStruct = pCVarStruct;
		pCVarStruct->iLink++;
	}

	CVarStruct * deal_param() {
		string name = pCVarStruct->get_name();

		if (0 == name.find("in_", 0)) {/* 运行时由外部输入过去变量值得变量 */
			((CInputVarStruct *) pCVarStruct)->load_self();		
		}
		return this->pCVarStruct;
	}

	~CVarParam() {
		if (pCVarStruct)
			if (0 == pCVarStruct->get_name().compare("tmpVar") && 0 == pCVarStruct->iLink)
				delete pCVarStruct;
			else
				pCVarStruct->iLink--;
	}

	void show() {
		if (NULL != pCVarStruct)
			pCVarStruct->show();
	}
};


class CParamList {
private:
	CParam *pCParam;
public:
	CParamList *pNext;
	CParamList(CParam *pCParam, CParamList *pNext) {
		this->pCParam = pCParam;
		this->pNext = pNext;
	}

	CParam * get_param() {
		return pCParam;
	}

	void set_param(CParam *pCParam) {
		this->pCParam = pCParam;
	}

	void show() {
		if (NULL != this->pCParam) {
			cout << "(";
			pCParam->show();
			cout << ")";
		}

		if (NULL != pNext)
			pNext->show();
	}

	~CParamList() {
		if (NULL != pCParam)
			delete pCParam;

		if (NULL != pNext)
			delete pNext;
	}
	
};

/* 可执行代码四元组定义 */
class CCodeStruct {
public:
	CParamList *pCParamList;	/* 计算元组计算需要的参数链表 */
	CVarStruct *pResValue;	/* 计算元组的运算结果 */

	CCodeStruct() {
		this->pResValue = NULL;
		this->pCParamList = NULL;
	}
	
	~CCodeStruct() {
		if (NULL != pCParamList)
			delete pCParamList;
		if (NULL != pResValue && 0 == pResValue->iLink)
			delete pResValue;
		else
			pResValue->iLink--;
	}

	void set_param_list(CParamList *pCParamList) { this->pCParamList = pCParamList; }

	virtual string get_op_name() { return ""; }

	/* 代码四元组解析执行调用的虚函数 */
	virtual CVarStruct* proc_code_stuct(bool& bReturnFlag) { return pResValue; }

	void show() {
		cout << "{" << get_op_name();
		if (NULL != pCParamList)
			pCParamList->show();

		if (NULL != pResValue)
			pResValue->show();
		printf("}");
	}
};

/* 顺序执行的四元组列表 */
class CCodeStructList : public CParam {
private:
	CCodeStruct *pCCodeStruct;
public:
	CCodeStructList *pNext;

	CCodeStructList() {
		set_param_kind(CodeK);
		pCCodeStruct = NULL;
		pNext = NULL;
	}

	CCodeStructList(CCodeStruct *pCCodeStruct) {
		set_param_kind(CodeK);
		this->pCCodeStruct = pCCodeStruct;
		pNext = NULL;
	}

	~CCodeStructList() {
		if (NULL != this->pNext)
			delete this->pNext;
		if (this->pCCodeStruct)
			delete this->pCCodeStruct;
	}

	void set_code_struct(CCodeStruct *pCCodeStruct) {
		if (this->pCCodeStruct)
			delete this->pCCodeStruct;
		this->pCCodeStruct = pCCodeStruct;
	}

	void set_next(CCodeStructList *pCCodeStructList) {
		if (this->pNext)
			delete this->pNext;
		this->pNext = pCCodeStructList;
	}

	CCodeStruct * get_code_struct() {
		return pCCodeStruct;
	}

	CVarStruct * deal_param() {
		return process();
	}

	CVarStruct * process() {
		CCodeStructList *pCur = this;
		CVarStruct *pCVarStruct = NULL;
		bool bFlag = false;
		
		while (NULL != pCur && !bFlag) {
			pCVarStruct = pCur->pCCodeStruct->proc_code_stuct(bFlag);
			pCur = pCur->pNext;
		}
		return pCVarStruct;
	}

	void show() {
		if (NULL != pCCodeStruct)
			pCCodeStruct->show();
		
		if (NULL != pNext)
			pNext->show();
	}
};



#endif

#pragma warning(default:4996)
