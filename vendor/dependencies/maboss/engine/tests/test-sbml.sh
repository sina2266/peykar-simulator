#!/bin/sh
#
# test-sbml.sh
# (very) basic test of SBML qual compatibility

. `dirname $0`/share.sh

return_code=0

if [ ! -x $MABOSS ]
then
    echo $MABOSS not found
    exit 1
fi

check_file()
{
    # echo
    if [ $? = 0 ]; then
	echo "File $1 OK"
    else
	echo "File $1 ** error: differences found **"
    return_code=1
    fi
}

echo
echo "Testing SBML compatibility"
rm -rf tmp; mkdir -p tmp
$LAUNCHER /usr/bin/time -p $MABOSS sbml/cell_fate.sbml -c sbml/cell_fate.cfg -o tmp/sbml_cell_fate
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py sbml/refer/cell_fate_probtraj.csv tmp/sbml_cell_fate_probtraj.csv --exact
check_file "projtraj"

$LAUNCHER /usr/bin/time -p $MABOSS sbml/cell_fate.bnd -c sbml/cell_fate.bnd.cfg -o tmp/cell_fate
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py tmp/sbml_cell_fate_probtraj.csv tmp/cell_fate_probtraj.csv --exact
check_file "projtraj"

exit $return_code