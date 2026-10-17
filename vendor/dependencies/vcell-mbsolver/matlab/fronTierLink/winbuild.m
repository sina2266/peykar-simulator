fprintf( 'Building frontTierCmd\n')
mex -f build.opts -I../src -I../src/util -DWIN32  -L..\bin -L..\lib -lExpressionParser -lfrontier -lfronTierStub -lmatlabUtilLib -lMovingBoundaryLib  -g -D_DEBUG fronTierCmd.cpp


%% Fei's command
mex fronTierCmd.cpp flink.cpp ../util/matlabStruct.cpp ../util/VCDictionary.cpp ../util/mlAssert.cpp -I../util -I../../Solver/include -I../../FronTierLib -I../../FronTierLib/util -I../../vcommons/include ../FronTierReference/FronTier_64b.lib