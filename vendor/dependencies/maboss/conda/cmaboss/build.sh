cd engine/src
make grammars
cd ../python
cp -r ../src cmaboss
$PYTHON -m pip install .
mkdir -p ${PREFIX}/tests
cp -r ../tests/* ${PREFIX}/tests

