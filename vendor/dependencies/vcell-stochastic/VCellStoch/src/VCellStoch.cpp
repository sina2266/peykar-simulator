#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "Exception.h"
#ifdef USE_MESSAGING
#include <VCELL/SimulationMessaging.h>
#endif
#include "Gibson.h"
#include <VCELL/GitDescribe.h>
using namespace std;

static void printUsage() {
	cout << "Usage: VCellStoch {gibson|gillespie} input_filename output_filename";
#ifdef USE_MESSAGING
	cout << " [-tid 0]" << endl;
#endif
	cout << endl;
}

static void loadJMSInfo(istream& ifsInput, int taskID) {
	// std::string rather than char[] buffers: C++20 dropped `istream >> char*`, and the
	// messaging library now pulls the whole target up to C++20.
	string broker, smqusername, password, qname, tname, vcusername;
	string nextToken;
	int simKey = 0, jobIndex = 0;

	while (!ifsInput.eof()) {
		nextToken = "";
		ifsInput >> nextToken;
		if (nextToken.size() == 0) {
			continue;
		} else if (nextToken[0] == '#') {
			getline(ifsInput, nextToken);
			continue;
		}  else if (nextToken == "JMS_PARAM_END") {
			break;
		} else if (nextToken == "JMS_BROKER") {
			ifsInput >> broker;
		} else if (nextToken == "JMS_USER") {
			ifsInput >> smqusername >> password;
		} else if (nextToken == "JMS_QUEUE") {
			ifsInput >> qname;
		} else if (nextToken == "JMS_TOPIC") {
			ifsInput >> tname;
		} else if (nextToken == "VCELL_USER") {
			ifsInput >> vcusername;
		} else if (nextToken == "SIMULATION_KEY") {
			ifsInput >> simKey;
			continue;
		} else if (nextToken == "JOB_INDEX") {
			ifsInput >> jobIndex;
			continue;
		} 
	}

#ifdef USE_MESSAGING
	if (taskID >= 0) {
		// the broker is reached over HTTP now, so the queue/topic names and JMS
		// credentials parsed above are no longer part of the handshake
		SimulationMessaging::getInstVar()->initialize_curl_messaging(false, broker.c_str(), vcusername.c_str(), simKey, jobIndex, taskID);
	} else {
		SimulationMessaging::getInstVar();
	}
#endif
}

static void errExit(int returnCode, string& errorMsg) {	
#ifdef USE_MESSAGING
	if (returnCode != 0) {	
		if (!SimulationMessaging::getInstVar()->isStopRequested()) {
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_FAILURE, errorMsg.c_str());
		}
	}
	// waits for the queue thread, then destroys the singleton -- the destructor is no longer public
	SimulationMessaging::cleanupInstanceVar();
#else
	if (returnCode != 0) {	
		cerr << errorMsg << endl;
	}

#endif
	
}

/* This file is the entrance of the Virtual Cell stochastic simulation package.
 * It parses the commandline arguments to load different simulators. Four parameters
 * are required for the command. The Usage is: 
 * VCellStoch gibson[gillespie] input_filename output_filename. 
 *
 * @Author: Tracy LI
 * @version:1.0 Beta
 * @CCAM,UCHC. May 26,2006
 */
int main(int argc, char *argv[])
{
    std::cout
	    << "Stochastic simulation version " << g_GIT_DESCRIBE
	    << std::endl; 
	if (argc != 4 && argc != 6) {
		cout << "Wrong arguments!" << endl;
		printUsage();
		exit(-1);
	}

	ifstream inputstream(argv[2]);
	if (!inputstream.is_open()) {
		cerr <<  "input file [" << argv[2] << "] doesn't exit!" << endl;
		exit(-1);
	}

	char* solver = argv[1];
	char* inputfile = argv[2];
	char* outputfile = argv[3];
	int taskID = -1;
	if (argc == 6) {
		taskID = atoi(argv[5]);
	}

	string errMsg = "Gibson solver failed : ";
	int returnCode = 0;
	
	try {

		string nextToken;		

		while (!inputstream.eof()) {			
			nextToken = "";
			inputstream >> nextToken;	
			if (nextToken.empty()) {
				continue;
			} else if (nextToken[0] == '#') {
				getline(inputstream, nextToken);
				continue;
			} else if (nextToken == "JMS_PARAM_BEGIN") {
				loadJMSInfo(inputstream, taskID);
				// no start() any more -- MessageEventManager owns the queue thread
				break;
			}
		}
		inputstream.close();

		string s2(solver);
				
		if (s2.compare("gibson")==0)
		{
			Gibson *gb=new Gibson(inputfile, outputfile); // e.g 
//			Gibson *gb = new Gibson("c:/sim.txt","c:/sim_out.txt");
   			gb->march();
			delete gb;
		}
		else if (s2.compare("gillespie")==0)
		{
			cout << "Gillespie method is under development.";
		}

	} catch (string& ex) {
		errMsg += ex;
		returnCode = -1;
	} catch (std::exception& ex) {
		errMsg += ex.what();
		returnCode = -1;
	} catch (const char* ex) {
		errMsg += ex;
		returnCode = -1;
	} catch (...) {
		errMsg += "unknown error";
		returnCode = -1;
	}

	errExit(returnCode, errMsg);
	return returnCode;
}//end of main()

