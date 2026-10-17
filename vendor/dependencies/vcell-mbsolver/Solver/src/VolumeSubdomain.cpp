#include <VolumeSubdomain.h>
#include <cstring>
using namespace moving_boundary;

VolumeSubdomain::VolumeSubdomain(const string & name, Physiology* physiology)
				:Subdomain(name, physiology)
{
}

VolumeSubdomain::~VolumeSubdomain()
{

}
