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
echo "Testing BNET compatibility"
rm -rf tmp; mkdir -p tmp
$LAUNCHER /usr/bin/time -p $MABOSS bnet/model.bnet -o tmp/bnet_model
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py bnet/refer/bnet_model_probtraj.csv tmp/bnet_model_probtraj.csv --exact
check_file "projtraj"

$LAUNCHER /usr/bin/time -p $MABOSS bnet/model_noheader.bnet -o tmp/bnet_model
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py bnet/refer/bnet_model_probtraj.csv tmp/bnet_model_probtraj.csv --exact
check_file "projtraj"

exit $return_code