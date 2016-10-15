#include <math.h>
#include "ext_code_struct.h"

#pragma warning(disable:4996)

/** if */
CVarStruct * CCodeStruct_IF::proc_code_stuct(bool& bReturnFlag) {
	CParamList *pParamList = this->pCParamList;
	CVarStruct *pVarStruct = pParamList->get_param()->deal_param();

	if ((long) atof(pVarStruct->get_value())) {
		pParamList->pNext->get_param()->deal_param();
	} else {
		CParamList *p = pParamList->pNext->pNext;
		while (NULL != p) {
			if (NULL != p->pNext) {/* 存在 else if 子句 */
				CVarStruct *pVar = p->get_param()->deal_param();
				if ((long) atof(pVar->get_value())) {
					p->pNext->get_param()->deal_param();
					break;
				} else if (p->pNext->pNext) {
					p = p->pNext->pNext;
				}
			} else {
				p->get_param()->deal_param();
				break;
			}
		}
	}
	return NULL;
}

/** while */
CVarStruct * CCodeStruct_WHILE::proc_code_stuct(bool& bReturnFlag) {
	CParamList *pParamList = this->pCParamList;

	while ((long) atof(pParamList->get_param()->deal_param()->get_value()) != 0) {
		pParamList->pNext->get_param()->deal_param();
	}
	return NULL;
}

/** user_def_function */
CVarStruct * CCodeStruct_UFunc::proc_code_stuct(bool& bReturnFlag) {
	*pResValue = *(pCUseFunc->process(this));
	return this->pResValue;
}

/** 附值语句 */
CVarStruct * CCodeStruct_Asign::proc_code_stuct(bool& bReturnFlag) {
	CVarStruct *pVarStruct = this->pCParamList->get_param()->deal_param();

	this->pResValue->copy(*pVarStruct);

	return this->pResValue;
}

/* 数值计算的codestruct类 */
CVarStruct * CCodeStruct_Cal::proc_code_stuct(bool& bReturnFlag) {
	CVarStruct *pPara1 = pCParamList->get_param()->deal_param();
	CVarStruct *pPara2 = pCParamList->pNext->get_param()->deal_param();
	char *str1, *str2;
	double f_num1, f_num2;

	if (StringK == pPara1->get_var_kind())
		str1 = pPara1->get_value();
	else
		f_num1 = atof(pPara1->get_value());
	if (StringK == pPara2->get_var_kind())
		str2 = pPara2->get_value();
	else
		f_num2 = atof(pPara2->get_value());
	switch (this->op) {
		case PLUS:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {//字符串相加
				if (NULL != pResValue->get_value())
					delete pResValue->get_value();
				char * sStr = new char[strlen(str1) + strlen(str2) + 1];
				sprintf(sStr, "%s%s", str1, str2);
				pResValue->set_value(sStr);
			} else if ((StringK == pPara1->get_var_kind() && FloatK == pPara2->get_var_kind())) {
				pResValue->set_value(str1 + (int) ceil(f_num2));
			} else if ((StringK == pPara2->get_var_kind() && FloatK == pPara1->get_var_kind())) {
				pResValue->set_value(str2 + (int) ceil(f_num1));
			} else {
				pResValue->set_value(f_num1 + f_num2);
			}
			break;
		case MINUS:
			pResValue->set_value(f_num1 - f_num2);
			break;
		case MULTIPLY:
			pResValue->set_value(f_num1 * f_num2);
			break;
		case DIVIDE:
			if (fabs(f_num2) < 0.000001)
				exit(1);
			else
				pResValue->set_value(f_num1 / f_num2);
			break;
	}
	return pResValue;
}


/* 比较计算的codestruct类 */
CVarStruct * CCodeStruct_Cmp::proc_code_stuct(bool& bReturnFlag) {
	CVarStruct *pPara1 = pCParamList->get_param()->deal_param();
	CVarStruct *pPara2 = pCParamList->pNext->get_param()->deal_param();

	char *str1, *str2;
	double f_num1, f_num2;

	if (StringK == pPara1->get_var_kind())
		str1 = pPara1->get_value();
	else
		f_num1 = atof(pPara1->get_value());
	if (StringK == pPara2->get_var_kind())
		str2 = pPara2->get_value();
	else
		f_num2 = atof(pPara2->get_value());

	switch (this->op) {
		case EQ:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {
				pResValue->set_value((strcmp(str1, str2) == 0) ? 1 : 0);
			} else {
				pResValue->set_value(fabs(f_num1 - f_num2) < 0.000001 ? 1 : 0);
			}
			break;
		case LT:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {
				pResValue->set_value(strcmp(str1, str2) < 0 ? 1 : 0);
			} else {
				pResValue->set_value(f_num1 <= f_num2 + 000001 ? 1 : 0);
			}
			break;
		case GT:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {
				pResValue->set_value(strcmp(str1, str2) > 0 ? 1 : 0);
			} else {
				pResValue->set_value(f_num1 >= f_num2 + 000001 ? 1 : 0);
			}
			break;
		case LEQ:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {
				pResValue->set_value(strcmp(str1, str2) <= 0 ? 1 : 0);
			} else {
				pResValue->set_value(f_num1 < f_num2 + 000001 ? 1 : 0);
			}
			break;
		case GEQ:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {
				pResValue->set_value(strcmp(str1, str2) >= 0 ? 1 : 0);
			} else {
				pResValue->set_value(f_num1 > f_num2 + 000001 ? 1 : 0);
			}
			break;
		case UEQ:
			if ((StringK == pPara1->get_var_kind() && StringK == pPara2->get_var_kind())) {
				pResValue->set_value(strcmp(str1, str2) != 0 ? 1 : 0);
			} else {
				pResValue->set_value(fabs(f_num1 - f_num2) > 0.000001 ? 1 : 0);
			}
			break;
	}
	return pResValue;
}

/* and， or 等的逻辑处理*/
CVarStruct * CCodeStruct_Logic::proc_code_stuct(bool& bReturnFlag) {
	CVarStruct *pPara1 = pCParamList->get_param()->deal_param();
	CVarStruct *pPara2 = pCParamList->pNext->get_param()->deal_param();

	int i_num1, i_num2;

	i_num1 = (int) ceil(atof(pPara1->get_value()));
	i_num2 = (int) ceil(atof(pPara2->get_value()));

	switch (this->op) {
		case AND:
			if (0 == i_num1)
				pResValue->set_value(0.00);
			else {
				pResValue->set_value((i_num2 == 0) ? 0.00 : 1.00);
			}
			break;
		case OR:
			if (1 == i_num1)
				pResValue->set_value(1.00);
			else {
				pResValue->set_value((i_num2 == 1) ? 1.00 : 0.00);
			}
			break;
	}
	return pResValue;
}

#pragma warning(default:4996)
