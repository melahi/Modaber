/*
 * SimpleModaber.cpp
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#include "LiftedModaber.h"


#include "VALfiles/TIM.h"
#include "ProblemPrinter.h"
#include "Translator.h"
#include <iostream>
#include <fstream>
#include "CVC4Problem.h"
#include "SketchyPlan.h"

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

using namespace mdbr;

void LiftedModaber::initialization (char *domainFilePath, char *problemFilePath, bool usingPlanningGraph){
	Modaber::initialization(domainFilePath, problemFilePath, usingPlanningGraph, false);

	if (usingPlanningGraph){
		nPG->constructingGraph();
	}
	myProblem.liftedInitializing();

	nSignificantTimePoints = 1;

}

bool LiftedModaber::tryToSolve(){

	bool foundSolution = false;
	bool updatingValues;

	while (!foundSolution){
		updatingValues = false;
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		cout << "solving ..." << endl;
		translator.prepare(myProblem.operators.size(), myProblem.nPropositionVariables, myProblem.nUnification, myProblem.partialAction.size(), myProblem.assignments.size(), myProblem.comparisons.size(), myProblem.nValues);
		foundSolution = translator.solve(nSignificantTimePoints);
		cout << "end solving" << endl;
		if (!foundSolution){
			nSignificantTimePoints += 1;
		}else{
			vector <pair <operator_ *, FastEnvironment> > solution;
			translator.getSolution(solution);
			foundSolution =	simulator.isValid(solution);

			if (!foundSolution){
				translator.printSolution(cout);
				cout << endl << endl << "------------------" << endl << "A plan has been found but is not valid!!!" << endl << endl;
				updatingValues = true;
			}
		}


		if (updatingValues){
			int nVariables = myProblem.variables.size();
			for (int i = 0; i < nVariables; ++i){
				myProblem.variables[i].completeDomainRange();
				myProblem.variables[i].write(cout);
				cout << endl;
			}
			myProblem.assignIdToValues();

			list <MyComparison>::iterator cmpIt, cmpItEnd;
			cmpIt = myProblem.comparisons.begin();
			cmpItEnd = myProblem.comparisons.end();

			for (; cmpIt != cmpItEnd; ++cmpIt){
//				cout << "=====" << cmpIt->possibleValues.size();
				cmpIt->findPossibleRanges();
//				cmpIt->write(cout);
//				cout << "=====" << cmpIt->possibleValues.size() << endl;

			}
			cmpIt = myProblem.goalComparisons.begin();
			cmpItEnd = myProblem.goalComparisons.end();
			for (; cmpIt != cmpItEnd; ++cmpIt){
//				cout << "=====" << cmpIt->possibleValues.size();
				cmpIt->findPossibleRanges();
//				cmpIt->write(cout);
//				cout << "=====" << cmpIt->possibleValues.size() << endl;

			}

			list <MyAssignment>::iterator asgnIt, asgnItEnd;
			asgnIt = myProblem.assignments.begin();
			asgnItEnd = myProblem.assignments.end();

			for (; asgnIt != asgnItEnd; ++asgnIt){
//				cout << "*****" << asgnIt->possibleValues.size();
				asgnIt->findPossibleRanges();
//				asgnIt->write(cout);
//				cout << "*****" << asgnIt->possibleValues.size() << endl;
			}

		}
	}
	return foundSolution;
}



LiftedModaber::LiftedModaber(char *domainFilePath, char *problemFilePath, bool usingPlanningGraph) {
	initialization(domainFilePath, problemFilePath, usingPlanningGraph);
	bool foundSolution;
	foundSolution = tryToSolve();
	if (foundSolution){
		cout << "The plan is: " << endl;
		translator.printSolution(cout);
		cout << endl << endl;
		cout << "Modaber finished his task!!! ;)" << endl;
		cout << "*******************************" << endl;
		ofstream fout ("solution");
		translator.printSolution(fout);
	}
}

LiftedModaber::~LiftedModaber() {
}

