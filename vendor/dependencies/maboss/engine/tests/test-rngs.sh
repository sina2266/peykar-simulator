#!/bin/sh
#
# test1.sh
#

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

rm -rf tmp; mkdir -p tmp

$LAUNCHER /usr/bin/time -p $MABOSS prngs/cellcycle.bnd -c prngs/cellcycle_runcfg.cfg -c prngs/physical.cfg -o tmp/Cell_cycle_phys
if [ $? != 0 ]; then return_code=1; fi

# echo "Comparing aproximatively the results of the different prngs"
# python compare_probtrajs.py tmp/Cell_cycle_glibc_probtraj.csv tmp/Cell_cycle_mt_probtraj.csv # || echo '**** error test #3 (multi threads) ****'
# check_file "probtrajs"
# python compare_probtrajs.py tmp/Cell_cycle_glibc_probtraj.csv tmp/Cell_cycle_phys_probtraj.csv # || echo '**** error test #3 (multi threads) ****'
# check_file "probtrajs"

exit $return_code