#include "SimpleSymbolTable.h"
#include "StoppedByUserException.h"
#include "VCellSundialsSolver.h"
#include "OdeResultSet.h"
#include <ranges>
#include <cassert>
#include <cmath>
#include <cstring>
#include <sstream>
#include <string>
#include <format>
#include <algorithm>

#ifdef USE_MESSAGING
#include <VCELL/SimulationMessaging.h>
#endif

/** * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Local Helper Function Prototypes
 * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static void trimString(std::string &str);

/** * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Instance Methods
 * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
VCellSundialsSolver::VCellSundialsSolver() {
	NUM_EQUATIONS = 0;
	NUM_PARAMETERS = 0;
	STARTING_TIME = 0.0;
	ENDING_TIME = 0.0;
	RelativeTolerance = 0.0;
	AbsoluteTolerance = 0.0;
	keepEvery = 0;
	maxTimeStep = 0.0;

	solver = nullptr;
	initialConditionSymbolTable = nullptr;
	initialConditionExpressions = nullptr;
	values = nullptr;
	tempRowData = nullptr;

	variableNames = nullptr;
	paramNames = nullptr;
	allSymbols = nullptr;
	numAllSymbols = 0;
	defaultSymbolTable = nullptr;

	numDiscontinuities = 0;
	odeDiscontinuities = nullptr;
	discontinuityValues = nullptr;
	discontinuitySymbolTable = nullptr;
	rootsFound = nullptr;

	odeResultSet = new OdeResultSet();
	y = nullptr;

	events = nullptr;
	numEvents = 0;
}

VCellSundialsSolver::~VCellSundialsSolver() {
	N_VDestroy_Serial(y);

	for (int i = 0; i < NUM_EQUATIONS; i++) { delete initialConditionExpressions[i]; }
	delete[] initialConditionExpressions;
	delete[] variableNames;
	delete[] paramNames;
	delete[] allSymbols;
	delete defaultSymbolTable;

	for (int i = 0; i < numDiscontinuities; i++) { delete odeDiscontinuities[i]; }
	delete[] odeDiscontinuities;
	delete[] rootsFound;
	delete[] discontinuityValues;
	delete discontinuitySymbolTable;

	delete[] values;
	delete[] tempRowData;
	delete initialConditionSymbolTable;
	delete odeResultSet;

	outputTimes.clear();

	for (int i = 0; i < numEvents; i++) { delete events[i]; }
	delete[] events;

	for (EventExecution* elem : eventExeList) {
		delete elem;
	}
	eventExeList.clear();
}

void VCellSundialsSolver::updateTempRowData(double currTime) {
	tempRowData[0] = currTime;
	for (int i = 0; i < NUM_EQUATIONS; i++) { tempRowData[i + 1] = NV_Ith_S(y, i); }
}

void VCellSundialsSolver::writeData(double currTime, FILE *outputFile) {
	updateTempRowData(currTime);
	odeResultSet->addRow(tempRowData);
	writeFileData(outputFile);
}

void VCellSundialsSolver::writeFileData(FILE *outputFile) {
	if (outputFile != nullptr) {
		fprintf(outputFile, "%0.17E", tempRowData[0]);
		for (int i = 1; i < NUM_EQUATIONS + 1; i++) { fprintf(outputFile, "\t%0.17E", tempRowData[i]); }
		fprintf(outputFile, "\n");
	}
}

void VCellSundialsSolver::writeFileHeader(FILE *outputFile) {
	if (outputFile != nullptr) {
		//  Print header...
		for (int i = 0; i < odeResultSet->getNumColumns(); i++) {
			fprintf(outputFile, "%s:", odeResultSet->getColumnName(i).data());
		}
		fprintf(outputFile, "\n");
	}
}

void VCellSundialsSolver::printProgress(double currTime, double &lastPercentile, clock_t &lastTime, double increment,
                                        FILE *outputFile) const {
	fflush(outputFile);

	if (currTime == STARTING_TIME) { // print 0%
		#ifdef USE_MESSAGING
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, lastPercentile, currTime);
		#else
		printf("[[[progress:%lg%%]]]", lastPercentile*100.0);
		fflush(stdout);
		#endif
	} else {
		clock_t currentTime = clock();
		double duration = (double) (currentTime - lastTime) / CLOCKS_PER_SEC;
		if (duration >= 2) { //send out message every 2 seconds
			double newPercentile = (long) ((currTime - STARTING_TIME) / (ENDING_TIME - STARTING_TIME) / increment) *
			                       increment;

			/*while (true) {
				double midTime = (percentile + increment) * (ENDING_TIME - STARTING_TIME);
				if (STARTING_TIME + midTime > currTime) {
					break;
				}
				percentile += increment;
			}*/
			if (lastPercentile != newPercentile) {
				#ifdef USE_MESSAGING
				SimulationMessaging::getInstVar()->
						setWorkerEvent(JobEvent::JOB_PROGRESS, newPercentile, currTime);
				SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, newPercentile, currTime);
				#else
				printf("[[[progress:%lg%%]]]", newPercentile*100.0);
				printf("[[[data:%lg]]]", currTime);
				fflush(stdout);
				#endif
				lastPercentile = newPercentile;
				lastTime = currentTime;
			}
		}
	}
}

void VCellSundialsSolver::configureFromInput(VCellSolverInputBreakdown& inputBreakdown) {
	try {
		if (solver != nullptr) { throw "configureFromInput should only be called once"; }
		if (inputBreakdown.solverType != getSolverType()) throw VCell::Exception("Fatal Error: Solver mismatch detected!");
		STARTING_TIME = inputBreakdown.timeCourseSettings.STARTING_TIME;
		ENDING_TIME = inputBreakdown.timeCourseSettings.ENDING_TIME;
		RelativeTolerance = inputBreakdown.timeCourseSettings.RELATIVE_TOLERANCE;
		AbsoluteTolerance = inputBreakdown.timeCourseSettings.ABSOLUTE_TOLERANCE;
		maxTimeStep = inputBreakdown.timeCourseSettings.MAX_TIME_STEP;
		keepEvery = inputBreakdown.timeCourseSettings.KEEP_EVERY;
		///TODO: replace "1e-12" with "SMALL_REAL" constant? Probably need to change outputTimes to `std::vector<realtype>`
		if (inputBreakdown.timeCourseSettings.OUTPUT_TIME_STEP != 0.0){
			for (int count = 1; STARTING_TIME + count * inputBreakdown.timeCourseSettings.OUTPUT_TIME_STEP < ENDING_TIME + 1E-12 * ENDING_TIME; count++)
				outputTimes.push_back(STARTING_TIME + count * inputBreakdown.timeCourseSettings.OUTPUT_TIME_STEP);
			ENDING_TIME = outputTimes.back();
		} else if (!inputBreakdown.timeCourseSettings.TIME_POINTS.empty()) {
			for (auto elem : inputBreakdown.timeCourseSettings.TIME_POINTS) outputTimes.push_back(elem);
		}
		numDiscontinuities = inputBreakdown.discontinuitiesSettings.DISCONTINUITIES.size();
		odeDiscontinuities = new OdeDiscontinuity*[numDiscontinuities];
		for (int i = 0; i < numDiscontinuities; i++) {
			odeDiscontinuities[i] = new OdeDiscontinuity();
			odeDiscontinuities[i]->discontinuitySymbol = inputBreakdown.discontinuitiesSettings.DISCONTINUITIES[i].symbol;
			odeDiscontinuities[i]->discontinuityExpression = inputBreakdown.discontinuitiesSettings.DISCONTINUITIES[i].discontinuityExpression;
			odeDiscontinuities[i]->rootFindingExpression = inputBreakdown.discontinuitiesSettings.DISCONTINUITIES[i].rootFindingExpression;
		}
		NUM_PARAMETERS = inputBreakdown.modelSettings.PARAMETER_NAMES.size();
		paramNames = new std::string[NUM_PARAMETERS];
		for (int i = 0; i < NUM_PARAMETERS; i++) {paramNames[i] = inputBreakdown.modelSettings.PARAMETER_NAMES[i];}
		NUM_EQUATIONS = inputBreakdown.modelSettings.VARIABLE_NAMES.size();
		variableNames = new std::string[NUM_EQUATIONS];
		initialConditionExpressions = new VCell::Expression*[NUM_EQUATIONS];
		readEquations(inputBreakdown);
		numEvents = inputBreakdown.eventSettings.EVENTS.size();
		events = new Event*[numEvents];
		for (int i = 0; i < inputBreakdown.eventSettings.EVENTS.size(); i++) {
			events[i] = new Event();
			events[i]->name = inputBreakdown.eventSettings.EVENTS[i].eventName;
			events[i]->bUseValuesAtTriggerTime = inputBreakdown.eventSettings.EVENTS[i].shouldUseValuesAtTriggerTime;
			try {
				events[i]->triggerExpression = new VCell::Expression(inputBreakdown.eventSettings.EVENTS[i].triggerExpression);
			} catch (VCell::Exception &ex) {
				throw VCell::Exception(std::string("trigger expression") + " " + ex.getMessage());
			}

			try {
				if (!inputBreakdown.eventSettings.EVENTS[i].delayDurationExpression.empty())
					events[i]->delayDurationExpression = new VCell::Expression(inputBreakdown.eventSettings.EVENTS[i].delayDurationExpression);
			}  catch (VCell::Exception &ex) {
				throw VCell::Exception(std::string("delay duration expression") + " " + ex.getMessage());
			}

			for (auto [varIndex, assignmentExpression]: inputBreakdown.eventSettings.EVENTS[i].eventAssignments) {
				EventAssignment* eventAssignment = new EventAssignment();
				eventAssignment->varIndex = varIndex;
				eventAssignment->assignmentExpression = new VCell::Expression(assignmentExpression);
				events[i]->eventAssignmentsVec->emplace_back(eventAssignment);
			}
		}
		initialize();
	} catch (char *ex) { throw VCell::Exception(std::string("VCellSundialsSolver::readInput() : ") + ex); } catch (
		VCell::Exception &ex) {
		throw VCell::Exception(std::string("VCellSundialsSolver::readInput() : ") + ex.getMessage());
	}
}



void VCellSundialsSolver::initialize() {
	// add parameters to symbol table
	numAllSymbols = 1 + NUM_EQUATIONS + NUM_PARAMETERS + numDiscontinuities;
	allSymbols = new std::string[numAllSymbols]; // t, variables, parameters, discontinuities

	//t
	allSymbols[0] = "t";
	odeResultSet->addColumn("t");
	// variables
	for (int i = 0; i < NUM_EQUATIONS; i++) {
		allSymbols[1 + i] = variableNames[i];
		odeResultSet->addColumn(variableNames[i]);
	}
	// parameters
	for (int i = 0; i < NUM_PARAMETERS; i++) { allSymbols[1 + NUM_EQUATIONS + i] = paramNames[i]; }
	//discontinuities
	for (int i = 0; i < numDiscontinuities; i++) {
		allSymbols[1 + NUM_EQUATIONS + NUM_PARAMETERS + i] = odeDiscontinuities[i]->discontinuitySymbol;
	}
	// default symbol table has variables, parameters and discontinuities.
	defaultSymbolTable = new SimpleSymbolTable(allSymbols, numAllSymbols);

	// initial condition can only be function of parameters.
	initialConditionSymbolTable = new SimpleSymbolTable(allSymbols + 1 + NUM_EQUATIONS, NUM_PARAMETERS);
	for (int i = 0; i < NUM_EQUATIONS; i++) { initialConditionExpressions[i]->bindExpression(initialConditionSymbolTable); }

	if (numDiscontinuities > 0) {
		rootsFound = new int[2 * numDiscontinuities];
		discontinuityValues = new double[numDiscontinuities];
		// discontinuities can't be function of discontinuity symbols
		discontinuitySymbolTable = new SimpleSymbolTable(allSymbols, 1 + NUM_EQUATIONS + NUM_PARAMETERS);
		for (int i = 0; i < numDiscontinuities; i++) {
			odeDiscontinuities[i]->discontinuityExpression->bindExpression(discontinuitySymbolTable);
			odeDiscontinuities[i]->rootFindingExpression->bindExpression(discontinuitySymbolTable);
		}
	}

	if (numEvents > 0) { for (int i = 0; i < numEvents; i++) { events[i]->bind(defaultSymbolTable); } }
	values = new realtype[1 + NUM_EQUATIONS + NUM_PARAMETERS + numDiscontinuities];
	tempRowData = new realtype[1 + NUM_EQUATIONS];

	y = N_VNew_Serial(NUM_EQUATIONS);
	if (y == nullptr) { throw "Out of Memory"; }
}

bool VCellSundialsSolver::updateDiscontinuities(realtype t, bool bOnRootReturn) {
	if (numDiscontinuities == 0) { return false; }

	bool bUpdated = false;
	updateTandVariableValues(t, y);
	for (int i = 0; i < numDiscontinuities; i++) {
		std::cout << odeDiscontinuities[i]->discontinuitySymbol << " "
			<< odeDiscontinuities[i]->discontinuityExpression->infix()
			<< " " << discontinuityValues[i];
		if (bOnRootReturn) {
			if (rootsFound[2 * i] && rootsFound[2 * i + 1]) {
				std::cout << " inverted ";
				discontinuityValues[i] = discontinuityValues[i] ? 0.0 : 1.0;
				bUpdated = true;
			} else if (rootsFound[2 * i] || rootsFound[2 * i + 1]) {
				std::cout << " evaluated ";
				double oldValue = discontinuityValues[i];
				discontinuityValues[i] = odeDiscontinuities[i]->discontinuityExpression->evaluateVector(values);
				if (oldValue != discontinuityValues[i]) { bUpdated = true; }
			} else { std::cout << " nonroot "; }
		} else {
			std::cout << " evaluated after event execution ";
			double oldValue = discontinuityValues[i];
			discontinuityValues[i] = odeDiscontinuities[i]->discontinuityExpression->evaluateVector(values);
			if (oldValue != discontinuityValues[i]) { bUpdated = true; }
		}
		std::cout << discontinuityValues[i] << std::endl;
	}
	std::cout << std::endl;

	// copy discontinuity values to values to evaluate RHS
	if (bUpdated) { memcpy(values + 1 + NUM_EQUATIONS + NUM_PARAMETERS, discontinuityValues, numDiscontinuities * sizeof(double)); }
	return bUpdated;
}

void VCellSundialsSolver::initDiscontinuities() {
	if (numDiscontinuities == 0) { return; }

	updateTandVariableValues(STARTING_TIME, y);
	// init discontinuities
	for (int i = 0; i < numDiscontinuities; i++) {
		discontinuityValues[i] = odeDiscontinuities[i]->discontinuityExpression->evaluateVector(values);
	}
	// copy discontinuity values to values to evaluate RHS
	memcpy(values + 1 + NUM_EQUATIONS + NUM_PARAMETERS, discontinuityValues, numDiscontinuities * sizeof(double));
}

void VCellSundialsSolver::checkDiscontinuityConsistency() const {
	if (numDiscontinuities == 0) { return; }

	for (int i = 0; i < numDiscontinuities; i++) {
		double realValue = odeDiscontinuities[i]->discontinuityExpression->evaluateVector(values);
		if (discontinuityValues[i] != realValue) {
			std::stringstream ss;
			ss << "at time " << values[0] << ", discontinuity "
				<< odeDiscontinuities[i]->discontinuityExpression->infix() << " evaluated to "
				<< (realValue ? "TRUE" : "FALSE") << ", solver assumed " << (discontinuityValues[i] ? "TRUE" : "FALSE")
				<< std::endl;
			throw ss.str();
		}
	}
}

void VCellSundialsSolver::solveInitialDiscontinuities(double t) {
	std::cout << "------------------solveInitialDiscontinuities--at-time-" << t << "--------------------------" <<
			std::endl;
	bool bFoundRoot = false;
	std::string roots_at_initial_str;
	updateTandVariableValues(t, y);
	for (int i = 0; i < numDiscontinuities; i++) {
		double v = odeDiscontinuities[i]->rootFindingExpression->evaluateVector(values);
		if (fabs(v) < AbsoluteTolerance) {
			roots_at_initial_str += odeDiscontinuities[i]->discontinuityExpression->infix() + "; ";
			bFoundRoot = true;
		}
	}

	if (bFoundRoot) {
		std::cout << "solveInitialDiscontinuities() : roots found at time " << t << "; " << roots_at_initial_str <<
				std::endl;
		int count = 0;
		int maxCount = (int) pow(2.0, numDiscontinuities);
		while (count < maxCount) {
			try { if (!fixInitialDiscontinuities(t)) { break; } } catch (const char *err) {
				std::stringstream str;
				str << "found discontinuities at time " << t << " but unable to initialize : " << err <<
						"\nDiscontinuities at time 0 are : " << roots_at_initial_str;
				throw str.str();
			}
			count++;
		}
		if (count >= maxCount) {
			std::string str =
					"found discontinuities at time 0 but unable to initialize due to max iterations.\nDiscontinuities at time 0 are : "
					+ roots_at_initial_str;
			throw str;
		}
	}
	std::cout << "------------------------------------------------" << std::endl;
}

void VCellSundialsSolver::printVariableValues(realtype t) {
	updateTandVariableValues(t, y);
	std::cout << "variable values are" << std::endl;
	std::cout << "t " << values[0] << std::endl;
	for (int i = 0; i < NUM_EQUATIONS; i++) { std::cout << variableNames[i] << " " << values[i + 1] << std::endl; }
	std::cout << std::endl;
}

void VCellSundialsSolver::printDiscontinuityValues() {
	std::cout << std::endl << "discontinuities values are" << std::endl;
	for (int i = 0; i < numDiscontinuities; i++) {
		std::cout << odeDiscontinuities[i]->discontinuitySymbol << " " << odeDiscontinuities[i]->discontinuityExpression
		                                                                                       ->infix() << " " <<
				discontinuityValues[i] << std::endl;
	}
	std::cout << std::endl;
}

int VCellSundialsSolver::RootFn(realtype t, N_Vector y, realtype *gout) {
	updateTandVariableValues(t, y);
	//cout << "RootFn " << endl;
	//printVariableValues();

	for (int i = 0; i < numDiscontinuities; i++) {
		double r = odeDiscontinuities[i]->rootFindingExpression->evaluateVector(values);
		if (r == 0) {
			gout[2 * i] = 1e-200;
			gout[2 * i + 1] = -1e-200;
		} else {
			gout[2 * i] = r;
			gout[2 * i + 1] = r;
		}
		//cout << "gout[" << i << "]=" << gout[i] << endl;
	}
	return 0;
}

void VCellSundialsSolver::testEventTriggers(realtype Time) {
	if (numEvents == 0) { return; }
	updateTandVariableValues(Time, y);

	std::vector<EventExecution*> eventExecutions;
	for (int i = 0; i < numEvents; i++) {
		bool oldTriggerValue = events[i]->triggerValue;
		bool newTriggerValue = events[i]->triggerExpression->evaluateVector(values) != 0.0;
		events[i]->triggerValue = newTriggerValue;
		if (!oldTriggerValue && newTriggerValue) { // triggered
			eventExecutions.emplace_back(new EventExecution(events[i]));
			eventExecutions.back()->timeToExecuteEventAt = Time;
			if (events[i]->hasDelay()) {
				eventExecutions.back()->timeToExecuteEventAt = Time + events[i]->delayDurationExpression->evaluateVector(values);
			}
			if (events[i]->bUseValuesAtTriggerTime) {
				const unsigned long numEventAssignments = events[i]->eventAssignmentsVec->size();
				eventExecutions.back()->targetValues = new double[numEventAssignments];
				for (unsigned long j = 0; j < numEventAssignments; j++) {
					eventExecutions.back()->targetValues[j] = events[i]->eventAssignmentsVec->at(j)->assignmentExpression->evaluateVector(values);
				}
			}
		}
	}
	auto compLambda = [](const EventExecution *e1, const EventExecution *e2) {
		return e1->timeToExecuteEventAt < e2->timeToExecuteEventAt;
	};
	// eventExeList is ordered by execution time, and executeEvents only ever looks at its front, so the new
	// executions must be merged into it, not appended: an event triggered now with a short delay can be due
	// before one already waiting. stable_sort + merge keep equal times in the order they were queued (existing
	// executions first, then the new ones in event order), as the legacy insertion did.
	std::ranges::stable_sort(eventExecutions, compLambda);
	std::list eventExecutionsSorted(std::make_move_iterator(eventExecutions.begin()), std::make_move_iterator(eventExecutions.end()));
	eventExeList.merge(eventExecutionsSorted, compLambda);
}

bool VCellSundialsSolver::executeEvents(const realtype realTimeVar) {
	if (numEvents == 0) { return false; }
	testEventTriggers(realTimeVar);

	static double epsilon = 1e-15;
	bool bExecuted = false;
	while (!eventExeList.empty()) {
		auto iter = eventExeList.begin();
		EventExecution *ee = *iter;

		if (ee->timeToExecuteEventAt > realTimeVar + epsilon) return bExecuted; // not time yet

		if (fabs(ee->timeToExecuteEventAt - realTimeVar) < epsilon) { // execute
			updateTandVariableValues(realTimeVar, y);
			double *y_data = NV_DATA_S(y); // assign the values
			for (int i = 0; i < ee->event0->eventAssignmentsVec->size(); i++) {
				EventAssignment *ea = ee->event0->eventAssignmentsVec->at(i);
				if (ee->event0->bUseValuesAtTriggerTime) { y_data[ea->varIndex] = ee->targetValues[i]; } else {
					y_data[ea->varIndex] = ea->assignmentExpression->evaluateVector(values);
				}
			}
			std::cout << std::endl << "Executed event " << ee->event0->name << " at time " << realTimeVar << std::endl;
			eventExeList.pop_front(); // delete from the list
			delete ee;
			bExecuted = true;
			testEventTriggers(realTimeVar); // retest all triggers again.
		} else if (ee->timeToExecuteEventAt < realTimeVar) {
			std::stringstream ss;
			ss << "missed Event '" << ee->event0->name << "' with trigger " << ee->event0->triggerExpression->infix()
					<< ", scheduled time = " << ee->timeToExecuteEventAt << ", current time = " << realTimeVar << std::endl;
			throw ss.str();
		}
	}
	return bExecuted;
}

double VCellSundialsSolver::getNextEventTime() {
	if (!eventExeList.empty()) {
		auto iter = eventExeList.begin();
		EventExecution *ee = *iter;
		return ee->timeToExecuteEventAt;
	}

	return DBL_MAX;
}

void VCellSundialsSolver::checkStopRequested(double time, long numIterations) {
	#ifdef USE_MESSAGING
	auto instVar = SimulationMessaging::getInstVar();
	if (nullptr == instVar) return;
	if (instVar->isStopRequested()) { throw StoppedByUserException("stopped by user"); }
	#endif
}

/** * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Static Methods
 * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

VCell::Expression *VCellSundialsSolver::readExpression(std::istream &inputstream) {
	std::string exp;
	getline(inputstream, exp);
	trimString(exp);
	if (*(exp.end() - 1) != ';') { throw VCell::Exception(BAD_EXPRESSION_MSG); }
	return new VCell::Expression(exp);
}

/** * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Local Helper Functions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static void trimString(std::string &str) {
	std::string::size_type pos = str.find_last_not_of(" \r\n");
	if (pos != std::string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(" \r\n");
		if (pos != std::string::npos) { str.erase(0, pos); }
	} else { str.erase(str.begin(), str.end()); }
}
