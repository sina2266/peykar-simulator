#include "IOException.h"

IOException::IOException(string msg) : Exception("IOException: " + msg)
{	
}

IOException::~IOException(void) throw( )
{
}
