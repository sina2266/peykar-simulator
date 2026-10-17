#include "StoppedByUserException.h"

StoppedByUserException::StoppedByUserException(const string& msg)
	: Exception("StoppedByUserException: " + msg){}

StoppedByUserException::~StoppedByUserException() noexcept = default;
