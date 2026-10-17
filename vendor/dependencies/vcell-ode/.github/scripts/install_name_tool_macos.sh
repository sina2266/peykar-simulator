#!/bin/bash

shopt -s -o nounset

#
# change path to bundled dylibs in executables from absolute path (usr/local/lib/) to same directory
#
for exe in `ls *_x64`
do
    echo "fixing paths in ${exe}"
    for libpath in `otool -L ${exe} | grep "/" | grep -v "/usr/lib" | grep -v "System" | awk '{print $1}'`
    do
		libfilename=${libpath##*/}
		echo install_name_tool -change $libpath @executable_path/$libfilename  $exe
		install_name_tool -change $libpath @executable_path/$libfilename  $exe
		cp libpath "$(pwd)"
  	done
done

#
# change path from each dylib to each other (dylibs can refer to other dylibs) from /usr/local/lib to @loader_path (same directory)
# if there is not dependency, then ignore the errors.
#
for libfilename in `ls *.dylib`
do
    echo "fixing paths in ${libfilename}"
	#
	# set id of dynamic library to @loader_path/libname
	#
	echo install_name_tool -id "@loader_path/$libfilename"  $libfilename
    install_name_tool -id "@loader_path/$libfilename"  $libfilename
  	
	for dependentlibpath in `otool -L ${libfilename} | grep "/" | grep -v "/usr/lib" | grep -v "System" | awk '{print $1}'`
	do
		dependentlibfilename=${dependentlibpath##*/}
		#
		# for each library, change path to dependent libraries to be based on @loader_path
		#
		echo install_name_tool -change $dependentlibpath @loader_path/$dependentlibfilename  $libfilename
		install_name_tool -change $dependentlibpath @loader_path/$dependentlibfilename  $libfilename
  	done
done
