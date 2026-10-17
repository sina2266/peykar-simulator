#!/bin/bash
#
# make-package.sh
#
# Author: Eric Viara Institut Curie(c) copyright 2017
# Date: Jan 2017
#

set -e

topdir=$(pwd)
cd engine/src
package=$(make package | grep "^Package:" | awk '{print $2}')

packname=MaBoSS-env-$(echo $package | cut -d'-' -f2 | sed s/.tgz// )
packdir=/tmp/$packname

mkdir $packdir

trap "rm -r $packdir" 0 1 2 3

cd $packdir
tar xvfz $package
mv MaBoSS-*/ engine
rm engine/README
mkdir -p engine/pub
mv engine/binaries binaries
mv engine/doc doc
mv engine/examples examples

cd $topdir
cp MaBoSS.env README.md check-requirements.sh $packdir

set +e
find tools ! -name \*~ | grep -v "/\." | grep -v /doc/ | cpio -pdmv $packdir

cd $topdir/tools
find doc ! -name \*~ | egrep -v "/\.|\.docx|\.tex|\.ppt" | cpio -pdmv $packdir

cd $topdir
find tutorial ! -name \*~ | egrep -v "/\.|\.docx|\.tex|\.ppt" | cpio -pdmv $packdir
set -e

cd $packdir
rmdir tools/doc

cd /tmp
echo "${packname}"
tar cvfz $packname.tgz $packname

echo
echo "MaBoSS package: $package"
echo "MaBoSS-env package: /tmp/${packname}.tgz"
