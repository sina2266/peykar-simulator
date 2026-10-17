
return_code=0

check()
{
    if [ $? = 0 ]; then
    	echo "$1 OK"
    else
	    echo "$1 ERR"
        return_code=1
    fi
}

export MABOSS=${PREFIX}/bin/MaBoSS
export MABOSS_128n=${PREFIX}/bin/MaBoSS_128n
export MABOSS_CLIENT=${PREFIX}/bin/MaBoSS-client
export MABOSS_SERVER=${PREFIX}/bin/MaBoSS-server
export MABOSS_SERVER_128n=${PREFIX}/bin/MaBoSS_128n-server
export POPMABOSS=${PREFIX}/bin/PopMaBoSS

cd ${PREFIX}/tests

bash ./test-cellcycle.sh
check "cellcycle"
bash ./test-ensemble.sh
check "ensemble"
bash ./test-ewing.sh
check "ewing"
bash ./test-server.sh
check "server"
bash ./test-bnet.sh
check "bnet"
bash ./test-prngs.sh
check "prngs"
bash ./test-rngs.sh
check "rngs"
bash ./test-popmaboss.sh
check popmaboss
bash ./test-sbml.sh
check "sbml"

cd ../..

exit $return_code
