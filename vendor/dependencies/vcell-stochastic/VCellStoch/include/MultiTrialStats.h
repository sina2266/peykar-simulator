//
// Created by Jim Schaff on 8/11/23.
// based on code by Frank Morgan
//

#ifndef NUMERICS_MULTITRIALSTATS_H
#define NUMERICS_MULTITRIALSTATS_H

#include <vector>
#include <string>
using std::vector;
using std::string;

class MultiTrialStats {
public:
    MultiTrialStats(int numVars, int numTimePoints);

    void startNewTrial();
    void addSample(int timeIndex, double timeValue, double* varVals);

    double getMean(int varIndex, int timeIndex);
    double getVariance(int varIndex, int timeIndex);
    double getMin(int varIndex, int timeIndex);
    double getMax(int varIndex, int timeIndex);

    int getNumVars() const { return numVars; }
    int getNumTimePoints() { return timeValues.size(); }
    double getTimePoint(int timeIndex) { return timeValues[timeIndex]; }
    void writeOutput(std::string outfilename, vector<string> listOfVarNames);

private:
    void init();
    int numVars;
    int numTimePoints;
    int currentTrial;

    vector<vector<double> > mean;
    vector<vector<double> > M2; // needed for incremental variance
    vector<vector<double> > variance;
    vector<vector<double> > statMin;
    vector<vector<double> > statMax;
    vector<double> timeValues;

    void writeHDF5(std::string outfilename, vector<string> listOfVarNames);
};

#endif //NUMERICS_MULTITRIALSTATS_H
