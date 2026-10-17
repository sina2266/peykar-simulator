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

echo
echo "Non regression test: Cell Cycle one thread"
rm -rf tmp; mkdir -p tmp
/usr/bin/time -p $LAUNCHER $MABOSS prngs/cellcycle.bnd -c prngs/cellcycle_runcfg.cfg -c prngs/glibc.cfg -o tmp/Cell_cycle_glibc
if [ $? != 0 ]; then return_code=1; fi
python compare_probtrajs.py prngs/refer/Cell_cycle_glibc_probtraj.csv tmp/Cell_cycle_glibc_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py prngs/refer/Cell_cycle_glibc_statdist.csv tmp/Cell_cycle_glibc_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"

/usr/bin/time -p $LAUNCHER $MABOSS prngs/cellcycle.bnd -c prngs/cellcycle_runcfg.cfg -c prngs/mt.cfg -o tmp/Cell_cycle_mt
if [ $? != 0 ]; then return_code=1; fi
python compare_probtrajs.py prngs/refer/Cell_cycle_mt_probtraj.csv tmp/Cell_cycle_mt_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py prngs/refer/Cell_cycle_mt_statdist.csv tmp/Cell_cycle_mt_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"

/usr/bin/time -p $LAUNCHER $MABOSS prngs/cellcycle.bnd -c prngs/cellcycle_runcfg.cfg -c prngs/rand48.cfg -o tmp/Cell_cycle_rand48
if [ $? != 0 ]; then return_code=1; fi
python compare_probtrajs.py prngs/refer/Cell_cycle_rand48_probtraj.csv tmp/Cell_cycle_rand48_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py prngs/refer/Cell_cycle_rand48_statdist.csv tmp/Cell_cycle_rand48_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"

# echo "Comparing aproximatively the results of the different prngs"
# python compare_probtrajs.py tmp/Cell_cycle_glibc_probtraj.csv tmp/Cell_cycle_mt_probtraj.csv # || echo '**** error test #3 (multi threads) ****'
# check_file "probtrajs"
# python compare_probtrajs.py tmp/Cell_cycle_glibc_probtraj.csv tmp/Cell_cycle_phys_probtraj.csv # || echo '**** error test #3 (multi threads) ****'
# check_file "probtrajs"

exit $return_code
