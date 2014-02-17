/*
 * EStepModaber.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: sadra
 */

#include "EStepModaber.h"
#include "VALfiles/TIM.h"
#include "UnrelatedFilter.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace VAL;
using namespace TIM;
using namespace Inst;

using namespace mdbr;


void EStepModaber::createMetricFunction(){
	comparison_op cmpOp;
	if (!current_analysis->the_problem->metric){
		return;
	}
	if (current_analysis->the_problem->metric->opt == E_MINIMIZE){
		cmpOp = E_LESS;
	}else{
		cmpOp = E_GREATER;
	}

	metricBound = new float_expression(0);


	comparison *metricComparison = new comparison(cmpOp, current_analysis->the_problem->metric->expr, metricBound);
	valOperator = new operator_ ();
	valOperator->precondition = metricComparison;
	env = new FastEnvironment(0);
	valAction = new instantiatedOp (valOperator, env);
	metricFunction.initialize(valAction, -10);
}

void EStepModaber::initialization (char *domainFilePath, char *problemFilePath){
	Modaber::initialization(domainFilePath, problemFilePath);

	UnrelatedFilter filterAllUnrelateActionsAndProposition;

	myProblem.assignIdToPropositions_EStep();

	createMetricFunction();

	planGraph = new PlanningGraph();

	nSignificantTimePoints = 1;

}

bool EStepModaber::tryToSolve(double bound){

	bool foundSolution = false;

	if (bound != infinite && bound != -infinite){
		metricBound->val = bound;
		myProblem.reconsiderValues_EStep();
		metricFunction.constructNumericalCondition();
	}

	while (!foundSolution){
		cout  << "nSignificantTimePoint: " << nSignificantTimePoints << endl;
		planGraph->constructingGraph(nSignificantTimePoints);
		cout << "Translating ..." << endl;
		delete (translator);
		translator = new EStepTranslator();
		translator->prepare(nSignificantTimePoints, &metricFunction);
		cout << "solving ..." << endl;
		foundSolution = translator->solve();
		cout << "end solving" << endl;
		if (!foundSolution){
			nSignificantTimePoints += 5;
		}else{
			vector <pair <operator_ *, FastEnvironment> > solution;
			translator->getSolution(solution);
			foundSolution =	simulator.isValid(solution);

			if (foundSolution && (current_analysis->the_problem->metric) && ((current_analysis->the_problem->metric->opt == E_MINIMIZE && simulator.metricValue >= bound) || (current_analysis->the_problem->metric->opt == E_MAXIMIZE && simulator.metricValue <= bound))){
				cout << simulator.metricValue << ' ' << bound << ' ' << current_analysis->the_problem->metric->opt << endl;
				CANT_HANDLE("INCREMENTAL IMPORVEMING, CAN'T IMPROVE THE METRIC FUNCTION!!!");
				foundSolution = false;
			}


			if (!foundSolution){
				translator->extractSolution(cout);
				cout << endl << endl << "------------------" << endl << "A plan has been found but is not valid!!!" << endl << endl;
				myProblem.reconsiderValues_EStep();
				if (bound != infinite && bound != -infinite){
					metricFunction.constructNumericalCondition();
				}
			}
		}
	}
	return foundSolution;
}



EStepModaber::EStepModaber(char *domainFilePath, char *problemFilePath, char *solutionFilePath): planGraph(NULL), translator(NULL), metricBound(NULL), env(NULL) {
	initialization(domainFilePath, problemFilePath);
	char outputFile [1000];
	double bound = infinite;
	bool hasMetricFunction = (current_analysis->the_problem->metric != NULL);
	if (hasMetricFunction && current_analysis->the_problem->metric->opt == E_MAXIMIZE){
		bound = -infinite;
	}
	int lastIndex = 1;
	while (true){
		tryToSolve(bound);
		cout << "The plan is: " << endl;
		translator->extractSolution(cout);
		sprintf(outputFile, "%s.%d", solutionFilePath, lastIndex++);
		ofstream fout (outputFile);
		translator->extractSolution(fout);
		if (!hasMetricFunction){
			break;
		}
		double temp = simulator.metricValue;
		cout << "Metric value is: " << temp << endl;
		if ((current_analysis->the_problem->metric->opt == E_MINIMIZE && temp >= bound) || (current_analysis->the_problem->metric->opt == E_MAXIMIZE && temp <= bound)){
			cout << temp << ' ' << bound << ' ' << current_analysis->the_problem->metric->opt << endl;
			CANT_HANDLE("INCREMENTAL IMPORVEMING, CAN'T IMPROVE THE METRIC FUNCTION!!!");
			break;
		}else{
			bound = temp;
		}
	}
}

EStepModaber::~EStepModaber() {
	delete (translator);
	delete (planGraph);
	delete (metricBound);
}
