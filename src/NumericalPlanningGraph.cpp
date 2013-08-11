

#include "MyProblem.h"
#include "NumericalPlanningGraph.h"
#include "VALfiles/parsing/ptree.h"
#include "VALfiles/instantiation.h"


#include <fstream>
#include <string>
#include <limits>

using namespace VAL;
using namespace Inst;

using namespace std;
using namespace mdbr;


void NumericalPlanningGraph::ignoreGraph(){
	/* If this function called, Numerical Planning Graph won't constructed */
	int nActions = 0;
	for (int i = 0; i < nActions; i++){
		myProblem.actions[i].firstVisitedLayer = 0;
	}

	int nProposition = 0;
	for (int i = 0; i < nProposition; i++){
		myProblem.propositions[i].firstVisitedLayer = 0;
	}

	numberOfLayers = 0;
	levelOff = true;

}

void NumericalPlanningGraph::constructingGraph (int maxNumberOfLayer /* = numeric_limits <int>::max() */){

	while (!levelOff && numberOfLayers < maxNumberOfLayer){
		levelOff = !extendOneLayer();
	}
}

void NumericalPlanningGraph::createInitialLayer(){
	if (numberOfLayers > 0 || levelOff == true){
		//Initial layer is created already
		return;
	}

	//Literals
	pc_list<simple_effect*>::const_iterator it = current_analysis->the_problem->initial_state->add_effects.begin();
	pc_list<simple_effect*>::const_iterator itEnd = current_analysis->the_problem->initial_state->add_effects.end();
	FastEnvironment env(0);

	for (; it != itEnd; it++){
		Literal lit ((*it)->prop, &env);
		Literal *lit2 = instantiatedOp::findLiteral(&lit);
		if (lit2->getStateID() != -1){
			myProblem.propositions[lit2->getStateID()].visiting(0, NULL);
		}
	}




	pc_list<assignment*>::const_iterator it2 = current_analysis->the_problem->initial_state->assign_effects.begin();
	pc_list<assignment*>::const_iterator itEnd2 = current_analysis->the_problem->initial_state->assign_effects.end();

	for (; it2 != itEnd2; ++it2){
		PNE pne ((*it2)->getFTerm(), &env);
		PNE *pne2 = instantiatedOp::findPNE(&pne);
		if (pne2->getStateID() != -1){
			myProblem.variables[pne2->getStateID()].findValue(myProblem.initialValue[pne2->getGlobalID()], 0, NULL);
		}
	}



	numberOfLayers = 1;
	numberOfDynamicMutexesInLastLayer = 0;

}

bool NumericalPlanningGraph::extendOneLayer(){

	if (numberOfLayers < 1){
		// First layer is not created yet, so we should first create it!
		createInitialLayer();
		return true;
	}

	bool canContinue = false;

	//If there is any instance of an action which is not executed before, check its applicability, if it can be applied, then apply it!!!
	int nAction = myProblem.actions.size();

	for (int i = 0; i < nAction; i++){
		canContinue |= myProblem.actions[i].computeGroundedAction(numberOfLayers - 1);
	}

	int tempNumberOfDynamicMutex = numberOfDynamicMutexesInLastLayer;
	numberOfDynamicMutexesInLastLayer = 0;

	list <MyAtom *> allFoundedAtoms;

	// Prepare allFoundedAtoms list
	int nPropositions = myProblem.propositions.size();
	for (int i = 0; i < nPropositions; i++){
		if (myProblem.propositions[i].firstVisitedLayer != -1){
			allFoundedAtoms.push_back(&(myProblem.propositions[i]));
		}
	}

	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; i++){
		map <double, MyValue>::iterator it, itEnd;
		it = myProblem.variables[i].domain.begin();
		itEnd = myProblem.variables[i].domain.end();
		for (; it != itEnd; ++it){
			allFoundedAtoms.push_back(&(it->second));
		}
	}


	//Prepare allFoundedGroundedActions
	list <MyGroundedAction *> allFoundedGroundedActions;
	for (int i = 0; i < nAction; i++){
		set <MyGroundedAction>::iterator it, itEnd;
		it = myProblem.actions[i].groundedActions.begin();
		itEnd = myProblem.actions[i].groundedActions.end();
		for (; it != itEnd; ++it){
			MyGroundedAction *groundedAction;
			groundedAction = const_cast <MyGroundedAction *> (&(*it));
			allFoundedGroundedActions.push_back(groundedAction);
		}
	}



	//Find  no-op actions and other actions mutex for the layer
	list <MyAtom *>::iterator atomIt, atomItEnd;
	atomItEnd = allFoundedAtoms.end();

	list <MyGroundedAction *>::iterator gActionIt, gActionItEnd;
	gActionIt = allFoundedGroundedActions.begin();
	gActionItEnd = allFoundedGroundedActions.end();

	for (; gActionIt != gActionItEnd; ++gActionIt) {
		atomIt = allFoundedAtoms.begin();
		for (; atomIt != atomItEnd; ++atomIt){
			if ((*atomIt)->firstVisitedLayer >= numberOfLayers){
				continue;
			}
			if ((*gActionIt)->parentAction->isAtomStaticallyMutex(numberOfLayers - 1, *atomIt)){
				continue;
			}
			if ((*gActionIt)->checkDynamicAtomMutex(numberOfLayers - 1, *atomIt)){
				numberOfDynamicMutexesInLastLayer += 1;
				(*gActionIt)->insertAtomMutex(numberOfLayers - 1, *atomIt);
			}
		}
	}



	//Find action mutex for new layer
	gActionIt = allFoundedGroundedActions.begin();
	for (;gActionIt != gActionItEnd; ++gActionIt){
		list <MyGroundedAction *>::iterator gActionIt2;
		gActionIt2 = allFoundedGroundedActions.begin();
		for (; gActionIt2 != gActionIt; ++gActionIt2){
			if ((*gActionIt)->checkDynamicMutex(numberOfLayers - 1, *gActionIt2)){
				numberOfDynamicMutexesInLastLayer += 1;
				(*gActionIt)->insertMutex(numberOfLayers - 1, *gActionIt2);
				(*gActionIt2)->insertMutex(numberOfLayers - 1, *gActionIt);
			}
		}
	}

	//Find proposition mutex for new layer
	atomIt = allFoundedAtoms.begin();
	for (; atomIt != atomItEnd; ++atomIt){
		list <MyAtom *>::iterator atomIt2;
		atomIt2 = allFoundedAtoms.begin();
		for (; atomIt2 != atomIt; ++atomIt2){
			if ((*atomIt)->checkMutex(numberOfLayers, *atomIt2)){
				numberOfDynamicMutexesInLastLayer += 1;
				(*atomIt)->insertMutex(numberOfLayers, *atomIt2);
				(*atomIt2)->insertMutex(numberOfLayers, *atomIt);
			}
		}
	}


	numberOfLayers++;


	if (numberOfDynamicMutexesInLastLayer < tempNumberOfDynamicMutex){
		canContinue = true;
	}
	return canContinue;
}

NumericalPlanningGraph::NumericalPlanningGraph(){

	numberOfLayers = 0;
	levelOff = false;
	createInitialLayer();

}

void NumericalPlanningGraph::write(ostream &sout){


	list <MyAtom *> atoms;
	list <MyAtom *>::iterator atomIt, atomItEnd;

	list <MyGroundedAction *> actions;
	list <MyGroundedAction *>::iterator actionIt, actionItEnd;


	int nPropositions = myProblem.propositions.size();
	for (int i = 0; i < nPropositions; i++){
		if (myProblem.propositions[i].firstVisitedLayer != -1){
			atoms.push_back(&myProblem.propositions[i]);
		}
	}

	int nVariables = myProblem.variables.size();
	for (int i = 0; i < nVariables; i++){
		map <double, MyValue>::iterator it, itEnd;
		it = myProblem.variables[i].domain.begin();
		itEnd = myProblem.variables[i].domain.end();
		for (; it != itEnd; ++it){
			atoms.push_back(&(it->second));
		}
	}

	int nActions = myProblem.actions.size();
	for (int i = 0; i < nActions; i++){
		set <MyGroundedAction>::iterator actionIt, actionItEnd;
		actionIt = myProblem.actions[i].groundedActions.begin();
		actionItEnd = myProblem.actions[i].groundedActions.end();
		for (; actionIt != actionItEnd; ++actionIt){
			MyGroundedAction *groundedAction;
			groundedAction = const_cast <MyGroundedAction *> (&(*actionIt));
			actions.push_back(groundedAction);
		}
	}

	actionItEnd = actions.end();
	atomItEnd = atoms.end();

	sout << "Number of Layers: " << numberOfLayers << endl;
	for (int i = 0; i < numberOfLayers; i++){

		//Print propositions
		atomIt = atoms.begin();
		for (; atomIt != atomItEnd; ++atomIt){
			if ((*atomIt)->firstVisitedLayer == i){
				(*atomIt)->write(sout);
				sout << ';';
			}
		}
		sout << endl;

		//Print propositions mutexes
		for (atomIt = atoms.begin(); atomIt != atomItEnd; ++atomIt){
			if ((*atomIt)->firstVisitedLayer > i){
				continue;
			}
			list <MyAtom *>::iterator atomIt2;
			atomIt2 = atoms.begin();
			for (atomIt2 = atoms.begin(); atomIt2 != atomIt; ++atomIt2){
				if ((*atomIt2)->firstVisitedLayer <= i){
					if ((*atomIt)->isMutex(i, *atomIt2)){
						sout << '(';
						(*atomIt)->write(sout);
						sout << ',';
						(*atomIt2)->write(sout);
						sout << ");";
					}
				}
			}
		}
		sout << endl;

		//Print actions
		for (actionIt = actions.begin(); actionIt != actionItEnd; ++actionIt){
			if ((*actionIt)->firstVisitedLayer == i){
				(*actionIt)->write(sout);
				sout << ';';
			}
		}
		sout << endl;

		//Print actions mutexes
		for (actionIt = actions.begin(); actionIt != actionItEnd; ++actionIt){
			if ((*actionIt)->firstVisitedLayer > i){
				continue;
			}
			list <MyGroundedAction *>::iterator actionIt2;
			for (actionIt2 = actions.begin(); actionIt2 != actionIt; ++actionIt2){
				if ((*actionIt2)->firstVisitedLayer <= i){
					if ((*actionIt)->isMutex(i, *actionIt2)){
						sout << '(';
						(*actionIt)->write(sout);
						sout << ',';
						(*actionIt2)->write(sout);
						sout << ");";
					}
				}
			}
		}
		sout << endl;
	}
}

NumericalPlanningGraph::~NumericalPlanningGraph() {
	// TODO Auto-generated destructor stub
}



