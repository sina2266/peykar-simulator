package org.vcell.data;

import java.io.*;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.*;

public class Resource {

    /**
     * Gets a URL to a resource in the classpath.
     */
    public static URL getResource(String path) {
        ClassLoader cl = Thread.currentThread().getContextClassLoader();
        URL url = cl.getResource(path);
        if (url == null) {
            throw new IllegalArgumentException("Resource not found: " + path);
        }
        return url;
    }

    /**
     * Copies the contents of a classpath resource to a File.
     */
    public static void copyToFile(URL resourceUrl, File targetFile) throws IOException {
        try (InputStream in = resourceUrl.openStream();
             OutputStream out = new FileOutputStream(targetFile)) {
            in.transferTo(out);  // Java 9+; for Java 8 use a buffer loop
        }
    }

    public static void copyFilesWithExtension(URL resourceDirUrl, Path targetDir, String extension) throws IOException, URISyntaxException {
        Path sourceDir = Paths.get(resourceDirUrl.toURI());

        try (DirectoryStream<Path> stream = Files.newDirectoryStream(sourceDir, "*" + extension)) {
            for (Path sourceFile : stream) {
                Path targetFile = targetDir.resolve(sourceFile.getFileName());
                Files.copy(sourceFile, targetFile, StandardCopyOption.REPLACE_EXISTING);
            }
        }
    }

    public static void copyFolderRecursively(URL sourceFolderUrl, Path targetDir) throws IOException, URISyntaxException {
        Path sourcePath = Paths.get(sourceFolderUrl.toURI());

        Files.walk(sourcePath).forEach(source -> {
            try {
                Path relative = sourcePath.relativize(source);
                Path target = targetDir.resolve(relative);
                if (Files.isDirectory(source)) {
                    Files.createDirectories(target);
                } else {
                    Files.copy(source, target, StandardCopyOption.REPLACE_EXISTING);
                }
            } catch (IOException e) {
                throw new UncheckedIOException("Failed to copy: " + source, e);
            }
        });
    }

}
