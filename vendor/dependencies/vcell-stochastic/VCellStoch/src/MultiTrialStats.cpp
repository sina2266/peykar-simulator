//
// Created by Jim Schaff on 8/11/23.
//

#include "MultiTrialStats.h"
#include <vector>
#include <iostream>
#include <limits>
#include <math.h>

using std::vector;
using std::string;

MultiTrialStats::MultiTrialStats(int numVars, int numTimePoints) {
    this->numVars = numVars;
    this->numTimePoints = numTimePoints;
    currentTrial = 0;
    init();
}

void MultiTrialStats::init() {
    mean.resize(numTimePoints);
    M2.resize(numTimePoints);
    variance.resize(numTimePoints);
    statMin.resize(numTimePoints);
    statMax.resize(numTimePoints);
    for (int i = 0; i < numTimePoints; ++i) {
        mean[i].resize(numVars,0);
        M2[i].resize(numVars,0);
        variance[i].resize(numVars,0);
        statMin[i].resize(numVars,std::numeric_limits<double>::max());
        statMax[i].resize(numVars,std::numeric_limits<double>::min());
    }
}

void MultiTrialStats::addSample(int timeIndex, double timeValue, double *varVals) {
    if (timeValues.size() <= timeIndex){
        timeValues.push_back(timeValue);
    }
    for (int i = 0; i < numVars; ++i) {
        double currValue = varVals[i];
        double delta = currValue - mean[timeIndex][i];
        mean[timeIndex][i] += delta / (currentTrial);
        M2[timeIndex][i] += delta * (currValue - mean[timeIndex][i]);
        variance[timeIndex][i] = M2[timeIndex][i] / (currentTrial-1);
        statMin[timeIndex][i] = std::min(currValue, statMin[timeIndex][i]) ;
        statMax[timeIndex][i] = std::max(currValue, statMax[timeIndex][i]) ;
    }
}

double MultiTrialStats::getMean(int varIndex, int timeIndex) {
    return mean[timeIndex][varIndex];
}

double MultiTrialStats::getVariance(int varIndex, int timeIndex) {
    return variance[timeIndex][varIndex];
}

double MultiTrialStats::getMin(int varIndex, int timeIndex) {
    return statMin[timeIndex][varIndex];
}

double MultiTrialStats::getMax(int varIndex, int timeIndex) {
    return statMax[timeIndex][varIndex];
}

void MultiTrialStats::startNewTrial() {
    currentTrial += 1;
}

void MultiTrialStats::writeOutput(std::string outfilename, vector<string> listOfVarNames) {
    writeHDF5(outfilename, listOfVarNames);
}

#ifdef USE_HDF5
#include <hdf5.h>

// Writes <outfilename>_hdf5, the file VCell reads for multi-trial (non-histogram) runs
// (SimulationData / MultiTrialNonspatialStochSimDataReader):
//   VarNames   string[numVars]            variable-length strings
//   SimTimes   double[numTimes]
//   StatMean, StatMin, StatMax, StatStdDev   double[numTimes][numVars]
// Same layout as the vcell-solvers implementation, without its variable-length stack
// arrays (not portable, and large runs could overflow the stack).
void MultiTrialStats::writeHDF5(string outfilename, vector<string> listOfVarNames){
    string ofhdf5(outfilename);
    ofhdf5.append("_hdf5");

    const hsize_t numTimes = timeValues.size();
    const hsize_t numVarNames = listOfVarNames.size();
    bool ok = true;

    hid_t file = H5Fcreate(ofhdf5.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file < 0) {
        std::cerr << "Error creating HDF5 file " << ofhdf5 << std::endl;
        return;
    }

    // variable names
    {
        hsize_t dims[1] = { numVarNames };
        hid_t space = H5Screate_simple(1, dims, NULL);
        hid_t strType = H5Tcopy(H5T_C_S1);
        H5Tset_size(strType, H5T_VARIABLE);
        hid_t dset = H5Dcreate2(file, "VarNames", strType, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        vector<const char*> chars;
        chars.reserve(listOfVarNames.size());
        for (const auto& name : listOfVarNames) {
            chars.push_back(name.c_str());
        }
        ok = ok && dset >= 0 && H5Dwrite(dset, strType, H5S_ALL, H5S_ALL, H5P_DEFAULT, chars.data()) >= 0;
        H5Dclose(dset);
        H5Tclose(strType);
        H5Sclose(space);
    }

    // time points
    {
        hsize_t dims[1] = { numTimes };
        hid_t space = H5Screate_simple(1, dims, NULL);
        hid_t dset = H5Dcreate2(file, "SimTimes", H5T_NATIVE_DOUBLE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        ok = ok && dset >= 0 && H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, timeValues.data()) >= 0;
        H5Dclose(dset);
        H5Sclose(space);
    }

    // statistics, row-major [time][var]; the variance is written as a standard deviation
    {
        hsize_t dims[2] = { numTimes, numVarNames };
        hid_t space = H5Screate_simple(2, dims, NULL);
        const char* statNames[4] = { "StatMean", "StatMin", "StatMax", "StatStdDev" };
        const vector<vector<double> >* stats[4] = { &mean, &statMin, &statMax, &variance };
        vector<double> buffer(numTimes * numVarNames);
        for (int statIndex = 0; statIndex < 4; statIndex++) {
            for (hsize_t t = 0; t < numTimes; ++t) {
                for (hsize_t v = 0; v < numVarNames; ++v) {
                    double value = (*stats[statIndex])[t][v];
                    buffer[t * numVarNames + v] = (statIndex == 3) ? sqrt(value) : value;
                }
            }
            hid_t dset = H5Dcreate2(file, statNames[statIndex], H5T_NATIVE_DOUBLE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            ok = ok && dset >= 0 && H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()) >= 0;
            H5Dclose(dset);
        }
        H5Sclose(space);
    }

    if (H5Fclose(file) < 0) {
        ok = false;
    }
    if (!ok) {
        std::cerr << "Error writing HDF5 file " << ofhdf5 << std::endl;
    }
}
#endif

#ifdef USE_PARQUET
#include <arrow/api.h>
#include <parquet/arrow/writer.h>

void MultiTrialStats::writeParquet(std::string outfilename, vector<string> listOfVarNames) {
    // Placeholder for Parquet implementation
    // To be implemented in next phase
    std::cout << "Parquet output not yet implemented" << std::endl;
}
#endif
