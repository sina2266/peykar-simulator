package edu.uchc.cam.langevin.langevinnovis01;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.junit.jupiter.api.*;
import org.junit.jupiter.api.condition.DisabledIfEnvironmentVariable;
import org.vcell.data.LangevinPostprocessor;
import org.vcell.data.NdJsonUtils;
import org.vcell.data.Resource;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.VCellMessagingNoop;

import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.*;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.assertNotNull;

public class ClusterAnalisysTest {

    public static final Logger lg = LogManager.getLogger(ClusterAnalisysTest.class);

    public enum InputSource {
        RESOURCES("classpath", "Loading from classpath resources."),
        LOCAL("filesystem", "Loading from local filesystem.");

        private final String identifier;
        private final String longDescription;

        InputSource(String identifier, String longDescription) {
            this.identifier = identifier;
            this.longDescription = longDescription;
        }
        public String getIdentifier() {
            return identifier;
        }
        public String getLongDescription() {
            return longDescription;
        }
    }

    private static InputSource inputSource;
    private static Path workDirPath;  // needed only for "classpath", must be cleaned up at the end of each @Test
    private static Path simDataFolderPath = null;   // needed only for "classpath" (using resources from repository)
    // list of temporary work folders for all the tests, should be empty at the time when we invoke tearDown()
    // may not be empty if we don't invoke cleanup() at the end of each @Test
    // populated only for "classpath", it stays empty for "filesystem"
    private static Map<String, Path> workDirPathMap = new LinkedHashMap<> ();

    public static final String ClustersFileExtension = ".json";
    public static final String IdaFileExtension = ".ida";

    // essential simulation names and locations, given the correct values in initialize() depending on inputSource
    // automatic created for "classpath" using resources from repository, hardcoded for "filesystem"
    public static String parent_dir = null;
    public static String sim_base_name = null;
    public static String inputFileName = null;
    public static Integer NumRuns = null;           // adjust accordingly at runtime

    // some test that appears right at the end of the input file, if this exists hopefully all the rest is there too
    public static final String inputFileValidityCheck = "*** SIMULATION OPTIONS ***";


    @BeforeAll
    public static void setUp() throws IOException {
        // comment out one or the other
        inputSource = InputSource.RESOURCES;    // use this for automatc testing
//        inputSource = InputSource.LOCAL;        // use this for testing from a local folder
    }

    /*
     * Call this in every @Test before anything else
     * Initializes some global variables (sim_base_name. inputFileName, parent_dir and NumRuns) properly,
     *  which are null by default
     */
    @BeforeEach
    public void initialize() throws IOException, URISyntaxException {
        if(inputSource == InputSource.RESOURCES) {
            // sim_base_name. inputFileName, parent_dir and NumRuns are initialized to null
            // they are properly set up here for "classpath" in generalInitialization()
            classpathInitialization();
        } else if(inputSource == InputSource.LOCAL) {
            // ATTENTION! only one @Test should be ran at a time, the correct running order is:
            //   - testMakeJsonFiles()
            //   - testReadJsonFiles()
            //    - testRunClusterAnalysis()
            // This way each test will create more files needed for the next test
            // Manual delete of the files created during @Test is needed
            // You should only start with the data_dir = parent_dir + "/" + sim_base_name + "_FOLDER" and its content,
            //    inputFileName and one .ida file for each run
            //
            // sim_base_name. inputFileName, parent_dir and NumRuns are initialized to null
            // must properly set them up here for "local" to your actual locations / names
//            parent_dir = "C:/TEMP/langevin-cli-test/cluster_analysis_big3";
//            sim_base_name = "SimID_35189106_0_";
//            inputFileName = sim_base_name + ".langevinInput";
//            NumRuns = 3;
            parent_dir = "C:/TEMP/langevin-cli-test/hairball";
            sim_base_name = "SimID_318661575_0_";
            inputFileName = sim_base_name + ".langevinInput";
            NumRuns = 3;
        } else {
            throw new IllegalArgumentException("Unexpected InputSource value");
        }

    }

    // initialization of names and folders for "classpath" (automatic testing using resources)
     // copies in a temporary folder whose name begins with @workDirName
     // most of the content of rhe resources/simdata folder in the repository,  which means
     // - the input file
     // - the @sim_base_name + _FOLDER/data folder and all its content (Run0, Run1, ... folders and their content
    public static void classpathInitialization() throws IOException, URISyntaxException {

        lg.info("Classpath: " + System.getProperty("java.class.path"));

        // override for "classpath"
        sim_base_name = "SimID_35189106_0_";
        String resourcesFolder = "simdata";
        // TODO: comment out the next 2 lines before commit
//        resourcesFolder = "all_reactions";
//        sim_base_name = "sim";

        inputFileName = sim_base_name + ".langevinInput";
        NumRuns = 3;

        // // the resources are in  \resources\simdata, \resources is expressed in CLASSPATH
        URL resourceInputFileUrl = Resource.getResource(resourcesFolder + "/" + inputFileName);
        if(resourceInputFileUrl == null) {
            throw new IllegalArgumentException("Resource not found: '" + resourcesFolder + "/" + inputFileName +"'");
        }

        final String workDirName = "000-langevintest";   // arbitrary prefix for our temp folder
        workDirPath = Files.createTempDirectory(workDirName);        // temp location for automatic testing
        workDirPathMap.put(workDirPath.toString(), workDirPath);
        parent_dir = workDirPath.toString();
        Path inputFilePath = workDirPath.resolve(inputFileName);
        if(inputFilePath == null) {
            throw new InvalidPathException(workDirName + "/" + inputFileName, "Unable to build temporary file");
        }
        Resource.copyToFile(resourceInputFileUrl, inputFilePath.toFile());
        boolean found = Files.lines(inputFilePath).anyMatch(line -> line.contains(inputFileValidityCheck));
        assertTrue(found, "File should contain the string: " + inputFileValidityCheck);

        URL resourceSimDataFolderURL = Resource.getResource(resourcesFolder + "/" + sim_base_name + "_FOLDER/data");
        simDataFolderPath = workDirPath.resolve(sim_base_name + "_FOLDER/data");
        Resource.copyFolderRecursively(resourceSimDataFolderURL, simDataFolderPath);
    }


    @AfterAll
    public static void tearDown() throws IOException {
        if(InputSource.LOCAL == inputSource) {
            lg.info("For InputSource.LOCAL manual delete of the files created during Tests is needed");
            return;
        }
        if (!workDirPathMap.isEmpty()) {
            // this should not happen since we call cleanUp() @AfterEach
            throw new RuntimeException("One or more temp directories still exist, cleanup() may have failed");
        }
        lg.info("Finished cleaning up all temporary folders successfully");
    }

    @AfterEach
    private void cleanUp() throws IOException {
        if(InputSource.LOCAL == inputSource) {
            lg.info("For InputSource.LOCAL manual delete of the files created during Tests is needed");
            return;
        }
        if (workDirPath != null && Files.exists(workDirPath)) {
            deleteRecursively(workDirPath);
        }
        if (Files.exists(workDirPath)) {
            lg.error("Failed to delete temp directory: " + workDirPath + ", retrying");
            System.gc(); // hint JVM to release file handles
            try {
                Thread.sleep(200);
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt(); // restore interrupt status
                lg.error("Interrupted during delete retry: " + workDirPath);
                return;
            }
            deleteRecursively(workDirPath);
            if (Files.exists(workDirPath)) {
                lg.error("Failed to delete temp directory after retry: " + workDirPath);
            } else {
                lg.info("Cleaned up temp directory after retry: " + workDirPath);
                workDirPathMap.remove(workDirPath.toString());
            }
        } else {
            lg.info("Cleaned up temp directory: " + workDirPath);
            workDirPathMap.remove(workDirPath.toString());
        }
    }

    private static void deleteRecursively(Path path) throws IOException {
        Files.walk(path)
                .sorted(Comparator.reverseOrder()) // delete children before parents
                .map(Path::toFile)
                .forEach(File::delete);
    }

// --------------------------------------------------------------------------------------------------------

/*
 * These tests are needed for debugging rather than for automatic run in a github action,
 * but we can do both for conformity
 * just select the inputSource with the appropriate enum in setUp()
 * see CliTest.testRunAndPostCommand for two full solver runs and postprocessing / cluster analysis,
 * basically doing everything done here and more (only slower because of the 2 consecutive full solver runs)
 */

    @Test
    public void testRunClusterAnalysis() throws IOException, URISyntaxException {

        if(inputSource == InputSource.RESOURCES) {  // more resource files needed specifically for this test
            URL resourceWorkingDirUrl = Resource.getResource("simdata");
            // this test needs the .json and .ida files
            Resource.copyFilesWithExtension(resourceWorkingDirUrl, workDirPath, ClustersFileExtension);
            Resource.copyFilesWithExtension(resourceWorkingDirUrl, workDirPath, IdaFileExtension);
        }

        VCellMessaging vcellMessaging = new VCellMessagingNoop();
        File modelFile = new File(parent_dir, sim_base_name+".langevinInput");
        File simulationFolder = new File(parent_dir);   // place of input file, and .ida and .json result files for all runs

        Global g = new Global(modelFile);
        ConsolidationPostprocessor cp = new ConsolidationPostprocessor(g, 50, false, vcellMessaging);
        cp.setSimulationFolder(simulationFolder);
        cp.setNumRuns(NumRuns);
        cp.setSimulationName(sim_base_name);

        cp.calculateLangevinPrimaryStatistics();
        File targetFile = new File(simulationFolder, sim_base_name + "_Avg.ida");
        assertTrue(targetFile.exists(), "expected file " + sim_base_name + "_Avg.ida to exist");

        cp.calculateLangevinAdvancedStatistics();   // cluster analysis
        File[] csvFiles = simulationFolder.isDirectory() ?
            simulationFolder.listFiles((dir, name) -> name.endsWith(".csv")) : new File[0];
        // we get 3 advanced statistics files - unrelated to NumRuns!
        assertTrue(3 == csvFiles.length, "number of .csv files should be equal to 3");

        lg.info("done");
    }

    @Test
    public void testReadJsonFiles() throws IOException, URISyntaxException {

        if(inputSource == InputSource.RESOURCES) {  // more resource files needed specifically for this test
            URL resourceWorkingDirUrl = Resource.getResource("simdata");
            Resource.copyFilesWithExtension(resourceWorkingDirUrl, workDirPath, ClustersFileExtension);
        }

        // this test reads the json files
        VCellMessaging vcellMessaging = new VCellMessagingNoop();
        File modelFile = new File(parent_dir, sim_base_name+".langevinInput");
        File simulationFolder = new File(parent_dir);   // place of input file, and .ida and .json result files for all runs

        Global g = new Global(modelFile);
        ConsolidationPostprocessor cp = new ConsolidationPostprocessor(g, NumRuns, false, vcellMessaging);
        cp.setSimulationFolder(simulationFolder);
        cp.setNumRuns(NumRuns);
        cp.setSimulationName(sim_base_name);
        Map<String, Integer> molecules = ConsolidationPostprocessor.getMolecules(g);

        ConsolidationClusterAnalizerInput cai = new ConsolidationClusterAnalizerInput();
        cai.readInputFiles(cp);

        Map<String, File> nameToJsonFileMap = cai.getNameToJsonFileMap();
        Map<Integer, Map<Double, LangevinPostprocessor.TimePointClustersInfo>> allRunsClusterInfoMap = cai.getAllRunsClusterInfoMap();
        assertTrue(NumRuns == nameToJsonFileMap.size(), "number of .json files should be equal to " + NumRuns);
        assertTrue(NumRuns == allRunsClusterInfoMap.size(), "number of .json files should be equal to " + NumRuns);

        lg.info("done");
    }

    @Test
    public void testMakeJsonFiles() throws IOException, URISyntaxException {

        // this test makes the json files for each run
        VCellMessaging vcellMessaging = new VCellMessagingNoop();
        File modelFile = new File(parent_dir, sim_base_name+".langevinInput");
        File simulationFolder = new File(parent_dir);   // place of input file, and .ida and .json result files for all runs

        Global g = new Global(modelFile);
        ConsolidationPostprocessor cp = new ConsolidationPostprocessor(g, 4, false, vcellMessaging);
        cp.setSimulationFolder(simulationFolder);
        cp.setNumRuns(NumRuns);
        cp.setSimulationName(sim_base_name);
        Map<String, Integer> molecules = ConsolidationPostprocessor.getMolecules(g);

        for(int runCounter=0; runCounter < NumRuns; runCounter++) {

            String data_dir = parent_dir + "/" + sim_base_name + "_FOLDER" + "/data";
            String runDataFolderString = data_dir + "/Run" + runCounter;
            File runDataFolder = new File(runDataFolderString);
            String newClustersFileName = sim_base_name + "_" + runCounter + ClustersFileExtension;
            File clustersFile = new File(parent_dir, newClustersFileName);
            LangevinPostprocessor.writeClustersFile(runDataFolder.toPath(), clustersFile.toPath());
            Map<Double, LangevinPostprocessor.TimePointClustersInfo> loadedClusterInfoMap = NdJsonUtils.loadClusterInfoMapFromNDJSON(clustersFile.toPath());
            assertNotNull(loadedClusterInfoMap, "loadedClusterInfoMap should not be null");
        }
        File[] jsonFiles = simulationFolder.listFiles((dir, name) -> name.toLowerCase().endsWith(".json"));
        assertNotNull(jsonFiles, "Directory listing failed or simulationFolder is not a directory");
        assertEquals(jsonFiles.length, (int) NumRuns, "Expected " + NumRuns + " JSON files, but found " + jsonFiles.length);
        lg.info("done");
    }

    // do not run on github actions, it's somewhat long
    @DisabledIfEnvironmentVariable(named = "GITHUB_ACTIONS", matches = "true")
    /*
     * This test exercises a more complex model where all reactions are used, also the ida files contain reaction statistics
     * Use it to test the multi-run postprocessor functionality by calculating the primary statistics (avg, std, min, max)
     * and the advanced statistics (cluster analysis) for all runs, and check that the expected files are created
     */
    @Test
    public void testAllReactions() throws IOException, URISyntaxException {

        //
        // TODO: remember to override resourcesFolder and sim_base_name in classpathInitialization() for this test
        // with:
        //         resourcesFolder = "all_reactions";
        //         sim_base_name = "sim";

        if(inputSource == InputSource.RESOURCES) {  // more resource files needed specifically for this test
            URL resourceWorkingDirUrl = Resource.getResource("all_reactions");
            // this test needs the .json and .ida files
            Resource.copyFilesWithExtension(resourceWorkingDirUrl, workDirPath, ClustersFileExtension);
            Resource.copyFilesWithExtension(resourceWorkingDirUrl, workDirPath, IdaFileExtension);
        }

        VCellMessaging vcellMessaging = new VCellMessagingNoop();
        File modelFile = new File(parent_dir, sim_base_name+".langevinInput");
        File simulationFolder = new File(parent_dir);   // place of input file, and .ida and .json result files for all runs

        Global g = new Global(modelFile);
        ConsolidationPostprocessor cp = new ConsolidationPostprocessor(g, 50, false, vcellMessaging);
        cp.setSimulationFolder(simulationFolder);
        cp.setNumRuns(NumRuns);
        cp.setSimulationName(sim_base_name);

        cp.calculateLangevinPrimaryStatistics();
        File targetFile = new File(simulationFolder, sim_base_name + "_Avg.ida");
        assertTrue(targetFile.exists(), "expected file " + sim_base_name + "_Avg.ida to exist");

        cp.calculateLangevinAdvancedStatistics();   // cluster analysis
        File[] csvFiles = simulationFolder.isDirectory() ?
                simulationFolder.listFiles((dir, name) -> name.endsWith(".csv")) : new File[0];
        // we get 3 advanced statistics files - unrelated to NumRuns!
        assertTrue(3 == csvFiles.length, "number of .csv files should be equal to 3");

        lg.info("done");
    }


}