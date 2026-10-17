//#define DEBUG //enable it when debug is needed.
#include <limits>
#include <stdexcept>
#include "Gibson.h"
//#undef DEBUG

#ifdef DEBUG
#include <Windows.h>
#endif
#include <string>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
using namespace std;

#include <ctime>
#include "IndexedTree.h"

#ifdef USE_MESSAGING
#include <VCELL/SimulationMessaging.h>
#endif
#include "VCellException.h"
const double double_infinity = numeric_limits<double>::infinity();
const double EPSILON = 1E-12;
const string Gibson::MY_T_STR = "t";

/*
 *Empty constructor of Gibson. It will use the defalt settings in StochModel.
 */
Gibson::Gibson()
	: savedSampleCount(1), lastTime (std::numeric_limits<long>::min()) {
	Tree = NULL;
	currvals = NULL;
    generator = new std::mt19937_64();
    distribution = new std::uniform_real_distribution<double>(0.0,1.0);
#ifdef USE_MESSAGING
	SimulationMessaging::getInstVar(); // creates the singleton on first use
#endif
}//end of constructor Gibson()

/*
 *This constructor of Gibson reads control information, variables and processes
 *from the input file and updates the attributes' values defined in StochModel().
 *Input para: srting, the input file(name), where the model info. is read.
 *            string, the output file(name), where the results are saved.
 */
Gibson::Gibson(const char* arg_infilename, const char* arg_outfilename) : Gibson(){ // Use delegating constructor
	this->outfilename = arg_outfilename;

	ifstream infile;
	string instring;
	infile.open(arg_infilename);
	if (!infile) {
		VCELL_EXCEPTION(runtime_error, "Unable to open file " << arg_infilename);
	}
	flag_savePeriod=false;
    bMultiButNotHisto = false;
	while(infile >> instring)
	{
		//load control info.
		if (instring == "STARTING_TIME"){
			infile >> STARTING_TIME;
		} else if (instring == "BMULTIBUTNOTHISTO"){
            infile >> bMultiButNotHisto;
        } else if (instring == "ENDING_TIME"){
			infile >> ENDING_TIME;
		} else if (instring == "SAVE_PERIOD"){
			infile >> SAVE_PERIOD;
			flag_savePeriod=true;
		} else if (instring == "MAX_ITERATION"){
			infile >> MAX_ITERATION;
		} else if (instring == "TOLERANCE"){
			infile >> TOLERANCE;
		} else if (instring == "SAMPLE_INTERVAL"){
			infile >> SAMPLE_INTERVAL;
		} else if (instring == "MAX_SAVE_POINTS"){
			infile >> MAX_SAVE_POINTS;
		} else if (instring == "NUM_TRIAL"){
			infile >> NUM_TRIAL;
		} else if (instring == "SEED"){
			infile >> SEED;
		} else if (instring == "TotalVars"){ //load listofvars
			int varCount;
			infile >> varCount;
			string name;
			double amount;
			for(int i=0;i<varCount;i++){
				infile >> name;
				infile >> amount;
				listOfVarNames.push_back(name);
				listOfIniValues.push_back((uint64_t)amount);
				StochVar *var=new StochVar((uint64_t)amount);
				listOfVars.push_back(var);
			}
		} else if (instring == "TotalProcesses"){ //load listOfProcesses
			int pCount;
			infile >> pCount;
			string name;
			for(int i=0;i<pCount;i++)
			{
				infile >> name;
				listOfProcessNames.push_back(name);
				Jump *jp=new Jump();
				listOfProcesses.push_back(jp);
			}
		} else if (instring == "TotalDescriptions") { //load process description to set up processes
			int dCount,idx;
			infile >> dCount;
			string name,str;
			for(int i=0;i<dCount;i++){ //loop through each process description
				infile >> name >> name;// "process name"
				idx=getProcessIndex(name);
				listOfProcesses[idx]->setNameIndex(idx);
				infile >> str;// "Propensity"
				char exp[2000];
				infile.getline(exp,2000);//get expression string
				int lenNames=listOfVarNames.size();
// New code
				vector<string> names(lenNames + 1);
				for (int k = 0; k < lenNames; k++)
					names[k] = listOfVarNames.at(k);
				names[lenNames] = MY_T_STR;
				listOfProcesses[idx]->setProbabilityExpression(exp, names.data(), (lenNames + 1));
// Original code
// 				string names[lenNames+1];
// 				for (int k=0;k<lenNames;k++)
// 					names[k]=listOfVarNames.at(k);
// 				names[lenNames]=MY_T_STR;
// 				listOfProcesses[idx]->setProbabilityExpression(exp,names,(lenNames+1));
				infile >> str;//"Effect"
				if(str=="Effect")
				{
					int numEffects;
					infile >> numEffects;
					string varName;
					string oper;
					double v;
					for(int j=0;j<numEffects;j++)
					{
						infile >> varName;
						infile >> oper;
						infile >> v;
						int idx2=getVarIndex(varName);
						StochVarContext *temp=new StochVarContext(listOfVars[idx2], oper, (int)v);
						listOfProcesses[idx]->addVarContext(temp);
					}
				}
				infile >> str;//"DependentProcesses"
				if(str=="DependentProcesses")
				{
					int numDependents;
					infile >> numDependents;
					string processName;
					for(int j=0;j<numDependents;j++)
					{
						infile >> processName;
						int idx3=getProcessIndex(processName);
						listOfProcesses[idx]->addDependentJump(listOfProcesses[idx3]);
					}
				}
			}
		}
	}
	Tree = new IndexedTree();
	for(auto & listOfProcesse : listOfProcesses){
		Tree->addProcess(listOfProcesse);
	}
	infile.close();

	if (NUM_TRIAL > MAX_ALLOWED_POINTS) {
		VCELL_EXCEPTION(invalid_argument,"Stochastic initialization: Server maximum number trials " << NUM_TRIAL << " exceeds limit of " << MAX_ALLOWED_POINTS);
	}
	if (MAX_SAVE_POINTS > MAX_ALLOWED_POINTS) {
		VCELL_EXCEPTION(invalid_argument,"Stochastic initialization: Server save points " << MAX_SAVE_POINTS << " exceeds limit of " << MAX_ALLOWED_POINTS);
	}

	if(NUM_TRIAL > 1 && !bMultiButNotHisto){
		MAX_SAVE_POINTS = NUM_TRIAL;
	}

	currvals=new double[listOfIniValues.size()+1];
    if (bMultiButNotHisto){
        this->multiTrialStats = new MultiTrialStats(listOfVars.size(), MAX_SAVE_POINTS);
    }
}

//Destructor
Gibson::~Gibson()
{
	//delete indexedTree
	delete Tree;
	//delete variables
	for(auto & listOfVar : listOfVars)
		delete listOfVar;
	listOfVars.clear();
	//delete proccesses
	for(auto & listOfProcesse : listOfProcesses)
		delete listOfProcesse;
	listOfProcesses.clear();
	//delete vectors
	listOfVarNames.clear();
	listOfIniValues.clear();
	listOfProcessNames.clear();
	//delete currvals
	delete[] currvals;
    delete distribution;
    delete generator;
}//end of destructor ~Gibson()

/*
 *The method is the core function of Gibson method, it does one run for Gibson simulation.
 *The loop will end either by ending_time or max_iteration.
 *For single trial, the values of variables against time will be output at each save_period.
 *For multiple trial, the results at the ending_time after each trial will be stored in a the file.
 */
int Gibson::core()
{
	double outputTimer = STARTING_TIME;
	double simtime = STARTING_TIME;
// New code
	vector<double> lastStepVals(listOfIniValues.size());
	vector<double> initialValues(listOfIniValues.size());
// Original code
// 	double lastStepVals[listOfIniValues.size()];
	double p, r;
	int saveIntervalCount = SAMPLE_INTERVAL;
	int iterationCounter=0;
	int i;
	int varLen = listOfIniValues.size();

	for(i=0;i<Tree->getSize();i++)
	{
		Jump *jump = Tree->getProcess(i);
		jump->setNode(i);

		for(int k=0;k<varLen;k++)
		{
			currvals[k]=listOfIniValues[k];
// New code
			lastStepVals[k]=*listOfVars.at(k)->getCurr();
// Original code
// 			lastStepVals[k]=*listOfVars.at(k)->getCurr();
		}
		currvals[varLen] = simtime;
		p = jump->getProbabilityRate(currvals);

		if(p < 0){
			VCELL_EXCEPTION(runtime_error,"at time point " << simtime << ", propensity of jump process "<< listOfProcessNames.at(jump->getNameIndex()) <<" evaluated to a negative value (" << p << "). Simulation abort!" << endl << jump->getProbabilityRateEvaluationSummary(currvals) );
		}
		do
		{
			r = getRandomUniform();
		}
		while(r <= 0);
		jump->setLogRand(-log(r));
		if(!(p>0)){
			jump->setTime(double_infinity);
		}
		else{
			jump->setTime(jump->getLogRand()/p);
		}
	}

	Tree->build();

    if (bMultiButNotHisto) {
        multiTrialStats->startNewTrial();
// New code
    	std::vector<double> initialValues(listOfIniValues.size());
        for (int k=0;k<listOfIniValues.size();k++)
            initialValues[k]=listOfIniValues[k];
        multiTrialStats->addSample(0, 0.0, initialValues.data());
// Original code
//         double initialValues[listOfIniValues.size()];
//         for (int k=0;k<listOfIniValues.size();k++)
//             initialValues[k]=listOfIniValues[k];
//         multiTrialStats->addSample(0, 0.0, initialValues);
    }
	//the while loop does one trial for simulation and ends by ending_time.
	while(simtime < ENDING_TIME)
	{
#ifdef USE_MESSAGING
		if (SimulationMessaging::getInstVar()->isStopRequested()) {
			break;
		}
#endif
		//save last step variables' values
		for(i = 0;i<varLen;i++){
			lastStepVals[i]=*listOfVars.at(i)->getCurr();
		}
	    //get next reaction with shortest absolute time
		Jump* event = Tree->getProcess(0);
		//update time
		simtime = event->getTime();
		if(simtime > ENDING_TIME)
		{
			//simulation time exceed ending time. Before we quit the simulation
			//output data between the last step simulation time and ending time if we have SAVE_PERIOD on.
			if((NUM_TRIAL ==1) && (flag_savePeriod))//SingleTrajectory_OutputInterval
			{	//use EPSILON here to make sure that a double 0.99999999999995(usually happen in C) is not regarded as a number that smaller than 1. It should be 1.
				while((outputTimer+SAVE_PERIOD+EPSILON) < ENDING_TIME)
				{
                    if(bMultiButNotHisto) {//Accumulate data mode
                        multiTrialStats->addSample(savedSampleCount, outputTimer + SAVE_PERIOD, lastStepVals.data());
                    }else {
                        accumOrSaveInit(varLen, outputTimer + SAVE_PERIOD, true);
                        for (i = 0; i < varLen; i++) {
                            outfile << lastStepVals[i] << "\t";
                        }
                    }
//                    outfile << outputTimer+SAVE_PERIOD << "\t";
//					for(i=0;i<varLen;i++){
//						outfile<< lastStepVals[i] << "\t";
//					}
					savedSampleCount = finalizeSampleRow(savedSampleCount,simtime);//outfile << endl;
					outputTimer = outputTimer + SAVE_PERIOD;
                }
			}
			break;
		}
		//update affected variables
		int numVars = event->getNumVars();
		for(i = 0;i<numVars;i++){
			event->getVar(i)->updateCurr();
		}
		//get current values for evaluate the probability expression
		for(int k=0;k<varLen;k++)
		{
			currvals[k]=*listOfVars.at(k)->getCurr();
		}
		currvals[varLen] = simtime;
		//update the jump that occured
		double r = getRandomUniform();
		p = event->getProbabilityRate(currvals);
		//amended Oct 11th, 2007. Stop the simulation and send error message back if
		//anyone of the propensity functions is negative.
		if(p < 0){
			VCELL_EXCEPTION(runtime_error,"at time point " << simtime << ", propensity of jump process "<< listOfProcessNames.at(event->getNameIndex()) <<" evaluated to a negative value (" << p << "). Simulation abort!" << endl << event->getProbabilityRateEvaluationSummary(currvals) );
		}
		//amended May 17th. The previous sentence will cause the time of a process stuck in double_infinity when r<=0
		if(r>0)
		{
			event->setLogRand(-log(r));
			if(p>0)
				Tree->updateTree(event, (event->getLogRand())/p+simtime);
			else
				Tree->updateTree(event, double_infinity);
		}
		else
		{
			Tree->updateTree(event, double_infinity);
		}
		//update dependent jumps
		int numDependentJumps = event->getNumDependentJumps();
		for(i=0;i<numDependentJumps;i++)
		{
			Jump *dJump = event->getDependent(i);
			double p_old = dJump->getOldProbabilityRate();
			double p_new = dJump->getProbabilityRate(currvals);
			//amended Oct 11th, 2007. Stop the simulation and send error message back if
			//anyone of the propensity functions is negative.
			if(p_new < 0){
				VCELL_EXCEPTION(runtime_error, "at time point " << simtime << ", propensity of jump process "<< listOfProcessNames.at(dJump->getNameIndex()) <<" evaluated to a negative value (" << p << "). Simulation abort!" << endl << dJump->getProbabilityRateEvaluationSummary(currvals) );
			}
			double tau = dJump->getTime();
			//amended May 17th. to make sure that tau is a finite double
			if(tau != double_infinity && (-tau != double_infinity) && tau == tau){
				dJump->setLogRand(p_old*(tau-simtime));
			}
			if(!((p_new)>0)){
				Tree->updateTree(dJump, double_infinity);
			}
			else{
				Tree->updateTree(dJump, (dJump->getLogRand())/p_new+simtime);
			}
		}
#ifdef DEBUG
		int treeLen = Tree->getSize();
		bool infAll=true;
		for(i=0; i<treeLen; i++)
		{
			if(Tree->getProcess(i)->getTime() != double_infinity)
				infAll=false;
		}
		if(infAll)
			cout << "all times are set infinity.";
#endif
		if(NUM_TRIAL ==1)//SingleTrajectory
		{
			if(saveIntervalCount == SAMPLE_INTERVAL)
			{
				//output the result to file if the num_trial is one and the outputTimer reaches the new save period
				if(flag_savePeriod)//OutputInterval
				{
					while((outputTimer+SAVE_PERIOD) < simtime)
					{
                        if(bMultiButNotHisto) {//Accumulate data mode
                            multiTrialStats->addSample(savedSampleCount, outputTimer + SAVE_PERIOD, lastStepVals.data());
                        }else {
                            accumOrSaveInit(varLen, outputTimer + SAVE_PERIOD, true);
                            for (i = 0; i < varLen; i++) {
                                outfile << lastStepVals[i] << "\t";
                            }
                        }
						savedSampleCount = finalizeSampleRow(savedSampleCount,simtime);//outfile << endl;
						outputTimer = outputTimer + SAVE_PERIOD;
					}
					if(outputTimer+SAVE_PERIOD <= simtime + EPSILON)
					{
						outfile << outputTimer+SAVE_PERIOD << "\t";
						for(i=0;i<varLen;i++){
							outfile<< *listOfVars.at(i)->getCurr() << "\t";
							savedSampleCount = finalizeSampleRow(savedSampleCount,simtime);//outfile << endl;
						}
						outputTimer = outputTimer + SAVE_PERIOD;
					}
				}
				else //KeepEvery
				{
					outfile << simtime << "\t";
					for(i=0;i<varLen;i++){
						outfile<< *listOfVars.at(i)->getCurr()<< "\t";
					}
					savedSampleCount = finalizeSampleRow(savedSampleCount,simtime);//outfile << endl;
				}
			}
			if(saveIntervalCount == 1)
				saveIntervalCount = SAMPLE_INTERVAL;
			else
				saveIntervalCount--;
		}//if(NUM_TRIAL ==1)
//		else{
//            frmprint(simtime, lastStepVals,sizeof(lastStepVals)/sizeof(lastStepVals[0]));
//        }
	}//end of while loop
//    frmprint(simtime, lastStepVals,sizeof(lastStepVals)/sizeof(lastStepVals[0]));
	//output the variable's vals at the ending time point.
	if((simtime > ENDING_TIME) && (NUM_TRIAL == 1) && (flag_savePeriod))//SingleTrajectory_OutputInterval
	{
        if(bMultiButNotHisto) {//Accumulate data mode
            multiTrialStats->addSample(savedSampleCount, ENDING_TIME, lastStepVals.data());
        }else {
            accumOrSaveInit(listOfVars.size(), ENDING_TIME, false);
            for (i = 0; i < listOfVars.size(); i++) {
                outfile << "\t" << *listOfVars.at(i)->getCurr();
            }
        }
		savedSampleCount = finalizeSampleRow(savedSampleCount,simtime);//outfile << endl;
	}
	//to save the (last-step)values of variables after one run of simulation
	if(NUM_TRIAL >1)//Histogram
	{
		for(i=0;i<listOfVars.size();i++)
		{
			outfile<< "\t" << *listOfVars.at(i)->getCurr();
		}
		savedSampleCount = finalizeSampleRow(savedSampleCount,simtime);//outfile << endl;
	}
    //return parameter 0:ends by ending_time  1:ends by max_iteration
	if(iterationCounter< MAX_ITERATION)
	{
		return 0;
	}
	else return 1;
}

void Gibson::accumOrSaveInit(int varLen,double timePoint,bool bAddTab) {
    if(bAddTab) {
        outfile << timePoint << "\t";
    }else{
        outfile << timePoint;
    }
}
//end of method core()
//void Gibson::frmprint(double simtime, const double *lastStepVals,int sizelsv) {
//    cout << "t:";
//    for(int i=0; i < listOfVarNames.size(); i++){
//        cout << listOfVarNames.at(i) << ":";
//    }
//    cout << endl;
//    cout << simtime <<":" << ENDING_TIME <<" ";
//    for(int row = 0; row < sizelsv; row++) {
//        cout << lastStepVals[row] << " ";
//    }
//    cout <<endl;
//}


int Gibson::finalizeSampleRow(int savedSampleCount,double simtime){
    if(!bMultiButNotHisto) {
        outfile << endl;
    }
//	cout << "savedSampleCount=" << savedSampleCount << endl;
	if(savedSampleCount > MAX_SAVE_POINTS) {
		VCELL_EXCEPTION(runtime_error,
			"Simulation exceeded maximum saved time points " << MAX_SAVE_POINTS 
			<< ". Partial results may be available. \nYou can increase the save interval (\"keep every\") or increase the maximum number of saved time points (\"keep at most\").");
	}
	//progress no more than every 2 seconds
	if (difftime(time(NULL),lastTime) > 2.0){
		lastTime = time(NULL);
        if(bMultiButNotHisto) {
            double percentile =  static_cast<double>(currMultiNonHistoIter)  / static_cast<double>(numMultiNonHisto);
#ifdef USE_MESSAGING
            SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, percentile, currMultiNonHistoIter);
#else
            printf("[[[progress:%lg%%]]]", percentile * 100);
            fflush(stdout);
#endif
        } else if(NUM_TRIAL > 1){//histogram
			double percentile =  static_cast<double>(savedSampleCount)  / NUM_TRIAL;
#ifdef USE_MESSAGING
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, percentile, savedSampleCount);
#else
			printf("[[[progress:%lg%%]]]", percentile * 100);
			fflush(stdout);
#endif
		}else{//single_trajectory
			double percentile = (simtime/ENDING_TIME);
#ifdef USE_MESSAGING
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, percentile, simtime);
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, percentile, simtime);
#else
			printf("[[[progress:%lg%%]]]", percentile * 100);
			printf("[[[data:%lg]]]", simtime);
			fflush(stdout);
#endif
		}
	}

	return savedSampleCount+1;
}


/*
 *This method is the control of trials, which will be called for Gibson simulation.
 */
void Gibson::march(){
#ifdef DEBUG
	// Count performance time in milliseconds for the simulation
	int ntime=0;
	LARGE_INTEGER ntime1,ntime2;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ntime1);
#endif

	//prepare for writing the results to output file
	this->outfile.open (this->outfilename, ofstream::out);//"c:/gibson_deploy/gibson_deploy/output/gibson_singleTrial.txt"
	this->outfile << setprecision(10); // set precision to output file
    if(this->bMultiButNotHisto){
//        char thebyte[1];
//        thebyte[0]=49;//progress char "1"
//        string ofProg(outfilename);
//        ofProg.append("_progress");
//        ofstream outfileProg;
//        outfileProg.open (ofProg.c_str(),ios::out | ios::app | ios::binary);

        this->numMultiNonHisto = this->NUM_TRIAL;
        this->NUM_TRIAL = 1;//set to 1 to use single trajectory logic in 'core'

        //output file header description for time and variable names
        this->outfile << "t:";
        for (const auto & varName : this->listOfVarNames) {
            this->outfile << varName << ":";
        }
        this->outfile << endl;

        //Execute NUM_TRIALS of core and accumulate the results
        for (this->currMultiNonHistoIter = 0; this->currMultiNonHistoIter < this->numMultiNonHisto; this->currMultiNonHistoIter++){
            this->generator->seed(this->currMultiNonHistoIter+SEED);
            //run the simulation
            this->core();
            //reset to initial values before next simulation
            this->savedSampleCount = 1;
            for(int i = 0; i < this->listOfIniValues.size(); i++){
                this->listOfVars[i]->setCurr(this->listOfIniValues.at(i));
            }
            for(int i = 0; i < this->listOfProcesses.size(); i++){
                this->Tree->setProcess(i, this->listOfProcesses.at(i));
            }
//            outfileProg.write((char *)&thebyte,1);
//            outfileProg.flush();
        }
//        outfileProg.close();

        //Calc and save averages of accumulated data
        for (int timeIndex = 0; timeIndex < this->multiTrialStats->getNumTimePoints(); ++timeIndex) {
            //timepoint
            this->outfile << this->multiTrialStats->getTimePoint(timeIndex) << "\t";
            for (int varIndex = 0; varIndex < this->multiTrialStats->getNumVars(); ++varIndex) {
                this->outfile << this->multiTrialStats->getMean(varIndex, timeIndex) << "\t";
            }
            this->outfile << endl;
        }

        this->multiTrialStats->writeOutput(this->outfilename, this->listOfVarNames);

    } else if(this->NUM_TRIAL==1){
        this->generator->seed(this->SEED);
        //output file header
        this->outfile << "t:";
        for(const auto & listOfVarName : this->listOfVarNames){
            this->outfile << listOfVarName << ":";
        }
        this->outfile << endl;
        //output initial condition at STARTING_TIME
        this->outfile << this->STARTING_TIME << "\t";
        for(const unsigned long long listOfIniValue : this->listOfIniValues){
            this->outfile << listOfIniValue << "\t";
        }
        outfile << endl;
        //run the simulation
        core();

    } else if (this->NUM_TRIAL > 1){
		//output file header
		this->outfile << "TrialNo:";
		for(const auto & listOfVarName : this->listOfVarNames){
			this->outfile<< listOfVarName << ":";
		}
		this->outfile << endl;
		for (long j = this->SEED; j < this->NUM_TRIAL + this->SEED; j++)
		{
#ifdef USE_MESSAGING
		if (SimulationMessaging::getInstVar()->isStopRequested()) {
			break;
		}
#endif

#ifdef DEBUG
			cout << "Trial No. " << j <<endl;
#endif
			this->generator->seed(j);
			//output trial number.  PS:results after each trial are printed in core() function.
			this->outfile << j - this->SEED + 1;//this expression should evaluate equal to 'savedSampleCount'
			this->core();//this will save 1 row of data (
			//reset to initial values before next simulation
			for(int i=0; i < this->listOfIniValues.size();i++){
				this->listOfVars[i]->setCurr(this->listOfIniValues.at(i));
			}
			for(int i=0;i < this->listOfProcesses.size();i++){
				this->Tree->setProcess(i, this->listOfProcesses.at(i));
			}
		}
	} else {
		VCELL_EXCEPTION(invalid_argument, "Number of trial smaller than 1!");
	}
#ifdef DEBUG
	//Count performance time for single trial
	QueryPerformanceCounter(&ntime2);
	//get ntime in millisecs
	ntime = (ntime2.QuadPart-ntime1.QuadPart)/(freq.QuadPart/1000);
	cout << endl << "Total time used(ms): " << ntime;
#endif
    this->outfile.close();

#ifdef USE_MESSAGING
	std::cout << "Arrived at goalpost 6" << std::endl;
	if (!SimulationMessaging::getInstVar()->isStopRequested()) {
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_COMPLETED, 1, ENDING_TIME);
	}
#endif
}//end of method march()

/*
 * This method generates a random number from uniform distribution.
 * Output: double, the random number generated.
 */
double Gibson::getRandomUniform()
{
    double uniform_sample = (*distribution)(*generator);
    return uniform_sample;
}//end of method getRandomUniform()
