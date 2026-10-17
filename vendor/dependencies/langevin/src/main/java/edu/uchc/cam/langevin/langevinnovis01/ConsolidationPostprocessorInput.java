package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.FileMapper;
import edu.uchc.cam.langevin.helpernovis.SolverResultSet;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Arrays;
import java.util.Map;

public class ConsolidationPostprocessorInput {

    public static final Logger lg = LogManager.getLogger(ConsolidationPostprocessorInput.class);

    private Map<String, File> nameToIdaFileMap;
    private Map<Integer, SolverResultSet> solverResultSetMap;

    public ConsolidationPostprocessorInput() {
        nameToIdaFileMap = null;
        solverResultSetMap = null;
    }

    public Map<String, File> getNameToIdaFileMap() {
        return nameToIdaFileMap;
    }
    public Map<Integer, SolverResultSet> getSolverResultSetMap() {
        return solverResultSetMap;
    }

    public void readInputFiles(ConsolidationPostprocessor cp) throws IOException {

        // read the tasks results (,ida files) and make a map where
        //   key = run name (without extension)
        //   value = task results File object
        nameToIdaFileMap = FileMapper.getFileMapByName(cp.getSimulationFolder(), cp.getSimulationName(), MySystem.IdaFileExtension);
        if(nameToIdaFileMap.size() != cp.getNumRuns()) {
            throw new RuntimeException("Expected ida file map size " + cp.getNumRuns() + " but found " + nameToIdaFileMap.size());
        }
        nameToIdaFileMap.forEach((name, file) -> lg.debug(name + " -> " + file.getAbsolutePath())
        );


        // read the name to Ida file map, use it to make the solver result set map
        //   key = run index (first index is 0)
        //   value = solver result set for the run with that index
        solverResultSetMap = FileMapper.filesToSolverResultSetMap(cp.getSimulationName(), nameToIdaFileMap);
        solverResultSetMap.forEach((key, resultSet) -> {
            lg.debug("Key: " + key);
            lg.debug("Columns: " + resultSet.getColumnDescriptions());
            lg.debug("Data:");
            resultSet.getValues().forEach(row -> lg.debug(Arrays.toString(row)));
        });
    }
}


