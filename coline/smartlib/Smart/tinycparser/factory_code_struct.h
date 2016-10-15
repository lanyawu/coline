/****************************************************/
/* File: factory_code_struct.h                      */
/* author: zhewei.fan                               */
/* CCodeStruct工厂类，单例模式使用                  */
/* 根据 token, tokenname 创建对应的 CCodeStruct     */
/****************************************************/
#ifndef _FACTORY_CODE_STRUCT_H_
#define _FACTORY_CODE_STRUCT_H_

#include <string>
#include "singleton.h"
#include "formula_public.h"
#include "ext_code_struct.h"
#include "sys_func_code_struct.h"

using namespace std;

class CFactoryCodeStruct : public Singleton<CFactoryCodeStruct> {
protected:
	CFactoryCodeStruct() {}
private:
	friend class Singleton<CFactoryCodeStruct>;
	CCodeStruct * create_sys_func(string sFuncName, CFormula *pCFormula) {
		if (0 == sFuncName.compare("atof")) {
			return new CCodeStruct_Atof();
		} else if (0 == sFuncName.compare("ceil")) {
			return new CCodeStruct_Ceil();
		}  else if (0 == sFuncName.compare("floor")) {
			return new CCodeStruct_Floor();
		}  else if (0 == sFuncName.compare("abs")) {
			return new CCodeStruct_Abs();
		}  else if (0 == sFuncName.compare("index")) {
			return new CCodeStruct_Index();
		}  else if (0 == sFuncName.compare("substr")) {
			return new CCodeStruct_Substr();
		}  else if (0 == sFuncName.compare("strlen")) {
			return new CCodeStruct_Strlen();
		}  else if (0 == sFuncName.compare("trim")) {
			return new CCodeStruct_Trim();
		}  else if (0 == sFuncName.compare("replace")) {
			return new CCodeStruct_Replace();
		}  else if (0 == sFuncName.compare("split")) {
			return new CCodeStruct_Split(pCFormula);
		}  else if (0 == sFuncName.compare("printf")) {
			return new CCodeStruce_Printf();
		}

		return NULL;
	}
public:
	CCodeStruct * create_code_struct(TokenType token, string sTokenName, CFormula *pCFormula) {
		CCodeStruct *p = NULL;
		switch (token) {
			case WHILE:
				p = new CCodeStruct_WHILE();
				break;
			case IF:
				p = new CCodeStruct_IF();
				break;
			case ASSIGN:
				p = new CCodeStruct_Asign();
				break;
			case AND:
			case OR:
				p = new CCodeStruct_Logic(token);
				break;
			case EQ:
			case LT:
			case GT:
			case GEQ:
			case LEQ:
			case UEQ:
				p = new CCodeStruct_Cmp(token);
				break;
			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
				p = new CCodeStruct_Cal(token);
				break;
			case CONSTNUM:
			case CONSTSTRING:
			case INPUTVAR:
			case NULLVAL:
			case OUTPUTVAR:
			case SPLITVAR:
			case SYSVAR:
				p = new CCodeStruct_Var();
				break;
			case FUNCTION:
				p = create_sys_func(sTokenName, pCFormula);
				break;
			case USERFUNC: 
				{
					CUseFunc *pCUseFunc = pCFormula->get_user_func(sTokenName);
					if (NULL != pCUseFunc)
						p = new CCodeStruct_UFunc(pCUseFunc);
				}
				break;
			default:
				p = NULL;
				break;
		}
		return p;
	}
};

#endif
