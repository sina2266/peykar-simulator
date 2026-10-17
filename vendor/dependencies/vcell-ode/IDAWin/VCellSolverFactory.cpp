//
// Created by Logan Drescher on 6/26/25.
//
#include "VCellSolverFactory.h"
#include "VCellSundialsSolver.h"
#include <cstddef>
#include <sstream>
#include <format>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include "Exception.h"
#include "VCellCVodeSolver.h"
#include "VCellIDASolver.h"
#ifdef USE_MESSAGING
#include <VCELL/SimulationMessaging.h>
#endif
// Forward Declarations
static void readDiscontinuities (std::istream &inputStream, VCellSolverInputBreakdown& inputBreakdown);
static void readEvents(std::istream &inputStream, VCellSolverInputBreakdown& inputBreakdown);
static std::string getExpressionString(std::istream& inputStream);
static std::string getExpressionString(std::string& unparsedString);
static void trimString(std::string &str);
// static VCell::Expression* readExpression(std::istream &inputstream);
static void collectCVodeEquations(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown);
static void collectIDAEquations(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown);
static void collectSteadyStateTerms(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown);
static void loadJMSInfo(std::istream &ifsInput, int taskID);

VCellSolver* VCellSolverFactory::produceVCellSolver(std::ifstream& inputFileStream, int taskID){
	VCellSolverInputBreakdown inputBreakdown = parseInputFile(inputFileStream, taskID);
	VCellSolver* desiredSolver;
	switch (inputBreakdown.solverType) {
		case VCellSolverTypes::CVODE:
			desiredSolver = new VCellCVodeSolver();
			break;
		case VCellSolverTypes::IDA:
			desiredSolver = new VCellIDASolver();
			break;
		// case VCellSolverTypes::STEADY_STATE:
		// 	break;
		default:
			throw new VCell::Exception("Unknown VCellSolver type");
	}
	try {
		desiredSolver->configureFromInput(inputBreakdown);
	} catch (...) {
		delete desiredSolver;
		throw;
	}

	return desiredSolver;
}

VCellSolverInputBreakdown VCellSolverFactory::parseInputFile(std::ifstream& inputFileStream, int taskID) {
	VCellSolverInputBreakdown inputBreakdown;
	while (!inputFileStream.eof()) {
		std::string nextToken;
		inputFileStream >> nextToken;
		if (nextToken.empty()) continue;
		if (nextToken[0] == '#') std::getline(inputFileStream, nextToken);
		else if (nextToken == "SOLVER") {
			std::string solverName;
			inputFileStream >> solverName;
			inputBreakdown.solverType = determineSolverType(solverName);
		} else if (nextToken == "JMS_PARAM_BEGIN") {
			loadJMSInfo(inputFileStream, taskID);
			// #ifdef USE_MESSAGING
			// SimulationMessaging::getInstVar()->start(); // start the thread
			// #endif
		}
		else if (nextToken == "STARTING_TIME") { inputFileStream >> inputBreakdown.timeCourseSettings.STARTING_TIME; }
		else if (nextToken == "ENDING_TIME") { inputFileStream >> inputBreakdown.timeCourseSettings.ENDING_TIME; }
		else if (nextToken == "RELATIVE_TOLERANCE") { inputFileStream >> inputBreakdown.timeCourseSettings.RELATIVE_TOLERANCE; }
		else if (nextToken == "ABSOLUTE_TOLERANCE") { inputFileStream >> inputBreakdown.timeCourseSettings.ABSOLUTE_TOLERANCE; }
		else if (nextToken == "MAX_TIME_STEP") { inputFileStream >> inputBreakdown.timeCourseSettings.MAX_TIME_STEP; }
		else if (nextToken == "KEEP_EVERY") { inputFileStream >> inputBreakdown.timeCourseSettings.KEEP_EVERY; }
		else if (nextToken == "OUTPUT_TIME_STEP"){ inputFileStream >> inputBreakdown.timeCourseSettings.OUTPUT_TIME_STEP; }
		else if (nextToken == "OUTPUT_TIMES") {
			std::size_t numOfTimePoints;
			inputFileStream >> numOfTimePoints;
			double timePoint;
			for (std::size_t i = 0; i < numOfTimePoints; i++) {
				inputFileStream >> timePoint;
				const bool isValidTimePoint = timePoint > inputBreakdown.timeCourseSettings.STARTING_TIME
											&& timePoint <= inputBreakdown.timeCourseSettings.ENDING_TIME;
				if (isValidTimePoint) inputBreakdown.timeCourseSettings.TIME_POINTS.push_back(timePoint);
			}
			if (inputBreakdown.timeCourseSettings.TIME_POINTS.back() < inputBreakdown.timeCourseSettings.ENDING_TIME)
				inputBreakdown.timeCourseSettings.TIME_POINTS.push_back(inputBreakdown.timeCourseSettings.ENDING_TIME);
		} else if (nextToken == "DISCONTINUITIES") {
			readDiscontinuities(inputFileStream, inputBreakdown);
		} else if (nextToken == "NUM_PARAMETERS") {
			std::size_t numParameters;
			inputFileStream >> numParameters;
			for (std::size_t i = 0; i < numParameters; i++) {
				std::string name;
				inputFileStream >> name;
				inputBreakdown.modelSettings.PARAMETER_NAMES.push_back(name);
			}
		} else if (nextToken == "NUM_EQUATIONS") { VCellSolverFactory::processEquations(inputFileStream, inputBreakdown); }
		else if (nextToken == "EVENTS") { readEvents(inputFileStream, inputBreakdown); }
		else if (nextToken == "STEADY_STATE") {
			std::string confirmation;
			inputFileStream >> confirmation;
			if (confirmation == "TRUE") { collectSteadyStateTerms(inputFileStream, inputBreakdown); }
		}
		else throw VCell::Exception("Unexpected token \"" + nextToken + "\" in the input file!");

	}
	return inputBreakdown;
}

VCellSolverTypes VCellSolverFactory::determineSolverType(const std::string& solverName){
	if (solverName == "CVODE")
		return VCellSolverTypes::CVODE;
	if (solverName == "IDA")
		return VCellSolverTypes::IDA;
	if (solverName == "CSSS")
		return VCellSolverTypes::STEADY_STATE; // copasi-style steady-state
	throw std::runtime_error("Unknown solver type: `" + solverName + "`");
}

void VCellSolverFactory::processEquations(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown) {
	if (inputBreakdown.solverType == VCellSolverTypes::IDA) collectIDAEquations(inputFileStream, inputBreakdown);
	else collectCVodeEquations(inputFileStream, inputBreakdown);
}

/** * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Local Helper Functions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

static void readDiscontinuities(std::istream &inputStream, VCellSolverInputBreakdown& inputBreakdown) {
	VCellSolverInputBreakdown::DiscontinuityComponents discontinuityComponents;
	std::size_t numOfDiscontinuities;
	inputStream >> numOfDiscontinuities;

	for (std::size_t i = 0; i < numOfDiscontinuities; i++) {
		inputStream >> discontinuityComponents.symbol;

		std::string line = "";
		std::getline(inputStream, line);
		std::string::size_type pos = line.find(";");
		if (pos == std::string::npos) {
			std::string msg = std::string("discontinuity expression ") + BAD_EXPRESSION_MSG;
			throw VCell::Exception(msg);
		}
		std::string exp = line.substr(0, pos + 1);
		trimString(exp);
		discontinuityComponents.discontinuityExpression = new VCell::Expression(exp);

		exp = line.substr(pos + 1);
		discontinuityComponents.rootFindingExpression = new VCell::Expression(getExpressionString(exp));

		inputBreakdown.discontinuitiesSettings.DISCONTINUITIES.push_back(std::move(discontinuityComponents));
	}
}

static void readEvents(std::istream &inputStream, VCellSolverInputBreakdown& inputBreakdown) {
	std::size_t numOfEvents;
	std::string token;

	inputStream >> numOfEvents;
	//events = new Event *[numEvents];
	for (std::size_t i = 0; i < numOfEvents; i++) {
		VCellSolverInputBreakdown::EventComponents eventComponents;
		while (true) { // Break on "EVENTASSIGNMENTS"
			inputStream >> token;
			if (token == "EVENT") { inputStream >> eventComponents.eventName; }
			else if (token == "TRIGGER") {
				try {
					eventComponents.triggerExpression = getExpressionString(inputStream);
				} catch (VCell::Exception &ex) {
					throw VCell::Exception(std::string("trigger expression") + " " + ex.getMessage());
				}
			} else if (token == "DELAY") {
				inputStream >> token;
				eventComponents.shouldUseValuesAtTriggerTime = (token == "true");
				try {
					eventComponents.delayDurationExpression = getExpressionString(inputStream);
				} catch (VCell::Exception &ex) {
					throw VCell::Exception(std::string("delay duration expression") + " " + ex.getMessage());
				}
			} else if (token == "EVENTASSIGNMENTS") {
				int numEventAssignments;
				inputStream >> numEventAssignments;
				for (int j = 0; j < numEventAssignments; j++) {
					std::pair<int, std::string> eventAssignment;
					inputStream >> eventAssignment.first; //varIndex
					try {
						eventAssignment.second = getExpressionString(inputStream); // assignment expression
					} catch (VCell::Exception &ex) {
						throw VCell::Exception(std::string("event assignment expression") + " " + ex.getMessage());
					}
					eventComponents.eventAssignments.push_back(std::move(eventAssignment));
					//eventComponents.eventAssignments.emplace_back(varIndex, assignmentExpression); // should try this in the future, more descriptive
				}
				break;
			} else { throw VCell::Exception("Unexpected event token \"" + token + "\" in the input file!"); }
		}
		inputBreakdown.eventSettings.EVENTS.push_back(std::move(eventComponents));
	}
}

static void trimString(std::string &str) {
	if (std::string::size_type pos = str.find_last_not_of(" \r\n"); pos != std::string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(" \r\n");
		if (pos != std::string::npos) { str.erase(0, pos); }
	} else { str.erase(str.begin(), str.end()); }
}

static std::string getExpressionString(std::istream& inputStream) {
	std::string exp;
	std::getline(inputStream, exp);
	return getExpressionString(exp);
}

static std::string getExpressionString(std::string& unparsedString) {
	trimString(unparsedString);
	if (*(unparsedString.end() - 1) != ';') { throw VCell::Exception(std::format("Expression `{}` is not terminated with a ';' character!", unparsedString)); }
	return unparsedString;
}

/**----------------------------------------------------
Input format:
	STARTING_TIME 0.0
	ENDING_TIME 0.1
	RELATIVE_TOLERANCE 1.0E-9
	ABSOLUTE_TOLERANCE 1.0E-9
	MAX_TIME_STEP 1.0
	KEEP_EVERY 1
	DISCONTINUITIES 1
	D_B0 (t > 0.0432); (-0.0432 + t);
	NUM_EQUATIONS 2
	ODE x_i INIT 0.0;
		 RATE ((20.0 * x_o * D_B0) - (50.0 * x_i));
	ODE x_o INIT 0.0;
		 RATE ( - ((20.0 * x_o * D_B0) - (50.0 * x_i)) + (1505000.0 * (3.322259136212625E-4 - (3.322259136212625E-4 * x_o) - (3.322259136212625E-4 * x_i))) - (100.0 * x_o));
--------------------------------------------------------------*/

static void collectCVodeEquations(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown) {
	std::size_t numOfEquations;
	inputFileStream >> numOfEquations;
	std::string exp;
	try {
		std::string keyword;
		for (std::size_t i = 0; i < numOfEquations; i++) {
			// ODE
			std::string varName;
			inputFileStream >> keyword >> varName;
			inputBreakdown.modelSettings.VARIABLE_NAMES.push_back(varName);

			// INIT
			inputFileStream >> keyword;
			try {
				std::string expression{getExpressionString(inputFileStream)};
				inputBreakdown.modelSettings.INITIAL_CONDITION_EXPRESSIONS.push_back(expression);
			} catch (VCell::Exception& ex) {
				const std::string errMsg{"Initial condition expression for [" + inputBreakdown.modelSettings.VARIABLE_NAMES.back() + "] " + ex.getMessage()};
				throw VCell::Exception(errMsg);
			}

			// RATE
			inputFileStream >> keyword;
			try {
				std::string expression{getExpressionString(inputFileStream)};
				inputBreakdown.modelSettings.RATE_EXPRESSIONS.push_back(expression);
			} catch (VCell::Exception& ex) {
				const std::string errMsg{"Rate expression for [" + inputBreakdown.modelSettings.VARIABLE_NAMES.back() + "] " + ex.getMessage()};
				throw VCell::Exception(errMsg);
			}
		}
		inputBreakdown.modelSettings.NUM_DIFFERENTIAL = inputBreakdown.modelSettings.RATE_EXPRESSIONS.size();
		inputBreakdown.modelSettings.NUM_ALGEBRAIC = 0;

	} catch (char* ex) {
		throw VCell::Exception(std::string("VCellSolverFactory::collectCVodeEquations() : ") + ex);
	} catch (VCell::Exception& ex) {
		throw VCell::Exception(std::string("VCellSolverFactory::collectCVodeEquations() : ") + ex.getMessage());
	}
}

/**----------------------------------------------------
Input format:
	STARTING_TIME 0.0
	ENDING_TIME 0.1
	RELATIVE_TOLERANCE 1.0E-9
	ABSOLUTE_TOLERANCE 1.0E-9
	MAX_TIME_STEP 1.0
	KEEP_EVERY 1
	DISCONTINUITIES 1
	D_B0 (t > 0.0432); (-0.0432 + t);
	NUM_EQUATIONS 2
	VAR x_i INIT (0.0);
	VAR x_o INIT (0.8);
	TRANSFORM
	3.322259136212625E-4 0.0
	0.0 1.0
	INVERSETRANSFORM
	3010.0 0.0
	0.0 1.0
	RHS DIFFERENTIAL 1 ALGEBRAIC 1
	(3.322259136212625E-4 * ((20.0 * x_o * D_B0) - (50.0 * x_i)));
	((1505000.0 * (3.3222591362126253E-4 - (3.322259136212625E-4 * x_i) - (3.322259136212625E-4 * x_o))) - (100.0 * x_o));
--------------------------------------------------------------*/
static void collectIDAEquations(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown) {
	std::size_t numOfEquations;
	inputFileStream >> numOfEquations;
	try {
		std::string keyword;
		std::string _ignored;

		// There should be <numOfEquations> number of:
		// * VAR statements
		// * TRANSFORM entries
		// * INVERSETRANSFORM entries
		// * RHS entries
		// We will remove sections when we've gotten the necessary amount of each
		std::unordered_map<std::string, int> sections = {{"VAR", 0}, {"TRANSFORM", 0} , {"INVERSETRANSFORM", 0}, {"RHS", 0} };

		while (!sections.empty()) {
			inputFileStream >> keyword;
			if (!sections.contains(keyword)) throw VCell::Exception(std::string("Encountered unexpected keyword `") + keyword + "`");

			if (keyword == "VAR") {
				std::string varName, initialConditionExpression;
				inputFileStream >> varName;
				inputBreakdown.modelSettings.VARIABLE_NAMES.push_back(std::move(varName));
				std::string initKeyword;
				inputFileStream >> initKeyword; // INIT keyword (not into `keyword`: that names the section counted below)
				try {
					inputBreakdown.modelSettings.INITIAL_CONDITION_EXPRESSIONS.push_back(getExpressionString(inputFileStream));
				} catch (VCell::Exception& ex) {
					throw VCell::Exception(std::string("Initial condition expression for [") + inputBreakdown.modelSettings.VARIABLE_NAMES.back() + "] " + ex.getMessage());
				}
				// check for completion
				if (sections[keyword] == numOfEquations - 1) sections.erase(keyword);
				else sections[keyword]++;
			} else if (keyword == "TRANSFORM") {
				std::getline(inputFileStream, _ignored); // go to next line
				for (std::size_t i = 0; i < numOfEquations * numOfEquations; i++) {
					std::string nextTerm;
					inputFileStream >> nextTerm;
					inputBreakdown.modelSettings.FLAT_TRANSFORM_MATRIX.push_back(nextTerm);
				}
				// Affirm completion
				sections.erase(keyword);
			} else if (keyword == "INVERSETRANSFORM") {
				std::getline(inputFileStream, _ignored); // go to next line
				for (std::size_t i = 0; i < numOfEquations * numOfEquations; i++) {
					std::string nextTerm;
					inputFileStream >> nextTerm;
					inputBreakdown.modelSettings.FLAT_INVERSE_TRANSFORM_MATRIX.push_back(nextTerm);
				}
				// Affirm completion
				sections.erase(keyword);
			} else if (keyword == "RHS") {
				std::size_t numDifferentials, numAlgebraics;
				std::string rhsKeyword; // not `keyword`: that must still say "RHS" when the section is erased below
				inputFileStream >> rhsKeyword >> numDifferentials;
				if (rhsKeyword != "DIFFERENTIAL") throw VCell::Exception("Error: Expected keyword `DIFFERENTIAL`, found `" + rhsKeyword +"`");
				inputBreakdown.modelSettings.NUM_DIFFERENTIAL = numDifferentials;
				inputFileStream >> rhsKeyword >> numAlgebraics;
				if (rhsKeyword != "ALGEBRAIC") throw VCell::Exception("Error: Expected keyword `ALGEBRAIC`, found `" + rhsKeyword +"`");
				inputBreakdown.modelSettings.NUM_ALGEBRAIC = numAlgebraics;
				if (numDifferentials + numAlgebraics != numOfEquations) {
					throw VCell::Exception(std::format("The sum of differential ({}) and algebraic ({}) equations does not equal the total number of equations ({})", numDifferentials, numAlgebraics, numOfEquations));
				}
				std::getline(inputFileStream, _ignored); // go to next line
				for (std::size_t i = 0; i < numOfEquations; i ++) {
					try {
						inputBreakdown.modelSettings.RATE_EXPRESSIONS.push_back(getExpressionString(inputFileStream));
					} catch (VCell::Exception& ex) {
						throw VCell::Exception(std::format("RHS[{}] {}", i, ex.getMessage()));
					}
				}
				// Affirm completion
				sections.erase(keyword);
			} else {
				std::string errMsg{"Fatal error: inconsistent state - `" + keyword + "` is a valid section, but no logic exists to deal with it!"};
				throw VCell::Exception(errMsg);
			}
		}
	} catch (const char* ex) {
		throw VCell::Exception(std::string("VCellSolverFactory::collectIDAEquations() : ") + ex);
	} catch (VCell::Exception& ex) {
		throw VCell::Exception(std::string("VCellSolverFactory::collectIDAEquations() : ") + ex.getMessage());
	} catch (...) {
		throw VCell::Exception("VCellSolverFactory::collectIDAEquations() caught unknown exception");
	}
}

/**----------------------------------------------------
Input format:
	... (STEADY_STATE TRUE)
	RESOLUTION 0.0000000001
	MAX_NEWTON_ITERATIONS
	MAXIMUM_FORWARD_DURATION
	DERIVATION_FACTOR 0.001
	STOP_CRITERIA DISTANCE_AND_RATE
	...
--------------------------------------------------------------*/
static void collectSteadyStateTerms(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown) {
	std::unordered_set<std::string> sections = {"RESOLUTION", "MAX_NEWTON_ITERATIONS",
		"MAXIMUM_FORWARD_DURATION", "DERIVATION_FACTOR", "STOP_CRITERIA"};

	do {
		std::string keyword;
		inputFileStream >> keyword;
		if (keyword == "RESOLUTION") { inputFileStream >> inputBreakdown.steadyStateSettings.RESOLUTION; }
		else if (keyword == "MAXIMUM_NEWTON_ITERATIONS") { inputFileStream >> inputBreakdown.steadyStateSettings.MAX_ITERATIONS; }
		else if (keyword == "MAXIMUM_FORWARD_DURATION") { inputFileStream >> inputBreakdown.steadyStateSettings.MAX_FORWARD_DURATION; }
		else if (keyword == "DERIVATION_FACTOR") { inputFileStream >> inputBreakdown.steadyStateSettings.DERIVATION_FACTOR; }
		else if (keyword == "STOP_CRITERIA") {
			std::string stopCriteria;
			inputFileStream >> stopCriteria;
			if (stopCriteria == "DISTANCE_ONLY") { inputBreakdown.steadyStateSettings.STOP_CRITERIA = VCellSteadyStateStopCriteria::DISTANCE_ONLY; }
			else if (stopCriteria == "RATE_ONLY") { inputBreakdown.steadyStateSettings.STOP_CRITERIA = VCellSteadyStateStopCriteria::RATE_ONLY; }
			else if (stopCriteria == "DISTANCE_AND_RATE") { inputBreakdown.steadyStateSettings.STOP_CRITERIA = VCellSteadyStateStopCriteria::DISTANCE_AND_RATE; }
			else if (stopCriteria == "DISTANCE_OR_RATE") { inputBreakdown.steadyStateSettings.STOP_CRITERIA = VCellSteadyStateStopCriteria::DISTANCE_OR_RATE; }
			else throw VCell::Exception("Unknown stop criteria `" + keyword + "`");
		} else throw VCell::Exception("Unknown steady state keyword `" + keyword + "`");
		if (!sections.contains(keyword)) throw VCell::Exception("Duplicate section `" + keyword + "` detected.");
		sections.erase(keyword);
	} while (!sections.empty());
}

static void loadJMSInfo(std::istream &ifsInput, int taskID) {
	#ifndef USE_MESSAGING
	// We'll still parse the section, as we can still execute the simulation; we'll just toss the values!
	std::cerr << "WARNING: Input file expects messaging capabilities; this build does not support JMS messaging!" << std::endl;
	#endif

	std::string broker;
	std::string smqUserName;
	std::string password;
	std::string qName;
	std::string topicName;
	std::string vCellUsername;
	int simKey, jobIndex;

	while (true) {
		if (ifsInput.eof())
			throw std::runtime_error("VCellSolverFactory::loadJMSInfo() reached end of file, but no `JMS_PARAM_END` reached!");

		std::string nextToken;
		ifsInput >> nextToken;
		if (nextToken.empty()) continue;
		if (nextToken[0] == '#') {
			// getline(ifsInput, nextToken); // Is this ignoring because of a comment?
			ifsInput.ignore('\n');
			continue;
		}
		if (nextToken == "JMS_PARAM_END") break; // Non-error stop condition

		if (nextToken == "JMS_BROKER") ifsInput >> broker;
		else if (nextToken == "JMS_USER") ifsInput >> smqUserName >> password;
		else if (nextToken == "JMS_QUEUE") ifsInput >> qName;
		else if (nextToken == "JMS_TOPIC") ifsInput >> topicName;
		else if (nextToken == "VCELL_USER") ifsInput >> vCellUsername;
		else if (nextToken == "SIMULATION_KEY") ifsInput >> simKey;
		else if (nextToken == "JOB_INDEX") ifsInput >> jobIndex;
	}

	#ifdef USE_MESSAGING
	// SimulationMessaging::getInstVar()->initialize_curl_messaging(broker.c_str(), smqUserName.c_str(),
	// 							password.c_str(), qName.c_str(), topicName.c_str(),
	// 							vCellUsername.c_str(), simKey, jobIndex, taskID);
	SimulationMessaging::getInstVar()->initialize_curl_messaging(false, broker.c_str(),
		vCellUsername.c_str(), simKey, jobIndex, taskID);
	#endif
}
