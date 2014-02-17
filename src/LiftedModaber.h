/*
 * SimpleModaber.h
 *
 *  Created on: Jun 22, 2013
 *      Author: sadra
 */

#ifndef LIFTEDMODABER_H_
#define LIFTEDMODABER_H_

#include "Modaber.h"
#include "LiftedTranslator.h"
#include "PlanningGraph.h"
#include "SATLiftedTranslator.h"

namespace mdbr {

class LiftedModaber: public Modaber {
private:

	SATLiftedTranslator *satLiftedTranslator;

	LiftedTranslator *myLiftedTranslator;

	LiftedCVC4Problem *liftedSMTProblem;

	PlanningGraph *planGraph;

	int nSignificantTimePoints;

	bool usingSMTSolver;


protected:

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve();
	virtual bool tryToSolve(double bound);

	double findPlanValue ();

public:
	LiftedModaber(char *domainFilePath, char *problemFilePath, char *solutionFilePath);
	virtual ~LiftedModaber();
};

} /* namespace mdbr */

#endif /* LIFTEDMODABER_H_ */
