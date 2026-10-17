#include <MPoint.h>
#include <iostream>
#include <vcellxml.h>
#include <vhdf5/file.h>
#include <sstream>
#include <Logger.h>
#include <StateClient.h>
#include <MBridge/MatlabDebug.h>
#include <tclap/CmdLine.h>
#include <ReportClient.h>
#include <JobMessaging.h>
#include <VCELL/GitDescribe.h>

/**
* usings and typedefs
*/
using tinyxml2::XMLElement;
using moving_boundary::MovingBoundaryElementClient;
using moving_boundary::MovingBoundarySetup;

namespace {
	/**
	* constants
	*/
	const char * const XML_ROOT_NAME = "MovingBoundarySetup";


	/**
	* forward declarations of functions in file
	*/
	void setupTrace(const XMLElement &root); 
	void setupMatlabDebug(const XMLElement &root); 
	void setupProgress(const XMLElement &root,moving_boundary::MovingBoundaryParabolicProblem & mbpp); 

	/**
	* shared variables 
	*/
	std::unique_ptr<vcell_util::FileDest> traceFileDestination;

	struct ExecuteStatus
	{
		int code;
		std::string message;

		ExecuteStatus(): code(0) {}
		ExecuteStatus(int c, const string& msg) : code(c), message(msg)
		{
		}
		bool isOK() const
		{
			return code == 0;
		}
	};

	void notifyExecuteStatus(const ExecuteStatus& executeStatus)
	{
		if (!SimulationMessaging::getInstVar()->isStopRequested())
		{
			if (!executeStatus.isOK())
			{
				SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_FAILURE, executeStatus.message.c_str());
			}
		}
		else if (!executeStatus.isOK())
		{
			std::cerr << executeStatus.message << std::endl;
		}
		// drains the event queue, joins the worker thread and destroys the singleton
		SimulationMessaging::cleanupInstanceVar();

	#ifdef CH_MPI
		if (returnCode != 0)
		{
			cout << "MPI::Abort starting" << endl;
			MPI_Abort(MPI_COMM_WORLD,returnCode);
			cout << "MPI::Abort complete" << endl;
		}
	#endif
	}
}

int main(int argc, char *argv[])
{
    std::cout
            << "MovingBoundarySolver version " << g_GIT_DESCRIBE
            << std::endl;

	std::string filename;
	std::string outname;
	std::string restorename;
	ExecuteStatus executeStatus;

	moving_boundary::enableJobMessaging(); // defaults to stdout-only reporting

	bool parseOnly = false;
	bool configPresent = false;
	int Nx = -1;
	int taskId = -1;

	// java passes -tid which will not work, we need either -t or --tid, so change it to -t
	for (int i = 0; i < argc; ++ i)
	{
		string argument = argv[i];
		if (!strcmp(argv[i], "-tid"))
		{
			argv[i][2] = '\0';
			break;
		}
	}

	namespace tclap = TCLAP;
	try {
		using namespace TCLAP; 
		CmdLine cmd("Moving boundary solve",' ',g_GIT_DESCRIBE);
		cmd.setExceptionHandling(false);
		ValueArg<std::string> config("c","config","Input XML file name",false,"","string",cmd);
		ValueArg<std::string> output("o","output","HDF5 output file name",false,"","string",cmd);
		ValueArg<std::string> restore("r","restore","stored state file name",false,"","string",cmd);
		MultiArg<std::string> ignore("i","ignore","ignore me",false,"string",cmd);
		ValueArg<int> paramTid("t", "tid", "Task ID", false, -1, "int", cmd);
		ValueArg<int> paramNx("n", "Nx", "Nx", false, -1, "int", cmd);
		SwitchArg ponly("p","parseonly","Parse XML without running simulation",cmd);
		cmd.parse(argc,argv);
		filename = config.getValue( );
		outname = output.getValue( );
		restorename = restore.getValue( );
		parseOnly = ponly.getValue( );
		Nx = paramNx.getValue();
		taskId = paramTid.getValue();
		configPresent = config.isSet( );
		if (!configPresent && !restore.isSet( ) )
		{
			std::stringstream ss;
			ss << "error, either -" << config.getName( ) << " or -" << restore.getName( ) << " must be set " << std::endl;
			executeStatus = ExecuteStatus(4, ss.str());
		}
	} catch(tclap::ArgException  &ae) {
		std::stringstream ss;
		ss << "error " << ae.error( ) << " arg " << ae.argId( ) << std::endl;
		executeStatus = ExecuteStatus(3, ss.str());
	} catch (tclap::ExitException &ee) {
		// --help / --version: TCLAP has printed what was asked for; stop here rather
		// than fall through to a solve with no arguments parsed.
		notifyExecuteStatus(ExecuteStatus());
		return ee.getExitStatus();
	}

	moving_boundary::MovingBoundaryParabolicProblem problem;
	if (executeStatus.isOK())
	{
		//moving_boundary::ProblemPackage package;

		//const char * const filename = argv[1];
		//const char * outname = argv[2];
		if (parseOnly)
		{
			std::cout <<  "parse XML only mode" << std::endl;
		}
		try
		{
			tinyxml2::XMLDocument doc;
			if (configPresent)
			{
				doc.LoadFile(filename.c_str( ));
				if (doc.ErrorID( ) != tinyxml2::XML_SUCCESS) {
					std::stringstream ss;
					ss <<  "Error " << doc.ErrorID( ) << " loading " << filename << std::endl;
					executeStatus = ExecuteStatus(2, ss.str());
				}

				if (executeStatus.isOK())
				{
					const tinyxml2::XMLElement & root = *doc.RootElement( );
					if (!strcmp(root.Name( ),XML_ROOT_NAME) == 0)
					{
						std::stringstream ss;
						ss <<  "Invalid XML root identifier " << root.Name( ) << ", " << XML_ROOT_NAME << " expected" << std::endl;
						executeStatus = ExecuteStatus(3, ss.str());
					}

					if (executeStatus.isOK())
					{
						setupTrace(root);
						setupMatlabDebug(root);

						using moving_boundary::MovingBoundarySetup;
						if (restorename.empty( )) {
							auto mbs = MovingBoundarySetup::setupProblem(root, taskId, Nx);
							problem = moving_boundary::MovingBoundaryParabolicProblem(mbs);
							moving_boundary::ReportClient::setup(root, outname, problem);
						}
					}
				}
			}
			if (executeStatus.isOK() && !restorename.empty( ))
			{
				try
				{
					using moving_boundary::StateClient;
					StateClient::ProblemState pState =  StateClient::restore(restorename);
					problem = pState.problem;
				}
				catch (std::exception & e)
				{
					std::stringstream ss;
					ss << argv[0] << " caught exception " << e.what( ) << " reading " << restorename << std::endl;
					executeStatus = ExecuteStatus(6, ss.str());
				}
			}

			if (executeStatus.isOK() && configPresent)
			{
				setupProgress(*doc.RootElement( ),problem);
			}
		}
		catch (std::exception & e) {
			std::stringstream ss;
			ss << argv[0] << " caught exception " << e.what( ) << " reading " << filename;
			executeStatus = ExecuteStatus(4, ss.str());
		}
		catch (H5::Exception & e) {
			std::stringstream ss;
			ss << argv[0] << " caught H5 exception" << e.getCDetailMsg( ) << " reading " << filename;
			executeStatus = ExecuteStatus(7, ss.str());
		}
		catch (...) {
			std::stringstream ss;
			ss << argv[0] << " caught unknown exception reading " << filename;
			executeStatus = ExecuteStatus(5, ss.str());
		}
	}

	if (executeStatus.isOK())
	{
		if (parseOnly) {
			std::cout <<  filename << " validated" << std::endl;
		}
		else
		{
			try
			{
				problem.run( );
				std::cout << "MovingBoundary input " << filename << ", output " << problem.getOutputFiles() << " finished" << std::endl;
			}
			catch (std::exception & e)
			{
				executeStatus = ExecuteStatus(6, e.what());
			}
			catch (const std::string & e)
			{
				executeStatus = ExecuteStatus(6, e);
			}
			catch (...)
			{
				executeStatus = ExecuteStatus(4, "unknown error");
			}
		}
	}

	notifyExecuteStatus(executeStatus);
	return executeStatus.code;
}

namespace {

	void setupTrace(const XMLElement &root) {
		const tinyxml2::XMLElement *trace = root.FirstChildElement("trace");
		if (trace != nullptr) {
			using vcell_xml::convertChildElementWithDefault;
			std::string tracefilename = convertChildElementWithDefault<std::string>(*trace,"traceFilename", "");
			std::string levelStr = convertChildElementWithDefault<std::string>(*trace,"level","fatal");

			using vcell_util::Logger;
			Logger & logger = Logger::get( );
			if (tracefilename.empty())
			{
				vcell_util::StdoutDest* stdoutDest = new vcell_util::StdoutDest();
				logger.setDestination(*stdoutDest);
			}
			else
			{
				traceFileDestination.reset(new vcell_util::FileDest(tracefilename.c_str( )) );
				logger.setDestination(*traceFileDestination);
			}
			Logger::Level level = Logger::readLevel(levelStr.c_str( ));
			logger.set(level);
			//set specific level keys
			const char * const KEYSET = "keyset";
			const tinyxml2::XMLElement *keySet = trace->FirstChildElement(KEYSET);
			while (keySet != nullptr) {
				using vcell_xml::convertChildElement;
				bool enabled = convertChildElement<bool>(*keySet,"enabled");
				std::string key = convertChildElement<std::string>(*keySet,"key");
				Logger::Key k = Logger::readKey(key.c_str( ));
				logger.set(k,enabled);
				keySet = keySet->NextSiblingElement(KEYSET);
			}
		}
	}

	void setupMatlabDebug(const XMLElement &root) {
		const tinyxml2::XMLElement *trace = root.FirstChildElement("matlabDebug");
		if (trace != nullptr) {
			const char * const TOKEN = "token";
			const tinyxml2::XMLElement *token = trace->FirstChildElement(TOKEN);
			if (token != nullptr) {
				std::ofstream *out = new std::ofstream("mdebug.m");
				matlabBridge::MatLabDebug::setDebug(*out);
			}
			while (token != nullptr) {
				using vcell_xml::convertChildElement;
				const std::string spec = vcell_xml::convertElement<std::string>(*token);
				matlabBridge::MatLabDebug::activate(spec);
				token = token->NextSiblingElement(TOKEN);
			}
		}
	}
	void setupProgress(const XMLElement &root,moving_boundary::MovingBoundaryParabolicProblem & mbpp) {
		const tinyxml2::XMLElement *prog = root.FirstChildElement("progress");
		if (prog != nullptr) {
			const unsigned short percent = vcell_xml::convertChildElement<unsigned short>(*prog,"percent");
			const bool estimateProgress =vcell_xml::convertChildElementWithDefault<bool>(*prog,"estimateProgress",false);
			mbpp.reportProgress(percent,estimateProgress);
		}
	}
}


