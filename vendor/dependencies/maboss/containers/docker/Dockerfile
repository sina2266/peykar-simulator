FROM ubuntu:22.04

# Preventing python3-tk from asking timezone
ENV DEBIAN_FRONTEND=noninteractive

# Installing dependencies
RUN apt-get -qq update
RUN apt-get install -yq git flex bison gcc g++ make libxml2-dev cmake wget \
    && apt clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/sbmlteam/libsbml/archive/refs/tags/v5.20.2.tar.gz \
    && tar -zxf v5.20.2.tar.gz \
    && cd libsbml-5.20.2 \
    && mkdir build \
    && cd build \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=/usr/lib \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_STANDARD_LIBRARIES=-lxml2 \
    -DWITH_SWIG=OFF \
    -DLIBXML_LIBRARY=/usr/lib/x86_64-linux-gnu/libxml2.so \
    -DLIBXML_INCLUDE_DIR=/usr/include/libxml2 \
    -DENABLE_COMP=ON \
    -DENABLE_FBC=ON \
    -DENABLE_GROUPS=ON \
    -DENABLE_LAYOUT=ON \
    -DENABLE_MULTI=ON \
    -DENABLE_QUAL=ON \
    -DENABLE_RENDER=ON \
    -DENABLE_DISTRIB=ON \
    -DWITH_CPP_NAMESPACE=ON \
    .. \
    && make \
    && make install \
    && cd ../../ \
    && rm -fr libsbml-5.20.2 \
    && rm -fr /opt/MaBoSS/.git
    
# Installing MaBoSS
ADD engine/src /opt/MaBoSS/engine/src
RUN bash -c "rm -fr /opt/MaBoSS/.git"
RUN bash -c "cd /opt/MaBoSS/engine/src/; make install SBML_COMPAT=1"
RUN bash -c "cd /opt/MaBoSS/engine/src/; make MAXNODES=128 install SBML_COMPAT=1"
RUN bash -c "cd /opt/MaBoSS/engine/src/; make MAXNODES=256 install SBML_COMPAT=1"
RUN bash -c "cd /opt/MaBoSS/engine/src/; make MAXNODES=512 install SBML_COMPAT=1"
RUN bash -c "cd /opt/MaBoSS/engine/src/; make MAXNODES=1024 install SBML_COMPAT=1"

# Exposing port 7777
EXPOSE 7777
EXPOSE 7778
EXPOSE 7779
EXPOSE 7780
EXPOSE 7781

# Running MaBoSS server
CMD /opt/MaBoSS/engine/pub/MaBoSS-server -q --host 0.0.0.0 --port 7777 \
 & /opt/MaBoSS/engine/pub/MaBoSS_128n-server -q --host 0.0.0.0 --port 7778 \
 & /opt/MaBoSS/engine/pub/MaBoSS_256n-server -q --host 0.0.0.0 --port 7779 \
 & /opt/MaBoSS/engine/pub/MaBoSS_512n-server -q --host 0.0.0.0 --port 7780 \
 & /opt/MaBoSS/engine/pub/MaBoSS_1024n-server -q --host 0.0.0.0 --port 7781
