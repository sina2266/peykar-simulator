package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.FileMapper;
import edu.uchc.cam.langevin.helpernovis.SolverResultSet;
import org.vcell.data.LangevinPostprocessor;
import org.vcell.data.NdJsonUtils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.file.Path;
import java.util.*;


public class ConsolidationClusterAnalizerInput {

    // key = name of simulation run, value = the json file with cluster info map for that simulation
    private Map<String, File> nameToJsonFileMap;

    // key = run index, value = cluster info map for that run
    private Map<Integer, Map<Double, LangevinPostprocessor.TimePointClustersInfo>> allRunsClusterInfoMap;

    public ConsolidationClusterAnalizerInput() {
        nameToJsonFileMap = null;
        allRunsClusterInfoMap = null;
    }

    public Map<String, File> getNameToJsonFileMap() {
        return nameToJsonFileMap;
    }
    public Map<Integer, Map<Double, LangevinPostprocessor.TimePointClustersInfo>> getAllRunsClusterInfoMap() {
        return allRunsClusterInfoMap;
    }

    //---------------------------------------------------------------------------------------------
    public void readInputFiles(ConsolidationPostprocessor cp) throws IOException {

        nameToJsonFileMap = FileMapper.getFileMapByName(cp.getSimulationFolder(), cp.getSimulationName(), MySystem.ClustersFileExtension);
        nameToJsonFileMap.forEach((name, file) -> System.out.println(name + " -> " + file.getAbsolutePath()));    // show results

        allRunsClusterInfoMap = FileMapper.getAllRunsClusterMap(cp.getSimulationName(), nameToJsonFileMap);
    }

    public Map<Integer, LangevinPostprocessor.TimePointClustersInfo> getRow(double currentTimepointValue) {
        Map<Integer, LangevinPostprocessor.TimePointClustersInfo> allRunsRowMap = new LinkedHashMap<> ();
        for(int runIndex = 0; runIndex < allRunsClusterInfoMap.size(); runIndex++) {
            Map<Double, LangevinPostprocessor.TimePointClustersInfo> currentRunClusterInfoMap = allRunsClusterInfoMap.get(runIndex);
            LangevinPostprocessor.TimePointClustersInfo timePointClustersInfo = currentRunClusterInfoMap.get(currentTimepointValue);
            allRunsRowMap.put(runIndex, timePointClustersInfo);
        }
        return allRunsRowMap;
    }
    public LangevinPostprocessor.TimePointClustersInfo getRow(double currentTimepointValue, int runIndex) {
        Map<Double, LangevinPostprocessor.TimePointClustersInfo> currentRunClusterInfoMap = allRunsClusterInfoMap.get(runIndex);
        LangevinPostprocessor.TimePointClustersInfo timePointClustersInfo = currentRunClusterInfoMap.get(currentTimepointValue);
        return timePointClustersInfo;
    }

    public List<Double> getTimeInSecondsList() {
        List<Double> timeInSecondsList = new ArrayList<> ();
        Map<Double, LangevinPostprocessor.TimePointClustersInfo> currentRunClusterInfoMap = allRunsClusterInfoMap.get(0);
        for (Map.Entry<Double, LangevinPostprocessor.TimePointClustersInfo> entry : currentRunClusterInfoMap.entrySet()) {
            double key = entry.getKey();
            timeInSecondsList.add(key);
        }
        return timeInSecondsList;
    }
}
