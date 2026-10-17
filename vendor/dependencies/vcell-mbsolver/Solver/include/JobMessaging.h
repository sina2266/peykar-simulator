#ifndef JobMessaging_h
#define JobMessaging_h
#include <VCELL/SimulationMessaging.h>
namespace moving_boundary {

	/**
	* Job-status messaging is opt-in.
	*
	* SimulationMessaging::getInstVar( ) lazily creates the singleton -- it never
	* returns null -- and the singleton reports to stdout by default.  Solver code
	* therefore has to ask before it emits worker events, otherwise embedders (the
	* Python extension, the test suites) get progress chatter on their streams.
	* Only the command line driver, which is what VCell drives as a job, opts in.
	*/
	void enableJobMessaging( );

	/**
	* @return the messaging singleton, or null when #enableJobMessaging has not been called
	*/
	SimulationMessaging * jobMessaging( );
}
#endif
