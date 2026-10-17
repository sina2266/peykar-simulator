#ifndef VCELLSUNDIALSSOLVER_H
#define VCELLSUNDIALSSOLVER_H

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <Expression.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>

#include "VCellSolver.h"

class SymbolTable;  // Defined in SymbolTable.h
class OdeResultSet; // Defined in OdeResultSet.h
struct Event;
struct EventAssignment;
struct EventExecution;
struct OdeDiscontinuity;

#define bytesPerSample 25
#define MaxFileSizeBytes 1000000000 /* 1 gigabyte */
#define BAD_EXPRESSION_MSG " is not terminated by ';'"
#define MAX_NUM_EVENTS_DISCONTINUITIES_EVAL 50

class VCellSundialsSolver : public VCellSolver {
	public:
		VCellSundialsSolver();
		virtual ~VCellSundialsSolver();

		// void readInput(std::istream &inputFileStream);

		void configureFromInput(VCellSolverInputBreakdown& inputBreakdown) override;

		[[nodiscard]] OdeResultSet *getResultSet() const { return this->odeResultSet; }
		[[nodiscard]] int getNumEquations() const { return this->NUM_EQUATIONS; }
		[[nodiscard]] VCell::Expression **getInitialConditionExpressions() const { return this->initialConditionExpressions; }
		void setStartingTime(const realtype newStartingTime) { this->STARTING_TIME = newStartingTime; }
		void setEndingTime(const realtype newEndingTime) { this->ENDING_TIME = newEndingTime; }
		//void setOutputTimes(int count, double* newOutputTimes);
		[[nodiscard]] SymbolTable *getSymbolTable() const { return this->defaultSymbolTable; }

		static void checkStopRequested(double, long);

	protected:
		// 0 : t
		// 1 ~ N : variable values
		// N+1 ~ N+NPARAM : parameter values
		// N+NPARAM+1 ~ N+NPARAM+numDiscontinuites : discontinuity values
		realtype *values;
		// 0 ~ N-1 : equations
		VCell::Expression **initialConditionExpressions;
		SymbolTable *initialConditionSymbolTable;
		OdeResultSet *odeResultSet; // mainly for parameter optimization use, but it also stores column names

		void *solver; // the memory for solver
		std::string recoverableErrMsg;

		int NUM_EQUATIONS;
		int NUM_PARAMETERS;
		realtype STARTING_TIME;
		realtype ENDING_TIME;
		realtype RelativeTolerance;
		realtype AbsoluteTolerance;
		long keepEvery;
		double maxTimeStep;
		std::vector<double> outputTimes;
		double *tempRowData; // data for current time to be written to output file and to be added to odeResultSet

		int numDiscontinuities;
		OdeDiscontinuity **odeDiscontinuities;
		SymbolTable *discontinuitySymbolTable;
		double *discontinuityValues;
		int *rootsFound;

		std::string *paramNames;
		std::string *variableNames; // variables
		std::string *allSymbols;
		int numAllSymbols;
		SymbolTable *defaultSymbolTable;
		N_Vector y;

		static VCell::Expression *readExpression(std::istream &inputstream);

		void writeData(double currTime, FILE *outputFile);
		virtual void updateTempRowData(double currTime);
		void writeFileData(FILE *outputFile);
		void writeFileHeader(FILE *outputFile);
		void printProgress(double currTime, double &lastPercentile, clock_t &lastTime, double increment, FILE *outputFile) const;
		virtual void readEquations(VCellSolverInputBreakdown& inputBreakdown) = 0;
		virtual void initialize();
		void initDiscontinuities();
		bool updateDiscontinuities(realtype t, bool bOnRootReturn);
		void checkDiscontinuityConsistency() const;
		void solveInitialDiscontinuities(double t);
		virtual bool fixInitialDiscontinuities(double t) =0;
		void printVariableValues(realtype t);
		void printDiscontinuityValues();
		virtual void updateTandVariableValues(realtype t, N_Vector y) =0;
		int RootFn(realtype t, N_Vector y, realtype *gout);
		virtual std::string getSolverName() =0;
		virtual VCellSolverTypes getSolverType() =0;
		bool executeEvents(realtype realTimeVar);
		double getNextEventTime();

	private:
		Event **events;
		int numEvents;
		std::list<EventExecution*> eventExeList;

		// void readEvents(std::istream &inputstream);
		void testEventTriggers(realtype Time);
};

struct EventAssignment {
	int varIndex;
	VCell::Expression *assignmentExpression;

	~EventAssignment() { delete assignmentExpression; }
	void bind(SymbolTable *symbolTable) const { assignmentExpression->bindExpression(symbolTable); }
};

struct EventExecution {
	realtype timeToExecuteEventAt;
	Event *event0;
	double *targetValues;

	EventExecution(Event *e) {
		this->timeToExecuteEventAt = RCONST(0.0);
		this->event0 = e;
		this->targetValues = nullptr;
	}

	~EventExecution() { delete this->targetValues; }
};

struct Event {
	std::string name;
	VCell::Expression *triggerExpression;
	bool bUseValuesAtTriggerTime;
	VCell::Expression *delayDurationExpression;
	std::vector<EventAssignment *> *eventAssignmentsVec;
	bool triggerValue;

	Event() {
		this->bUseValuesAtTriggerTime = false;
		this->triggerExpression = nullptr;
		this->triggerValue = false;
		this->delayDurationExpression = nullptr;
		this->eventAssignmentsVec = new std::vector<EventAssignment *>;
	}

	~Event() {
		delete this->triggerExpression;
		delete this->delayDurationExpression;
		for (const EventAssignment *elem: *this->eventAssignmentsVec) delete elem;
		this->eventAssignmentsVec->clear();
		delete this->eventAssignmentsVec;
	}

	[[nodiscard]] bool hasDelay() const { return this->delayDurationExpression != nullptr; }

	void bind(SymbolTable *symbolTable) const {
		this->triggerExpression->bindExpression(symbolTable);
		if (this->delayDurationExpression != nullptr) { this->delayDurationExpression->bindExpression(symbolTable); }
		for (const EventAssignment *elem: *this->eventAssignmentsVec) { elem->bind(symbolTable); }
	}
};

struct OdeDiscontinuity {
	std::string discontinuitySymbol;
	VCell::Expression *discontinuityExpression;
	VCell::Expression *rootFindingExpression;

	~OdeDiscontinuity() {
		delete this->discontinuityExpression;
		delete this->rootFindingExpression;
	}
};

#endif
