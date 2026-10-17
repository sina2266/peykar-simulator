#ifndef StateClient_h
#define StateClient_h
#include <ReportClient.h>
#include <MovingBoundaryParabolicProblem.h>
namespace moving_boundary {

	/**
	* client which periodically records state. Currently includes direct reference to ReportClient -- more general solution would use factory for this
	* but this is deferred per YAGNI ("You ain't gonna need it")
	*/
	struct StateClient : public MovingBoundaryTimeClient {
		/**
		* @param name used to generate names of output files
		* @param startTime when to save first state 
		* @param increment how often to save after that 
		*/
		StateClient(MovingBoundaryParabolicProblem & problem, const ReportClient & reportClient, const std::string & name, double startTime, double increment);

		virtual std::string outputName( ) const
		{
			return "";
		}
		virtual void time(double t, unsigned int generationCount, bool last, const GeometryInfo<moving_boundary::CoordinateType> & geometryInfo); 
		virtual void simulationComplete( );
		struct ProblemState {
			MovingBoundaryParabolicProblem problem;
			ReportClient *reportClient;
			StateClient *stateClient;
		};
		/**
		* restore previously persisted problem and clients from file. Caller must delete ReportClient and State Client
		*/
		static ProblemState restore(const std::string &name);

		 /**
		 * @param root document root containing problem section
		 * @param filename optional override of file name in XML, may be empty string
		 * @param mbpp problem to report on
		 */
	 static StateClient *setup(const tinyxml2::XMLElement &root, MovingBoundaryParabolicProblem &mbpp,const ReportClient & reportClient);

		/**
		* version number for tracking format
		*/
		static const int MB_VERSION = 4;
	private:
		StateClient(MovingBoundaryParabolicProblem & problem, const std::string & reportClientXML, std::istream &); 
		static void registerTypes( );

		MovingBoundaryParabolicProblem  problem;
		const std::string reportClientXML; 
		std::string baseName;
		double startTime;
		double lastTimeSaved;
		double timeIncrement; 
		/**
		* number digits for output filenames
		*/
		unsigned char requiredDigits;
		/**
		* how many digits after decimal
		*/
		static const unsigned char afterDecimal = 5;
		/**
		* type used for xml string length
		*/
		typedef unsigned short XMLLengthType;
	};
}
#endif
