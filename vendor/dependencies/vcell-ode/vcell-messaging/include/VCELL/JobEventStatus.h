//
// Created by Logan Drescher on 11/25/25.
//

#ifndef VCELL_ODE_NUMERICS_JOBEVENTSTATUS_H
#define VCELL_ODE_NUMERICS_JOBEVENTSTATUS_H
#include <string>
// enum JobEventStatus {
// 	JOB_STARTING = 999,
// 	JOB_DATA = 1000,
// 	JOB_PROGRESS = 1001,
// 	JOB_COMPLETED = 1003,
// 	JOB_FAILURE = 1002,
// 	JOB_ALIVE = 1004,
// };

namespace JobEvent {
	enum Status {
		JOB_STARTING = 999,
		JOB_DATA = 1000,
		JOB_PROGRESS = 1001,
		JOB_COMPLETED = 1003,
		JOB_FAILURE = 1002,
		JOB_ALIVE = 1004,
	};
	std::string toString(Status status);
}

#endif //VCELL_ODE_NUMERICS_JOBEVENTSTATUS_H