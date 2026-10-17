#ifndef matlabAssert_h
#define matlabAssert_h
namespace matlabLink {
	void mlAssert(bool condition,const char *message, const char * file, unsigned int line);
}
#define ML_ASSERT(x) matlabLink::mlAssert(x,#x,__FILE__,__LINE__);

#endif
