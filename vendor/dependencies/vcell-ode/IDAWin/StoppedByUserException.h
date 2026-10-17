#ifndef STOPPEDBYUSEREXCEPTION_H
#define STOPPEDBYUSEREXCEPTION_H

#include <Exception.h>

class StoppedByUserException final : public VCell::Exception {
public:
	explicit StoppedByUserException(const string& msg);
	~StoppedByUserException() noexcept override;
};

#endif
