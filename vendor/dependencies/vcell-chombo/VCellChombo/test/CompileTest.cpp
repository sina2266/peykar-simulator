#include <iostream>
#include <EBIndexSpace.H>
//minimum program to test access to Chombo singleton
int main( ) {
	EBIndexSpace* ebisPtr = Chombo_EBIS::instance();
	EBIndexSpace* ebisPtr2 = Chombo_EBIS::instance();
	if (ebisPtr != ebisPtr2) {
		std::cout << "Error: pointer " << ebisPtr << " != " << ebisPtr2 << std::endl;
	}
	std::cout << "end of test" << std::endl;
}
