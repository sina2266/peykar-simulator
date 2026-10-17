package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.cli.CliMain;
import edu.uchc.cam.langevin.helpernovis.FileMapper;
import edu.uchc.cam.langevin.helpernovis.SolverResultSet;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;
import picocli.CommandLine;

import java.io.*;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotEquals;

public class CliTest {

    String inputFileContents =
            """
            Total time: 0.0025
            dt: 1.0E-8
            dt_data: 5.0E-4
            dt_spring: 1.0E-9
            dt_image: 1.0E-4
            
            *** SYSTEM INFORMATION ***
            L_x: 0.1
            L_y: 0.1
            L_z_out: 0.01
            L_z_in: 0.09
            Partition Nx: 10
            Partition Ny: 10
            Partition Nz: 10
            
            *** MOLECULES ***
            
            MOLECULE: "MT0" Intracellular Number 20 Site_Types 2 Total_Sites 2 Total_Links 1 is2D false
            {
                 TYPE: Name "Site0" Radius 1.00000 D 1.000 Color RED STATES "state0"
                 TYPE: Name "Site1" Radius 1.00000 D 1.000 Color RED STATES "state0" "state1"
            
                 SITE 0 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site0" Radius 1.00000 D 1.000 Color RED STATES "state0"
                      x 0.00000 y 0.00000 z 0.00000
                 SITE 1 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site1" Radius 1.00000 D 1.000 Color RED STATES "state0" "state1"
                      x 0.00000 y 0.00000 z 0.00000
            
                 LINK: Site 0 ::: Site 1
            
                 Initial_Positions: Random
            }
            
            *** MOLECULE FILES ***
            
            MOLECULE: MT0 null
            
            *** CREATION/DECAY REACTIONS ***
            
            'MT0' : kcreate  0  kdecay  0
            
            *** STATE TRANSITION REACTIONS ***
            
            'r0' ::     'MT0' : 'Site1' : 'state0' --> 'state1'  Rate 50.0  Condition Free
            
            *** ALLOSTERIC REACTIONS ***
            
            
            *** BIMOLECULAR BINDING REACTIONS ***
            
            'r1'       'MT0' : 'Site0' : 'Any_State'  +  'MT0' : 'Site0' : 'Any_State'  kon  10.0  koff 23.0  Bond_Length 1.0
            
            *** MOLECULE COUNTERS ***
            
            'MT0' : Measure Total Free Bound
            
            *** STATE COUNTERS ***
            
            'MT0' : 'Site0' : 'state0' : Measure Total Free Bound
            'MT0' : 'Site1' : 'state0' : Measure Total Free Bound
            'MT0' : 'Site1' : 'state1' : Measure Total Free Bound
            
            *** BOND COUNTERS ***
            
            'r1' : Counted
            
            *** SITE PROPERTY COUNTERS ***
            
            'MT0' Site 0 : Track Properties true
            'MT0' Site 1 : Track Properties true
            
            *** CLUSTER COUNTERS ***
            
            Track_Clusters: true
            
            *** SYSTEM ANNOTATIONS ***
            
            
            *** MOLECULE ANNOTATIONS ***
            
            
            *** REACTION ANNOTATIONS ***
            
            
            *** SIMULATION OPTIONS ***
            
            RandomSeed: 164200191287356961681
            
            """;

    private void deleteDirectory(File directoryToBeDeleted) {
        File[] allContents = directoryToBeDeleted.listFiles();
        if (allContents != null) {
            for (File file : allContents) {
                deleteDirectory(file);
            }
        }
        directoryToBeDeleted.delete();
    }

    String parent_dir = "C:/TEMP/langevin-cli-test";
    String sim_base_name = "sim";
    String temp_dir_name = "test_simulation";
    int runCounter = 0;

// --------------------------------------------------------------------------

//    @Test
//    public void testConsolidation() throws IOException {
//
//        int numRuns = 2;
//
//        String simulationFolderName = parent_dir + File.separator + temp_dir_name;
//        File simulationFolder = new File(simulationFolderName);
//
//        // read the tasks results (,ida files) and make a map where key = run name, value = task results File object
//        Map<String, File> nameToIdaFileMap = FileMapper.getFileMapByName(simulationFolder, sim_base_name, MySystem.IdaFileExtension);
//        nameToIdaFileMap.forEach((name, file) -> System.out.println(name + " -> " + file.getAbsolutePath()));    // show results
//        assertTrue(nameToIdaFileMap.size() == numRuns, "expected size " + numRuns + " but found " + nameToIdaFileMap.size());
////        assertEquals(numRuns, fileMap.size());
//
//        // read the name to Ida file map, key = run index (first index is 0), value = solver result set for the run with that index
//        Map<Integer, SolverResultSet> solverResultSetMap = FileMapper.filesToSolverResultSetMap(sim_base_name, nameToIdaFileMap);
//        solverResultSetMap.forEach((key, resultSet) -> {
//            System.out.println("Key: " + key);
//            System.out.println("Columns: " + resultSet.getColumnDescriptions());
//            System.out.println("Data:");
//            resultSet.getValues().forEach(row -> System.out.println(Arrays.toString(row)));
//        });
//        assertTrue(solverResultSetMap.size() == numRuns, "expected size " + numRuns + " but found " + solverResultSetMap.size());
//
//    }

    @Test
    public void testRunAndPostCommand() throws IOException, InterruptedException {

//        Path parentFolder = Paths.get(parent_dir);
//        Path tempDirectory = parentFolder.resolve(temp_dir_name);   // use a convenient location for debugging
//        Files.createDirectories(tempDirectory);

        Path tempDirectory = Files.createTempDirectory(temp_dir_name);        // correct temp location for automatic testing
        Path modelFile = tempDirectory.resolve(sim_base_name+".langevinInput");
        Path logFile_0 = tempDirectory.resolve(sim_base_name+".log");
        Path logFile_1 = tempDirectory.resolve(sim_base_name+"_1.log");
        Path logFile_P = tempDirectory.resolve(sim_base_name+"_P.log");
        Path idaFile_0 = tempDirectory.resolve(sim_base_name+".ida");
        Path idaFile_1 = tempDirectory.resolve(sim_base_name+"_1.ida");
        Path jsonClustersFile_0 = tempDirectory.resolve(sim_base_name+".json");
        Path jsonClustersFile_1 = tempDirectory.resolve(sim_base_name+"_1.json");

        Files.writeString(modelFile, inputFileContents);
//        VCellMessaging vcellMessaging = new VCellMessagingLocal();

        assertEquals(true, modelFile.toFile().exists(), "Model file should exist");

        String[] args = {       // command arguments, run 0
                "simulate",
                modelFile.toFile().getAbsolutePath(),   // Langevin model file
                "0",                               // we absolutely need a run counter of 0 to properly initialize dirs
                "--output-log", logFile_0.toFile().getAbsolutePath(), // Output log file
                "--vc-print-status"                // Enable status printing
        };
        String[] args1 = {       // command arguments, run 1
                "simulate",
                modelFile.toFile().getAbsolutePath(),   // Langevin model file
                "1",
                "--output-log", logFile_1.toFile().getAbsolutePath(), // Output log file
                "--vc-print-status"                // Enable status printing
        };

        int exitCode = -1;
        try {
            CommandLine cmd = new CommandLine(new CliMain());
            exitCode = cmd.execute(args);

            assertEquals(0, exitCode, "Expected command to execute successfully");
            assertEquals(true, logFile_0.toFile().exists(), "Log file 0 should exist");
            assertEquals(true, idaFile_0.toFile().exists(), "ida file 0 should exist");
            assertEquals(true, jsonClustersFile_0.toFile().exists(), "json clusters file 0 should exist");

            exitCode = cmd.execute(args1);
            assertEquals(0, exitCode, "Expected command to execute successfully");
            assertEquals(true, logFile_1.toFile().exists(), "Log file 1 should exist");
            assertEquals(true, idaFile_1.toFile().exists(), "ida file 1 should exist");
            assertEquals(true, jsonClustersFile_1.toFile().exists(), "json clusters file 1 should exist");

            // -------------------------------------------------------------------------------------------------

            // final results after postprocessing
            Path consolidated_Avg = tempDirectory.resolve(sim_base_name + "_Avg" + ".ida");
            Path consolidated_Max = tempDirectory.resolve(sim_base_name + "_Max" + ".ida");
            Path consolidated_Min = tempDirectory.resolve(sim_base_name + "_Min" + ".ida");
            Path consolidated_Std = tempDirectory.resolve(sim_base_name + "_Std" + ".ida");
            Path clusters_Mean = tempDirectory.resolve(sim_base_name + "_clusters_mean" + ".csv");
            Path clusters_Overall = tempDirectory.resolve(sim_base_name + "_clusters_overall" + ".csv");
            Path clusters_Counts = tempDirectory.resolve(sim_base_name + "_clusters_counts" + ".csv");

            String[] argsP = {       // command arguments, postprocessing run
                    "postprocess",
                    modelFile.toFile().getAbsolutePath(),   // Langevin model file
                    "2",        // number of runs
                    "--output-log", logFile_P.toFile().getAbsolutePath(), // Output log file
                    "--vc-print-status"                // Enable status printing
            };
            exitCode = cmd.execute(argsP);
            assertEquals(0, exitCode, "Expected command to execute successfully");
            assertEquals(true, consolidated_Avg.toFile().exists(), consolidated_Avg.toFile().getName() + " should exist");
            assertEquals(true, consolidated_Max.toFile().exists(), consolidated_Max.toFile().getName() + " should exist");
            assertEquals(true, consolidated_Min.toFile().exists(), consolidated_Min.toFile().getName() + " should exist");
            assertEquals(true, consolidated_Std.toFile().exists(), consolidated_Std.toFile().getName() + " should exist");
            assertEquals(true, clusters_Mean.toFile().exists(), clusters_Mean.toFile().getName() + " should exist");
            assertEquals(true, clusters_Overall.toFile().exists(), clusters_Overall.toFile().getName() + " should exist");
            assertEquals(true, clusters_Counts.toFile().exists(), clusters_Counts.toFile().getName() + " should exist");

            File fileToDelete = modelFile.toFile();
            int attempts = 0;
            while (fileToDelete.exists() && attempts < 10) {
                System.gc(); // hint JVM to release file handles
                try {
                    Files.deleteIfExists(modelFile);
                } catch (IOException e) {
                    System.err.println("Failed to delete file, retrying...");
                }
                attempts++;
                Thread.sleep(200);
            }
        } finally {
            // uncomment this for automatic run
            deleteDirectory(tempDirectory.toFile());
            System.out.println("finally!");
        }
    }

    @Test
    public void testInvalidModelFile() {
        // Simulate arguments with an invalid model file path
        String[] args = {
                "simulate",
                "nonexistent_model.txt", // Nonexistent model file
                "0"                      // Run counter
        };

        CommandLine cmd = new CommandLine(new CliMain());
        int exitCode = cmd.execute(args);

        // Verify the exit code (1 means error)
        assertEquals(1, exitCode, "Expected error exit code 1 due to invalid model file");
    }

    // ========================= WATCHDOG TESTS ==========================
    /*
     * Two simple early tests in one:
     * - verify that we deal properly with missing required argument (should fail early with picocli
     *   returning error code 2)
     * - verify that we deal properly with a missing model file (should fail early with error code 1 during
     *   the execution of WatchCommand.call())
     */
    @Test
    public void testFailingWatchdogEarly() throws Exception {

        // Create temp directory for the test
        Path tempDirectory = Files.createTempDirectory("test_watchdog_fail");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

        // first test: we are missing a required parameter, picocli should return an error code 2
        // Build CLI args for watchdog
        String[] args = {
                "watchdog",
                modelFile.toString(),
                "3",
                                            // !!! required --watchdog-tick missing
                "--watchdog-timeout", "20"
        };
        CommandLine cmd = new CommandLine(new CliMain());
        int exitCode = cmd.execute(args);
        // The watchdog should fail early, even before WatchCommand.call() because the required --watchdogTick missing
        assertEquals(2, exitCode, "Watchdog should fail early with code 2 when required parameter is missing");

        // second test, model file does not exist
        // IMPORTANT: Do NOT create the model file yet
        assertFalse(modelFile.toFile().exists(), "Model file should NOT exist for this failure test");
        // Build CLI args for watchdog
        String[] args2 = {
                "watchdog",
                modelFile.toString(),       // !!! nonexistent model file
                "3",                        // valid number of runs
                "--watchdog-tick", "3",     // check every *** seconds
                "--watchdog-timeout", "20"  // give up after *** seconds if no logfile appears at all
        };

        CommandLine cmd2 = new CommandLine(new CliMain());
        int exitCode2 = cmd2.execute(args2);
        // The watchdog should fail early, during the execution of WatchCommand.call() because the model file does not exist
        assertEquals(1, exitCode2, "Watchdog should fail early when model file is missing");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }

    }

    /*
     * Exercising the --watchdog-timeout argument:
     * For the simple case where simulation 0 never starts, so it never creates a log file
     * There is no stale log file from a previous run of simulation 0
     * Expected behavior is that the watchdog will time out and return non-zero
     */
    @Test
    public void testWatchdogMissingLogs() throws Exception {

        Path tempDirectory = Files.createTempDirectory("test_watchdog_missing_logs");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

        // Create the model file (valid)
        Files.writeString(modelFile, inputFileContents);

        // No log files created
        String[] args = {
                "watchdog",
                modelFile.toString(),
                "3",                        // numRuns
                "--watchdog-tick", "3",     // check every *** seconds
                "--watchdog-timeout", "10"  // give up after *** seconds if no logfile appears at all
        };

        CommandLine cmd = new CommandLine(new CliMain());
        int exitCode = cmd.execute(args);

        // Watchdog should time out and return non-zero
        assertNotEquals(0, exitCode, "Watchdog should fail due to missing logs");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
     * Exercising the --watchdog-timeout argument differently:
     * There is only a stale log file for run 0, which means simulation 0 never starts for the current batch run
     * Normally it should start rather quickly and create a fresh log file
     * Nevertheless, we'll give it a generous timeout in production code to account for the fact that slurm may need
     * to delay it a lot if the node is too busy
     * Expected behavior is that the watchdog will time out because something is wrong
     */
    @Test
    public void testWatchdogStaleLog() throws Exception {

        Path tempDirectory = Files.createTempDirectory("test_watchdog_stale_log");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

        // Create the model file (valid)
        Files.writeString(modelFile, inputFileContents);

        // Create a stale log file for run 0
        Path logFile0 = tempDirectory.resolve("SimID_123456789_0_0.log");
        Files.writeString(logFile0, "");   // empty is fine

        // Make the log file stale by setting lastModified to 10 seconds ago
        File staleLog = logFile0.toFile();
        long now = System.currentTimeMillis();
        long staleTime = now - 10_000;   // 10 seconds old
        staleLog.setLastModified(staleTime);

        // Build CLI args
        String[] args = {
                "watchdog",
                modelFile.toString(),
                "3",                        // numRuns
                "--watchdog-tick", "3",     // check every 3 seconds
                "--watchdog-timeout", "10"  // give up after 10 seconds
        };

        CommandLine cmd = new CommandLine(new CliMain());
        int exitCode = cmd.execute(args);

        // Watchdog should time out because the log file is stale
        assertNotEquals(0, exitCode, "Watchdog should fail due to stale log file being ignored");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
     * There is a stale log file for run 0 but a fresh log file will be created for run 0 after a delay
     * Obviously the delat has to be shorter than the timeout
     * The watchdog should detect the fresh log file and enter the main infinite loop and start recording progress
     * We will eventually interrupt the watchdog to stop the test
     * Note that no --vc-print-status or --vc-send_status-config is used, so the watchdog will use VCellMessagingNoop()
     * and not send any progress messages, like [[[progress:3.0%]]] or [[[alive]]] in the
     * output, we will only see lg.info messages if not commented out in the code (as they will be eventually)
     * The only thing we'll see is the simulation progress messages in the log file, and logger messages from the
     * thread simulating progress, like "log 0++"
     */
    @Disabled("Manual-only test; excluded from CI")  // excluded for CI, as it takes time and we only want to visually
                                                     // verify some partial functionality. The next test is a complete
                                                     // exercise of all the functionality and that's the one
                                                     // that should be used for CI
    @Test
    public void testWatchdogEntersProgressLoop() throws Exception {

        Path tempDirectory = Files.createTempDirectory("test_watchdog_progress_loop");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

        // Create valid model file
        Files.writeString(modelFile, inputFileContents);

        // Create the stale log file for run 0, execution should stay in the first while() loop for a while
        Path logFile0 = tempDirectory.resolve("SimID_123456789_0_0.log");

        // Write some stale progress messages
        String staleProgress =
                "Simulation 1% complete. Elapsed time: 0.392 sec.\n" +
                        "Simulation 2% complete. Elapsed time: 0.769 sec.\n" +
                        "Simulation 3% complete. Elapsed time: 1.151 sec.\n";
        Files.writeString(logFile0, staleProgress);

        // Make the log file stale by setting lastModified to 10 seconds ago
        File staleLog = logFile0.toFile();
        long now = System.currentTimeMillis();
        staleLog.setLastModified(now - 10000);

        // Build CLI args, note no --vc-print-status and no --vc-send_status-config , uses VCellMessagingNoop()
        String[] args = {
                "watchdog",
                modelFile.toString(),
                "3",                        // numRuns
                "--watchdog-tick", "2",     // check every second
                "--watchdog-timeout", "10"  // timeout for first loop (won't be used)
        };

        // Run watchdog in a separate thread so that we can kill it
        CommandLine cmd = new CommandLine(new CliMain());
        Thread watchdogThread = new Thread(() -> {cmd.execute(args); });
        watchdogThread.start();

        // Create a fresh log file in another thread after a delay
        Thread freshLog0Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 0, 100, 5000, 3000);
        freshLog0Thread.start();

        // Let watchdog run long enough to detect the fresh log and enter the infinite loop
        Thread.sleep(30000);

        // At this point, watchdog should be inside the second loop
        assertTrue(watchdogThread.isAlive(), "Watchdog should be running in the infinite loop");

        // Kill the watchdog thread
        watchdogThread.interrupt();

        // Give it a moment to stop
        Thread.sleep(500);

        assertFalse(watchdogThread.isAlive(), "Watchdog thread should have been interrupted and stopped");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
     * Here we really exercise the log file parser and the algorithm that detects and computes progress
     * If VcellMessagingLocal works, we should see something like [[[progress:3.0%]]] or [[[alive]]]
     * A few assertions are made to verify that the watchdog is working at all.
     */
    @Test
    public void testWatchdogMonitorsProgress() throws Exception {

        Path tempDirectory = Files.createTempDirectory("test_watchdog_monitors_progress");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

        // Create valid model file
        Files.writeString(modelFile, inputFileContents);

        // Build CLI args, note --vc-print-status", uses VCellMessagingLocal()
        String[] args = {
                "watchdog",
                modelFile.toString(),
                "3",                        // numRuns
                "--watchdog-tick", "3",     // check every *** seconds
                "--watchdog-timeout", "20", // timeout in seconds for first loop (won't be used)
                "--vc-print-status"         // uses: vcellMessaging = new VCellMessagingLocal();
        };

        CommandLine cmd = new CommandLine(new CliMain());

        // Run watchdog in a separate thread so we can kill it
        Thread watchdogThread = new Thread(() -> {cmd.execute(args); });
        watchdogThread.start();

        // Create multiple log files and keep appending percentage growth
        Thread freshLog0Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 0, 100, 3000, 7000);
        Thread freshLog1Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 1, 100, 7000, 8000);
        Thread freshLog2Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 2, 7, 11000, 9000);
        freshLog0Thread.start();
        freshLog1Thread.start();
        freshLog2Thread.start();

        // Let watchdog run long enough to detect the fresh log and enter the infinite loop
        Thread.sleep(20000);

        // At this point, watchdog should be inside the second loop
        assertTrue(watchdogThread.isAlive(), "Watchdog should be running in the infinite loop");

        // we run some more, then kill the watchdog thread
        Thread.sleep(20000);
        watchdogThread.interrupt();

        // Give it a moment to stop
        Thread.sleep(500);

        assertFalse(watchdogThread.isAlive(), "Watchdog thread should have been interrupted and stopped");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
     * Same as above but we are capturing the stdout and check the outputs against expected values
     * Nothing will show at the console since we are redirecting System.out to a ByteArrayOutputStream.
     * If VcellMessagingLocal works, we should see something like [[[progress:3.0%]]] or [[[alive]]]
     * Comprehensive assertions are made to verify that the watchdog is doing what it should be doing,
     * including detecting progress changes and messaging.
     * Not working on the Mac, so we disable it in the CI run, but it can be run manually on a Mac to verify that it
     * works there too (which it does, tested on Jim's machine)
     */
    @Disabled("Manual-only test; excluded from CI")     // disabled for CI because of the Mac failure
    @Test
    public void testWatchdogMonitorsProgressWithCapture() throws Exception {

        Path tempDirectory = Files.createTempDirectory("test_watchdog_monitors_progress");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

            // Create valid model file
            Files.writeString(modelFile, inputFileContents);

            // Build CLI args, note --vc-print-status", uses VCellMessagingLocal()
            String[] args = {
                    "watchdog",
                    modelFile.toString(),
                    "3",                        // numRuns
                    "--watchdog-tick", "3",     // check every *** seconds
                    "--watchdog-timeout", "20", // timeout in seconds for first loop (won't be used)
                    "--vc-print-status"         // uses: vcellMessaging = new VCellMessagingLocal();
            };

            // Redirect System.out so that we can capture watchdog output
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            PrintStream ps = new PrintStream(baos, true);
            System.setOut(ps);                      // redirect stdout to this
            StringWriter stdout = new StringWriter();

            CommandLine cmd = new CommandLine(new CliMain());
            cmd.setOut(new PrintWriter(stdout));

            // Run watchdog in a separate thread so we can kill it
            Thread watchdogThread = new Thread(() -> {cmd.execute(args); });
            watchdogThread.start();

            // Create multiple log files and keep appending percentage growth
            Thread freshLog0Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 0, 100, 3000, 7000);
            Thread freshLog1Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 1, 100, 7000, 8000);
            Thread freshLog2Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 2, 7, 11000, 9000);
            freshLog0Thread.start();
            freshLog1Thread.start();
            freshLog2Thread.start();

            // Let watchdog run long enough to detect the fresh log and enter the infinite loop
            Thread.sleep(20000);

            // At this point, watchdog should be inside the second loop
            assertTrue(watchdogThread.isAlive(), "Watchdog should be running in the infinite loop");

            // we run some more, then kill the watchdog thread
            Thread.sleep(20000);
            watchdogThread.interrupt();

            // Give it a moment to stop
            Thread.sleep(500);

            assertFalse(watchdogThread.isAlive(), "Watchdog thread should have been interrupted and stopped");

            // let's see what the cat brought in
            String sysOutText = baos.toString();
            System.err.println("Captured System.out:\n" + sysOutText);

            assertTrue(sysOutText.contains("Watchdog"), "1. Expected progress output missing");
            assertTrue(sysOutText.contains("started"), "2. Expected progress output missing");
            assertTrue(sysOutText.contains("analyzing"), "3. Expected progress output missing");
            assertTrue(sysOutText.contains("folder"), "4. Expected progress output missing");
            assertTrue(sysOutText.contains("entering doWork"), "5. Expected progress output missing");
            assertTrue(sysOutText.contains("monitoring loop"), "6. Expected progress output missing");
            assertTrue(sysOutText.contains("loop tick"), "7. Expected progress output missing");
            assertTrue(sysOutText.contains("progress unchanged"), "8. Expected progress output missing");
            assertTrue(sysOutText.contains("[[[progress:0.0%]]]"), "9. Expected progress output missing");     // vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(...
            assertTrue(sysOutText.contains("progress changed"), "10. Expected progress output missing");
            assertTrue(sysOutText.contains("interrupted"), "11. Expected progress output missing");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
     * Here we exercise sending WorkerEvent via REST to ActiveMQ server
     * Obviously it's not going to work as is because we don't have a real ActiveMQ server running
     * We'll get exceptions:
     *    java.net.ConnectException
	 *       at java.net.http/jdk.internal.net.http.HttpClientImpl.send(HttpClientImpl.java:573)
	 *       at java.net.http/jdk.internal.net.http.HttpClientFacade.send(HttpClientFacade.java:123)
	 *       at org.vcell.messaging.VCellMessagingRest.sendWorkerEvent(VCellMessagingRest.java:165)
     *       ...
     *       Exception sending WorkerEvent via REST to ActiveMQ server: null
     */
    @Disabled       // exclude this from any automated run including github; this should be run manually with
                    // a real ActiveMQ server running and the config file pointing to it
    @Test
    public void testWatchdogMessaging() throws Exception {

        Path tempDirectory = Files.createTempDirectory("test_watchdog_monitors_progress");
        Path modelFile = tempDirectory.resolve("SimID_123456789_0_.langevinInput");

        try {

        // Create valid model file
        Files.writeString(modelFile, inputFileContents);

        // Create a fake VCell messaging config file
        Path configFile = tempDirectory.resolve("vc_config.properties");
        Files.writeString(configFile,
                "broker_host=localhost\n" +
                        "broker_port=8165\n" +
                        "broker_username=msg_user\n" +
                        "broker_password=msg_pswd\n" +
                        "vc_username=vcell_user\n" +
                        "simKey=123456789\n" +
                        "taskID=0\n" +
                        "jobIndex=0\n");

        // Build CLI args, note the --vc-send-status-config option, uses VCellMessagingRest(config)
        String[] args = {
                "watchdog",
                modelFile.toString(),
                "3",                        // numRuns
                "--vc-send-status-config", configFile.toString(),   // uses: vcellMessaging = new VCellMessagingRest(config);
                "--watchdog-tick", "3",     // check every *** seconds
                "--watchdog-timeout", "20"  // timeout in seconds for first loop (won't be used)
        };

        // Run watchdog in a separate thread so we can kill it
        CommandLine cmd = new CommandLine(new CliMain());
        Thread watchdogThread = new Thread(() -> {cmd.execute(args); });
        watchdogThread.start();

        // Create multiple log files and keep appending percentage growth
        Thread freshLog0Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 0, 100, 3000, 4000);
        Thread freshLog1Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 1, 100, 9000, 5000);
        Thread freshLog2Thread = createLogWriterThread(tempDirectory, "SimID_123456789_0_", 2, 7, 15000, 6000);
        freshLog0Thread.start();
        freshLog1Thread.start();
        freshLog2Thread.start();

        // Let watchdog run long enough to detect the fresh log and enter the infinite loop
        Thread.sleep(30000);

        // At this point, watchdog should be inside the second loop
        assertTrue(watchdogThread.isAlive(), "Watchdog should be running in the infinite loop");

        // we run some more, then kill the watchdog thread
        Thread.sleep(30000);
        watchdogThread.interrupt();

        // Give it a moment to stop
        Thread.sleep(500);

        assertFalse(watchdogThread.isAlive(), "Watchdog thread should have been interrupted and stopped");

        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    //
    // ---------------------------- Utility functions -------------------------------------
    //
    private Thread createLogWriterThread(
            Path simulationFolder,
            String simulationName,      // ex: SimID_123456789_0_
            int logIndex,               // run index that we're simulating: 0, 1, 2...
            int numEntries,             // number of progress entries to write, normally 100 but we can stop early
            long initialDelayMillis,    // Initial delay before creating the fresh log file
            long writeIntervalMillis
    ) {
        return new Thread(() -> {
            try {
                // Build the log file path for this index
                Path logFile = simulationFolder.resolve(simulationName + logIndex + ".log");

                // Initial delay before creating the fresh log file
                // this simulates the short time between the moment we launch the watchdog and the moment
                // the simulation starts writing to the log file
                Thread.sleep(initialDelayMillis);

                // Rewrite file to make it fresh
                Files.writeString(logFile, "");
                logFile.toFile().setLastModified(System.currentTimeMillis());

                // Now write real progress steps at the specified interval
                for (int i = 1; i <= numEntries; i++) {
                    Thread.sleep(writeIntervalMillis);
                    String line = "Simulation " + i + "% complete. Elapsed time: " + (i * (writeIntervalMillis / 1000.0)) + " sec.\n";
                    System.out.println("    log " + logIndex + " at " + i + "%");   // ... + line.trim()
//                    System.err.println("    log " + logIndex + ": " + line.trim());
                    Files.writeString(logFile, line, StandardOpenOption.APPEND);
                }

            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        });
    }

}