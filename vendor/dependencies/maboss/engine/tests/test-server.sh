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
    if [ $? = 0 ]; then
	echo "File $1 OK"
    else
	echo "File $1 ** error: differences found **"
    return_code=1
    fi
}

echo
echo "Server test"
rm -rf tmp; mkdir -p tmp
$MABOSS_SERVER -q --host 0.0.0.0 --port 7777 $VERBOSE &
SERVER_PID=$!;
echo "Launched server on pid ${SERVER_PID}"
sleep 2


/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host 127.0.0.1 --port 7777 cellcycle/cellcycle.bnd -c cellcycle/cellcycle_runcfg.cfg -c cellcycle/cellcycle_runcfg-thread_1.cfg -o tmp/Cell_cycle_thread_1 $VERBOSE $FINAL
if [ $? != 0 ]; then exit 1; fi

if [[ -z "$FINAL" ]]; then
    python compare_probtrajs.py cellcycle/refer/Cell_cycle_thread_1_probtraj.csv tmp/Cell_cycle_thread_1_probtraj.csv --exact
    check_file "projtraj"

    python compare_statdist.py cellcycle/refer/Cell_cycle_thread_1_statdist.csv tmp/Cell_cycle_thread_1_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
    check_file "statdist"
else
    diff_sort cellcycle/refer/Cell_cycle_thread_1_finalprob.csv tmp/Cell_cycle_thread_1_finalprob.csv
    if [ $? != 0 ]; then
	echo "File final projtraj ** error: differences found **"
    else
	echo "File finalprob OK"
    fi
fi

if [ "$ONE_THREAD_ONLY" != "" ]; then kill $SERVER_PID; exit 0; fi

/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host 127.0.0.1 --port 7777 cellcycle/cellcycle.bnd -c cellcycle/cellcycle_runcfg.cfg -c cellcycle/cellcycle_runcfg-thread_6.cfg -o tmp/Cell_cycle_thread_6 $FINAL
if [ $? != 0 ]; then exit 1; fi

if [[ -z "$FINAL" ]]; then
    python compare_probtrajs.py cellcycle/refer/Cell_cycle_thread_6_probtraj.csv tmp/Cell_cycle_thread_6_probtraj.csv --exact
    check_file "projtraj"

    python compare_statdist.py cellcycle/refer/Cell_cycle_thread_6_statdist.csv tmp/Cell_cycle_thread_6_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
    check_file "statdist"
else
    diff_sort cellcycle/refer/Cell_cycle_thread_6_finalprob.csv tmp/Cell_cycle_thread_6_finalprob.csv
    if [ $? != 0 ]; then
	echo "File final projtraj ** error: differences found **"
    else
	echo "File finalprob OK"
    fi

    diff_sort cellcycle/refer/Cell_cycle_thread_1_finalprob.csv tmp/Cell_cycle_thread_6_finalprob.csv
    if [ $? != 0 ]; then
	echo "File final projtraj ** error: differences found **"
    else
	echo "File finalprob OK"
    fi
fi

kill $SERVER_PID

if [[ ! -x "$MABOSS_SERVER_128n" ]]; then echo "$MABOSS_SERVER_128n not found; skipping ewing test"; exit 0; fi

$MABOSS_SERVER_128n -q --host 0.0.0.0 --port 7778 &
SERVER_128_PID=$!;
echo "Launched server on pid ${SERVER_128_PID}";
sleep 2


/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host 127.0.0.1 --port 7778 ewing/ewing_full.bnd -c ewing/ewing.cfg -c ewing/ewing_runcfg-thread_1.cfg -o tmp/ewing_thread_1
if [ $? != 0 ]; then exit 1; fi
python compare_probtrajs.py ewing/refer/ewing_thread_1_probtraj.csv tmp/ewing_thread_1_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py ewing/refer/ewing_thread_1_statdist.csv tmp/ewing_thread_1_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"

/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host 127.0.0.1 --port 7778 ewing/ewing_full.bnd  -c ewing/ewing.cfg -c ewing/ewing_runcfg-thread_6.cfg -o tmp/ewing_thread_6
if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py ewing/refer/ewing_thread_6_probtraj.csv tmp/ewing_thread_6_probtraj.csv --exact
check_file "projtraj"

python compare_statdist.py ewing/refer/ewing_thread_6_statdist.csv tmp/ewing_thread_6_statdist.csv --exact #|| echo '**** error test #2.b (non regression) ****'
check_file "statdist"

kill $SERVER_128_PID;
rm -rf tmp; 

exit $return_code
