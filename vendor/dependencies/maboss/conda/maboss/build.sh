cd engine/src

# export CXXFLAGS="${CXXFLAGS} -I${PREFIX}/include -I${PREFIX}/include/libxml2"
export LDFLAGS="${LDFLAGS} -L${BUILD_PREFIX}/lib -lxml2"

make SBML_COMPAT=1 install
make SBML_COMPAT=1 MAXNODES=128 install
make SBML_COMPAT=1 MAXNODES=256 install
make SBML_COMPAT=1 MAXNODES=512 install
make SBML_COMPAT=1 MAXNODES=1024 install
mkdir -p ${PREFIX}/bin
mv ../pub/MaBoSS  ../pub/MaBoSS_*n ${PREFIX}/bin
mv ../pub/PopMaBoSS  ../pub/PopMaBoSS_*n ${PREFIX}/bin
mv ../pub/MaBoSS-server  ../pub/MaBoSS_*n-server ${PREFIX}/bin
mv ../pub/MaBoSS-client ${PREFIX}/bin
cd ../..
mkdir -p "${PREFIX}/share/MaBoSS"
#mv doc tutorial examples ${PREFIX}/share/MaBoSS/
mv tools/* ${PREFIX}/bin
mkdir -p ${PREFIX}/tests
cp -r engine/tests/* ${PREFIX}/tests

