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

python3 -m unittest test
check "tests on classic"
python3 -m unittest test_128n
check "tests on bitsets"
cd ../..

exit $return_code