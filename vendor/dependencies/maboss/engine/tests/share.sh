
if [ -z "$MABOSS" ] ; then MABOSS=../src/MaBoSS; fi
if [ -z "$MABOSS_MPI" ] ; then MABOSS_MPI=../src/MaBoSS.MPI; fi
if [ -z "$MABOSS_DN" ] ; then MABOSS_DN=../src/MaBoSS_dn; fi
#if [ -z "$MABOSS_BITSET" ] ; then MABOSS_BITSET=../src/MaBoSS_bitset; fi
if [ -z "$MABOSS_128n" ] ; then MABOSS_128n=../src/MaBoSS_128n; fi
if [ -z "$MABOSS_128n_MPI" ] ; then MABOSS_128n_MPI=../src/MaBoSS_128n.MPI; fi
if [ -z "$MABOSS_CLIENT" ] ; then MABOSS_CLIENT=../src/MaBoSS-client; fi
if [ -z "$MABOSS_SERVER" ] ; then MABOSS_SERVER=../src/MaBoSS-server; fi
if [ -z "$MABOSS_SERVER_128n" ] ; then MABOSS_SERVER_128n=../src/MaBoSS_128n-server; fi
if [ -z "$POPMABOSS" ] ; then POPMABOSS=../src/PopMaBoSS; fi
export DYLD_LIBRARY_PATH=../src:$DYLD_LIBRARY_PATH

if [[ ! -z "$USE_MABOSS_DYNBITSET" ]]; then
    echo "using $MABOSS_DN"
    MABOSS=${MABOSS_DN}
elif [[ ! -z "$USE_BITSET" ]]; then
    echo "using $MABOSS_BITSET"
    MABOSS=${MABOSS_BITSET}
fi

diff_sort() {
    file1="$1"
    file2="$2"
    tmpfile1=/tmp/$(basename ${file1})_1.$$
    tmpfile2=/tmp/$(basename ${file2})_2.$$
    sort $file1 > $tmpfile1
    sort $file2 > $tmpfile2
    diff $tmpfile1 $tmpfile2
    status=$?
    rm -r $tmpfile1 $tmpfile2
    return $status
}
