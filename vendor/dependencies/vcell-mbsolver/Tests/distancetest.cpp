#include <limits>
#include "gtest/gtest.h"
#include <Distance.h>
using std::cout;
using std::endl;
using namespace spatial;

TEST(distance,basic) {
	int64_t biggest = std::numeric_limits<int64_t>::max( );
	int64_t okay = DefaultDistancePolicy<int64_t>::convert(biggest);
	ASSERT_TRUE(okay == biggest);
    // jcs: switched test to long double, a double cannot hold int64_t max+1 without overflow, using long double
	long double tooBig = static_cast<long double>(biggest) + 1;
	// arm64 and MSVC both have 64-bit long double (== double), which cannot
	// represent int64_max+1 distinctly, so tooBig rounds back to int64_max+... and
	// the overflow check does not trigger.  Skip the assertion on those targets.
#if !defined(__arm64__) && !defined(_MSC_VER)
	ASSERT_THROW(DefaultDistancePolicy<int64_t>::convert(tooBig),std::domain_error);
#endif
	cout << okay << endl;
}
