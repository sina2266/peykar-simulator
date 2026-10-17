#include <VCellAllocator.h>
#include <cassert>
#include <vector>
#include <iterator>
#include <cstdlib>
#include "gtest/gtest.h"
using std::cout;
using std::endl;

namespace {
	size_t indext = 0;
	int buffer[100000];
	typedef std::allocator<int> A;
	const size_t ALIGN = 16;
}

template<class T, int max>
class MyAlloc {
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	T first[max + 1];
	T second[max + 1];
	T * current;


public:
	MyAlloc( ) 
		:current(second) {}


	void * allocate(const size_type n, void *hint)
	{  
		std::cout << "a " << n << std::endl; 
#		ifndef NDEBUG 
		//MS does some dummy allocation in debug builds
		if (n == 1) {
			T * ptr = static_cast<T *>(malloc(sizeof(T)) ); 
			std::cout << "new " << ptr << std::endl;
			return ptr;
		}
#		endif
		assert(n <= max); 
		if (current == second) {
			current = first;
			std::cout << "a first " << std::endl;
		}
		else {
			current = second;
			std::cout << "a second " << std::endl;
		}
		return current;

#if 0
		static char * start = reinterpret_cast<char *>(buffer);
		char *ptr = start + indext; 
		std::cout << "a " << bytes << " from " << static_cast<void *>(ptr) << std::endl; 
		indext += bytes;
		size_t pad = indext%ALIGN;
		indext += ALIGN - pad;
		assert(indext%ALIGN == 0);
		return ptr;
#endif
	}
	void free(void * const block, size_t n)
	{
	}
};

TEST(alloc,basic) {
	MyAlloc<int,12> source;
	typedef vcell_util::VCellAllocator<int,MyAlloc<int,12> > A;
	const A a(&source);
	{
		std::vector<int, A> demo(a);
		demo.push_back(5);
		demo.push_back(4);
		demo.reserve(5);
		demo.push_back(3);
		demo.push_back(2);
		demo.push_back(1);
		demo.reserve(8);
		demo.reserve(10);
		std::ostream_iterator<int> iout(std::cout,",");
		std::copy(demo.begin( ),demo.end( ),iout);
	}

}
namespace {
	struct NoDef {
		NoDef(int x_, int y_)
			:x(x_),
			y(y_) {}

		int x;
		int y;
	};
	std::ostream &operator<<(std::ostream & os, const NoDef & nd) {
		os << nd.x << ',' << nd.y;
		return os;
	}
}
TEST(alloc,vt) {
	using namespace std;
	std::vector<NoDef> nds;
	for (int i = 0; i < 20; i++) {
		nds.push_back(NoDef(i, i + 3));
		if (nds.capacity( ) != nds.size( ) ) {
			cout << nds.capacity( ) << ',' << nds.size( ) << endl;
		}
	}
	/*
	std::vector<int> ic;
	for (int i = 0; i < 1000; i++) {
		ic.push_back(i);
		if (ic.capacity( ) != ic.size( ) ) {
			cout << ic.capacity( ) << ',' << ic.size( ) << endl;
		}
	}
	*/
	
}
