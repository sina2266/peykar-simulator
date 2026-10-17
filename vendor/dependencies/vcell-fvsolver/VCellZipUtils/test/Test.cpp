/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <iostream>
#include <sstream>
using std::stringstream;
using std::cout;
using std::endl;
#include <stdlib.h>

#include <VCELL/ZipUtils.h>
#include <cstring>

#include <zip.h>


#if ( !defined(WIN32) && !defined(WIN64) ) // UNIX
#include <unistd.h>
#endif

int main (int argc, char *argv[]) {

	if (argc != 4){
		cout << "usage: " << argv[0] << " [append|extract] file ziparchive " << endl;
		exit(1);
	}

	const char* operation = argv[1];
	const char* file = argv[2];
	const char* ziparchive = argv[3];
	try {
		if (argc == 4 && std::strcmp(argv[1],"append")==0){
			addFilesToZip(ziparchive, file);
		}else if (argc == 4 && std::strcmp(argv[1],"extract")==0){
			extractFileFromZip(ziparchive, file);
		}
	} catch (char const* str) {
        std::cerr << "Exception: " << str << std::endl;
        return (-1);
	} catch (std::exception& exc) {
		std::cerr << exc.what() << std::endl;
		return (-1);
	}
	return (0);
}
