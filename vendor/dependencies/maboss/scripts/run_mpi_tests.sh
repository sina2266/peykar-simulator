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

cd engine/tests

bash ./test-mpi-cellcycle.sh 2
check "cellcycle 2 nodes"
bash ./test-mpi-cellcycle.sh 4
check "cellcycle 4 nodes"
bash ./test-mpi-ewing.sh 2
check "ewing 2 nodes"
bash ./test-mpi-ewing.sh 4
check "ewing 4 nodes"
bash ./test-mpi-ensemble.sh 2
check "ensemble 2 nodes"
bash ./test-mpi-ensemble.sh 4
check "ensemble 4 nodes"

cd ../..

exit $return_code