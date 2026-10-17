#ifndef STOCHMODEL_H
#define STOCHMODEL_H

#include <string>
#include <vector>
#include <stdint.h>
#include "StochVar.h"
#include "Jump.h"

/* This class defines stochastic model which can be inherited by concrete 
 * stochastic algorithms. It defines control information (STARTING_TIME,
 * ENDING_TIME, SAVE_PERIOD, MAX_ITERATION, TOLERANCE, NUM_TRIAL), 
 * list of variables and list of processes.
 * Reference method descriptions in StochModel.cpp.
 *
 * @Author:Tracy LI
 * @version:1.0 Beta
 * @CCAM,UCHC. June 2,2006
 */
class StochModel
{
    public:
	   StochModel();
	   int getNumOfVars();
	   int getNumOfProcesses();
	   int getVarIndex(string);
	   int getProcessIndex(string);
	   virtual int core(){
		   return 0;//virtual function, to be overridden in derived classes.
	   };
	   virtual void march(){
		   //virtual function, to be overridden in derived classes.
	   };
       
    protected:
       double STARTING_TIME;//simulation staring time
	   double ENDING_TIME;//simulation ending time
	   double SAVE_PERIOD;//the time interval to save the results
	   long MAX_SAVE_POINTS;//the maximum output points to be saved
	   long MAX_ITERATION;//the restriction of maximum loops in a simulation
	   double TOLERANCE;//the lowest value for propensity
	   int SAMPLE_INTERVAL;//the sampling interval, e.g SAMPLE_INTERVAL=3 means sampling once for every three time points
	   long NUM_TRIAL;//number of trials
	   int SEED;//random seed
	   vector <StochVar*> listOfVars;//list of stochasic variables in the model
	   vector <string> listOfVarNames;//list of variable names 
	   vector <uint64_t> listOfIniValues;//keep original values for multiple trials
	   vector <Jump*> listOfProcesses;//keep original values for multiple trials
	   vector <string> listOfProcessNames;//list of process names
};

#endif
