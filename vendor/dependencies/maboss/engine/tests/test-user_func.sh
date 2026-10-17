#!/bin/sh
#
# test1.sh
#

. `dirname $0`/share.sh

return_code=0

check_file()
{
    if [ $? = 0 ]; then
	echo "File $1 OK"
    else
	echo "File $1 ** error: differences found **"
    return_code=1
    fi
}

cp user_func/user_func.cc ../src
cd ../src
make clean; make FUNC_MODULE=user_func maboss
if [ $? != 0 ]; then exit 1; fi
cd ../tests

rm -rf tmp; mkdir -p tmp
cp ../src/user_func.so tmp/user_func.so

echo
echo "Cell Cycle 1 threads, with user func"
/usr/bin/time -p $LAUNCHER $MABOSS --load-user-functions tmp/user_func.so user_func/cellcycle.bnd -c user_func/cellcycle_runcfg.cfg -c user_func/cellcycle_runcfg-thread_1.cfg -o tmp/Cell_cycle_thread_1
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py user_func/refer/Cell_cycle_thread_1_probtraj.csv tmp/Cell_cycle_thread_1_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py user_func/refer/Cell_cycle_thread_1_statdist.csv tmp/Cell_cycle_thread_1_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"

if [ "$ONE_THREAD_ONLY" != "" ]; then exit 0; fi

echo
echo "Cell Cycle 6 threads, with user func"
/usr/bin/time -p $LAUNCHER $MABOSS --load-user-functions tmp/user_func.so user_func/cellcycle.bnd -c user_func/cellcycle_runcfg.cfg -c user_func/cellcycle_runcfg-thread_6.cfg -o tmp/Cell_cycle_thread_6

python compare_probtrajs.py user_func/refer/Cell_cycle_thread_6_probtraj.csv tmp/Cell_cycle_thread_6_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py user_func/refer/Cell_cycle_thread_6_statdist.csv tmp/Cell_cycle_thread_6_statdist.csv --exact #|| echo '**** error test #2.b (non regression) ****'
check_file "statdist"

echo
echo "Non regression test: checking differences between one and 6 threads results"
python compare_probtrajs.py tmp/Cell_cycle_thread_1_probtraj.csv tmp/Cell_cycle_thread_6_probtraj.csv # || echo '**** error test #3 (multi threads) ****'
check_file "probtrajs"
python compare_statdist.py tmp/Cell_cycle_thread_1_statdist.csv tmp/Cell_cycle_thread_6_statdist.csv --exact # || echo '**** error test #3 (multi threads) ****'
check_file "statdist"

exit $return_code
