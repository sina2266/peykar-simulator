package edu.uchc.cam.langevin.cli;

import edu.uchc.cam.langevin.logging.BundledLog4jConfigurationFactory;
import org.apache.logging.log4j.core.config.ConfigurationFactory;
import picocli.CommandLine;

@CommandLine.Command(
        name = "langevin",
        mixinStandardHelpOptions = true,
        versionProvider = Version.class,
        description = "Langevin solver and utilities.",
        subcommands = {
                RunCommand.class,
                PostCommand.class,
                WatchCommand.class
        })
public class CliMain {
    public static void main(String[] args) {
        // Pin logging to the bundled log4j2.xml before any logger is initialized, so the
        // compiled-in configuration is authoritative and cannot be redirected by the
        // log4j2.configurationFile system property / environment or an external file.
        ConfigurationFactory.setConfigurationFactory(new BundledLog4jConfigurationFactory());
        int exitCode = -1;
        try {
            CommandLine commandLine = new CommandLine(new CliMain());
            exitCode = commandLine.execute(args);
        } catch (Throwable t){
            t.printStackTrace();
        } finally {
            System.exit(exitCode);
        }
    }
}
