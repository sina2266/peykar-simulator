
#include <cassert>
#include <algorithm>
#include <gtest/gtest.h>
#include <cstdlib>
#include <limits>
#include <NumericConvert.h>
using std::cout;
using std::endl;
using namespace vcell_util; 

TEST(vcell_util,convert) {
	ASSERT_TRUE(ConvertUp<int>(4.1) == 5);
	ASSERT_TRUE(ConvertDown<int>(4.1) == 4);
	ASSERT_TRUE(ConvertUp<int>(-4.1) == -4);
	ASSERT_TRUE(ConvertDown<int>(-4.1) == -5);

	ASSERT_TRUE(ConvertUp<long>(4.1) == 5);
	ASSERT_TRUE(ConvertDown<long>(4.1) == 4);
	ASSERT_TRUE(ConvertUp<long>(-4.1) == -4);
	ASSERT_TRUE(ConvertDown<long>(-4.1) == -5);

	ASSERT_TRUE(ConvertUp<double>(4.1) == 4.1);
	ASSERT_TRUE(ConvertDown<double>(4.1) == 4.1);
}
TEST(vcell_util,valid) {
	short maxS = std::numeric_limits<short>::max( );
	short f1 = 30;
	short f2 = maxS / f1;
	ASSERT_TRUE(validMultiply<short>(f1,f2));
	++f2;
	ASSERT_FALSE(validMultiply<short>(f1,f2));

}
//namespace {
//	void testc(const char *c) {
//		char *end;
//		unsigned long ul = std::strtoul(c,&end,10);
//		int len = end - c;
//		cout << "in " << c << " out " << ul << " length " << len << endl;
//	}
//}
//TEST(vcell_util,conversion) {
//	testc("4");
//	testc("3dognight");
//	testc("-4");
//	testc("cat");
//}
