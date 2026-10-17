#include <Hdf5OutputWriter.h>
#include <MPoint.h>
#include <map>
#include <set>
#include <iomanip>
#include <algo.h>
#include <Mesh.h>
#include <Logger.h>
#include <Mesh.h>
#include <MeshElementNode.h>
#include <VolumeVariable.h>
#include <MovingBoundaryParabolicProblem.h>
#include <vcellxml.h>
#include <ReportClient.h>
#include <Physiology.h>
#include <TextReportClient.h>
#include <MBridge/MatlabDebug.h>
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>

namespace
{
	struct PointSubdomainPositionType
	{
		double positionX;
		double positionY;

		static H5::DataType getType( )
		{
			H5::CompType compType( sizeof (PointSubdomainPositionType) );
			compType.insertMember("PositionX", HOFFSET(PointSubdomainPositionType,positionX), PredType::NATIVE_DOUBLE);
			compType.insertMember("PositionY", HOFFSET(PointSubdomainPositionType,positionY), PredType::NATIVE_DOUBLE);
			return compType;
		}
	};
}

Hdf5OutputWriter::Hdf5OutputWriter(std::string& a_xml, std::string& baseFileName, H5File& a_h5File, int steps, double interval,
	WorldType & world_,
	const moving_boundary::MovingBoundaryParabolicProblem &mbpp)
	: ReportClient(baseFileName),
		xml(a_xml),
		h5File(a_h5File),
		outputNumSteps(steps),
		outputTimeStep(interval),
	theProblem(mbpp),
	world(world_),
	lastReportIteration(0),
	bLastIter(false),
	volumeSolution(nullptr),
	pointconverter(world.pointConverter( ))
{
	logFileName = baseFileName + LOG_FILE_EXT;
	h5FileName = baseFileName + H5_FILE_EXT;
	init();
}

Hdf5OutputWriter::~Hdf5OutputWriter()
{
	delete[] volumeSolution;
}

void Hdf5OutputWriter::init()
{
	remove(logFileName.c_str());

	numElements = theProblem.meshDef().numCells();
	numVolumeVariables = theProblem.physiology()->numVolumeVariables();
	volumeSolution = new double[numElements];

	initMeshGroup();
	initTimesDataSet();
	// solution group
	solutionGroup = h5File.createGroup(Group_Solution);
}

void Hdf5OutputWriter::initTimesDataSet()
{
	int timesRank = 1;
	hsize_t timesDims = 10;
	hsize_t maxDims = H5S_UNLIMITED;
	DataSpace timesDataSpace(timesRank, &timesDims, &maxDims);
	// enable chunking
	DSetCreatPropList cparms;
	hsize_t chunkDims = 500;
	cparms.setChunk(timesRank, &chunkDims);
	int fill_val = -1;
	cparms.setFillValue(PredType::NATIVE_INT, &fill_val);
	// create dataset
	timesDataSet = h5File.createDataSet(DataSet_Times, PredType::NATIVE_DOUBLE, timesDataSpace, cparms);
}

void Hdf5OutputWriter::initMeshGroup()
{
	// mesh group
	meshGroup = h5File.createGroup(Group_Mesh);
	const int geoDimension = theProblem.meshDef().numDim();

	// dim
	{
	hsize_t hdf5Rank = 1;
	hsize_t hdf5Dims = 1;
	DataSpace dataspace(hdf5Rank, &hdf5Dims);
	DataSet dataSet = meshGroup.createDataSet("dimension", PredType::NATIVE_INT, dataspace);
	dataSet.write(&geoDimension, PredType::NATIVE_INT, H5S_ALL, H5S_ALL);
	}

	// size
	{
	const int* N = theProblem.mesh().Nx().data();
	hsize_t hdf5Rank = 2;  // Nx, Ny
	hsize_t hdf5Dims[2] = {1, static_cast<hsize_t>(geoDimension)};
	DataSpace dataspace = DataSpace(hdf5Rank, hdf5Dims);
	DataSet dataSet = meshGroup.createDataSet("size", PredType::NATIVE_INT, dataspace);
	dataSet.write(N, PredType::NATIVE_INT, H5S_ALL, H5S_ALL);
	}

	// origin
	{
	const double* origin = theProblem.mesh().geoOrigin().data();
	hsize_t hdf5Rank = 2;  // lo, hi
	hsize_t hdf5Dims[2] = {1, static_cast<hsize_t>(geoDimension)};
	DataSpace dataspace = DataSpace(hdf5Rank, hdf5Dims);
	DataSet dataSet = meshGroup.createDataSet("origin", PredType::NATIVE_DOUBLE, dataspace);
	dataSet.write(origin, PredType::NATIVE_DOUBLE, H5S_ALL, H5S_ALL);
	}

	// extent
	{
	const double* extentX = theProblem.setup().extentX.data();
	const double* extentY = theProblem.setup().extentY.data();
	hsize_t hdf5Rank = 2; // extentX, extentY
	hsize_t hdf5Dims[2] = {1, static_cast<hsize_t>(geoDimension)};
	double data[DIM] = { extentX[1] - extentX[0], extentY[1] - extentY[0]	};
	DataSpace dataspace = DataSpace(hdf5Rank, hdf5Dims);
	DataSet dataSet = meshGroup.createDataSet("extent", PredType::NATIVE_DOUBLE, dataspace);
	dataSet.write(data, PredType::NATIVE_DOUBLE, H5S_ALL, H5S_ALL);
	}
}

void Hdf5OutputWriter::time(double t, unsigned int numIter, bool last, const moving_boundary::GeometryInfo<moving_boundary::CoordinateType> & geometryInfo) {
		currentTime = t;
		currentIter = numIter;
		bLastIter = last;
		bShouldReport = currentIter == 0 || bLastIter || (outputNumSteps != -1 ? currentIter % outputNumSteps == 0 : std::abs(currentTime - timeList.back() - outputTimeStep) < 1e-8);

		if (bShouldReport)
		{
			int timesRank = 1;
			hsize_t timesDims = 1;
			hsize_t maxDims = H5S_UNLIMITED;
			DataSpace timesDataSpace(timesRank, &timesDims, &maxDims);

			// write current time
			hsize_t size = timeList.size() + 1;
			timesDataSet.extend(&size);
			hsize_t dim = 1;
			hsize_t offset = timeList.size();
			DataSpace fspace = timesDataSet.getSpace();
			fspace.selectHyperslab(H5S_SELECT_SET, &dim, &offset);
			timesDataSet.write(&currentTime, PredType::NATIVE_DOUBLE, timesDataSpace, fspace);

			writeSolution();
			writeLog();
			timeList.push_back(currentTime);
		}
}

void Hdf5OutputWriter::writeSolution()
{
	char timeGroupName[128];
	sprintf(timeGroupName, "time%06d", (int)timeList.size());
	Group timeGroup = solutionGroup.createGroup(timeGroupName);
	// time attribute
	DataSpace attributeDataSpace(H5S_SCALAR);
	Attribute attribute = timeGroup.createAttribute("time", PredType::NATIVE_DOUBLE, attributeDataSpace);
	attribute.write(PredType::NATIVE_DOUBLE, &currentTime);

	// dt attribute
	double dt = theProblem.frontTimeStep();
	attribute = timeGroup.createAttribute("dt", PredType::NATIVE_DOUBLE, attributeDataSpace);
	attribute.write(PredType::NATIVE_DOUBLE, &dt);

	writeVolumeSolution(timeGroup);
	writePointSolution(timeGroup);
}

void Hdf5OutputWriter::writeVolumeSolution(Group& timeGroup)
{
	// for each variable, we create a dataset
	for (int ivar = 0; ivar < numVolumeVariables; ++ ivar)
	{
		const VolumeVariable* volVar = theProblem.physiology()->getVolumeVariable(ivar);
		std::memset(volumeSolution, 0, numElements * sizeof(double));
		int elementIndex = 0;
		for (spatial::Mesh<moving_boundary::CoordinateType, 2, moving_boundary::MeshElementNode>::const_iterator iter = theProblem.mesh().begin();
				iter != theProblem.mesh().end( ); ++ iter, ++ elementIndex)
		{
			const moving_boundary::MeshElementNode &e = *iter;
			const double* concValues = e.priorConcentrations();
			if (concValues != nullptr)
			{
				volumeSolution[elementIndex] = concValues[ivar];
			}
		}
		writeVariableDataSet(timeGroup, volVar, volumeSolution, numElements, "Volume");
	}
}

void Hdf5OutputWriter::writePointSolution(Group& timeGroup)
{
	for (int ips = 0; ips < theProblem.physiology()->numPointSubdomains(); ++ ips)
	{
		PointSubdomain* ps = theProblem.physiology()->getPointSubdomain(ips);

		DataType type = PointSubdomainPositionType::getType();
		hsize_t hdf5Rank = 1;
		hsize_t hdf5Dims = 1;
		DataSpace dataspace(hdf5Rank, &hdf5Dims);
		DataSet dataSet = timeGroup.createDataSet(ps->name().c_str(), type, dataspace);

		// Position
		DataSpace attributeDataSpace(H5S_SCALAR);
		StrType attributeStrType(0, 64);

		// name attribute
		Attribute attribute = dataSet.createAttribute("name", attributeStrType, attributeDataSpace);
		attribute.write(attributeStrType, ps->name().c_str());

		// type attribute
		attribute = dataSet.createAttribute("type", attributeStrType, attributeDataSpace);
		attribute.write(attributeStrType, "PointSubDomain");

		dataSet.write(ps->getPositionValues(), type, H5S_ALL, H5S_ALL);

		// for each variable, we create a dataset
		for (int ivar = 0; ivar < ps->numVariables(); ++ ivar)
		{
			int size = 1;
			PointVariable* pv = (PointVariable*)ps->getVariable(ivar);
			writeVariableDataSet(timeGroup, pv, pv->getCurrSol(), 1, "Point");
		}
	}
}

void Hdf5OutputWriter::writeVariableDataSet(Group& timeGroup, const Variable* var, double* solution, int size, const string& type)
{
		hsize_t hdf5Rank = 1;
		hsize_t hdf5Dims = size;
		DataSpace dataspace(hdf5Rank, &hdf5Dims);
		DataSet dataSet = timeGroup.createDataSet(var->name().c_str(), PredType::NATIVE_DOUBLE, dataspace);

		DataSpace attributeDataSpace(H5S_SCALAR);
		StrType attributeStrType(0, 64);

		// size
		Attribute attribute = dataSet.createAttribute("size", PredType::NATIVE_INT, attributeDataSpace);
		attribute.write(PredType::NATIVE_INT, &size);

		// name attribute
		attribute = dataSet.createAttribute("name", attributeStrType, attributeDataSpace);
		attribute.write(attributeStrType, var->name().c_str());

		// name attribute
		attribute = dataSet.createAttribute("type", attributeStrType, attributeDataSpace);
		attribute.write(attributeStrType, type.c_str());

		// write dataset
		dataSet.write(solution, PredType::NATIVE_DOUBLE, H5S_ALL, H5S_ALL);
}

void Hdf5OutputWriter::writeLog()
{
	std::ofstream logOf(logFileName.c_str(), std::ios_base::app);
	if (currentIter == 0)
	{
		logOf << MBSDATA_IDENTIFIER << std::endl;
	}
	logOf << currentIter << " " << h5FileName << " " << currentTime << std::endl;
	logOf.close();
}

void Hdf5OutputWriter::element(const moving_boundary::MeshElementNode &e) {
	// do everything in time function
}

void Hdf5OutputWriter::iterationComplete( )
{
	// do everything in time function
}

void Hdf5OutputWriter::simulationComplete( )
{
}



