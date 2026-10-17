#ifndef VolumeSubdomain_h
#define VolumeSubdomain_h
#include <string>
#include <Subdomain.h>
using std::string;

namespace moving_boundary
{
	struct VolumeSubdomain : public Subdomain
	{
		VolumeSubdomain(const string & name, Physiology* physiology);
		virtual ~VolumeSubdomain();

		SubdomainType getType()
		{
			return subdomain_volume;
		}

	};
}

#endif
