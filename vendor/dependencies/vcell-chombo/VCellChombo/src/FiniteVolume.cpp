#ifdef CH_CYGWIN
#include <CH_Timer.H>
#endif
#include <parstream.H>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include <VCELL/FVSolver.h>
#include <sys/stat.h>
#include <VCELL/SimTool.h>
#include <string.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/GitDescribe.h>
#include <Exception.h>

#ifdef CH_MPI
#include <mpi.h>
#endif

static bool bNormalReturn = false;

void printUsage()
{
	pout() << "To run a simulation:" << endl;
	pout() << "\tArguments: ";
#ifdef USE_MESSAGING
	pout() << " [-tid taskID]";
#endif
	pout() << " fvInputFile" <<  endl;
	pout() << "To generate vcell output (serial) from chombo output (parallel):" << endl;
	pout() << "\tArguments: -ccd fvInputFile" << endl;
}

void vcellExit(int returnCode, string& errorMsg)
{
	if (SimulationMessaging::getInstVar() == 0)
	{
		if (returnCode != 0)
		{
			cerr << errorMsg << endl;
		}
	} 
	else if (!SimulationMessaging::getInstVar()->isStopRequested())
	{
		if (returnCode != 0)
		{
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_FAILURE, errorMsg.c_str());
		}
#ifdef USE_MESSAGING
		// SimTool is only created once FVSolver has parsed the input file, so it
		// is still null when we fail early (bad arguments, unreadable input).
		SimTool* simTool = SimTool::getInstance();
		if (simTool == 0 || simTool->isRootRank())
		{
			SimulationMessaging::getInstVar()->waitUntilFinished();
		}
#endif
	}
	// The singleton owns its destructor (it has to join the sending thread), so
	// it is torn down through cleanupInstanceVar() rather than deleted here.
	SimulationMessaging::cleanupInstanceVar();
	delete SimTool::getInstance();

#ifdef CH_MPI
	if (returnCode != 0)
	{
		pout() << "MPI::Abort starting" << endl;
		MPI_Abort(MPI_COMM_WORLD,returnCode);
		pout() << "MPI::Abort complete" << endl;
	}
#endif
}

bool bConsoleOutput = true;
char stdErrBuffer[1024];
void onExit()
{
	if (bNormalReturn)
	{
		return;
	}
	// chombo errors
	if (strlen(stdErrBuffer) > 0)
	{
		string errMsg = "";
		if (!bConsoleOutput)
		{
			errMsg = stdErrBuffer;
		}
		vcellExit(1, errMsg);
	}
}

int main(int argc, char *argv[])
{
#ifdef CH_MPI
	const char * const mpiStatus = "(MPI Parallel)";
#else
	const char * const mpiStatus = "(Serial)";
#endif

	pout()
	<< "Chombo solver " << mpiStatus << CH_SPACEDIM << "D version " << g_GIT_DESCRIBE
	<< std::endl;
	printUsage();
	
#ifdef CH_MPI
	pout() << "MPI::Init starting" << endl;
  MPI_Init(&argc, &argv);
	pout() << "MPI::Init complete" << endl;
#endif

	char* timerEnv = getenv("CH_TIMER");
	pout() << "********** CH_TIMER is " << (timerEnv == NULL ? "off" : "on") << " **********" << endl;

	int returnCode = 0;
	string errorMsg = "Exception : ";

	char* fvInputFile = 0;
	ifstream ifsInput;
	bool convertChomboData = false;
	try
	{
		int taskID = -1;
		if (argc < 2)
		{
			throw "Missing arguments";
		}
		for (int i = 1; i < argc; i ++)
		{
			if (!strcmp(argv[i], "-ccd"))  // convert chombo data
			{
				convertChomboData = true;
			}
			else if (!strcmp(argv[i], "-tid"))
			{
#ifdef USE_MESSAGING
				i ++;
				if (i >= argc)
				{
					throw  "Missing taskID!";
				}
				for (int j = 0; j < (int)strlen(argv[i]); j ++)
				{
					if (argv[i][j] < '0' || argv[i][j] > '9')
					{
						stringstream ss;
						ss << "Wrong argument : " << argv[i] << ", taskID must be an integer!" << endl;
						throw ss.str();
					}
				}
				taskID = atoi(argv[i]);
				if (taskID >= 0)
				{
					bConsoleOutput = false;
				}
#else
				stringstream ss;
				ss << "Wrong argument : " << argv[i] << endl;
				throw ss.str();
#endif
			}
			else if (argv[i][0] == '-')
			{
				stringstream ss;
				ss << "Wrong argument : " << argv[i] << endl;
				throw ss.str();
			}
			else
			{
				fvInputFile = argv[i];
			}
		}
			
		atexit(onExit);
		// redirect std::cerr because Chombo would write out to std::cerr and exit.
		//std::cerr.rdbuf(stdErrBuffer.rdbuf());
		stdErrBuffer[0] = '\0';
		setvbuf (stderr , stdErrBuffer , _IOFBF , 1024 );
		// strip " in case that file name has " around
		int fl = strlen(fvInputFile);
		if (fvInputFile[0] == '"' && fvInputFile[fl-1] == '"')
		{
			fvInputFile[fl-1] = 0;
			fvInputFile ++;
		}
		ifsInput.open(fvInputFile);
		if (!ifsInput.is_open())
		{
			stringstream ss;
			ss << "Solver input file fvinput doesn't exist: " << fvInputFile << endl;
			throw ss.str();
		}

#ifdef CH_MPI
		if (convertChomboData)
		{
			throw "Generating vcell output (serial) from chombo output (parallel) is not supported in parallel";
		}
#endif
		
		FVSolver* fvSolver = new FVSolver(ifsInput, taskID);
		ifsInput.close();

		pout( ) << "start solve" << endl;
		fvSolver->solve(convertChomboData);
		pout( ) << "end solve" << endl;
	} 
	catch (const char *exStr)
	{
		errorMsg += exStr;
		returnCode = 1;
	} 
	catch (string& exStr)
	{
		errorMsg += exStr;
		returnCode = 1;
	} 
	catch (VCell::Exception& ex)
	{
		errorMsg += ex.getMessage();
		returnCode = 1;
	} 
	catch (...)
	{
		errorMsg += "unknown error";
		returnCode = 1;
	}

	if (ifsInput.is_open())
	{
		ifsInput.close();
	}

	vcellExit(returnCode, errorMsg);

#ifdef CH_MPI
	pout() << endl << "MPI::Finalize starting" << endl;
	//MPI::Finalize();
	MPI_Finalize();
	pout() << "MPI::Finalize complete" << endl;
#endif
	
	bNormalReturn = true;
	return returnCode;
}
