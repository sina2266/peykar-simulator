package edu.uchc.cam.langevin.helpernovis;

import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.*;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class SolverResultSet {

    public static final Logger lg = LogManager.getLogger(SolverResultSet.class);

    public static final String TIME = "t";
    public static final String TIME_COLUMN = TIME;
    public static final int TIME_COLUMN_INDEX = 0;

    public static final String HeaderSeparator = ":";
    public static final String ValuesSeparator = " ";

    // list of rows, each row is a double[] (values of the variables at a particular timepoint
    // since time is always the first column, for each row the first double is the time
    public ArrayList<double[]> values;
    public List<ColumnDescription> columnDescriptions;  // list of variables, each entry is a variable

    public enum DuplicateMode {
        CopyValues,
        ZeroInitialize
    }
    static class MoleculeInfo {
        String bondType;
        String moleculeName;
        String siteName = "any";   // Default assumption
        String stateName = "any";  // Default assumption

        public MoleculeInfo(String bondType, String moleculeName, String siteName, String stateName) {
            this.bondType = bondType;
            this.moleculeName = moleculeName;
            this.siteName = siteName;
            this.stateName = stateName;
        }

        @Override
        public String toString() {
            return "MoleculeInfo{" +
                    "bondType='" + bondType + '\'' +
                    ", moleculeName='" + moleculeName + '\'' +
                    ", siteName='" + siteName + '\'' +
                    ", stateName='" + stateName + '\'' +
                    '}';
        }
    }

    public SolverResultSet() {
        this.columnDescriptions = new ArrayList<>();
        this.values = new ArrayList<>();
    }

    // find the column index by variable name
    private int getColumnIndex(String variableName) {
        for (int i = 0; i < columnDescriptions.size(); i++) {
            if (columnDescriptions.get(i).getVariableName().equals(variableName)) {
                return i;
            }
        }
        return -1;
    }
    // get column by variable name
    public List<Double> getColumn(String variableName) {
        int columnIndex = getColumnIndex(variableName);
        if (columnIndex == -1) {
            throw new IllegalArgumentException("Variable name not found: " + variableName);
        }
        return getColumn(columnIndex);
    }
    // get column by index
    public List<Double> getColumn(int columnIndex) {
        List<Double> columnValues = new ArrayList<>();
        for (double[] row : values) {
            columnValues.add(row[columnIndex]);
        }
        return columnValues;
    }
    // get timepoint series by time
    public double[] getValuesByTime(double time) {
        for (double[] row : values) {
            if (row[TIME_COLUMN_INDEX] == time) {
                return row;  // row where time matches
            }
        }
        throw new IllegalArgumentException("Time value not found: " + time);
    }
    // get timepoint series by index
    public double[] getValuesByIndex(int index) {
        if (index < 0 || index >= values.size()) {
            throw new IndexOutOfBoundsException("Invalid timepoint index: " + index);
        }
        return values.get(index);
    }



    public static SolverResultSet deepCopy(SolverResultSet original, DuplicateMode mode) {
        SolverResultSet copy = new SolverResultSet();
        copy.columnDescriptions = new ArrayList<>(original.columnDescriptions);
        copy.values = new ArrayList<>();
        for (double[] originalRow : original.values) {
            double[] copyRow = new double[originalRow.length];
            copyRow[0] = originalRow[0];    // we always copy time
            if(mode == DuplicateMode.CopyValues) {
                // we copy the rest of the doubles in the array, from column 1 to the last column
                System.arraycopy(originalRow, 1, copyRow, 1, originalRow.length-1);
            }
            copy.values.add(copyRow);
        }
        return copy;
    }


    public void setColumnDescriptions(List<ColumnDescription> columnDescriptions) {
        this.columnDescriptions = columnDescriptions;
    }
    public List<ColumnDescription> getColumnDescriptions() {
        return columnDescriptions;
    }
    public ArrayList<double[]> getValues() {
        return values;
    }

    public double[] getRow(int row){
        return values.get(row);
    }
    public int getRowCount(){
        return (values.size());
    }
//    public List<double[]> getRows(){
//        return Collections.unmodifiableList(this.values);       // see also getValues() above
//    }
    public ColumnDescription getColumnDescriptions(int index) {
        if(index < columnDescriptions.size()) {
            return columnDescriptions.get(index);
        } else {
            throw new ArrayIndexOutOfBoundsException("RowColumnResultSet:getColumnDescriptions(int index), index=" + index + ", total count=" + columnDescriptions.size());
        }
    }



    public static void parseFile(File file, List<ColumnDescription> columnDescriptions, ArrayList<double[]> values) throws IOException {
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            // read the first line (column names)
            String headerLine = reader.readLine();
            if (headerLine == null) {
                throw new IOException("empty file: " + file.getName());
            }

            // We don't need these. parseHeaders() does NOT reflect the modern naming conventions Do NOT use unless updated.
//            List<String> reactions = new ArrayList<>();
//            List<MoleculeInfo> molecules = new ArrayList<>();
//            parseHeaders(headerLine, reactions, molecules);


            String[] headers = headerLine.split(HeaderSeparator);
            columnDescriptions.clear();
            for (String header : headers) {
                columnDescriptions.add(new ColumnDescription(header));
            }

            // read and parse the data lines
            String line;
            while ((line = reader.readLine()) != null) {
                String[] tokens = line.split(ValuesSeparator);
                double[] rowData = Arrays.stream(tokens)
                        .mapToDouble(Double::parseDouble)
                        .toArray();
                values.add(rowData);
            }

//            // evaluate triviality for each column
//            for (int i = 0; i < columnDescriptions.size(); i++) {
//                columnDescriptions.get(i).evaluateTriviality(values, i);
//            }
        }
    }

    /**
     * Legacy header parser.
     * This method is not used anywhere in the current pipeline.
     * It does NOT reflect the modern naming conventions (ReactionType, cluster naming, etc).
     * Kept only as a reference for future refactoring. Should not be called.
     */
    @SuppressWarnings("unused")
    // separates entities in the header in bond type, molecule name, site name, state name
    // no use right now but may be useful in the future, we have molecules, reactions aso in Global
    public static void parseHeaders(String headerLine, List<String> reactions, List<MoleculeInfo> molecules) {
        String[] headers = headerLine.split(":");

//        Pattern moleculePattern = Pattern.compile("(TOTAL|FREE|BOUND)_(\\w+)(?:__(Site\\d+))?(?:__(state\\d+))?");
        Pattern moleculePattern = Pattern.compile("(TOTAL|FREE|BOUND)_([A-Za-z0-9]+)(?:__(Site\\d+))?(?:__(state\\d+))?");

        for (String header : headers) {
            if (!header.contains("__") && !header.matches("(TOTAL|FREE|BOUND)_.*")) {
                // No "__" -> It's a reaction
                reactions.add(header);
            } else {
                Matcher matcher = moleculePattern.matcher(header);
                if (matcher.matches()) {
                    String bondType = matcher.group(1);
                    String moleculeName = matcher.group(2);
                    String siteName = matcher.group(3) != null ? matcher.group(3) : "any";
                    String stateName = matcher.group(4) != null ? matcher.group(4) : "any";

                    molecules.add(new MoleculeInfo(bondType, moleculeName, siteName, stateName));
                }
            }
        }
    }

}
