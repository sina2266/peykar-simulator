#include <StateClient.h>
#include <limits>
#include <fstream>
#include <iomanip>
#include <NumericConvert.h>
#include <vcellxml.h>
using namespace moving_boundary;
namespace {
	const double NEVER_SAVED = std::numeric_limits<double>::min( );
}

/**
* create, add to prb
* @param prb problem to solve; this automatically added to it
* @param rc reportClient 
* @param name base name of output files 
* @param startTime_ when to start recording state files
* @param increment how often after start to record 
*/
StateClient::StateClient(MovingBoundaryParabolicProblem & prb, const ReportClient & rc, const std::string & name, double startTime_, double increment)
	:problem(prb),
	reportClientXML(rc.getXML( )),
	baseName(name),
	startTime(startTime_),
	timeIncrement(increment),
	lastTimeSaved(NEVER_SAVED),
	requiredDigits( ){
		registerTypes( );
		problem.registerInstanceType( );
		problem.add(*this);
		auto endInt = static_cast<unsigned long>(problem.endTime( ));
		requiredDigits = vcell_util::numberDigits(endInt) + afterDecimal + 1;
}

StateClient::StateClient(MovingBoundaryParabolicProblem & prb, const std::string & rcxml, std::istream &in) 
	:problem(prb),
	reportClientXML(rcxml),
	baseName(vcell_persist::StdString<>::restore(in)) {
		vcell_persist::binaryRead(in,startTime);
		vcell_persist::binaryRead(in,timeIncrement);
		vcell_persist::binaryRead(in,lastTimeSaved);
		vcell_persist::binaryRead(in,requiredDigits);

		registerTypes( );
		problem.registerInstanceType( );
		problem.add(*this);
}

/**
* use same types on save and restore
*/
void StateClient::registerTypes( ) {
	vcell_persist::Registrar::reg<StateClient>("State Client");
	MovingBoundarySetup::registerType( );
	MovingBoundaryParabolicProblem::registerType( );
	Universe<2>::registerType( );
	World<CoordinateType,2>::registerType( );
}

void StateClient::time(double t, unsigned int generationCount, bool last, const GeometryInfo<moving_boundary::CoordinateType> & geometryInfo){
	if (t > startTime) {
		if (lastTimeSaved == NEVER_SAVED || t - lastTimeSaved >= timeIncrement) { 
			lastTimeSaved = t;
			std::ostringstream oss;
			oss << baseName << std::setfill('0') << std::setw(requiredDigits) << std::fixed << std::setprecision(afterDecimal) << t << std::ends; 
			std::string filename(oss.str());
			std::replace(filename.begin( ), filename.end( ),'.','-');
			filename += ".dat";
			std::ofstream out(filename, std::ios::binary|std::ios::trunc);
			vcell_persist::WriteFormatter wf(out, MB_VERSION, true);

			//write report client XML
			vcell_persist::StdString<XMLLengthType>( ).save(out,reportClientXML);

			problem.setup( ).persist(out);
			Universe<2>::get( ).persist(out);
			World<CoordinateType,2>::get( ).persist(out);
			problem.persist(out);

			//write ourselves
			vcell_persist::StdString<>::save(out,baseName);
			vcell_persist::binaryWrite(out,startTime);
			vcell_persist::binaryWrite(out,timeIncrement);
			vcell_persist::binaryWrite(out,lastTimeSaved);
			vcell_persist::binaryWrite(out,requiredDigits);

		}
	}
}

void StateClient::simulationComplete( ) {}

StateClient::ProblemState StateClient::restore(const std::string & filename) {
	ProblemState rval;
	registerTypes( );
	MovingBoundarySetup::registerType( );
	MovingBoundaryParabolicProblem::registerType( );
	std::ifstream in(filename,std::ios::binary);
	vcell_persist::ReadFormatter rf(in, MB_VERSION);
	std::string xmlString = vcell_persist::StdString<XMLLengthType>( ).restore(in);
	MovingBoundarySetup mbs(in);
	Universe<2>::get( ).restore(in);
	World<CoordinateType,2>::get( ).restore(in);
	rval.problem = MovingBoundaryParabolicProblem(mbs,in);

	tinyxml2::XMLDocument doc;
	doc.Parse(xmlString.c_str( ));
	ReportClient::setup(*doc.RootElement( ),std::string( ),rval.problem);
	rval.stateClient = new StateClient(rval.problem, xmlString, in);
	return rval; 
}

using tinyxml2::XMLElement;
StateClient *StateClient::setup(const tinyxml2::XMLElement &root, MovingBoundaryParabolicProblem &mbpp,const ReportClient & reportClient) {
		const XMLElement *save = vcell_xml::query(root,"save");
		if (save != nullptr) {
			std::string name = vcell_xml::convertChildElement<std::string>(*save,"filenamePrefix");
			const double startTime = vcell_xml::convertChildElement<double>(*save,"startTime");
			const double increment = vcell_xml::convertChildElement<double>(*save,"increment");
			return new StateClient(mbpp,reportClient,name,startTime,increment);
		}
		return nullptr;
 }
