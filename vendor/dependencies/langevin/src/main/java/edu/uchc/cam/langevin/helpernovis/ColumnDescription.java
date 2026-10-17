package edu.uchc.cam.langevin.helpernovis;

import java.util.List;
import java.util.Objects;

public class ColumnDescription {

    private String variableName = new String();

    public ColumnDescription(String variableName) {
        this.variableName = variableName;
    }

    public String getVariableName() {
        return variableName;
    }
    public void setVariableName(String variableName) {
        this.variableName = variableName;
    }

     @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ColumnDescription that = (ColumnDescription) o;
        return Objects.equals(variableName, that.variableName);
    }

    @Override
    public int hashCode() {
        return Objects.hash(variableName);
    }

    @Override
    public String toString() {
        return "ColumnDescription{variableName='" + variableName + "'}";
    }
}
