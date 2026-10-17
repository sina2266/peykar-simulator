FROM python:3.12-slim@sha256:2c941e860699f878900b0edc2403613c234d4b32eda3cc9fa7036991a2a63c4a
RUN apt-get update && apt-get install -y --no-install-recommends g++ make bison flex libgomp1 && rm -rf /var/lib/apt/lists/*
COPY vendor/upstream/physicell /opt/peykar/physicell
COPY vendor/dependencies/maboss /opt/peykar/physicell/addons/PhysiBoSS/MaBoSS
WORKDIR /opt/peykar/physicell/addons/PhysiBoSS/MaBoSS/engine/src
RUN make -j2 install_alib MAXNODES=64
WORKDIR /opt/peykar/physicell
RUN make physiboss-cell-lines-sample && make -j2 CFLAGS="-O3 -fomit-frame-pointer -fopenmp -std=c++11" CFLAGS_LINK="-O3 -fomit-frame-pointer -std=c++11"
RUN dpkg-query -W > /opt/peykar/native-packages.tsv
ENV OMP_NUM_THREADS=2
WORKDIR /work
ENTRYPOINT ["/opt/peykar/physicell/PhysiBoSS_Cell_Lines"]
