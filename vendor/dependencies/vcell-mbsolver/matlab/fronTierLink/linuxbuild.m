fprintf( 'Building frontTierCmd\n')
mex  -I../src -I../src/util -L../bin -L../lib -lExpressionParser -lfronTierStub -lfrontier -lmatlabUtilLib -g -D_DEBUG fronTierCmd.cpp

%% Fei's command, run it under cmake-build/fronTierLink
mex -L../../../cmake-build/FronTierLib -L../util -L. -lfronTierStub -lfrontier -lmatlabUtilLib fronTierCmd.cpp