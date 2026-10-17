package edu.uchc.cam.langevin.langevinnovis01;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.Map;

public class ConsolidationClusterAnalizerOutput {

    private File simulationFolder;
    private String simulationName;

    public enum RunStatisticsOutputMode {   // 2 possible formats for individual runs reporting
        BY_TIMEPOINT_BY_RUN, // one file per timepoint, multiple runs inside
        BY_RUN_BY_TIMEPOINT  // one file per run, multiple timepoints inside
    }

    public ConsolidationClusterAnalizerOutput(ConsolidationPostprocessor cp) {
        simulationFolder = cp.getSimulationFolder();
        simulationName = cp.getSimulationName();
    }

    // write overall and mean statistics
    public void writeOutput(Map<Double, ClusterStatisticsCalculator.Statistics> perTimepointOverallRunStatistics,
                            Map<Double, ClusterStatisticsCalculator.Statistics> perTimepointMeanRunStatistics) {
        File overallResults = new File(simulationFolder, simulationName + "_clusters" + "_overall.csv");
        File meanResults = new File(simulationFolder, simulationName + "_clusters" + "_mean.csv");
        File countResults = new File(simulationFolder, simulationName + "_clusters_counts.csv");

        writeStatisticsToFile(overallResults, perTimepointOverallRunStatistics);
        writeStatisticsToFile(meanResults, perTimepointMeanRunStatistics);
        writeClusterSizeFrequencyToFile(countResults, perTimepointMeanRunStatistics);
    }

    // write individual run statistics
    public void writeOutput(Map<Double, Map<Integer, ClusterStatisticsCalculator.Statistics>> perTimepointPerRunStatistics,
                            RunStatisticsOutputMode mode) {

        if (mode == RunStatisticsOutputMode.BY_TIMEPOINT_BY_RUN) {
            // one file per timepoint, timepoint index used in the file name
            int timepointIndex = 0;
            for (Map.Entry<Double, Map<Integer, ClusterStatisticsCalculator.Statistics>> entry : perTimepointPerRunStatistics.entrySet()) {
                double timepoint = entry.getKey();  // using this inside the filename looks clumsy, better the timepoint index
                File file = new File(simulationFolder, simulationName + "_clusters" + "_time_" + timepointIndex + ".csv");
                writeStatisticsPerRunToFile(file, timepoint, entry.getValue());
                timepointIndex++;
            }
        } else if (mode == RunStatisticsOutputMode.BY_RUN_BY_TIMEPOINT) {
            // one file per run, iterating over timepoints
            Map<Integer, FileWriter> writers = new LinkedHashMap<>();

            try {
                // initialize file writers for each run
                for (Integer runIndex : perTimepointPerRunStatistics.values().iterator().next().keySet()) {
                    File file = new File(simulationFolder, simulationName + "_clusters" + "_run_" + runIndex + ".csv");
                    FileWriter writer = new FileWriter(file);
                    writer.write("t, Run, ACS, SD, ACO\n");
                    writers.put(runIndex, writer);
                }

                // populate files with timepoint data for each run
                for (Map.Entry<Double, Map<Integer, ClusterStatisticsCalculator.Statistics>> entry : perTimepointPerRunStatistics.entrySet()) {
                    double timepoint = entry.getKey();
                    for (Map.Entry<Integer, ClusterStatisticsCalculator.Statistics> runEntry : entry.getValue().entrySet()) {
                        FileWriter writer = writers.get(runEntry.getKey());
                        writer.write(timepoint + "," +
                                        runEntry.getKey() + "," +
                                runEntry.getValue().averageClusterSize + "," +
                                runEntry.getValue().standardDeviation + "," +
                                runEntry.getValue().averageClusterOccupancy + "\n");
                    }
                }

                for (FileWriter writer : writers.values()) {
                    writer.close();
                }

            } catch (IOException e) {
                System.err.println("Error writing run statistics files.");
                e.printStackTrace();
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------

    // helper for cluster counts, overall and mean statistics
    private void writeClusterSizeFrequencyToFile(File file, Map<Double, ClusterStatisticsCalculator.Statistics> statisticsMap) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(file))) {
            // write header with cluster sizes directly from the pre-padded map
            writer.write("t");
            statisticsMap.values().iterator().next().clusterSizeFrequencyMap.keySet().forEach(size -> {
                try {
                    writer.write("," + size);
                } catch (IOException e) {
                    throw new RuntimeException("Error writing header: ", e);
                }
            });
            writer.newLine();

            // write frequency data for each timepoint
            for (Map.Entry<Double, ClusterStatisticsCalculator.Statistics> entry : statisticsMap.entrySet()) {
                writer.write(entry.getKey().toString()); // timepoint
                for (Double freq : entry.getValue().clusterSizeFrequencyMap.values()) {
                    writer.write("," + freq);
                }
                writer.newLine();
            }
        } catch (IOException e) {
            System.err.println("Error writing cluster size frequency CSV: " + e.getMessage());
        }
    }

    private void writeStatisticsToFile(File file, Map<Double, ClusterStatisticsCalculator.Statistics> statisticsMap) {
        try (FileWriter writer = new FileWriter(file)) {
            writer.write("t, ACS, SD, ACO\n");
            for (Map.Entry<Double, ClusterStatisticsCalculator.Statistics> entry : statisticsMap.entrySet()) {
                writer.write(entry.getKey() + "," +
                        entry.getValue().averageClusterSize + "," +
                        entry.getValue().standardDeviation + "," +
                        entry.getValue().averageClusterOccupancy + "\n");
            }
        } catch (IOException e) {
            System.err.println("Error writing file: " + file.getAbsolutePath());
            e.printStackTrace();
        }
    }

    // helper for per run statistics
    private void writeStatisticsPerRunToFile(File file, double timepoint, Map<Integer, ClusterStatisticsCalculator.Statistics> statisticsMap) {
        try (FileWriter writer = new FileWriter(file)) {
            writer.write("t, Run, ACS, SD, ACO\n");
            for (Map.Entry<Integer, ClusterStatisticsCalculator.Statistics> entry : statisticsMap.entrySet()) {
                writer.write(timepoint + "," +
                        entry.getKey() + "," +
                        entry.getValue().averageClusterSize + "," +
                        entry.getValue().standardDeviation + "," +
                        entry.getValue().averageClusterOccupancy + "\n");
            }
        } catch (IOException e) {
            System.err.println("Error writing file: " + file.getAbsolutePath());
            e.printStackTrace();
        }
    }
}
