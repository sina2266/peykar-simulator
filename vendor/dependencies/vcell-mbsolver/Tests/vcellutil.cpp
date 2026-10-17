#include "gtest/gtest.h"
#include <limits>
#include <random>
#include <fstream>
#include <vcellutil.h>
#include <NumericConvert.h>
#include <MPoint.h>
#include <vcellstring.h>
#include <Mesh.h>
#include <persistcontainer.h>
#include <Volume.h>
#include <Segment.h>
#include <SVector.h>
#include <boundaryProviders.h>
#include <vcarray.h>
#include <NoChangeSentinel.h>
#include <VCDictionary.h>
#include "mockpoint.inc"
#include <OstreamSpy.h>
#include <iomanip>
using namespace vcell_util;
namespace {
	double dx = 2.0 / 3 - 0.0001;
	double dy = 2.0 / 7 + 0.0001;
	double dz = 14.0 / 5 + 0.0301;
	void binaryOpen(std::ofstream &out, const char *name) {
		using std::ios;
		out.open(name,ios::trunc|ios::binary);
		out.exceptions(ios::badbit|ios::failbit|ios::eofbit);
	}

	void binaryOpen(std::ifstream &in, const char *name) {
		using std::ios;
		in.open(name, ios::binary);
		in.exceptions(ios::badbit|ios::failbit);
	}
}
TEST(vcellutil,digits) {
	using namespace std;
	cout << setfill('0') << setw(8) << fixed << setprecision(5) << 1.23<< endl;
	std::cout << std::numeric_limits<int>::is_integer << std::endl;
	for (int i = 0; i < 103; i++) {
		//std::cout << i << ' ' << vcell_util::numberDigits(i) << std::endl;
	}
	for (double d = 0; d < 11; d += 0.3) {
	 //std::cout << d << ' ' << vcell_util::numberDigits(d) << std::endl;
	}
}

TEST(vcellutil,multiply) {
	ASSERT_TRUE(validMultiply(3,4));
	ASSERT_FALSE(validMultiply<char>(127,127));
}
TEST(vcellutil,vcarray) {
	using vcell_util::vcarray;
	vcarray<double,3> cat;
	vcarray<double,cat.ArraySize> dog;
	static_assert(cat.ArraySize == 3, "size");
	cat[0] = dog[0] = 3;
}
TEST(vcellutil,print) {
	const int spec = 12;
	const double lvalue = 123456789012;
	std::cout << std::setw(spec) << std::setprecision(spec) << lvalue << std::endl;
	std::cout << std::setprecision(spec) << lvalue << std::endl;
	std::cout << std::setw(spec) << lvalue << std::endl;
	std::cout << lvalue << std::endl;
}
TEST(vcellutil,nochange) {
	int x = 3;
	float y = 3.4f;
	double z = 7.1;
	
	auto nc1 = makeSentinel("x",x);
	auto nc2 = makeSentinel("y",y);
	auto nc3 = makeSentinel("z",12,z);
	x = 4;
	z = 7.11;
}
TEST(vcellutil,dictionary) {
	Dictionary<int> tdict("names");
	tdict["bob"] = 3;
	tdict["sally"] = 4;
	std::cout << tdict.options( ) << std::endl;
	std::cout << tdict.get("bob") << std::endl;
    ASSERT_THROW(tdict.get("jane"), std::domain_error);
}



#include <streambuf>
template <typename char_type,
          typename traits = std::char_traits<char_type> >
class basic_teebuf:
    public std::basic_streambuf<char_type, traits>
{
public:
    typedef typename traits::int_type int_type;
    
    basic_teebuf(std::basic_streambuf<char_type, traits> * sb1,
                 std::basic_streambuf<char_type, traits> * sb2)
      : sb1(sb1)
      , sb2(sb2)
    {
    }
    
private:    
    virtual int sync()
    {
        int const r1 = sb1->pubsync();
        int const r2 = sb2->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }
    
    virtual int_type overflow(int_type c)
    {
        int_type const eof = traits::eof();
        
        if (traits::eq_int_type(c, eof))
        {
            return traits::not_eof(c);
        }
        else
        {
            char_type const ch = traits::to_char_type(c);
            int_type const r1 = sb1->sputc(ch);
            int_type const r2 = sb2->sputc(ch);
            
            return
                traits::eq_int_type(r1, eof) ||
                traits::eq_int_type(r2, eof) ? eof : c;
        }
    }
    
private:
    std::basic_streambuf<char_type, traits> * sb1;
    std::basic_streambuf<char_type, traits> * sb2;
};




typedef basic_teebuf<char> teebuf;

TEST(vcellutil,teebuf) {
	std::cout << "Beginning " << std::endl;
	std::ofstream copy("copy.txt");
	teebuf tb(std::cout.rdbuf( ),copy.rdbuf( ));
	std::basic_streambuf<char> * oldbuf = std::cout.rdbuf(&tb);
	std::cout << "Have a very nice pi " << 3.14159 << " day!" << std::endl;
	std::cout.rdbuf(oldbuf);
	std::cout << "eot" << std::endl;

}
namespace {
	struct TestSpy {
		TestSpy(const char *fname ) 
			:record(fname) {}
		std::ofstream record;
		void intercept(char ch) {
			record << ch;
		}


	};
}
TEST(vcellutil,spy1) {
	std::cout << "Beginning spy test" << std::endl;
	using vcell_util::Spybuf;
	{
		TestSpy spy("spy1.txt");
		std::basic_streambuf<char> * outbuf = std::cout.rdbuf();
		Spybuf<TestSpy,char> sb(spy,outbuf);
		std::basic_streambuf<char> * ebuf = std::cout.rdbuf(&sb);
		ASSERT_TRUE(outbuf == ebuf);
		std::cout << "Meet at the donut shop" << std::endl;
		std::cout.rdbuf(outbuf);
	}
	std::cout << " for coffee" << std::endl;
}

TEST(vcellutil,spy) {
	std::cout << "Beginning spy test" << std::endl;
	using vcell_util::OStreamSpy;

	{
		TestSpy spy("spy.txt");
		OStreamSpy<TestSpy, char> ospy(std::cout, spy);

		std::cout << "Down at the corner" << std::endl;
	}
	std::cout << " at the bottom of the street" << std::endl;
}
