static library for MATLAB evaluation and "mex" files for compiling mexw32 and mexw64 matlab callable libraries

*cpm files are not compiled into library; there are copied/renamed into *cpp files in the CMake binary directory
for compiling into mexw.. libaries with build.m script.

buildnn.opts files are same are copies of option files from MATLAB "mexopts" directory with compile switch changed 
from /MD to /MTd to resolve link issues with Visual Studio compiled static library
