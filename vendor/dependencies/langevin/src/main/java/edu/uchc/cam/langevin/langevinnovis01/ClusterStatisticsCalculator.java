package edu.uchc.cam.langevin.langevinnovis01;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.vcell.data.LangevinPostprocessor;

import java.util.*;
import java.util.stream.Collectors;

public class ClusterStatisticsCalculator {

    public static final Logger lg = LogManager.getLogger(ClusterStatisticsCalculator.class);

    public static class Statistics {
        double averageClusterSize;
        double standardDeviation;
        double averageClusterOccupancy;
        Map<Integer, Double> fractionalFrequency;
        Map<Integer, Double> fractionOfTotalMolecules;
        Map<Integer, Double> clusterSizeFrequencyMap;  // CSF (cluster size frequency), tracks frequency of each cluster size
        Map<Integer, Double> normalizedClusterSizeFrequencyMap; // normalized CSF to total clusters

        public Statistics() {
            fractionalFrequency = new LinkedHashMap<>();
            fractionOfTotalMolecules = new LinkedHashMap<>();
            clusterSizeFrequencyMap = new LinkedHashMap<>();    //  key=cluster size, value=number of clusters of that size
            normalizedClusterSizeFrequencyMap = new LinkedHashMap<>();
        }

        @Override
        public String toString() {
            return String.format("ACS: %.4f, SD: %.4f, ACO: %.4f",
                    averageClusterSize, standardDeviation, averageClusterOccupancy);
        }
    }


    // new version
    public static ClusterStatisticsCalculator.Statistics computeIndividualRunStatistics(
            LangevinPostprocessor.TimePointClustersInfo clusterInfo) {

        ClusterStatisticsCalculator.Statistics stats = new ClusterStatisticsCalculator.Statistics();

        Map<Integer, Double> csf = stats.clusterSizeFrequencyMap;   // size → count

        int totalClustersThisRun = clusterInfo.getTimePointTotalClusters();   // includes size 1+
        List<LangevinPostprocessor.ClusterInfo> nonTrivial = clusterInfo.getTimePointClusterInfoList();

        int nonTrivialClustersCount = 0;
        int moleculesInNonTrivialClusters = 0;

        // Count non-trivial clusters (size >= 2)
        for (LangevinPostprocessor.ClusterInfo ci : nonTrivial) {
            int size = ci.getSize();
            if (size < 2) continue;

            nonTrivialClustersCount++;
            moleculesInNonTrivialClusters += size;

            csf.merge(size, 1.0, Double::sum);
        }

        // Count trivial clusters (size == 1)
        int trivialClustersCount = totalClustersThisRun - nonTrivialClustersCount;
        if (trivialClustersCount > 0) {
            csf.merge(1, (double) trivialClustersCount, Double::sum);
        }
        double totalMoleculesThisRun = moleculesInNonTrivialClusters + trivialClustersCount;    // Total molecules for this run

        // ClusterSizeFrequencyMap
        stats.clusterSizeFrequencyMap =                             // sort by cluster size (key)
                csf.entrySet().stream()
                        .sorted(Map.Entry.comparingByKey())
                        .collect(Collectors.toMap(
                                Map.Entry::getKey,
                                Map.Entry::getValue,
                                (a, b) -> a,
                                LinkedHashMap::new
                        ));

        // NormalizedClusterSizeFrequencyMap (per-run)
       stats.normalizedClusterSizeFrequencyMap.clear();
        for (Map.Entry<Integer, Double> e : stats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            stats.normalizedClusterSizeFrequencyMap.put(size, count / totalClustersThisRun);
        }

        // Fractional Frequency (FF)
        stats.fractionalFrequency.clear();
        for (Map.Entry<Integer, Double> e : stats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            stats.fractionalFrequency.put(size, count / totalClustersThisRun);
        }

        // Average Cluster Size (ACS)
        stats.averageClusterSize = totalMoleculesThisRun / totalClustersThisRun;

        // Fraction of Total Molecules (FOTM)
        stats.fractionOfTotalMolecules.clear();
        for (Map.Entry<Integer, Double> e : stats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            stats.fractionOfTotalMolecules.put(size, (size * count) / totalMoleculesThisRun);
        }

        // Average Cluster Occupancy (ACO)
        double aco = 0.0;
        for (Map.Entry<Integer, Double> e : stats.fractionOfTotalMolecules.entrySet()) {
            int size = e.getKey();
            double fraction = e.getValue();
            aco += size * fraction;
        }
        stats.averageClusterOccupancy = aco;

        // Standard Deviation (SD)
        double mean = stats.averageClusterSize;
        double variance = 0.0;
        for (Map.Entry<Integer, Double> e : stats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            variance += count * Math.pow(size - mean, 2);
        }
        variance /= totalClustersThisRun;
        stats.standardDeviation = Math.sqrt(variance);

        return stats;
    }

    // we calculate agregate statistics for all runs at this timepoint
    public static ClusterStatisticsCalculator.Statistics computeOverallRunStatistics(
            Map<Integer, LangevinPostprocessor.TimePointClustersInfo> allRunsAtTimepoint) {

        ClusterStatisticsCalculator.Statistics overallStats = new ClusterStatisticsCalculator.Statistics();

        Map<Integer, Double> csf = overallStats.clusterSizeFrequencyMap; // size → count

        double totalMoleculesAllRuns = 0.0;     // sum of molecules across all runs
        double totalClustersAllRuns = 0.0;      // sum of clusters across all runs
        int numberOfRuns = allRunsAtTimepoint.size();

        int nonTrivialPerTimepoint = 0;
        for (LangevinPostprocessor.TimePointClustersInfo tpInfo : allRunsAtTimepoint.values()) {
            nonTrivialPerTimepoint += tpInfo.getTimePointClusterInfoList().size();
        }

        // Aggregate cluster counts and molecule counts across runs
        for (LangevinPostprocessor.TimePointClustersInfo tpInfo : allRunsAtTimepoint.values()) {

            int totalClustersThisRun = tpInfo.getTimePointTotalClusters();              // includes size 1+
            totalClustersAllRuns += totalClustersThisRun;
            List<LangevinPostprocessor.ClusterInfo> nonTrivial = tpInfo.getTimePointClusterInfoList(); // size >= 2

            // Count non-trivial clusters (size >= 2) for this run
            int nonTrivialClustersCount = 0;
            int moleculesInNonTrivialClusters = 0;

            for (LangevinPostprocessor.ClusterInfo ci : nonTrivial) {
                int size = ci.getSize();
                if (size < 2) continue;

                nonTrivialClustersCount++;
                moleculesInNonTrivialClusters += size;

                csf.merge(size, 1.0, Double::sum); // add one cluster of this size
            }

            // Count trivial clusters (size == 1)
            int trivialClustersCount = totalClustersThisRun - nonTrivialClustersCount;
            if (trivialClustersCount > 0) {
                csf.merge(1, (double) trivialClustersCount, Double::sum);
            }

            int totalMoleculesThisRun = moleculesInNonTrivialClusters + trivialClustersCount;   // total molecules for this run at this timepoint
            totalMoleculesAllRuns += totalMoleculesThisRun;     // accumulate across all runs at this timepoint
        }

        overallStats.clusterSizeFrequencyMap =              // sort by cluster size (key)
                csf.entrySet().stream()
                        .sorted(Map.Entry.comparingByKey())
                        .collect(Collectors.toMap(
                                Map.Entry::getKey,
                                Map.Entry::getValue,
                                (a, b) -> a,
                                LinkedHashMap::new
                        ));

        // Compute normalizedClusterSizeFrequencyMap
        overallStats.normalizedClusterSizeFrequencyMap.clear();
        for (Map.Entry<Integer, Double> e : overallStats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            overallStats.normalizedClusterSizeFrequencyMap.put(size, count / totalClustersAllRuns);
        }

        // Fractional Frequency (FF) — including trivial clusters
        overallStats.fractionalFrequency.clear();

        for (Map.Entry<Integer, Double> e : overallStats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            overallStats.fractionalFrequency.put(size, count / totalClustersAllRuns);
        }

        // Average Cluster Size (ACS)
        overallStats.averageClusterSize = totalMoleculesAllRuns / totalClustersAllRuns;

        // Fraction of Total Molecules (FOTM)
        overallStats.fractionOfTotalMolecules.clear();

        for (Map.Entry<Integer, Double> e : overallStats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            double moleculesOfThisSize = size * count;
            overallStats.fractionOfTotalMolecules.put(size, moleculesOfThisSize / totalMoleculesAllRuns);
        }

        // Average Cluster Occupancy (ACO)
        double aco = 0.0;
        for (Map.Entry<Integer, Double> e : overallStats.fractionOfTotalMolecules.entrySet()) {
            int size = e.getKey();
            double fraction = e.getValue();
            aco += size * fraction;
        }
        overallStats.averageClusterOccupancy = aco;

        // Standard Deviation (SD)
        double mean = overallStats.averageClusterSize;
        double variance = 0.0;

        for (Map.Entry<Integer, Double> e : overallStats.clusterSizeFrequencyMap.entrySet()) {
            int size = e.getKey();
            double count = e.getValue();
            variance += count * Math.pow(size - mean, 2);
        }

        variance /= totalClustersAllRuns;
        overallStats.standardDeviation = Math.sqrt(variance);

        return overallStats;
    }

    // Here we do NOT compute fractionalFrequency (FF) or fractionOfTotalMolecules (FOTM)
    // FF and FOTM are per-run ratios whose denominators vary from run to run:
    //    FF(size)  = count(size) / totalClustersThisRun
    //    FOTM(size) = (size * count(size)) / totalMoleculesThisRun
    // Because totalClustersThisRun and totalMoleculesThisRun differ across runs (due to creation/decay reactions),
    // FF and FOTM cannot be averaged meaningfully.
    // Averaging ratios with different denominators produces values that do not correspond to any real cluster or molecule distribution.
    // Therefore, mean-run statistics only average quantities that are additive or scalar per run (ACS, ACO, SD, and CSF).
    public static ClusterStatisticsCalculator.Statistics computeMeanRunStatistics(
            Map<Integer, ClusterStatisticsCalculator.Statistics> runStatisticsMap, int numRuns) {

        ClusterStatisticsCalculator.Statistics meanStats = new ClusterStatisticsCalculator.Statistics();

        double sumACS = 0.0, sumACO = 0.0;
        List<Double> acsValues = new ArrayList<>();
        Map<Integer, Double> aggregatedFrequency = new LinkedHashMap<>();

        for (ClusterStatisticsCalculator.Statistics runStats : runStatisticsMap.values()) {
            sumACS += runStats.averageClusterSize;
            sumACO += runStats.averageClusterOccupancy;
            acsValues.add(runStats.averageClusterSize); // For SD calculation

            // CSF aggregate cluster size frequencies across runs
            runStats.clusterSizeFrequencyMap.forEach((size, count) ->
                    aggregatedFrequency.merge(size, count.doubleValue(), Double::sum));
        }

        meanStats.averageClusterSize = sumACS / numRuns;
        meanStats.averageClusterOccupancy = sumACO / numRuns;
        meanStats.clusterSizeFrequencyMap = aggregatedFrequency.entrySet().stream()     // raw CSF
                .collect(Collectors.toMap(entry -> entry.getKey(), entry -> entry.getValue() / numRuns));
        meanStats.clusterSizeFrequencyMap =
                aggregatedFrequency.entrySet().stream()
                        .sorted(Map.Entry.comparingByKey())
                        .collect(Collectors.toMap(
                                Map.Entry::getKey,
                                e -> e.getValue() / numRuns,
                                (a,b)->a,
                                LinkedHashMap::new
                        ));

        // normalize CSF
        double totalClustersAcrossRuns = meanStats.clusterSizeFrequencyMap.values().stream()
                .mapToDouble(Double::doubleValue)
                .sum();
        meanStats.normalizedClusterSizeFrequencyMap = meanStats.clusterSizeFrequencyMap.entrySet().stream()
                .collect(Collectors.toMap(entry -> entry.getKey(), entry -> entry.getValue() / totalClustersAcrossRuns));

        // Compute Standard Deviation for ACS
        double varianceACS = acsValues.stream()
                .mapToDouble(acs -> Math.pow(acs - meanStats.averageClusterSize, 2))
                .sum() / numRuns;
        meanStats.standardDeviation = Math.sqrt(varianceACS);

        return meanStats;
    }

    public static void fillEmptyClusterFrequencies(Map<?, ClusterStatisticsCalculator.Statistics> allStats, int maxClusterSize) {
        // ensure all frequency maps include every cluster size up to maxClusterSize
        for (ClusterStatisticsCalculator.Statistics stats : allStats.values()) {
            for (int size = 1; size <= maxClusterSize; size++) {
                stats.clusterSizeFrequencyMap.putIfAbsent(size, 0.0);           // fill missing sizes with 0
                stats.normalizedClusterSizeFrequencyMap.putIfAbsent(size, 0.0);
            }
        }
    }
    static int getMaxClusterSize(Map<?, ClusterStatisticsCalculator.Statistics> referenceStats) {
        // key may be Double or Integer
        int maxClusterSize = referenceStats.values().stream()   // find the longest cluster size across all statistics
                .flatMap(stats -> stats.clusterSizeFrequencyMap.keySet().stream())
                .max(Integer::compare)
                .orElse(1); // default to 1 if no clusters exist
        return maxClusterSize;
    }


}
