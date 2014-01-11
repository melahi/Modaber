/*
 * MyProblem.h
 *
 *  Created on: Aug 6, 2013
 *      Author: sadra
 */

#ifndef MYPROBLEM_H_
#define MYPROBLEM_H_


#include "MyAction.h"
#include "MyAtom.h"
#include "MyObject.h"
#include "MyOperator.h"
#include "MyPartialAction.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"

#include <map>
#include <vector>


using namespace std;
using namespace VAL;
using namespace Inst;


namespace mdbr {

class MyProblem {
private:

	void assignIdToPropositions();
	void assignIdToVariables();

public:

	vector <vector <MyAction> >actions;
	vector <MyProposition> propositions;
	vector <MyVariable> variables;

	vector <MyOperator *> operators;
	list <MyPartialAction> partialAction;


	int nPropositionIDs;  //The number of variables needed for propositions for each layer in SAT formula
	int nPartialActions;
	int nVariableIDs;

	vector <double> initialValue;  //The initial value for each variable


	MyProblem(): nPropositionIDs(0), nPartialActions(0){}


	void updateInitialLayer ();


	void initializing();

	void liftedInitializing ();

	void write(ostream &sout);

	void writeType (ostream &sout, MyType * type, int indent);

	virtual ~MyProblem() {
		int nOperators = operators.size();
		for (int i = 0; i < nOperators; i++){
			delete(operators[i]);
		}
	}
};

extern MyProblem myProblem;

} /* namespace mdbr */
#endif /* MYPROBLEM_H_ */
