#include <VolumeVariable.h>
#include <cstring>
using moving_boundary::VolumeVariable;

VolumeVariable::VolumeVariable(const string & name)
				:Variable(name)
{
}

VolumeVariable::~VolumeVariable()
{
}

