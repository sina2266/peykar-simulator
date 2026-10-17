#include <JobMessaging.h>

namespace {
	bool enabled = false;
}

namespace moving_boundary {

	void enableJobMessaging( ) {
		enabled = true;
		SimulationMessaging::getInstVar( ); //create the singleton and its worker thread up front
	}

	SimulationMessaging * jobMessaging( ) {
		return enabled ? SimulationMessaging::getInstVar( ) : nullptr;
	}
}
