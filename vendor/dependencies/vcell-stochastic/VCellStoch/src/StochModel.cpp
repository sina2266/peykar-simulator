#include <assert.h>
#include <math.h>
#include <time.h>
#include <sstream>
#include <string>
#include "StochModel.h"

using namespace std;
/*
 *The constructor of Class StochModel which sets up the simulation controls by
 *default values and initializes the vectors storing variables and processes. 
 */
StochModel::StochModel()
{
	STARTING_TIME=0;
	ENDING_TIME=10;
	SAVE_PERIOD=0.01;
	MAX_ITERATION=10000;
	SAMPLE_INTERVAL=1;
	MAX_SAVE_POINTS=1000;
	TOLERANCE=1e-9;
	NUM_TRIAL=2000;
	SEED = (unsigned)time( NULL );
	if (listOfVars.size()>0){
		listOfVars.erase(listOfVars.begin(),listOfVars.end());
	}
	if (listOfVarNames.size()>0){
		listOfVarNames.erase(listOfVarNames.begin(),listOfVarNames.end());
	}
	if (listOfIniValues.size()>0){
		listOfIniValues.erase(listOfIniValues.begin(),listOfIniValues.end());
	}
	if (listOfProcesses.size()>0){
		listOfProcesses.erase(listOfProcesses.begin(),listOfProcesses.end());
	} 
	if (listOfProcessNames.size()>0){
		listOfProcessNames.erase(listOfProcessNames.begin(),listOfProcessNames.end());
	}
}//end of constructor StochModel()

/*
 *Get the total number of variables
 */
int StochModel::getNumOfVars()
{
	return listOfVars.size();
}//end of method getNumOfVars()

/*
 *Get the total number of processes
 */
int StochModel::getNumOfProcesses()
{
	return listOfProcesses.size();
}//end of method getNumOfProcesses()

/*
 *Find the variable index in listOfVars by looking through listOfVarNames.
 *Input para:string, variable name used to look for the index.
 */
int StochModel::getVarIndex(string varName)
{
	for(int i=0;i<listOfVarNames.size();i++)
	{
		if(varName==listOfVarNames.at(i))
			return i;
	}
	stringstream ss;
	ss << "variable name " << varName << " not found in model";
	string errStr = ss.str();   // call system to stop
	throw errStr;
}//end of method getVarInex()

/*
 *Find the process index in listOfProcesses by looking through listOfProcessNames.
 *Input para:string, process name used to look for the index.
 */
int StochModel::getProcessIndex(string processName)
{
	for(int i=0;i<listOfProcessNames.size();i++)
	{
		if(processName==listOfProcessNames.at(i))
			return i;
	}
	stringstream ss;
	ss << "process name " << processName << " not found in model";
	string errStr = ss.str();   // call system to stop
	throw errStr;
}//end of method getProcessIndex()

