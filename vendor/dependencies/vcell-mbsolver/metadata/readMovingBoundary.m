function [ rval ] = readMovingBoundary( f )
%readMovingBoundary Read moving boundary hdf5 file
%   Pass filename as string, complex data structure contains most values
%   of interest
    rval.scaleFactor = h5readatt(f,'/','scaleFactor');
	rval.maxT = h5read(f,'/lastTimeIndex');
	rval.endTime = h5read(f,'/endTime');
	rval.generationTimes = h5read(f,'/generationTimes');
	rval.moveTimes = h5read(f,'/moveTimes');
	rval.timeStep = h5read(f,'/timeStep');
	rval.timeStepTimes = h5read(f,'/timeStepTimes');
    try
    rval.solverTimeStep = h5read(f,'/solverTimeStep');
    catch
    end
    rval.runTime = h5read(f,'/runTime');
    rval.meshXValues = h5read(f,'/meshXvalues');
    
    rval.meshYValues = h5read(f,'/meshYvalues');
    
    %only read active data
    start = [1 1 1 1];
    elementInfo = h5info(f,'/species');
    dsSize = elementInfo.Dataspace.Size;
    dsSize(4) = rval.maxT;
    rval.sp = h5read(f,'/species',start,dsSize);
    
    %remove first dimension
    epathname= '/elements';
    dsSize(1) = 0;
    dsSize = nonzeros(dsSize);
    start = [1 1 1];
    rval.element = h5read(f,epathname,start,dsSize);
    
    rval.nxnodes = h5readatt(f,epathname,'numX');
    rval.nynodes = h5readatt(f,epathname,'numY');
    rval.hx = h5readatt(f,epathname,'hx');
    rval.hy = h5readatt(f,epathname,'hy');
           
    md = cast(rval.maxT,'double');
	
    rval.boundaries = h5read(f,'/boundaries',[1],md);


end

