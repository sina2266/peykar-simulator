#!/bin/bash

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

cd engine/tests/

bash ./test-cellcycle.sh
check "cellcycle"
bash ./test-ensemble.sh
check "ensemble"
bash ./test-ewing.sh
check "ewing"
bash ./test-sbml.sh
check "sbml"
bash ./test-bnet.sh
check "bnet"
bash ./test-prngs.sh
check "prngs"
bash ./test-popmaboss.sh
check popmaboss

if [[ -n $RUNNER_OS ]] && [[ $RUNNER_OS != "Windows" ]]; then
    bash ./test-server.sh
    check "server"
    bash ./test-rngs.sh    
    check "rngs"
fi

bash ./test-user_func.sh
check "user_func"

cd ../..

exit $return_code