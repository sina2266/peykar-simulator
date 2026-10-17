#include <stdexcept>
#include "gtest/gtest.h"
#include <VCellChrono.h>

using namespace vcell_util;
namespace HMSFormat = HoursMinutesSecondsFormat;
typedef HoursMinutesSeconds<std::chrono::seconds> HMSdef;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::FIXED> HMSfixed;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::ALL> HMSall;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::FIXED|HMSFormat::ALL> HMSfixedall;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::FIXED|HMSFormat::SHORT_UNITS> HMSfixedshortu;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::FIXED|HMSFormat::LONG_UNITS> HMSfixedlongu;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::ALL> HMSall;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::ALL|HMSFormat::SHORT_UNITS> HMSallshortu;
typedef HoursMinutesSeconds<std::chrono::seconds, HMSFormat::ALL|HMSFormat::LONG_UNITS> HMSall_longu;
namespace {


	void demo(const std::chrono::seconds & s) {
		using namespace std::chrono;
		std::cout << HMSdef(s) << std::endl;
		std::cout << HMSfixed(s) << std::endl;
		std::cout << HMSall(s) << std::endl;
		std::cout << HMSfixedall(s) << std::endl;
		std::cout << HMSfixedshortu(s) << std::endl;
		std::cout << HMSfixedlongu(s) << std::endl;
		std::cout << HMSallshortu(s) << std::endl;
		std::cout << HMSall_longu(s) << std::endl;
	}

}

TEST(chrono,hms) {
	using namespace std::chrono;
	seconds s(3661);
	demo(s);
	seconds two(125);
	demo(two);
	seconds three(37);
	demo(three);

	milliseconds ms(4357);
	std::cout << HoursMinutesSeconds<milliseconds,1>(ms) << std::endl;
	nanoseconds ns(1);
	std::cout << HoursMinutesSeconds<nanoseconds,2>(ns) << std::endl;
	

}
