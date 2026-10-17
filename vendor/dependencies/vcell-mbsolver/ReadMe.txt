Directory structure:

main libraries:
boost_1_53_0		:boost libraries; used for Voronoi determination
ExpressionParser	:Vcell expression parser for implementing front and velocity equations
FronTierLib			:FronTier library from http://frontier.ams.sunysb.edu/download/download.php (07/06/09)
VCellFronTier		:VCell/C++ implementation of Igor's work (in progress)
	requires libraries above.

test libraries:
gtest-1.6.0			:google c++ library
VCellFronTierTest	:unit test cases for VCellFronTier; requires gtest and main libraries 

matlabLink			:static library to support building MATLAB mex modules. requires main libraries 
		

CMakeLists.txt
	CMake configuration file. main and VCellFronTierTest may be compiled with VS 2010 or VS 2012
		matlabLink must be compiled with VS 2010; use "Visual Studio 10" cmake generator for 32 bit MATLAB
		and "Visual Studio 10 Win65" for 64 bit MATLAB 
