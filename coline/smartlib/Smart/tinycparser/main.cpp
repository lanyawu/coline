#include <cstdlib>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "formula_group.h"
#include "formula_public.h"
#include "util.h"

using namespace std;

#pragma warning(disable:4996)

static int load_var(CVarStruct *pVarStruct) {
	char mystr[500];
	char *tmp;

	cout << "please input for the Value:" << pVarStruct->get_name() << endl;
	scanf("%s", mystr);
	tmp = pVarStruct->get_value();
	pVarStruct->set_value(copy_string(mystr));
	if (NULL != tmp)
		delete tmp;
	return 0;
}

int main(int argc, char * argv[])
{
	ifstream fin("F:\\lanya\\workarea\\SmartLib\\bin\\debug\\my.cfg");
	assert(fin);
	char fileInfo[4096];
	fin.read(fileInfo, 4096);
	int iLen = fin.gcount();

	cout << "read the file length:" << iLen << endl;
	fileInfo[iLen] = 0;
	
	if (fin.bad()) {
		cerr << "error in read data" << endl;
		exit(1);
	}
	
	string oriStr(fileInfo);
	CFormulaGroup::get_inst().build_formula(1, oriStr, &load_var);
	cout << "Parse is ok!" << endl;
	CFormulaGroup::get_inst().show_formula(1);
	cout << "show is ok!" << endl;
	CFormulaGroup::get_inst().exec_formula(1);
	CFormulaGroup::get_inst().show_formula(1);
	cout << "exec is ok!" << endl;

    system("PAUSE");
    return EXIT_SUCCESS;
}

#pragma warning(disable:4996)
