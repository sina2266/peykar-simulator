#ifndef IOEXCEPTION_H
#define IOEXCEPTION_H

#include "Exception.h"
using namespace VCell;

class IOException : public Exception
{
public:
	IOException(string msg);
	~IOException(void) throw( );
};
#endif
