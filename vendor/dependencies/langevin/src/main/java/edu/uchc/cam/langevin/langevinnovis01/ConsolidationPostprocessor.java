/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.counter.ReactionCounter;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.helpernovis.ColumnDescription;
import edu.uchc.cam.langevin.helpernovis.SolverResultSet;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.vcell.data.LangevinPostprocessor;
import org.vcell.messaging.VCellMessaging;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.*;

public class ConsolidationPostprocessor {

    public static final Logger lg = LogManager.getLogger(ConsolidationPostprocessor.class);

    private Global g;
    private final VCellMessaging vcellMessaging;
    private int numRuns;        // number of runs
    private final boolean useOutputFile;

    private String simulationName;          // model / simulation name (without extension)
    private File simulationFolder;          // top folder, where the input file is (and also the .ida and ,json files are)


    public ConsolidationPostprocessor(Global g, int numRuns, boolean useOutputFile, VCellMessaging vcellMessaging) {
        this.g = g;
        this.numRuns = numRuns;
        this.useOutputFile = useOutputFile;
        this.vcellMessaging = vcellMessaging;

        folderSetup();
    }

    public int getNumRuns() {                   // getters
        return numRuns;
    }
    public String getSimulationName() {
        return simulationName;
    }
    public File getSimulationFolder() {
        return simulationFolder;
    }
    public void setNumRuns(int numRuns) {       // setters
        this.numRuns = numRuns;
    }
    public void setSimulationName(String simulationName) {
        this.simulationName = simulationName;
    }
    public void setSimulationFolder(File simulationFolder) {
        this.simulationFolder = simulationFolder;
    }

    private void folderSetup() {
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        File inputFile = g.getInputFile();      // model / simulation input file (the .langevininput file)
        simulationName = inputFile.getName();

        String filePath = inputFile.getAbsolutePath();
        filePath = filePath.substring(0, filePath.length() - simulationName.length());  // Strip the file name off of the path
        simulationFolder = new File(filePath);

        int dotIndex = simulationName.lastIndexOf('.');     // Strip extension off the file name
        if (dotIndex > 0) {
            simulationName = simulationName.substring(0, dotIndex);
        } else {
            throw new IllegalArgumentException("Input file name must have an extension: '" + simulationName + "'");
        }
    }

    // -------------------------------------------------------------------------------------------------------
    // The simulate step writes the Run-0 per-particle trajectory ("viewer") file into a
    //   <simulationName>_FOLDER/viewer_files/<simulationName>_VIEW_Run0.txt
    // subfolder (see MySystem.folderSetup / writePositions). VCell's data layer only discovers
    // flat, canonically-named files directly in the user dir (the same place the aggregate .ida
    // files land), so copy the viewer file up to a flat name <simulationName>_VIEW_Run0.txt. This
    // is a cheap file copy (no computation); it lets VCell serve the trajectory to the SaLaD 3D
    // renderer. Absence of the viewer file is not an error (e.g. a run that produced no frames).
    //
    // Both canonicalize* methods only ADD a file beside the existing outputs — nothing the
    // standalone SpringSaLaD application reads is moved, renamed or changed.
    public void canonicalizeTrajectoryFile() {
        String viewerFileName = simulationName + "_VIEW_Run0.txt";
        Path source = simulationFolder.toPath()
                .resolve(simulationName + "_FOLDER")
                .resolve("viewer_files")
                .resolve(viewerFileName);
        canonicalize(source, viewerFileName, "trajectory viewer");
    }

    // The simulate step also writes, for each run, a SiteIDs.csv naming every site it created:
    //   <simulationName>_FOLDER/data/Run0/SiteIDs.csv
    // holding "<siteId>,<MoleculeName> Site <n> SiteType <TypeName>" per site (see
    // MySystem.writeSiteIDs). It is what lets a consumer of the Run-0 trajectory say which molecule
    // and site each particle is, rather than guessing from its color and radius. Copy it up to a
    // flat name for the same reason as the viewer file, and additionally because the _FOLDER may be
    // pruned once a run is archived, which would otherwise lose the names.
    public void canonicalizeSiteIdsFile() {
        Path source = simulationFolder.toPath()
                .resolve(simulationName + "_FOLDER")
                .resolve("data")
                .resolve("Run0")
                .resolve("SiteIDs.csv");
        canonicalize(source, simulationName + "_SiteIDs_Run0.csv", "site id");
    }

    // Copy one of the simulate step's subfolder outputs up to a flat, canonically-named file in the
    // simulation folder. A missing source is not an error: a run may legitimately not have produced
    // it, and neither file is required for the run's primary results.
    private void canonicalize(Path source, String targetName, String description) {
        Path target = simulationFolder.toPath().resolve(targetName);
        if (!Files.exists(source)) {
            lg.warn("Run-0 " + description + " file not found, skipping canonicalization: " + source);
            return;
        }
        try {
            Files.copy(source, target, StandardCopyOption.REPLACE_EXISTING);
            lg.info("Canonicalized " + description + " file to " + target);
        } catch (IOException e) {
            lg.warn("Failed to canonicalize " + description + " file " + source + " -> " + target, e);
        }
    }

    // -------------------------------------------------------------------------------------------------------
    public void calculateLangevinPrimaryStatistics() throws IOException {

        ConsolidationPostprocessorInput cpi = new ConsolidationPostprocessorInput();
        cpi.readInputFiles(this);

        Map<String, File> nameToIdaFileMap = cpi.getNameToIdaFileMap();
        Map<Integer, SolverResultSet> solverResultSetMap = cpi.getSolverResultSetMap();

        SolverResultSet tempSolverResultSet = solverResultSetMap.get(0);
        List<Double> timeInSecondsList = tempSolverResultSet.getColumn(SolverResultSet.TIME_COLUMN);

        // the output result sets
        SolverResultSet averagesResultSet = SolverResultSet.deepCopy(tempSolverResultSet, SolverResultSet.DuplicateMode.ZeroInitialize);
        SolverResultSet stdResultSet = SolverResultSet.deepCopy(tempSolverResultSet, SolverResultSet.DuplicateMode.ZeroInitialize);
        SolverResultSet minResultSet = SolverResultSet.deepCopy(tempSolverResultSet, SolverResultSet.DuplicateMode.CopyValues);
        SolverResultSet maxResultSet = SolverResultSet.deepCopy(tempSolverResultSet, SolverResultSet.DuplicateMode.CopyValues);

        int numTrials = solverResultSetMap.size();
        for(int trialIndex = 0; trialIndex < numTrials; trialIndex++) {
            SolverResultSet sourceOsrs = solverResultSetMap.get(trialIndex);
            int rowCount = sourceOsrs.getRowCount();
            for (int row = 0; row < rowCount; row++) {
                double[] sourceRowData = sourceOsrs.getRow(row);
                double[] averageRowData = averagesResultSet.getRow(row);    // destination average
                double[] minRowData = minResultSet.getRow(row);             // destination min
                double[] maxRowData = maxResultSet.getRow(row);             // destination max

                for (int i = 1; i < averageRowData.length; i++) {   // we skip time, assuming it's always the first column
                    averageRowData[i] += sourceRowData[i] / numTrials;
                    if (minRowData[i] > sourceRowData[i]) {
                        minRowData[i] = sourceRowData[i];
                    }
                    if (maxRowData[i] < sourceRowData[i]) {
                        maxRowData[i] = sourceRowData[i];
                    }
                }
            }
        }
        for(int trialIndex = 0; trialIndex < numTrials; trialIndex++) {
            SolverResultSet sourceOsrs = solverResultSetMap.get(trialIndex);
            int rowCount = sourceOsrs.getRowCount();
            for (int row = 0; row < rowCount; row++) {
                double[] sourceRowData = sourceOsrs.getRow(row);
                double[] averageRowData = averagesResultSet.getRow(row);
                double[] stdRowData = stdResultSet.getRow(row);    // destination std

                for (int i = 1; i < averageRowData.length; i++) {
                    double variance = Math.pow(sourceRowData[i] - averageRowData[i], 2);
                    stdRowData[i] += variance / numTrials;
                }
            }
        }
        int rowCount = stdResultSet.getRowCount();
        for (int row = 0; row < rowCount; row++) {
            double[] stdRowData = stdResultSet.getRow(row);
            for (int i = 1; i < stdRowData.length; i++) {
                double variance = stdRowData[i];
                stdRowData[i] = Math.sqrt(variance);
            }
        }
        ConsolidationPostprocessorOutput cpo = new ConsolidationPostprocessorOutput(this);
        cpo.writeResultFiles(averagesResultSet, stdResultSet, minResultSet, maxResultSet);
    }


    public void calculateLangevinAdvancedStatistics() throws IOException {

        ConsolidationClusterAnalizerInput cai = new ConsolidationClusterAnalizerInput();
        cai.readInputFiles(this);

        List<Double> timeInSecondsList = cai.getTimeInSecondsList();
        int numTimePoints = timeInSecondsList.size();
        int totalMolecules = getTotalMolecules(g);

        Map<Double, Map<Integer, ClusterStatisticsCalculator.Statistics>> perTimepointPerRunStatistics = new LinkedHashMap<>(); // results for individual runs
        Map<Double, ClusterStatisticsCalculator.Statistics> perTimepointMeanRunStatistics = new LinkedHashMap<>();              // results for mean run
        Map<Double, ClusterStatisticsCalculator.Statistics> perTimepointOverallRunStatistics = new LinkedHashMap<>();           // results for overall run

        for (int timepointIndex = 0; timepointIndex < numTimePoints; timepointIndex++) {
            double currentTimepointValue = timeInSecondsList.get(timepointIndex);
            Map<Integer, LangevinPostprocessor.TimePointClustersInfo> allRunsAtTimepoint = cai.getRow(currentTimepointValue);

            Map<Integer, ClusterStatisticsCalculator.Statistics> runStatisticsMap = new LinkedHashMap<>();  // compute Individual run statistics
            for (Map.Entry<Integer, LangevinPostprocessor.TimePointClustersInfo> entry : allRunsAtTimepoint.entrySet()) {

                int runIndex = entry.getKey();
                LangevinPostprocessor.TimePointClustersInfo clusterInfo = entry.getValue();
                ClusterStatisticsCalculator.Statistics stats = ClusterStatisticsCalculator.computeIndividualRunStatistics(clusterInfo);
                runStatisticsMap.put(runIndex, stats);
            }
            perTimepointPerRunStatistics.put(currentTimepointValue, runStatisticsMap);

            ClusterStatisticsCalculator.Statistics overallStats = ClusterStatisticsCalculator.computeOverallRunStatistics(
                    allRunsAtTimepoint);
            perTimepointOverallRunStatistics.put(currentTimepointValue, overallStats);

            ClusterStatisticsCalculator.Statistics meanStats = ClusterStatisticsCalculator.computeMeanRunStatistics(
                    runStatisticsMap, numRuns);
            perTimepointMeanRunStatistics.put(currentTimepointValue, meanStats);
        }

        // fill in all holes in clusterSizeFrequencyMap in preparation for file output
        int maxClusterSize = ClusterStatisticsCalculator.getMaxClusterSize(perTimepointMeanRunStatistics);
        perTimepointPerRunStatistics.forEach((timepoint, referenceStats) ->
                ClusterStatisticsCalculator.fillEmptyClusterFrequencies(referenceStats, maxClusterSize)
        );
        ClusterStatisticsCalculator.fillEmptyClusterFrequencies(perTimepointOverallRunStatistics, maxClusterSize);
        ClusterStatisticsCalculator.fillEmptyClusterFrequencies(perTimepointMeanRunStatistics, maxClusterSize);

        // write all statistic to files
        ConsolidationClusterAnalizerOutput cao = new ConsolidationClusterAnalizerOutput(this);
        cao.writeOutput(perTimepointOverallRunStatistics, perTimepointMeanRunStatistics);
//        cao.writeOutput(perTimepointPerRunStatistics,
//            ConsolidationClusterAnalizerOutput.RunStatisticsOutputMode.BY_RUN_BY_TIMEPOINT);
//        cao.writeOutput(perTimepointPerRunStatistics,
//            ConsolidationClusterAnalizerOutput.RunStatisticsOutputMode.BY_TIMEPOINT_BY_RUN);

    }


    // -------------------------------------------------------------------------------------
    static Map<String, Integer> getMolecules(Global g) {
        Map<String, Integer> molecules = new LinkedHashMap<>();
        for(GMolecule m : g.getMolecules()) {
            molecules.put(m.getName(), m.getNumber());
        }
        return molecules;
    }
    static int getTotalMolecules(Global g) {
        int totalMolecules = 0;
        for(GMolecule m : g.getMolecules()) {
            totalMolecules += m.getNumber();
        }
        return totalMolecules;
    }

}

// TODO: Attention: we may get multimodal distributions, like bimodal cluster size outputs across runs, where summary
//  statistics like mean or even standard deviation fail to convey the structure.
//  The average becomes misleading, and worse, it can mask important behaviors in the system.
//
//    What we are looking for: Distribution-Aware Postprocessing
//
//    We should enhance the aggregation code to detect and report when simulation results diverge into multiple
//    regimes. Here’s a rough sketch of how that could work:
//
//   Step 1: Collect All Scalar Cluster Size Frequencies
//        Instead of just computing averages:
//        - Collect all cluster sizes for all timepoints across all simulation runs
//        - Flatten them into a single list or histogram
//        Then we can apply a bit of statistics or signal detection.
//
//   Step 2: Detect Peaks via Density Estimation
//     Options:
//      a. Histogram peak detection:
//        - Bin cluster sizes and look for multiple local maxima
//        - Flag when there’s significant separation between peaks
//      b. Kernel density estimation (KDE):
//        - Smooth the size distribution and detect multiple modes
//        - Java libraries like Apache Commons Math or Smile can help
//      c. Gaussian Mixture Modeling (GMM):
//        - Fit multiple Gaussian components to the distribution
//        - Detect number of modes (e.g., 1 vs 2 vs 3)
//
//   Step 3: Surface Interpretive Statistics
//     If multiple peaks are detected:
//       Report them explicitly:
//        - “Detected bimodal distribution in cluster sizes: peak1 ≈ 2.1, peak2 ≈ 18.7”
//       Include per-mode statistics:
//        - Count, average, stddev per peak
//        - Fraction of total simulations contributing to each peak
//       If possible, annotate simulations that belong to each regime, that will be helpful for diagnostics
//
//    Step 4: Warn on Summary Misinterpretation
//      When multiple peaks are present:
//        - Flag summary average as nonrepresentative
//        - Instead, print a note: “Mean cluster size may obscure structural modes. Multiple distributions detected.”
//
//
// TODO:  We should prototype a simple histogram-based detector first.
//        Write the logic in Java using just arrays and basic math, so that we won't need extra libraries.
//        It'll give us a lightweight mode detector to get started—and we can later swap in KDE or GMM.
