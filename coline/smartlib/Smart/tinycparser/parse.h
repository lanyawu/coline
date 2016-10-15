#ifndef _PARSE_
#define _PARSE_

#include <string>
#include <fstream>
#include "formula_public.h"
#include "formula.h"
using namespace std;

typedef enum {
	START, INCOMMENT, INNUM, INFLOAT, INID, SPLITID, INCONSTSTR, INCOMPARE, INLOGIC, DONE, END
} StateType;

const int MAXRESERVED = 20;

/* 定义字符串与 token 的关系 */
static struct ReservedInfo {
	const string sKey;
	const TokenType iToken;
} ReservedInfo[MAXRESERVED] = {
	{"function", 	FUNCDEF},
	{"NULL", 		NULLVAL},
	{"if", 			IF},
	{"elseif", 		ELSEIF},
	{"else", 		ELSE},
	{"while", 		WHILE},
	{"break", 		BREAK},
	{"continue", 	CONTINUE},
	{"return", 		RETURN},
	{"atof", 		FUNCTION},
	{"ceil", 		FUNCTION},
	{"floor", 		FUNCTION},
	{"abs", 		FUNCTION},
	{"index", 		FUNCTION},
	{"substr", 		FUNCTION},
	{"strlen", 		FUNCTION},
	{"trim", 		FUNCTION},
	{"replace", 	FUNCTION},
	{"split",		FUNCTION},
	{"printf",		FUNCTION}
};
class CSmallCParse {
private:
	string sOriScript;	// 需解析的类 C 脚本
	int iPos;			// sOriScript当前解析点的位置
	TokenType token;
	string sTokenString;
	CFormula * pCFormula;
	CUseFunc * pCUseFunc;
	
	/* private function -- scan input script*/
	char get_next_char() 
	{ 
		if (iPos <= this->pCFormula->get_ori_str().size())
			return (this->pCFormula->get_ori_str())[iPos++]; 
		else
		{
			iPos ++;
			return ' ';
		}
	}
	void unget_next_char() { iPos--; }

	TokenType get_func_token(string sStr) {
		int i;
		for (i = 0; i < MAXRESERVED; i++) {
			if (!sStr.compare(ReservedInfo[i].sKey)) {
				return ReservedInfo[i].iToken;
			}
		}
		return USERFUNC;
	}

	TokenType get_string_token(string sStr) {
		int i;
		for (i = 0; i < MAXRESERVED; i++) {
			if (0 == sStr.compare(ReservedInfo[i].sKey)) {
				return ReservedInfo[i].iToken;
			}
		}
		if (0 == sStr.compare(0, IN_VAR_HEADER.size(), IN_VAR_HEADER))
			return INPUTVAR;
		else if (0 == sStr.compare(0, OUT_VAR_HEADER.size(), OUT_VAR_HEADER))
			return OUTPUTVAR;
		return SYSVAR;
	}

	//是否为带空格的保留字
	bool is_reserv_with_blank(string sStr) {
		if (sStr.compare(0, 4, "else", 4))
			return true;
		return false;
	}
	void next_token();

	/* private function -- parse script */
	bool match(TokenType expected);
	bool stmt_sequence(CCodeStructList*& pPrev);
	bool statement(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool function_define();
	bool if_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool while_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool assign_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool return_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool expr_logic(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool expr_cmp(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool simple_exp(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool term(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool factor(CCodeStructList*& pPrev, CCodeStructList*& pNext);
	bool function(CCodeStructList*& pPrev, CCodeStructList*& pNext);
public:
	CSmallCParse(CFormula *pCFormula) {
		this->sOriScript = pCFormula->get_ori_str();
		this->pCUseFunc = NULL;
		this->pCFormula = pCFormula;
		iPos = 0; 
	}

	CFormula * parse();


};


#endif
