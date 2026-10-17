FROM python:3.12-slim@sha256:2c941e860699f878900b0edc2403613c234d4b32eda3cc9fa7036991a2a63c4a
RUN apt-get update && apt-get install -y --no-install-recommends g++ make libgomp1 \
    && rm -rf /var/lib/apt/lists/*
COPY vendor/upstream/physicell /opt/peykar/physicell
WORKDIR /opt/peykar/physicell
RUN make template && make -j2 CFLAGS="-O3 -fomit-frame-pointer -fopenmp -std=c++11" \
    CFLAGS_LINK="-O3 -fomit-frame-pointer -std=c++11" && dpkg-query -W > /opt/peykar/dpkg-inventory.txt
ENV OMP_NUM_THREADS=2
WORKDIR /work
ENTRYPOINT ["/opt/peykar/physicell/project"]
