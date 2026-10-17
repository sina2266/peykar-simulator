#!/bin/sh
#
# test1.sh
#

. `dirname $0`/share.sh

return_code=0

# if [ ! -x $MABOSS ]
# then
#     echo $MABOSS not found
#     exit 1
# fi

if [ ! -x $MABOSS_128n ]
then
    echo $MABOSS_128n not found
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
echo "Non regression test: Ewing one thread"
rm -rf tmp; mkdir -p tmp
/usr/bin/time -p $LAUNCHER $MABOSS_128n ewing/ewing_full.bnd -c ewing/ewing.cfg -c ewing/ewing_runcfg-thread_1.cfg -o tmp/ewing_thread_1
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py ewing/refer/ewing_thread_1_probtraj.csv tmp/ewing_thread_1_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py ewing/refer/ewing_thread_1_statdist.csv tmp/ewing_thread_1_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"

python compare_fixpoints.py ewing/refer/ewing_thread_1_fp.csv tmp/ewing_thread_1_fp.csv
check_file "fixpoints"
    
if [ "$ONE_THREAD_ONLY" != "" ]; then exit 0; fi

echo
echo "Non regression test: Ewing 6 threads"
/usr/bin/time -p $LAUNCHER $MABOSS_128n ewing/ewing_full.bnd -c ewing/ewing.cfg -c ewing/ewing_runcfg-thread_6.cfg -o tmp/ewing_thread_6
if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py ewing/refer/ewing_thread_6_probtraj.csv tmp/ewing_thread_6_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py ewing/refer/ewing_thread_6_statdist.csv tmp/ewing_thread_6_statdist.csv --exact #|| echo '**** error test #2.b (non regression) ****'
check_file "statdist"

python compare_fixpoints.py ewing/refer/ewing_thread_6_fp.csv tmp/ewing_thread_6_fp.csv
check_file "fixpoints"

echo
echo "Non regression test: checking differences between one and 6 threads results"
python compare_probtrajs.py tmp/ewing_thread_1_probtraj.csv tmp/ewing_thread_6_probtraj.csv # || echo '**** error test #3 (multi threads) ****'
check_file "probtrajs"
python compare_statdist.py tmp/ewing_thread_1_statdist.csv tmp/ewing_thread_6_statdist.csv --exact # || echo '**** error test #3 (multi threads) ****'
check_file "statdist"

exit $return_code
