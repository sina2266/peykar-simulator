package edu.uchc.cam.langevin.cli;

import picocli.CommandLine;

import java.io.IOException;
import java.io.InputStream;

public class Version implements CommandLine.IVersionProvider {
    public static final String GIT_VERSION;

    static {
        String gitHash = "unknown_version";

        try {
            Process process = Runtime.getRuntime().exec("git describe --always --dirty --long --tags");
            try (InputStream inputStream = process.getInputStream()) {
                byte[] buffer = new byte[1024];
                int bytesRead = inputStream.read(buffer);
                long start = System.currentTimeMillis();
                gitHash = (new String(buffer, 0, bytesRead).replace("\n", "")).trim();
            }
        } catch (Exception ex) {
            System.err.println("Failed to get git hash: " + ex.getMessage());
        }
        GIT_VERSION = gitHash;
    }

    public String[] getVersion() {
        return new String[]{ GIT_VERSION };
    }

}
