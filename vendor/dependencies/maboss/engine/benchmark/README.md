# MaBoSS Benchmarking

## Compiling MaBoSS

### Dependencies

    gcc or clang
    flex
    bison

### Threads only version

    cd engine/src
    make install MAXNODES=150
    
This will compile MaBoSS_150n (a version for models up to 150 nodes) and copy the executable in engine/pub

### MPI version

    cd engine/src
    make install MPI_COMPAT=1 CXX=mpic++ MAXNODES=150
    
This will compile the mpi-compatible version of MaBoSS_150n and copy the executable in engine/pub


## Running MaBoSS

### Threads only version
 
    cd engine/benchmark
    ../pub/MaBoSS_150n \
        ../examples/Sizek/sizek.bnd -c ../examples/Sizek/sizek.cfg \ # This is the model we are simulating, defined by its bnd and cfg files
        -o results/small/res_32 \ # This is defining the prefix of the output files of the simulation
        -e thread_count=32 \ # This is the number of threads we are using to simulate
        1>>results/small/stdout_32 2>>results/small/stderr_32 # Also saving stdout and stderr 

Here is an example of command line to launch a MaBoSS simulation using several threads. A number of threads is usually defined in the cfg file of the model, but here for testing convenience we can override it from the command line.

### MPI version

    cd engine/benchmark
    mpirun -np 4 ../pub/MaBoSS_150n \
        ../examples/Sizek/sizek.bnd -c ../examples/Sizek/sizek.cfg \ # This is the model we are simulating, defined by its bnd and cfg files
        -o results/small/res_32 \ # This is defining the prefix of the output files of the simulation
        -e thread_count=32 \ # This is the number of threads we are using to simulate
        1>>results/small/stdout_32 2>>results/small/stderr_32 # Also saving stdout and stderr 
        
Here we just add the mpirun and its number of nodes. On the cluster I did my tests I had to add "--bind-to core --map-by socket:PE=19" to be able to use several cores on each node (otherwise it would just use one). Not sure why is this, but it might be a weird config of the cluster. 
I had the same problem running the MPI version of MaBoSS without the mpirun. Could it be coming from my code ?


## Benchmark performed

### Test models

The model is composed of two files : the model (.bnd) and it's simulation settings (.cfg). They are defined in the command line as such :

    path_bnd_file -c path_cfg_file
    
#### Sizek model

    ../examples/Sizek/sizek.bnd -c ../examples/Sizek/sizek.cfg

This model contains 87 nodes, so it needs to be run with a MaBoSS version compiled for at least 87 nodes. It comes with multiple settings, to vary the size of the results (via the number of nodes) and look at it's effect on the parallelization efficiency

    sizek.cfg : 4 nodes
    sizek_medium.cfg : 8 nodes
    sizek_large.cfg : 12 nodes
    sizek_xlarge.cfg : 20 nodes
    sizek_xxlarge.cfg : 30 nodes

#### Montagud model

    ../examples/Montagud_Prostate/Montagud2021_Prostate_Cancer.bnd \
    -c ../examples/Montagud_Prostate/Montagud2021_Prostate_Cancer.cfg

This model contains 133 nodes, so it needs to be run with a MaBoSS version compiled for at least 133 nodes. It comes with two settings, to vary the size of the results (via the number of nodes) and look at it's effect on the parallelization efficiency

    Montagud2021_Prostate_Cancer.cfg : 3 nodes
    Montagud2021_Prostate_Cancer_large.cfg : 20 nodes
    

### Slurm scripts
I'm including examples of the slurm scripts I ran :

    maboss_threads_benchmark[_low|_vlow] : Tests on 1 to 32 cores, divided into three scripts.
    
    maboss_mpi_benchmark[_low] : Tests on 1 to 8 nodes, 19 cores on each node, divided into two scripts.
    
In these scripts I'm only simulating the small (sizek.cfg) and xlarge (sizek_xlarge.cfg) simulation settings.
