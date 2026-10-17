#include <iostream>
#include <cstdlib>
/**
* on GCC, it appears some static initialization is corrupting the heap. This test file 
* is used to track that down.
*/

/*
#include <vcellxml.h>
#include <vhdf5/file.h>
#include <MovingBoundaryParabolicProblem.h>
#include <HDF5Client.h>
#include <Logger.h>
#include <boundaryProviders.h>
#include <MBridge/MatlabDebug.h>
using tinyxml2::XMLElement;
*/
using namespace std;
struct B;
struct C;
class A {
	friend struct B;
	void call(B *) {
		cout << "B" << endl;
	}
	void call(C *) {
		cout << "C" << endl;
	}

};
struct B {
	virtual void f(A &a) {
		a.call(this);
	}
};
struct C : public B {
	virtual void f(A &a) {
		B::f(a);
	}
};

int main(int argc, char *argv[])
{
	A a;
	B b;
	C c;
	b.f(a);
	c.f(a);

	size_t s = 256600;
	if (argc > 1) {
		s = std::strtoul(argv[1],nullptr,10);
	}
	std::cout << "allocating " << s << std::endl;
	void * raw = malloc(s);
	free(raw);
	std::cout << "allocation compete" << std::endl;
	return 0;
}
