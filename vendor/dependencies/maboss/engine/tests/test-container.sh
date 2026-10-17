#!/bin/sh
#
# test1.sh
#

. `dirname $0`/share.sh

return_code=0

if [ ! -x $MABOSS_CLIENT ]
then
    echo $MABOSS_CLIENT not found
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
echo "Server test through container"
rm -rf tmp; mkdir -p tmp
/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host localhost --port 7777 -c cellcycle/cellcycle_runcfg.cfg -c cellcycle/cellcycle_runcfg-thread_1.cfg -o tmp/Cell_cycle_thread_1 cellcycle/cellcycle.bnd 
if [ $? != 0 ]; then exit 1; fi
python3 compare_probtrajs.py cellcycle/refer/Cell_cycle_thread_1_probtraj.csv tmp/Cell_cycle_thread_1_probtraj.csv --exact
check_file "projtraj"

python3 compare_statdist.py cellcycle/refer/Cell_cycle_thread_1_statdist.csv tmp/Cell_cycle_thread_1_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"


/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host localhost --port 7777 cellcycle/cellcycle.bnd -c cellcycle/cellcycle_runcfg.cfg -c cellcycle/cellcycle_runcfg-thread_6.cfg -o tmp/Cell_cycle_thread_6
if [ $? != 0 ]; then exit 1; fi

python3 compare_probtrajs.py cellcycle/refer/Cell_cycle_thread_6_probtraj.csv tmp/Cell_cycle_thread_6_probtraj.csv --exact
check_file "projtraj"

python3 compare_statdist.py cellcycle/refer/Cell_cycle_thread_6_statdist.csv tmp/Cell_cycle_thread_6_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"


/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host localhost --port 7778 ewing/ewing_full.bnd -c ewing/ewing.cfg -c ewing/ewing_runcfg-thread_1.cfg -o tmp/ewing_thread_1
if [ $? != 0 ]; then exit 1; fi
python3 compare_probtrajs.py ewing/refer/ewing_thread_1_probtraj.csv tmp/ewing_thread_1_probtraj.csv --exact
check_file "projtraj"

python3 compare_statdist.py ewing/refer/ewing_thread_1_statdist.csv tmp/ewing_thread_1_statdist.csv --exact # || echo '**** error test #1.b (non regression) ****'
check_file "statdist"


/usr/bin/time -p $LAUNCHER $MABOSS_CLIENT --host localhost --port 7778 ewing/ewing_full.bnd  -c ewing/ewing.cfg -c ewing/ewing_runcfg-thread_6.cfg -o tmp/ewing_thread_6
if [ $? != 0 ]; then exit 1; fi

python3 compare_probtrajs.py ewing/refer/ewing_thread_6_probtraj.csv tmp/ewing_thread_6_probtraj.csv --exact
check_file "projtraj"

python3 compare_statdist.py ewing/refer/ewing_thread_6_statdist.csv tmp/ewing_thread_6_statdist.csv --exact #|| echo '**** error test #2.b (non regression) ****'
check_file "statdist"


rm -rf tmp; 

exit $return_code
