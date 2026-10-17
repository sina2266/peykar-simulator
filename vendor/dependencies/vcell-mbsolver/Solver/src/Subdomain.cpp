#include <Subdomain.h>
#include <cstring>
using moving_boundary::Subdomain;

Subdomain::Subdomain(const string & name, Physiology* a_physiology)
				:name_(name),
				 physiology(a_physiology)
{
}

Subdomain::~Subdomain()
{

}
