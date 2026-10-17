package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.ColumnDescription;
import edu.uchc.cam.langevin.helpernovis.SolverResultSet;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;


public class ConsolidationPostprocessorOutput {

    public static final Logger lg = LogManager.getLogger(ConsolidationPostprocessorOutput.class);

    private File simulationFolder;
    private String simulationName;

    public ConsolidationPostprocessorOutput(ConsolidationPostprocessor cp) {
        simulationFolder = cp.getSimulationFolder();
        simulationName = cp.getSimulationName();
    }

    public enum ResultType {
        AVG("Avg", "Average"),
        STD("Std", "Standard Deviation"),
        MIN("Min", "Minimum"),
        MAX("Max", "Maximum");

        private final String suffix;
        private final String description;

        ResultType(String suffix, String description) {
            this.suffix = suffix;
            this.description = description;
        }

        public String getSuffix() {
            return suffix;
        }

        public String getDescription() {
            return description;
        }

        public static ResultType fromSuffix(String suffix) {
            for (ResultType rt : ResultType.values()) {
                if (rt.suffix.equalsIgnoreCase(suffix)) {
                    return rt;
                }
            }
            throw new IllegalArgumentException("No matching enum for suffix: " + suffix);
        }

        public static ResultType fromDescription(String description) {
            for (ResultType rt : ResultType.values()) {
                if (rt.description.equalsIgnoreCase(description)) {
                    return rt;
                }
            }
            throw new IllegalArgumentException("No matching enum for description: " + description);
        }
    }


    public void writeResultFiles(SolverResultSet averagesResultSet, SolverResultSet stdResultSet,
                                 SolverResultSet minResultSet, SolverResultSet maxResultSet) throws IOException {

        writeCsvFile(averagesResultSet, ResultType.AVG);
        writeCsvFile(stdResultSet, ResultType.STD);
        writeCsvFile(minResultSet, ResultType.MIN);
        writeCsvFile(maxResultSet, ResultType.MAX);
    }

    private void writeCsvFile(SolverResultSet resultSet, ResultType resultType) throws IOException {

        String fileName = simulationName + "_" + resultType.getSuffix() + ".ida";
        File outputFile = new File(simulationFolder, fileName);

        try (FileWriter writer = new FileWriter(outputFile)) {

            List<ColumnDescription> columns = resultSet.getColumnDescriptions();    // Write header from column descriptions
            for (int i = 0; i < columns.size(); i++) {
                writer.append(columns.get(i).getVariableName());
                if (i < columns.size() - 1) {
                    writer.append(SolverResultSet.HeaderSeparator);     // use same delimiters as for the input ida files
                }
            }
            writer.append("\n");

            for (double[] row : resultSet.getValues()) {
                for (int i = 0; i < row.length; i++) {
                    writer.append(Double.toString(row[i]));
                    if (i < row.length - 1) {
                        writer.append(SolverResultSet.ValuesSeparator);
                    }
                }
                writer.append("\n");
            }
        }
    }

}
