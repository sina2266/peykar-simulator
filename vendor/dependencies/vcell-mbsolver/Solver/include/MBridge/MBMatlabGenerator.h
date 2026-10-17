#ifndef MBMatlabGenerator_h
#define MBMatlabGenerator_h
#include <vector>
#include <iostream>
#include <sstream>
namespace matlabBridge {

	/**
	* base class "mixin"
	* generates sequential names, if sequenceNumber non-zero
	*/
	struct MatlabGenerator {
		protected:
			MatlabGenerator(const char *baseName, int sequenceNumber_ = 0)
				:base(baseName),
				sequenceNumber(sequenceNumber_)
			{}

		const std::string vName( ) const {
			if (sequenceNumber == 0) {
				return base; 
			}
			std::stringstream n;
			n << base << sequenceNumber;
			return n.str( );
		}
		int sequenceNumber;
		private:
			const std::string base;
	};
}
#endif
