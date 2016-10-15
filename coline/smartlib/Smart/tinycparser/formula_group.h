#include <vector>
#include "formula.h"
#include "parse.h"
#include "formula_public.h"
#include "singleton.h"

using namespace std;

class CFormulaGroup : public Singleton<CFormulaGroup> {
private:
	friend class Singleton<CFormulaGroup>;
	vector<CFormula *> groups;

	CFormula *get_formula(int id);

	void erase(int id);
protected:
	CFormulaGroup() {}
public:

	bool build_formula(int id, string oriStr, int (* load_var)(CVarStruct *pCVarStruct));

	bool show_formula(int id);
	bool exec_formula(int id);
};
