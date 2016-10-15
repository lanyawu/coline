#include "formula_group.h"

bool CFormulaGroup::build_formula(int id, string oriStr, int (* load_var)(CVarStruct *pCVarStruct)) {
	CFormula *pFm = new CFormula(id, oriStr, load_var);
	CSmallCParse *parse = new CSmallCParse(pFm);
	parse->parse();
	erase(id);
	groups.push_back(pFm);

	delete parse;
	return true;
}

bool CFormulaGroup::show_formula(int id) {
	CFormula *pFm = get_formula(id);
	if (NULL != pFm) {
		pFm->show_formula();
		return true;
	}
	return false;				
}
bool CFormulaGroup::exec_formula(int id) {
	CFormula *pFm = get_formula(id);
	if (NULL != pFm) {
		pFm->process();
		return true;
	}
	return false;	
}

CFormula * CFormulaGroup::get_formula(int id) {
	vector<CFormula *>::iterator it;
	for (it = groups.begin(); it != groups.end(); it++) {
		CFormula *pFm = *it;
		if (pFm->get_id() == id) {
			return pFm;
		}
	}
	return NULL;
}

void CFormulaGroup::erase(int id) {
	vector<CFormula *>::iterator it;
	for (it = groups.begin(); it != groups.end(); it++) {
		CFormula *pFm = *it;
		if (pFm->get_id() == id) {
			groups.erase(it);
		}
	}
}
