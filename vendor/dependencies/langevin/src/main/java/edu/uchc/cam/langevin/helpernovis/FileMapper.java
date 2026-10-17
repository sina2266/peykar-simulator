package edu.uchc.cam.langevin.helpernovis;

import org.vcell.data.LangevinPostprocessor;
import org.vcell.data.NdJsonUtils;

import java.io.*;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class FileMapper {

    // Map<String, File> nameToIdaFileMap

    public static Map<Integer, SolverResultSet> filesToSolverResultSetMap(File directory, String prefix, String extension) throws IOException {

        Map<String, File> fileMap = getFileMapByName(directory, prefix, extension); // get filtered files

        Map<Integer, SolverResultSet> solverResultSetMap;
        solverResultSetMap = filesToSolverResultSetMap(prefix, fileMap);
        return solverResultSetMap;
    }

    /*
     * read the name to Ida file map, use it to make the solver result set map
     *    key = run index (first index is 0)
     *    value = solver result set for the run with that index
     */
    public static Map<Integer, SolverResultSet> filesToSolverResultSetMap(String prefix, Map<String, File> fileMap) throws IOException {

        Map<Integer, SolverResultSet> solverResultSetMap = new TreeMap<>();

        Pattern pattern = Pattern.compile("^" + prefix + "(?:_(\\d+))?" + "$");

        for (Map.Entry<String, File> entry : fileMap.entrySet()) {
            String fileName = entry.getKey();
            File file = entry.getValue();

            Matcher matcher = pattern.matcher(fileName);
            if (matcher.matches()) {
                int key = matcher.group(1) == null ? 0 : Integer.parseInt(matcher.group(1)); // base file is key=0

                SolverResultSet resultSet = new SolverResultSet();
                List<ColumnDescription> columnDescriptions = new ArrayList<>();
                ArrayList<double[]> values = new ArrayList<>();

                // parse file
                SolverResultSet.parseFile(file, columnDescriptions, values);
                resultSet.setColumnDescriptions(columnDescriptions);
                resultSet.getValues().addAll(values);

                solverResultSetMap.put(key, resultSet);
            }
        }
        return solverResultSetMap;
    }

    /*
     * read a directory, identifies files that match a naming convention (prefix, numeric suffix, extension)
     * returns a map with names as keys, File as values
     * sorts they key by the numeric suffix, keeps the one without a counter (if present) as first element,
     * ignores the ones with non-numeric suffixes
     */
    public static Map<String, File> getFileMapByName(File directory, String prefix, String extension) throws FileNotFoundException {

        if (directory == null || !directory.exists() || !directory.isDirectory()) {
            throw new FileNotFoundException("The specified directory does not exist: " + directory);
        }

        Map<Integer, File> sortedMap = new TreeMap<>(); // treeMap ensures numeric sorting
        File baseFile = null;       // store the file without a numeric counter, if present

        Pattern pattern = Pattern.compile("^" + prefix + "(?:_(\\d+))?" + extension + "$");

        for (File file : directory.listFiles()) {
            String fileName = file.getName();
            Matcher matcher = pattern.matcher(fileName);

            if (matcher.matches()) {
                String counterStr = matcher.group(1); // get the numeric counter if present
                if (counterStr == null) {
                    baseFile = file; // store the base file if it has no counter
                } else {
                    int counter = Integer.parseInt(counterStr);
                    sortedMap.put(counter, file);
                }
            }
        }
        LinkedHashMap<String, File> fileMap = new LinkedHashMap<>();    // merge results into LinkedHashMap (maintaining order)
        if (baseFile != null) {
            fileMap.put(prefix, baseFile); // put the base file first
        }
        sortedMap.forEach((counter, file) -> fileMap.put(prefix + "_" + counter, file));
        return fileMap;
    }

    /*
     * read the name to json file map, use it to make the result map
     *    key = run index (first index is 0)
     *    value = ClusterInfoMap for the run with that index
     */
    public static Map<Integer, Map<Double, LangevinPostprocessor.TimePointClustersInfo>> getAllRunsClusterMap(String prefix, Map<String, File> fileMap) throws IOException {

        Map<Integer, Map<Double, LangevinPostprocessor.TimePointClustersInfo>> allRunsClusterInfoMap = new TreeMap<>();

        Pattern pattern = Pattern.compile("^" + prefix + "(?:_(\\d+))?" + "$");

        for (Map.Entry<String, File> entry : fileMap.entrySet()) {
            String fileName = entry.getKey();
            File file = entry.getValue();

            Matcher matcher = pattern.matcher(fileName);
            if (matcher.matches()) {
                int key = matcher.group(1) == null ? 0 : Integer.parseInt(matcher.group(1)); // base file is key=0

                Map<Double, LangevinPostprocessor.TimePointClustersInfo> currentClusterInfoMap =
                            NdJsonUtils.loadClusterInfoMapFromNDJSON(file.toPath());
                allRunsClusterInfoMap.put(key, currentClusterInfoMap);

            }
        }
        return allRunsClusterInfoMap;
    }
//    public static void parseFile(File file, List<ColumnDescription> columnDescriptions, ArrayList<double[]> values) throws IOException {
//        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
//            // read the first line (column names)
//            String headerLine = reader.readLine();
//            if (headerLine == null) {
//                throw new IOException("empty file: " + file.getName());
//            }
//
//            String[] headers = headerLine.split(":");
//            columnDescriptions.clear();
//            for (String header : headers) {
//                columnDescriptions.add(new ColumnDescription(header));
//            }
//
//            // read and parse the data lines
//            String line;
//            while ((line = reader.readLine()) != null) {
//                String[] tokens = line.split(" ");
//                double[] rowData = Arrays.stream(tokens)
//                        .mapToDouble(Double::parseDouble)
//                        .toArray();
//                values.add(rowData);
//            }
//
//            // evaluate triviality for each column
//            for (int i = 0; i < columnDescriptions.size(); i++) {
//                columnDescriptions.get(i).evaluateTriviality(values, i);
//            }
//        }
//    }

}