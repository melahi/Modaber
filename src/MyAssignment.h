/*
 * MyAssignment.h
 *
 *  Created on: Oct 13, 2013
 *      Author: sadra
 */

#ifndef MYASSIGNMENT_H_
#define MYASSIGNMENT_H_

#include "MyOperator.h"
#include "MyObject.h"
#include "MyAtom.h"
#include "MyComparison.h"

#include <map>
#include <string>

#include <VALfiles/parsing/ptree.h>



using namespace std;
using namespace VAL;

namespace mdbr {

class MyOperator;
class MyComparison;

class MyLiftedAssignment {
private:
	bool grounded;
	map <string, MyObject *> selectedObject;
	map <string, int> objectId;

public:
	map <string, MyType *>  types;
	map <string, int> placement;
	assignment *originalAssignment;

	MyOperator *op;

	void prepare (MyOperator *op_, assignment *originalAssignment_);

	void findTypes (const expression *exp);

	void grounding();

	void grounding (map <string, MyType *>::iterator it);

	MyLiftedAssignment() {}
	virtual ~MyLiftedAssignment() {}
};

class MyAssignment {

private:
	list <MyVariable *> myCreatedVariables;

	map <const func_term *, MyValue *> selectedValues;

	bool isMutex (MyAssignment *other);
	bool isMutex (MyComparison *other);

	bool aVariableNotFounded;

public:
	map <const func_term *, MyVariable *> variables;
	list < pair < list < MyValue* >, MyValue*> > possibleValues;


	map <string, MyObject *> selectedObject;
	map <string, int> objectId;

	MyOperator *op;
	MyLiftedAssignment *liftedAssignment;

	int assignmentId;



	list <MyAssignment *> assignmentMutex;
	list <MyComparison *> comparisonMutex;

	MyAssignment();
	void prepare (MyOperator *op_, MyLiftedAssignment *liftedAssignment_, map <string, MyObject *> &selectedObject_, map <string, int> &objectId_, int assignmentId_);

	void findVariables (const expression *exp);

	void findPossibleValues ();
	void findPossibleValues ( map <const func_term *, MyVariable *>::iterator it);

	MyValue *evalute ();

	double evalute (const expression *exp);

	void findAllMutexes();

	void write (ostream &sout);

	virtual ~MyAssignment(){
		list <MyVariable *>::iterator it, itEnd;
		it = myCreatedVariables.begin();
		itEnd = myCreatedVariables.end();
		for (; it != itEnd; ++itEnd){
			delete(*it);
		}
	}
};

} /* namespace mdbr */
#endif /* MYASSIGNMENT_H_ */