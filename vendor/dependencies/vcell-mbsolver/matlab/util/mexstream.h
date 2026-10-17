#ifndef mexstream_h
#define mexstream_h
#include <mex.h>
namespace matlabLink {
	/**
	* ostream implementation which prints to MATLAB on sync
	*/
	struct mexStream: public std::ostream
	{
		mexStream( )
			:std::ostream(&buffer),
			buffer( ) {}
	private:
		class mexBuf: public std::stringbuf
		{
		public:
			mexBuf( ) {}

			virtual int sync ( )
			{
				std::string s = str( );
				mexPrintf("%s",s.c_str( ));
				pubseekpos(0);
				return 0;
			}
		};

		mexBuf buffer;
	};
}

#endif
