package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.EtaTracker;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.DisabledIfEnvironmentVariable;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.VCellMessagingLocal;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertNotNull;

public class MySystemTest {

    public static final Logger lg = LogManager.getLogger(MySystemTest.class);

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
            L_z_out: 0.010000000000000009
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
            
            'r1'       'MT0' : 'Site0' : 'Any_State'  +  'MT0' : 'Site0' : 'Any_State'  kon  10.0  koff 0.0  Bond_Length 1.0
            
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
            """;

    String expected_idafile_contents_2_lines = """
            t:r1:TOTAL_MT0:FREE_MT0:BOUND_MT0:TOTAL_MT0__Site0__state0:FREE_MT0__Site0__state0:BOUND_MT0__Site0__state0:TOTAL_MT0__Site1__state0:FREE_MT0__Site1__state0:BOUND_MT0__Site1__state0:TOTAL_MT0__Site1__state1:FREE_MT0__Site1__state1:BOUND_MT0__Site1__state1:CREATION_MT0:DECAY_MT0:BINDING_r1:UNBINDING_r1:TRANSITION_r0
            0.0 0 20 20 0 20 20 0 20 20 0 0 0 0 0 0 0 0 0
            """;

    void deleteDirectory(File directoryToBeDeleted) {
        File[] allContents = directoryToBeDeleted.listFiles();
        if (allContents != null) {
            for (File file : allContents) {
                deleteDirectory(file);
            }
        }
        directoryToBeDeleted.delete();
    }

    @Test
    public void testMySystem() throws IOException {
        // create a temporary directory
        // create an input file in the temporary directory with the contents of inputFileContents
        String sim_base_name = "sim";
        int runCounter = 0;
        Path tempDirectory = Files.createTempDirectory("test_simulation");
        Path modelFile = tempDirectory.resolve(sim_base_name+".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name+".log");
        Path idaFile = tempDirectory.resolve(sim_base_name+".ida");
        Files.writeString(modelFile, inputFileContents);
        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        try {
            // run simulation
            Global g = new Global(modelFile.toFile(), logFile.toFile());
            MySystem sys = new MySystem(g, runCounter, true, vcellMessaging);
            sys.runSystem();
            // check that the ida file was created and starts with the expected contents
            Assertions.assertTrue(Files.exists(idaFile));
            String idaFileContents = Files.readString(idaFile);
            Assertions.assertTrue(idaFileContents.startsWith(expected_idafile_contents_2_lines));
            lg.debug(idaFileContents.substring(0,500)+"\n...\n"+idaFileContents.substring(idaFileContents.length()-500));
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
    this is the springsalad tutorial, as described on vcell.org/ssalad-2
    on: https://vcell.org/webstart/SpringSaLaD/SpringSaLaDUsersGuideAndTutorial.pdf
     */
    @Test
    public void loadFileFromResource() throws Exception {
        URL url = getClass().getResource("/tutorial.ssld");
        assertNotNull(url);

        File modelFile = new File(url.toURI());
        File logFile = File.createTempFile("tutorialTest", ".log");

        VCellMessaging vcellMessaging = new VCellMessagingLocal();

        try {
            Global g = new Global(modelFile, logFile);
            assertNotNull(g);

            MySystem sys = new MySystem(g, 0, true, vcellMessaging);
            assertNotNull(sys);
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            logFile.delete();
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    @Test
    public void simulateFileFromResource() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 0;

        // Load the real file from src/test/resources
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/tutorial.ssld")) {
            assertNotNull(is, "Resource tutorial.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_simulation");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + ".log");
        Path idaFile = tempDirectory.resolve(sim_base_name + ".ida");

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g, "Global object should not be null");

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys, "MySystem object should not be null");

            sys.runSystem();
            Assertions.assertTrue(Files.exists(idaFile));

        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * TransitionCondBound.ssld is a file that was used to test a specific bug related to conditional bound transition
     * where the bond object remains hanging if no reverse rate is to be found, and the simulation would crash with a
     * null pointer exception when the bond object is being updated. This test is meant to ensure that this bug is fixed
     * and does not regress in the future, but also to serve as a general test for bond management after transitioning.
     */
    @Test
    public void transitionConditionBoundFromResource() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 0;

        // Load the real file from src/test/resources
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/TransitionCondBound.ssld")) {
            assertNotNull(is, "Resource TransitionCondBound.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_simulation");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + ".log");
        Path idaFile = tempDirectory.resolve(sim_base_name + ".ida");

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g, "Global object should not be null");

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys, "MySystem object should not be null");

            sys.runSystem();
            Assertions.assertTrue(Files.exists(idaFile));

        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * BindingWithZeroKf.ssld is a file that was used to test a specific bug related to bimolecular binding
     * reactions with zero forward rate constant - technically a pure unbinding reaction
     * in r2, if kon is greater than zero, say 0.001 then the simulation runs fine,
     * but if kon is set to zero, then the reaction is skipped entirely and unbinding never happens
     */
    @Test
    public void bindingWithZeroKfFromResource() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 0;

        // Load the real file from src/test/resources
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/BindingWithZeroKf.ssld")) {
            assertNotNull(is, "Resource BindingWithZeroKf.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_simulation");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + ".log");
        Path idaFile = tempDirectory.resolve(sim_base_name + ".ida");

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g, "Global object should not be null");

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys, "MySystem object should not be null");

            sys.runSystem();
            Assertions.assertTrue(Files.exists(idaFile));

        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * AllReactions.ssld is a file that was used to test a combination of all reaction types, including creation,
     * decay, state transition, allosteric, and bimolecular binding reactions
     * Also helped in implementation and debugging the reaction counter feature
     * Good number for quick runs:
     * MT0 - 4
     * MT1 - 15
     * Total time: 0.01
     */
    @Test
    public void allReactionsFromResource() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 0;

        // Load the real file from src/test/resources
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/AllReactions.ssld")) {
            assertNotNull(is, "Resource AllReactions.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_simulation");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + ".log");
        Path idaFile = tempDirectory.resolve(sim_base_name + ".ida");

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g, "Global object should not be null");

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys, "MySystem object should not be null");

            sys.runSystem();
            Assertions.assertTrue(Files.exists(idaFile));
            sys.getReactionCounter().printCounts();
            sys.getReactionCounter().printDetailedCounts();
            System.out.println("done");
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * We'll test a lengthy simulation with a lot of reactions, to see if the estimated duration is reasonable
     * Keep in mind that if molecules are being created and destroyed, the number of reactions will vary and the
     * estimated duration will be less accurate
     * We'll use both hardcoded and default schedule
     */
    @Test
    public void estimateDurationFromResource() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 1;     // must be run index 1, this means batch run, also only run 1 computes estimates

        // Load the real file from src/test/resources
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/AllReactions.ssld")) {
            assertNotNull(is, "Resource AllReactions.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_simulation");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + "_1.log");
        Path idaFile = tempDirectory.resolve(sim_base_name + "_1.ida");

        // change the initial number of molecules, so that the simulation will run longer
        // and we can test the estimated duration
        inputFileContents = setInitialValue(inputFileContents, "MT0", 200);
        inputFileContents = setInitialValue(inputFileContents, "MT1", 200);

        // create the input file
        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g, "Global object should not be null");

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys, "MySystem object should not be null");

            // change the ETA calculator parameters so that we can model the behavior / accuracy
            // during long simulations (without really running them for weeks), and see if the estimated
            // duration is reasonable
            sys.getEtaTracker().setUseHardcodedSchedule(true); // enable hardcoded schedule (default is true but we do it as a reminder)
            sys.getEtaTracker().setEtaHardcodedScheduleSeconds(new int[] {2,4,6,8}); // custom schedule
            sys.getEtaTracker().setUseDefaultSchedule(true);    // enable default schedule
            sys.getEtaTracker().setDefaultScheduleIntervalMs(5000); // every xxx miliseconds
            sys.getEtaTracker().setEtaLoggingCutoffMs(20_000);  // cutoff for default schedule, miliseconds


            // here all the work is done
            long startTime = System.currentTimeMillis();
            sys.runSystem();
            long endTime = System.currentTimeMillis();
            Assertions.assertTrue(Files.exists(idaFile));
            long predictedDurationNs = sys.getEtaTracker().getLastEtaTotalNs();     // nanoseconds
            long actualDurationMs = endTime - startTime;                            // miliseconds
            System.out.println("Actual duration:    " + IOHelp.formatNanoseconds(2, actualDurationMs*1_000_000L));
            System.out.println("Predicted duration: " + IOHelp.formatNanoseconds(2, predictedDurationNs));
            System.out.println("Difference:       " + IOHelp.formatNanoseconds(2, Math.abs(actualDurationMs*1_000_000 - predictedDurationNs)));

            System.out.println("done");
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * We have disabled the default schedule, and we want to make sure that the hardcoded schedule is used instead
     */
    @Test
    public void estimateDurationHardcodedScheduleOnly() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 1;

        // Load the real file
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/AllReactions.ssld")) {
            assertNotNull(is, "Resource AllReactions.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_hardcoded_schedule");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + "_1.log");
        Path idaFile = tempDirectory.resolve(sim_base_name + "_1.ida");

        // Make simulation run long enough to hit 1s, 2s, 3s ETA points
        inputFileContents = setInitialValue(inputFileContents, "MT0", 40);
        inputFileContents = setInitialValue(inputFileContents, "MT1", 40);

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g);

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys);

            EtaTracker eta = sys.getEtaTracker();

            // *** Hardcoded schedule only ***
            eta.setEtaHardcodedScheduleSeconds(new int[] {1, 2, 3, 6, 12, 20});     // ETA at 1s, 2s, 3s...
            eta.setUseDefaultSchedule(false);   // disable fallback
            eta.setEtaLoggingCutoffMs(10_000);  // IRRELEVANT! fallback is disabled, we stop after the hardcoded schedule

            // Run simulation
            sys.runSystem();

            // Simulation must produce output
            Assertions.assertTrue(Files.exists(idaFile));

            // *** Verify ETA fired at least once ***
            long lastEta = eta.getLastEtaTimeMs();
            Assertions.assertNotEquals(-1, lastEta,
                    "ETA should have been computed at least once using the hardcoded schedule");

            System.out.println("Hardcoded schedule ETA test completed.");
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * We have disabled the hardcoded schedule, and we want to make sure that the default schedule is used instead
     */
    @Test
    public void estimateDurationDefaultScheduleOnly() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 1;

        // Load the real file
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/AllReactions.ssld")) {
            assertNotNull(is, "Resource AllReactions.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_default_schedule");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + "_1.log");
        Path idaFile = tempDirectory.resolve(sim_base_name + "_1.ida");

        // Make simulation run a bit longer
        inputFileContents = setInitialValue(inputFileContents, "MT0", 20);
        inputFileContents = setInitialValue(inputFileContents, "MT1", 20);

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g);

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys);

            // *** Hardcoded schedule disabled ***
            sys.getEtaTracker().setUseHardcodedSchedule(false);

            // *** Default schedule enabled ***
            sys.getEtaTracker().setUseDefaultSchedule(true);
            sys.getEtaTracker().setDefaultScheduleIntervalMs(5000); // every 5 seconds

            // Cutoff at 12 seconds (should allow 2 ETA logs)
            sys.getEtaTracker().setEtaLoggingCutoffMs(12_000);

            // Run simulation
            sys.runSystem();

            // Simulation must still produce output
            Assertions.assertTrue(Files.exists(idaFile));

            // *** Verify ETA WAS computed ***
            long lastEta = sys.getEtaTracker().getLastEtaTimeMs();
            Assertions.assertNotEquals(-1, lastEta,
                    "ETA should have been computed at least once using the default schedule");

            System.out.println("Default schedule ETA test completed.");
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /*
     * We have totaly disabled the ETA computation, and we want to make sure that it is not computed at all
     * This is fast, no need to disable it for github actions
     */
    @Test
    public void estimateDurationEtaDisabled() throws IOException {
        String sim_base_name = "sim";
        int runCounter = 1;

        // Load the real file
        String inputFileContents;
        try (InputStream is = getClass().getResourceAsStream("/AllReactions.ssld")) {
            assertNotNull(is, "Resource AllReactions.ssld not found");
            inputFileContents = new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }

        Path tempDirectory = Files.createTempDirectory("test_eta_disabled");
        Path modelFile = tempDirectory.resolve(sim_base_name + ".langevinInput");
        Path logFile = tempDirectory.resolve(sim_base_name + "_1.log");
        Path idaFile = tempDirectory.resolve(sim_base_name + "_1.ida");

        // Make simulation run very fast, we just want to see that no ETA gets computed
        inputFileContents = setInitialValue(inputFileContents, "MT0", 5);
        inputFileContents = setInitialValue(inputFileContents, "MT1", 5);

        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = null;
        MySystem sys = null;

        try {
            g = new Global(modelFile.toFile(), logFile.toFile());
            assertNotNull(g);

            sys = new MySystem(g, runCounter, true, vcellMessaging);
            assertNotNull(sys);

            // *** Disable ETA completely ***
            sys.getEtaTracker().setUseHardcodedSchedule(false);    // no hardcoded schedule
            sys.getEtaTracker().setUseDefaultSchedule(false);       // no default schedule

            // Run simulation
            sys.runSystem();

            // Simulation must still produce output
            Assertions.assertTrue(Files.exists(idaFile));

            // *** Verify ETA was never computed ***
            Assertions.assertEquals(-1, sys.getEtaTracker().getLastEtaTimeMs(),
                    "ETA should be disabled and never computed");

            System.out.println("ETA disabled test completed.");
        } catch (Exception e) {
            Assertions.fail("Unexpected exception during test: " + e.getMessage());
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    // ----------------------------------------------------------------------------------

    // helper function to modify initial counts for molecule, so that I could test shorter and longer simulations
    public static String setInitialValue(String input, String molecule, int newValue) {
        // Regex explanation:
        // MOLECULE: "MT0" Intracellular Number 50
        // We capture:
        //   group 1: the prefix up to "Number "
        //   group 2: the old number
        //
        // Then we replace group 2 with newValue.
        //
        String regex = "(MOLECULE:\\s*\"" + molecule + "\"\\s+Intracellular\\s+Number\\s+)(\\d+)";
        return input.replaceAll(regex, "$1" + newValue);
    }


    // Logger sanity check. Confirms that Log4j2 configuration is found on the classpath,
    // that the logger instance lg is valid and that the logger is working. This test should always pass.
    @Test
    public void loggerTest() {

        Assertions.assertDoesNotThrow(() -> {

            System.out.println("Logger test");

            if(lg.getLevel() == org.apache.logging.log4j.Level.DEBUG) {
                System.out.println("Logger level is DEBUG");
            } else {
                System.out.println("Logger level is " + lg.getLevel());
            }

            lg.debug("Debug message");
            lg.info("Info message");
            lg.error("Error message");

        });         // no exception should be thrown
    }

}
