#ifndef Hdf5OutputWriter_h
#define Hdf5OutputWriter_h

#include <MovingBoundaryTypes.h>
#include <World.h>
#include <vector>
#include <string>
#include <ReportClient.h>
#include <H5Cpp.h>
using std::vector;
using std::string;
using namespace H5;
using namespace moving_boundary;

using moving_boundary::World ;
using moving_boundary::CoordinateType;
using moving_boundary::Volume2DClass;

struct Hdf5OutputWriter :public moving_boundary::ReportClient
{
	typedef World<CoordinateType,2> WorldType;
	/**
	* HDF5 spatial chunk size (x and y dimensions)
	*/
	static const size_t spatialChunkSize = 10;
	/**
	* HDF5 spatial chunk size (time dimension)
	*/
	static const size_t timeChunkSize = 50;

	/**
	* worldDim, speciesDim index
	*/
	static const size_t timeArrayIndex = 0;

	/**
	* worldDim, speciesDim index
	*/
	static const size_t xArrayIndex = 1;

	/**
	* worldDim, speciesDim index
	*/
	static const size_t yArrayIndex = 2;

	/**
	* speciesDim index
	*/
	static const size_t speciesIndex = 3;

	static constexpr const char* H5_FILE_EXT = ".h5";
	static constexpr const char* LOG_FILE_EXT = ".log";
	static constexpr const char* MBSDATA_IDENTIFIER = "MBSData";
	static constexpr const char* DataSet_Times = "/Times";
	static constexpr const char* Group_Solution = "/Solution";
	static constexpr const char* Group_Mesh = "/Mesh";


	/**
	* @param f file to write to
	* @param mbpp the problem
	* @param baseName name of dataset in HDF5 file if not default
	*/
	Hdf5OutputWriter(std::string& a_xml, std::string& baseFileName, H5File& a_h5File, int steps, double interval, WorldType & world_, const moving_boundary::MovingBoundaryParabolicProblem &mbpp);
	~Hdf5OutputWriter();

	virtual void time(double t, unsigned int generationCounter, bool last, const moving_boundary::GeometryInfo<moving_boundary::CoordinateType> & geometryInfo);

	/**
	* state of inside / boundary nodes
	*/
	virtual void element(const moving_boundary::MeshElementNode &e);
	/**
	* notify client they've received all elements
	*/
	virtual void iterationComplete( );

	void simulationComplete( );

	std::string getXML() const
	{
		return xml;
	}

	virtual std::string outputName( ) const
	{
		return h5FileName;
	}

	bool shouldReport() const
	{
		return bShouldReport;
	}

private:
	void init();
	void writeLog();
	void writeSolution();
	void initTimesDataSet();
	void initMeshGroup();
	void writeVolumeSolution(Group& timeGroup);
	void writePointSolution(Group& timeGroup);
	void writeVariableDataSet(Group& timeGroup, const Variable* var, double* solution, int size, const string& type);

	const moving_boundary::MovingBoundaryParabolicProblem &theProblem;
	const WorldType & world;

	/**
	* XML used to create
	*/
	double* volumeSolution;
	bool bShouldReport;
	unsigned int numVolumeVariables;
	unsigned int numElements;
	std::string xml;
	std::string h5FileName;
	H5File h5File;
	DataSet timesDataSet;
	Group solutionGroup;
	Group meshGroup;
	std::string logFileName;
	double currentTime;
	int currentIter;
	bool bLastIter;
	unsigned long lastReportIteration;
	unsigned int outputNumSteps;
	double outputTimeStep;
	vector<double> timeList;
	DataSet solutionDataSet;

	WorldType::PointConverter pointconverter;
};

#endif
