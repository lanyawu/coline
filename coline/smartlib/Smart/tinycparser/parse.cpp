#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "formula.h"
#include "user_func.h"
#include "ext_code_struct.h"
#include "factory_code_struct.h"
#include "parse.h"

using namespace std;

void CSmallCParse::next_token() {
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to sTokenString */
    bool save;

	this->sTokenString = "";
    while (state != DONE) {
        char c = get_next_char();
        save = true;
        switch (state) {
            case START:
                if (isdigit(c)) {
                    state = INNUM;
                } else if (isalpha(c)) {
                    state = INID;
                } else if (c == '$') {
					state = SPLITID;
				} else if ((c == '=') || (c == '>') || (c == '<') || (c == '>') || (c == '!')) {
                    state = INCOMPARE;
                } else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
                    save = FALSE;
                } else if (c == '#') {
                    save = FALSE;
                    state = INCOMMENT;
                } else if (c == '\"') {
                	state = INCONSTSTR;
                } else if ((c == '|') || (c == '&')) {
                	state = INLOGIC;
                } else {
                    state = DONE;
                    switch (c) {
                        case '\0':
                            save = false;
                            token = ENDSTR;
                            break;
                        case '+':
                            token = PLUS;
                            break;
                        case '-':
                            token = MINUS;
                            break;
                        case '*':
                            token = MULTIPLY;
                            break;
                        case '/':
                            token = DIVIDE;
                            break;
                        case '{':
                            token = LBRACE;
                            break;
                        case '}':
                            token = RBRACE;
                            break;
                        case '(':
                            token = LPAREN;
                            break;
                        case ')':
                            token = RPAREN;
                            break;
                        case ',':
                            token = COMMA;
                            break;
                        case ';':
                            token = SEMI;
                            break;
                        default:
                            token = ERROR;
                            break;
                    }
                }
                break;
            case INCOMMENT:
                save = false;
                if (c == '\n') {/* comment is one line */
                    state = START;
                }
                break;
            case INLOGIC:
            	state = DONE;
            	if (c == '|') {
            		if ('|' == sTokenString[0])
            			token = OR;
                    else
                    	token = ERROR;
            	} else if (c == '&') {
            		if ('&' == sTokenString[0])
            			token = AND;
                    else
                    	token = ERROR;
                }
                break;
            case INCOMPARE:
                state = DONE;
                if (c == '=') {
                	switch (sTokenString[0]) {
                    	case '>':
                            token = GEQ;
                            break;
                    	case '<':
                            token = LEQ;
                            break;
                        case '=':
                            token = EQ;
                            break;
                        case '!':
                            token = UEQ;
                            break;
                        default:
                            token = ERROR;
                	}
                } else {
                    /* backup in the input */
                    unget_next_char();
                    save = false;
                    switch (sTokenString[0]) {
                    	case '>':
                            token = GT;
                            break;
                    	case '<':
                            token = LT;
                            break;
                        case '=':
                            token = ASSIGN;
                            break;
                        default:
                            token = ERROR;
                    }
                }
                break;
            case INNUM:
                if (!isdigit(c) && c != '.') {
                    /* backup in the input */
                    unget_next_char();
                    save = false;
                    state = DONE;
                    token = CONSTNUM;
                } else if (c == '.') {
                	state = INFLOAT;
                }
                break;
            case INFLOAT:
            	if (!isdigit(c)) {
                    /* backup in the input */
                    unget_next_char();
                    save = false;
                    state = DONE;
                    token = CONSTNUM;
                }
            	break;
            case INID:
				if (!is_reserv_with_blank(sTokenString) && (c == ' ')) {
					continue;
                } else if (!isalnum(c) && c != '_') {
                    /* backup in the input */
                    if (c == '(')
                    	token = FUNCTION;
                    else
                    	token = ID;
                    unget_next_char();
                    save = false;
                    state = DONE;
                }
                break;
			case SPLITID:
				if (!isdigit(c)) {
                    /* backup in the input */
                    unget_next_char();
                    save = false;
                    state = DONE;
                    token = SPLITVAR;
                }
            	break;
			case INCONSTSTR:
            	if (c == '\"') {
            		save = false;
                    state = DONE;
                    token = CONSTSTRING;
            	}
            	break;
            case DONE:
            default:
                /* should never happen */
                cerr << "Scanner Bug: state= %d\n" << endl;
                state = DONE;
                token = ERROR;
				break;
        }
        if (save && (state != INCONSTSTR || (state == INCONSTSTR && c != '\"'))) {
				sTokenString.append(1, c);
        }
        if (state == DONE) {
            if (token == ID) {
                token = get_string_token(sTokenString);
            } else if (token == FUNCTION) {
                token = get_func_token(sTokenString);
            }
        }
    }
    cout << sTokenString << endl;
    if (ENDSTR == token)
    	cout << endl;
}

/* begin parse */
bool CSmallCParse::match(TokenType expected) {
	if (token == expected) {
		next_token();
		return false;
	} else {
		cout << "match unexpected token ->" << TokenTypeMap[token]<< ", " << TokenTypeMap[expected] << ", " << sTokenString << endl;
		return true;
	}
}

bool CSmallCParse::function(CCodeStructList *&pPrev, CCodeStructList *&pNext) {
	TokenType funcToken = (token == FUNCTION ? FUNCTION : USERFUNC);
	CCodeStruct * pFun = CFactoryCodeStruct::get_inst().create_code_struct(token, sTokenString, this->pCFormula);
	if (NULL == pFun) {
		cerr << "The function:" << sTokenString << " doesn't exists!" << endl;
		return true;
	}
		

	CCodeStructList *pFunList = new CCodeStructList(pFun);
	
	if (match(funcToken) || match(LPAREN))
		return true;
	if (RPAREN != token) {
		CCodeStructList *pPara = NULL;
		CParamList *pParamList = NULL;

		if (expr_logic(pPrev, pPara))
			return true;
		pParamList = new CParamList(new CVarParam(pPara->get_code_struct()->pResValue), NULL);
		pFun->set_param_list(pParamList);

		while (token == COMMA) {
			if (match(COMMA))
				return true;

			if (expr_logic(pPara, pPara))
				return true;

			pParamList = pParamList->pNext = new CParamList(new CVarParam(pPara->get_code_struct()->pResValue), NULL);
		}
		pPara->pNext = pFunList;
	} else {
		if (pPrev) {
			pPrev->pNext = pFunList;
		} else {
			pPrev = pFunList;
		}
	}
	if (match(RPAREN))
		return true;

	pNext = pFunList;
	return false;
}

/* 解析自定义函数 */
bool CSmallCParse::function_define() {
	CUseFunc * p;

	if (match(FUNCDEF))
		return true;
	pCUseFunc = p = pCFormula->add_user_func(sTokenString);
	if (match(USERFUNC) || match(LPAREN))
		return true;
	if (NULL != p) {
		if (token == SYSVAR) {
			p->add_func_para(sTokenString);
			if (match(SYSVAR))
				return true;
			while (token == COMMA) {
				if (match(COMMA))
					return true;
				if (token == SYSVAR) {
					p->add_func_para(sTokenString);
				}
				if (match(SYSVAR))
					return true;
			}
		}
	}
	if (match(RPAREN) == 1 || match(LBRACE) == 1)
		return true;
	if (stmt_sequence(p->pCCodeStructList) == 1)
		return true;
	if (match(RBRACE) == 1)
		return true;

	pCUseFunc = NULL;

	return false;
}

bool CSmallCParse::factor(CCodeStructList *&pPrev, CCodeStructList *&pNext) {
	CCodeStructList *p = NULL;

	switch (token) {
		case NULLVAL:
		case CONSTNUM :
		case CONSTSTRING : {
			p = new CCodeStructList(CFactoryCodeStruct::get_inst().create_code_struct(token, "", this->pCFormula));
			if (token == CONSTSTRING || NULLVAL == token) {
				p->get_code_struct()->pResValue = new CVarStruct("tmpVar", sTokenString.c_str());
			} else {
				istringstream istr(sTokenString);
				float fValue;
				istr >> fValue;
				p->get_code_struct()->pResValue = new CVarStruct("tmpVar", fValue);
			}

			if (pPrev) {
				pPrev->pNext = p;
			} else {
				pPrev = p;
			}
			if (match(token))
				return true;
			break;
		}
		case SPLITVAR :
			p = new CCodeStructList(CFactoryCodeStruct::get_inst().create_code_struct(token, "", this->pCFormula));
			//p->get_code_struct()->pResValue = new CVarStruct(sTokenString, (char *) NULL);
			p->get_code_struct()->pResValue = &((pCFormula->get_split_var())[atoi(sTokenString.c_str() + 1)]);

			if (pPrev) {
				pPrev->pNext = p;
			} else {
				pPrev = p;
			}
			if (match(token))
				return true;
			break;
		case SYSVAR :
		case INPUTVAR :
		case OUTPUTVAR : {
			p = new CCodeStructList(CFactoryCodeStruct::get_inst().create_code_struct(token, sTokenString, this->pCFormula));

			if (NULL == this->pCUseFunc)//当前语句不是自定义函数
				p->get_code_struct()->pResValue = pCFormula->get_sysvar(sTokenString, sTokenString, 1);
			else {// 当前为自定义函数
				if (NULL == (p->get_code_struct()->pResValue = pCUseFunc->get_func_param(sTokenString)))//非自定义函数参数
					p->get_code_struct()->pResValue = pCFormula->get_sysvar(sTokenString, sTokenString, 1);
			}
			if (pPrev)
				pPrev->pNext = p;
			else
				pPrev = p;
			
			if (match(token))
				return true;
			break;
		}
	 	case FUNCTION : 
		case USERFUNC :{
		 	if (function(pPrev, p))
				return true;
			break;
		}
		case LPAREN : {
			if (match(LPAREN))
				return true;
			if (expr_logic(pPrev, p))
				return true;
			if (match(RPAREN))
				return true;
			break;
		}
		case MINUS : {
			if (match(MINUS))
				return true;
			
			p = new CCodeStructList(CFactoryCodeStruct::get_inst().create_code_struct(token, "", this->pCFormula));
			if (token != CONSTNUM) {
				return true;
			}
			istringstream istr(sTokenString);
			float fValue;
			istr >> fValue;
			p->get_code_struct()->pResValue =new CVarStruct("tmpVar", 0 - fValue);

			if (pPrev) 
				pPrev->pNext = p;
			else
				pPrev = p;
			
			if (match(token))
				return true;
			break;
		}
		default: {
			cout << "factor unexpected token -> " << TokenTypeMap[token] << ", " << sTokenString << endl;
			next_token();
			return true;
			break;
		}
	}
	pNext = p;
	return false;
}

bool CSmallCParse::term(CCodeStructList *&pPrev, CCodeStructList *&pNext) {
	CCodeStructList *pPara1 = NULL;
	
	if (factor(pPrev, pPara1))
		return true;
	while ((token == MULTIPLY) || (token == DIVIDE)) {
		CCodeStruct *pCal = CFactoryCodeStruct::get_inst().create_code_struct(token, sTokenString, this->pCFormula);
		CCodeStructList *pPara2;
		CCodeStructList *pCalList = new CCodeStructList(pCal);

		if (match(token))
			return true;
		if (factor(pPara1, pPara2))
			return true;

		pCal->set_param_list(new CParamList(new CVarParam(pPara1->get_code_struct()->pResValue),NULL));
		pCal->pCParamList->pNext = new CParamList(new CVarParam(pPara2->get_code_struct()->pResValue),NULL);

		pPara2->pNext = pCalList;
		pPara1 = pCalList;
	}
	pNext = pPara1;
	return false;
}

bool CSmallCParse::simple_exp(CCodeStructList *&pPrev, CCodeStructList *&pNext) {
	CCodeStructList *pPara1 = NULL;
	
	if (term(pPrev, pPara1))
		return true;
	while ((token == PLUS) || (token == MINUS)) {
		CCodeStruct *pCal = CFactoryCodeStruct::get_inst().create_code_struct(token, sTokenString, this->pCFormula);
		CCodeStructList *pPara2;
		CCodeStructList *pCalList = new CCodeStructList(pCal);

		if (match(token))
			return true;
		if (term(pPara1, pPara2))
			return true;

		pCal->set_param_list(new CParamList(new CVarParam(pPara1->get_code_struct()->pResValue),NULL));
		pCal->pCParamList->pNext = new CParamList(new CVarParam(pPara2->get_code_struct()->pResValue),NULL);

		pPara2->pNext = pCalList;
		pPara1 = pCalList;
	}
	pNext = pPara1;
	return false;
}

bool CSmallCParse::expr_cmp(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	CCodeStructList *pCmp, *pPara1, *pPara2;
	pCmp = pPara1 = pPara2 = NULL;

	if (simple_exp(pPrev, pPara1))
		return true;
	if ((token == EQ) || (token == LT) || (token == GT) || (token == GEQ) || (token == LEQ) || (token == UEQ)) {
		CCodeStruct *pCs = CFactoryCodeStruct::get_inst().create_code_struct(token, sTokenString, this->pCFormula);
		pCmp = new CCodeStructList(pCs);

		if (match(token))
			return true;
		if (simple_exp(pPara1, pPara2))
			return true;

		pCs->set_param_list(new CParamList(new CVarParam(pPara1->get_code_struct()->pResValue),NULL));
		pCs->pCParamList->pNext = new CParamList(new CVarParam(pPara2->get_code_struct()->pResValue),NULL);

		pPara2->pNext = pCmp;
	} else {
		pCmp = pPara1;
	}
	pNext = pCmp;

	return false;
}

bool CSmallCParse::expr_logic(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	CCodeStructList *pPara1 = NULL;

	if (expr_cmp(pPrev, pPara1))
		return true;
	while (AND == token || OR == token) {
		CCodeStruct *pCsLogic = CFactoryCodeStruct::get_inst().create_code_struct(token, sTokenString, this->pCFormula);
		
		CCodeStructList *pCSList = new CCodeStructList(pCsLogic);
		CCodeStructList *p1 = NULL, *p2 = NULL;
        CParamList *pParamList = NULL;

		pPara1->set_next(pCSList);

				
		match(token);
		CVarStruct *pCVarStruct = pPara1->get_code_struct()->pResValue;
		pCsLogic->set_param_list(new CParamList(new CVarParam(pCVarStruct), NULL));
		pParamList = pCsLogic->pCParamList;
			
		if (expr_cmp(p1, p2))
			return true;
		pParamList->pNext = new CParamList(p1, NULL);
		
		pPara1 = pCSList;		
	}
	pNext = pPara1;
	return false;
}

bool CSmallCParse::assign_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	CCodeStructList* pExpList = NULL;
	CCodeStruct* pAss = CFactoryCodeStruct::get_inst().create_code_struct(ASSIGN, "=", this->pCFormula);
	CCodeStructList* pAssList = new CCodeStructList(pAss);

	pAss->pResValue = this->pCFormula->get_sysvar(sTokenString, sTokenString, 1);
	if (match(token) || match(ASSIGN))
		return true;
	if (expr_logic(pPrev, pExpList))
		return true;

	pAss->set_param_list(new CParamList(new CVarParam(pExpList->get_code_struct()->pResValue), NULL));
	pExpList->pNext = pAssList;
		
	pNext = pAssList;
	return false;
}

bool CSmallCParse::return_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	CCodeStructList *pExp = NULL;
	CCodeStruct *pReturn = NULL;

	pReturn = new CCodeStruct_Return();

	if (match(RETURN))
		return true;

	if (SEMI != token && NULL != pCUseFunc) {/* 仅当前处理的语句属于函数时允许有返回值 */
		if (simple_exp(pPrev, pExp))
			return true;

		pReturn->set_param_list(new CParamList(new CVarParam(pExp->get_code_struct()->pResValue), NULL));
	}

	CCodeStructList *pRetList = new CCodeStructList(pReturn);
	pExp->pNext = pRetList;
//	pRetList->pPrev = pExp;

	pNext = pRetList;
	return false;
}


bool CSmallCParse::while_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	CCodeStruct *pCsWhile = CFactoryCodeStruct::get_inst().create_code_struct(WHILE, "while", this->pCFormula);
	CCodeStructList *pCSList = new CCodeStructList(pCsWhile);
	CParamList *pParamList = NULL;

	if (match(WHILE) || match(LPAREN))
		return true;
	if (pCsWhile) {
		CCodeStructList* p1 = NULL, * p2 = NULL;
		if (expr_logic(p1, p2))
			return true;
		pCsWhile->set_param_list(new CParamList(p1, NULL));
		pParamList = pCsWhile->pCParamList;
		
		if (match(RPAREN) || match(LBRACE))
			return true;
		
		p1 = NULL;
		if (stmt_sequence(p1))
			return true;
		pParamList->pNext = new CParamList(p1, NULL);
		
		if (match(RBRACE))
			return true;
	}

	if (pPrev) {
		pPrev->pNext = pCSList;
	} else {
		pPrev = pCSList;
	}
	pNext = pCSList;

	return false;
}

bool CSmallCParse::if_stmt(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	CCodeStruct *pCsIf = CFactoryCodeStruct::get_inst().create_code_struct(IF, "if", this->pCFormula);
	CCodeStructList *pCSList = new CCodeStructList(pCsIf);
	
	CParamList *pParamList = NULL;

	if (match(IF) || match(LPAREN))
		return true;
	if (pCsIf) {
		CCodeStructList *p1 = NULL, *p2 = NULL;
		
		/* 条件表达式 */
		if (expr_logic(p1, p2))
			return true;
		pCsIf->set_param_list(new CParamList(p1, NULL));
		pParamList = pCsIf->pCParamList;

		if (match(RPAREN) || match(LBRACE))
			return true;

		/* 满足条件时执行的子句 */
		p1 = NULL;
		if (stmt_sequence(p1))
			return true;
		pParamList->pNext = new CParamList(p1, NULL);
		pParamList = pParamList->pNext;

		if (match(RBRACE))
			return true;
	}
	
	while (token == ELSEIF) {/* 循环匹配 elseif 子句 */
		if (match(ELSEIF) || match(LPAREN))
			return true;
		CCodeStructList *p1 = NULL, *p2 = NULL;
		
		/* 条件表达式 */
		if (expr_logic(p1, p2))
			return true;
		pParamList->pNext = new CParamList(p1, NULL);
		pParamList = pParamList->pNext;

		if (match(RPAREN) || match(LBRACE))
			return true;

		/* 满足条件时执行的子句 */
		p1 = NULL;
		if (stmt_sequence(p1))
			return true;
		pParamList->pNext = new CParamList(p1, NULL);
		pParamList = pParamList->pNext;
		if (match(RBRACE))
			return true;
	}

	if (token == ELSE) {/* 匹配 else 语句 */
		if (match(ELSE) || match(LBRACE))
			return true;
		if (pCsIf) {
			CCodeStructList *p1 = NULL;
			
			if (stmt_sequence(p1))
				return true;
			pParamList->pNext = new CParamList(p1, NULL);
		}
		if (match(RBRACE))
			return true;
	}

	if (pPrev) {
		pPrev->pNext = pCSList;
	} else {
		pPrev = pCSList;
	}
	pNext = pCSList;

	return false;
}

bool CSmallCParse::statement(CCodeStructList*& pPrev, CCodeStructList*& pNext) {
	switch (token) {
		case IF :
			if (if_stmt(pPrev, pNext))
				return true;
			break;
		case WHILE :
			if (while_stmt(pPrev, pNext))
				return true;
			break;
		case SYSVAR :
		case OUTPUTVAR:
			if (assign_stmt(pPrev, pNext))
				return true;
			if (match(SEMI))
				return true;
			break;
		case FUNCTION :
		case USERFUNC :
			if (function(pPrev, pNext))
				return true;
			if (match(SEMI))
				return true;
			break;
		case FUNCDEF :
			if (function_define())
				return true;
			if (statement(pPrev, pNext))
				return true;
			break;
		case RETURN :
			if (return_stmt(pPrev, pNext))
				return true;
			if (match(SEMI))
				return true;
			break;
		default :
			cerr << "statement unexpected token -> " << TokenTypeMap[token] << sTokenString << endl;
			next_token();
			return true;
	} /* end case */
	return false;
}

bool CSmallCParse::stmt_sequence(CCodeStructList*& pPrev) {
	CCodeStructList *pCur = NULL;
	if (statement(pPrev, pCur))
		return true;

	while ((token != ENDSTR) && (token != RBRACE)) {
		if (statement(pCur, pCur))
			return true;
	}
	return false;
}


CFormula * CSmallCParse::parse() {
	next_token();
	
	CCodeStructList *p1 = NULL;
	if (stmt_sequence(p1))
		return NULL;
	if (NULL != p1) {
		this->pCFormula->set_c_s_list(p1);
		this->pCFormula->format_formula();
	}
	if (token != ENDSTR) {
		cerr << "Code ends before file" << endl;
		return NULL;
	}

	return this->pCFormula;
}
