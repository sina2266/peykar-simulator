package org.vcell.data;

import com.fasterxml.jackson.databind.ObjectMapper;

public class EnvCheck {

    public static void main(String[] args) {
        System.out.println("==== Java Environment Diagnostic ====");

        System.out.println("java.version          = " + System.getProperty("java.version"));
        System.out.println("java.runtime.name     = " + System.getProperty("java.runtime.name"));
        System.out.println("java.vm.name          = " + System.getProperty("java.vm.name"));
        System.out.println("java.vendor           = " + System.getProperty("java.vendor"));
        System.out.println("os.name               = " + System.getProperty("os.name"));

        // Check if running inside a GraalVM native image
        try {
            boolean inImage = Class.forName("org.graalvm.nativeimage.ImageInfo")
                    .getMethod("inImageCode")
                    .invoke(null)
                    .equals(Boolean.TRUE);
            System.out.println("Detected GraalVM native image runtime = " + inImage);
        } catch (Exception e) {
            System.out.println("Not running as a native image (no org.graalvm.nativeimage.ImageInfo available)");
        }

        // Bonus: Check Jackson's take on your class
        try {
            ObjectMapper mapper = new ObjectMapper();
            System.out.println("Attempting to introspect: TimePointClustersInfo");
            System.out.println(mapper.writerWithDefaultPrettyPrinter()
                    .writeValueAsString(new org.vcell.data.LangevinPostprocessor.TimePointClustersInfo()));
        } catch (Exception e) {
            System.out.println("Jackson failed to serialize: " + e.getMessage());
        }
    }
}
