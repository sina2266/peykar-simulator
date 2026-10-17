#include <sstream>
#include <vcellstring.h>
using std::string;
namespace vcell_util
{
std::string convertNonPrintable(const std::string &source) {
	string rval;
	rval.reserve(source.size( ) * 4); //near worse case each character replaced with \0x--
	for (string::const_iterator iter = source.begin( ); iter != source.end( ); ++iter) {
		unsigned char c = *iter;
		if (isprint(c)) {
			rval.push_back(c);
		}
		else {
			std::ostringstream oss;
			oss << '<' << std::hex << static_cast<unsigned int>(c) << '>'; 
			rval.append(oss.str( ));
		}
	}
	return rval;
}

bool endsWith(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size())
	{
		return false;
	}
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

}
