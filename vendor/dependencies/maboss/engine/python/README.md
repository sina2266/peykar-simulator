## CMaBoSS : MaBoSS Python bindings using C API

## Files description

- setup.py : Python package definition. It creates multiple extensions by compiling using different MAXNODES values. 
- maboss_module.cpp : Definition of the module. Initialisation functions, Objects registration. 
- maboss_sim.cpp : Simulation class. Load a bnd and a cfg, and run the simulation
- maboss_res.cpp : Result class. Return by the run() method of the simulation class
 
 
- test.py : Basic test class for the <=64 nodes version
- test_128n.py : Basic test class for the <=128 nodes version
 
 
- build-wheels.sh : Script used by manylinux docker image to build wheels