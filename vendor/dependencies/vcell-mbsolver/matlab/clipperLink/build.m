%fprintf( 'Building mexFT_Step\n')
%mex -I../src -I../src/util -I../VCell/include -DWIN32  -L..\Debug -lFronTier  -lmsvcrtd -g mexFT_Step.cpp
%mex -I../src -I../boost_1_53_0 -I../src/util -I../VCell/include -DWIN32  -L..\Debug -lFronTier -lVCellFronTier -lmsvcrtd -g -D_DEBUG mexFT_Step.cpp 
fprintf( 'Building clipperlink\n')
mex -f build.opts -I../src -I../src/util -DWIN32  -L..\bin -L..\lib -lExpressionParser -lfrontier -lclipperStub -lmatlabUtilLib -lMovingBoundaryLib  -g -D_DEBUG clipperLink.cpp
%fprintf( 'Building clipper\n')
%mex -f build.opts -I../src -I../src/util -DWIN32  -L..\bin -lExpressionParser -lfrontier -lmatlabLink -matlabUtilLib -lMovingBoundaryLib  -g -D_DEBUG clipper.cpp
