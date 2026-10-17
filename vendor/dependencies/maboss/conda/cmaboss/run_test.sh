
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

cd ${PREFIX}/tests

python -m unittest test
check "test 64 nodes"
python -m unittest test_128n
check "test 128 nodes"

cd ../..

exit $return_code
