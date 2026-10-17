/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
/////////////////////////////////////////////////////////////
// SimTool.h
///////////////////////////////////////////////////////////
#ifndef SIMTOOL_H
#define SIMTOOL_H
#include <filesystem>

#include <VCELL/Timer.h>
#include <VCELL/SundialsSolverOptions.h>

#include <smoldyn.h>


#ifndef DIRECTORY_SEPARATOR
#if ( defined(WIN32) || defined(WIN64) )
#define DIRECTORY_SEPARATOR '\\'
#else
#define DIRECTORY_SEPARATOR '/'
#endif
#endif

class VCellModel;
class Mesh;
class SimulationExpression;
class Variable;
class PostProcessingHdf5Writer;

class SimTool {
public:
	SimTool();
	virtual ~SimTool();

	virtual void start();
	virtual void loadFinal();

	void setModel(VCellModel* model);
	void setSimulation(SimulationExpression* sim);
	void setTimeStep(double period);
	void setSmoldynStepMultiplier(int steps);
	void setCheckSpatiallyUniform();

	bool isCheckingSpatiallyUniform() const { return bCheckSpatiallyUniform; }
	void setEndTimeSec(double timeSec) { simEndTime = timeSec; }
	double getEndTime() const { return simEndTime; }

	void setKeepEvery(int ke) { keepEvery = ke; }
	void setKeepAtMost(int kam) { keepAtMost = kam; }
	void setBaseFilename(const std::filesystem::path& fname);
	[[nodiscard]] std::filesystem::path getBaseFileName() const { return baseFileName; }
	[[nodiscard]] std::filesystem::path getBaseDirName() const { return baseDirName; }
	void setStoreEnable(bool enable) { bStoreEnable = enable; }
	void setFileCompress(bool compress) { bSimFileCompress = compress; }
	void requestNoZip();

	SimulationExpression* getSimulation() const { return simulation; }
	VCellModel* getModel() const { return vcellModel; }
	static bool checkStopRequested();
	TimerHandle getTimerHandle(string& timerName);
	void        startTimer(TimerHandle hnd) const;
	void        stopTimer(TimerHandle hnd) const;
	double      getElapsedTimeSec(TimerHandle hnd) const;
	virtual void showSummary(FILE *fp);

	void setSolver(const string& s);
	bool isSundialsPdeSolver() const;

	void setDiscontinuityTimes(int num, double* times) {
		numDiscontinuityTimes = num;
		discontinuityTimes = times;
	}
	int getNumDiscontinuityTimes() const { return numDiscontinuityTimes; }
	double* getDiscontinuityTimes() const { return discontinuityTimes; }
	
	void setSundialsSolverOptions(const SundialsSolverOptions& sso) {
		sundialsSolverOptions = sso;
	}

	const SundialsSolverOptions& getSundialsSolverOptions() const { return sundialsSolverOptions; }

	void setSpatiallyUniformErrorTolerance(double atol, double rtol) {
		spatiallyUniformAbsTol = atol;
		spatiallyUniformRelTol = rtol;
	}

	void setPCGRelativeErrorTolerance(double rtol) {
		pcgRelTol = rtol;
	}
	double getPCGRelativeErrorTolerance() const
	{
		return pcgRelTol;
	}

	double getSimStartTime() const { return simStartTime; }
	void setSundialsOneStepOutput() { bSundialsOneStepOutput = true; }
	bool isSundialsOneStepOutput() const { return bSundialsOneStepOutput; }
	
	void setSerialParameterScans(int numScans, double** values);
	void setLoadFinal(bool b) {
		bLoadFinal = b;
	}
	void checkTaskIdLockFile() const;

	void setSmoldynInputFile(const string& inputfile);

private:
	FILE* lockForReadWrite() const;

	bool checkSpatiallyUniform(Variable*);	
	void updateLog(double progress,double time,int iteration);
	void clearLog();
	static int	getZipCount(const std::filesystem::path& zipFileName);
	void start1();
	void copyParticleCountsToConcentration();

	static SimTool* instance;

	bool bSimZip;
	VCellModel* vcellModel;
	SimulationExpression  *simulation;
	Timer  *_timer;

	bool bSimFileCompress;
	double simEndTime;
	double simStartTime;
	bool bCheckSpatiallyUniform;
	double simDeltaTime;
	int smoldynStepMultiplier;
	int keepEvery;
	bool bStoreEnable;
	std::filesystem::path baseFileName;
	int simFileCount;
	std::filesystem::path baseSimName;
	std::filesystem::path baseDirName;
	int zipFileCount;
	string solver;
	double* discontinuityTimes;
	int numDiscontinuityTimes;
	bool bLoadFinal;

	SundialsSolverOptions sundialsSolverOptions;
	double spatiallyUniformAbsTol, spatiallyUniformRelTol;
	double pcgRelTol;

	bool bSundialsOneStepOutput;
	int keepAtMost;

	double** serialScanParameterValues;
	int numSerialParameterScans;

	PostProcessingHdf5Writer* postProcessingHdf5Writer;
	simptr smoldynSim;
	string smoldynInputFile;
};

#endif
