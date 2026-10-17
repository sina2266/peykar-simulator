# MaBoSS : Markovian Boolean Stochastic Simulator 
[![Linux Workflow](https://github.com/sysbio-curie/MaBoSS/actions/workflows/maboss-ubuntu.yml/badge.svg)](https://github.com/sysbio-curie/MaBoSS/actions/workflows/maboss-ubuntu.yml) [![MacOS Workflow](https://github.com/sysbio-curie/MaBoSS/actions/workflows/maboss-macos.yml/badge.svg)](https://github.com/sysbio-curie/MaBoSS/actions/workflows/maboss-macos.yml) [![Linux Workflow](https://github.com/sysbio-curie/MaBoSS/actions/workflows/maboss-windows.yml/badge.svg)](https://github.com/sysbio-curie/MaBoSS/actions/workflows/maboss-windows.yml)

[![Anaconda-Server Badge](https://anaconda.org/colomoto/maboss/badges/version.svg)](https://anaconda.org/colomoto/maboss) [![PyPI version](https://badge.fury.io/py/cmaboss.svg)](https://badge.fury.io/py/cmaboss) <img align="right" height="100" src="https://maboss.curie.fr/images/maboss_logo.jpg">

MaBoSS is a C++ software for simulating continuous/discrete time Markov processes, applied on a Boolean network.

MaBoSS uses a specific language for associating transition rates to each node. Given some initial conditions, MaBoSS applies Monte-Carlo kinetic algorithm (or Gillespie algorithm) to the network to produce time trajectories. Time evolution of probabilities are estimated. In addition, global and semi-global characterizations of the whole system are computed. 

### References

Stoll, G., Viara, E., Barillot, E., & Calzone, L. (2012). Continuous time Boolean modeling for biological signaling: application of Gillespie algorithm. *BMC systems biology, 6(1), 1-18.* DOI : [10.1186/1752-0509-6-116](https://bmcsystbiol.biomedcentral.com/articles/10.1186/1752-0509-6-116)

Stoll, G., Caron, B., Viara, E., Dugourd, A., Zinovyev, A., Naldi, A., ... & Calzone, L. (2017). MaBoSS 2.0: an environment for stochastic Boolean modeling. *Bioinformatics, 33(14), 2226-2228.* DOI : [10.1093/bioinformatics/btx123](https://academic.oup.com/bioinformatics/article/33/14/2226/3059141)

### Tutorials

The directory tutorial contains two tutorials: 

- [MaBoSS 2.0 Tutorial](https://github.com/sysbio-curie/MaBoSS/tree/master/tutorial/MaBoSS-2.0), describing MaBoSS environment tools usage on model describing DNA damage effects on p53 pathway.
- [MaBoSS 2.5.0 Tutorial](https://github.com/sysbio-curie/MaBoSS/tree/master/tutorial/Montagud_2022_Prostate_Cancer), describing usage of MaBoSS command line, pyMaBoSS (python bindings) and WebMaBoSS (web interface) on a prostate cancer model.

### Conda repository

MaBoSS is available as a conda package for Linux and MacOSX in the [CoLoMoTo repository](https://anaconda.org/colomoto/maboss).

To install it, run 

    conda install -c colomoto maboss
    
Note that this package doesn't include the MPI version of MaBoSS, which still needs to be built manually from source. 

### Python bindings

MaBoSS is accessible via pyMaBoSS, its python bindings, which are available on [the GitHub of the CoLoMoTo organisation](https://github.com/colomoto/pyMaBoSS).

### Web interface

MaBoSS can also be used via WebMaBoSS, a web interface, at [https://maboss.curie.fr/webmaboss/](https://maboss.curie.fr/webmaboss/).

### Package Contents
MaBoSS is composed of:
- MaBoSS engine 2.5.6: C++ core program simulating continuous/discrete time Markov processes, applied on a Boolean network.
- MaBoSS tools 2.0: perl and python scripts using MaBoSS engine 2.0
- cMaBoSS 1.0.0b23: python bindings using Python C API.
- PopMaBoSS engine 0.0.1: Simulating continuous/discrete time Markov processes, applied on a Population of Boolean network state.

### Tested platforms
- Linux: Ubuntu (Ubuntu 4.3.2-1ubuntu11 and higher), RedHat and CentOS
- MacOS X x86
- Windows with cygwin

### Requirements
##### MaBoSS engine 2.0 and higher:

- gcc: version 4.0.1 or higher
- bison: version 2.3 or higher
- flex: version 2.5.35 or higher
- cygwin is needed on Windows

##### MaBoSS engine 2.4.0 and higher with SBML-qual compatibility

- libsbml 5.19.0, with sbml-qual package. 

##### MaBoSS engine 2.5.0 and higher with MPI compatibility

- MPI library, such as OpenMPI.

### Engine Compilation

    cd engine/src
    make install

The executable file will be located in engine/pub and is named MaBoSS.

This compiled version supports up to 64 nodes per network.

If you need more nodes per network, you have to add an extra hint to the compilation command, for instance to compile a version supporting up to 100 nodes:
    
    make MAXNODES=100 install

The executable file will also be located in engine/pub and will be named MaBoSS_100n.

Notes:
- if you manage only networks with up to 64 nodes, we recommend you to use the default compiled version as for networks with more than 64 nodes, the implementation is very different and will be slower.
- generally speaking, a version compiled with a given number of nodes will be slower and will use more memory than a version compiled with a lesser number of nodes.

If you need MaBoSS with SBML-qual compatibility, you need libSBML installed, with support for the qual package. To compile it, you also need an extra arguement to the compilation command :
    
    make SBML_COMPAT=1 install
    
Finally, it you need the MPI compatible version, you need to have a MPI library installed (such as OpenMPI), add MPI_COMPAT=1 flag and specify the mpic++ compiler in the compilation command : 

    make MPI_COMPAT=1 CXX=mpic++ install

### MaBoSS Engine Usage

    cd engine/pub

    ./MaBoSS --version
    MaBoSS version 2.5.6 [networks up to 64 nodes]

    ./MaBoSS_100n --version
    MaBoSS version 2.5.6 [networks up to 100 nodes]

The usage is described in the [reference card](https://github.com/sysbio-curie/MaBoSS/blob/master/engine/doc/MaBoSS-RefCard.pdf).

To use the engine compiled with MPI capability : 

    mpirun -np 2 ./MaBoSS.MPI --version

### Binary Distribution

To avoid installing compilation tools, we provide binary versions for linux x86, MacOS X x86 and Windows x86:
- linux   : [MaBoSS-linux64.zip](https://github.com/sysbio-curie/MaBoSS/releases/latest/download/MaBoSS-linux64.zip)
- MacOS X : [MaBoSS-osx64.zip](https://github.com/sysbio-curie/MaBoSS/releases/latest/download/MaBoSS-osx64.zip)
- Windows : [MaBoSS-win64.zip](https://github.com/sysbio-curie/MaBoSS/releases/latest/download/MaBoSS-win64.zip)

All these binary versions are provided "as is", they may not work on your OS. In such a case, you need to compile MaBoSS.

If you want to use the binary version, extract the binaries to a path and add it to you PATH environment variable.

Important notes on the Windows version:
- to execute MaBoSS.exe, cygwin must be installed (http://www.cygwin.com/)
- because of the cygwin emulation, the windows version is very slow (about 4 times slower than the linux and Mac OS versions). We urge you to run MaBoSS on linux or Mac OS X if possible.

### PopMaBoSS Engine compilation

    cd engine/src
    make install

The executable file will be located in engine/pub and is named PopMaBoSS.This compiled version supports up to 64 nodes per network.

### PopMaBoSS Engine usage

    cd engine/pub

    ./PopMaBoSS --version
    PopMaBoSS version 0.0.1 [networks up to 64 nodes]

This is a simple check which returns the version of the engine

    ./PopMaBoSS -c ../examples/popmaboss/Toy.cfg ../examples/popmaboss/Toy.pbnd -o res

This will simulate a toy example available in the engine/example/popmaboss directory, and generate results files : 

- res_fp.csv : list of fixed points of the boolean network encountered during the simulation
- res_pop_probtraj.csv : Population states probability distribution trajectories (as with MaBoSS, but this time on populations of boolean states) 
- res_simple_pop_probtraj.csv : Simplified population output, with an average population size for every boolean state.

##### MaBoSS tools:

- perl
- python3
- python3 modules: matplotlib (matplotlib.cm, matplotlib.gridspec, matplotlib.patches, matplotlib.pylab), numpy, pandas, seaborn, xlsxwriter

To check requirements on a Unix platform (Linux, MacOS X), you can launch the script check-requirements.sh as follows:
./check-requirements

MaBoSS engine requirements are checked first.

The output must be:

Checking MaBoSS engine 2.0 requirements...

  flex: OK
  bison: OK
  gcc: OK
  g++: OK

MaBoSS engine 2.0 requirements: OK

If an error is displayed, you have to fix it, as neither the engine, nor the tools will be able to be launched.

MaBoSS tools requirements are then checked.

The most frequent errors are that the following python3 modules are missing: matplotlib, numpy, pandas, seaborn, xlsxwriter.

You must install the missing modules if you want to use: MBSSf_DrugSim.py, MBSS_PieChart.py, MBSS_PrepareProjectFilePieChart.py, MBSS_PrepareProjectFileTrajectoryFig.py or MBSS_TrajectoryFig.py.

### Environment for MaBoSS tools

Go to the directory of MaBoSS and perform:
    
    source MaBoSS.env

Then, the MaBoSS engine and all the tools will be accessible from your environment:

Type MBSS_ followed by a Tab, this should be displayed:

    MBSS_DrugSim.py                          MBSS_MutBndCfg.pl                        MBSS_PrepareProjectFile.sh
    MBSS_FormatTable.pl                      MBSS_MutBnd.pl                           MBSS_PrepareProjectFileTrajectoryFig.py
    MBSS_InitCondFromTrajectory.pl           MBSS_PieChart.py                         MBSS_SensitivityAnalysis.pl
    MBSS_MultipleSim.py                      MBSS_PrepareProjectFilePieChart.py       MBSS_TrajectoryFig.py

Type MaB followed by a Tab, the MaBoSS engine program should be displayed:

    MaBoSS

### Examples

The directory examples contains two examples:
- ToyModel
- p53_Mdm2

To test an example (for instance p53_Mdm2):
- change to the directory containing the model (examples/p53_Mdm2)
- ../../pub/MaBoSS -c p53_Mdm2_runcfg.cfg -o p53_Mdm2_out p53_Mdm2.bnd
- the files p53_Mdm2_out_probtraj.csv and p53_Mdm2_out_statdist.csv will be created.
The description of these files can be found at https://maboss.curie.fr/pub/DescriptionOutputFile.pdf

### Contact
Institut Curie 

26 rue d'Ulm 75248 PARIS CEDEX 05 

Contact: [maboss.bkmc@gmail.com](mailto://maboss.bkmc@gmail.com) 

Web Site: [https://maboss.curie.fr](https://maboss.curie.fr)

### Copyright

BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)  
                                                                         
Copyright (c) 2011-2022 Institut Curie, 26 rue d'Ulm, Paris, France      
All rights reserved.                                                     
                                                                         
Redistribution and use in source and binary forms, with or without       
modification, are permitted provided that the following conditions are   
met:                                                                     
                                                                         
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.                    
                                                                         
2. Redistributions in binary form must reproduce the above copyright     
notice, this list of conditions and the following disclaimer in the      
documentation and/or other materials provided with the distribution.     
                                                                         
3. Neither the name of the copyright holder nor the names of its         
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.                      
                                                                         
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS      
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,      
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR       
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF   
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS       
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.             


### Acknowledgements

The development was partially supported by European Union's Horizon 2020 Programme under agreement no. 951773 (PerMedCoE project). 
