package edu.uchc.cam.langevin.langevinnovis01;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.VCellMessagingLocal;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

/**
 * The postprocess step copies two of the simulate step's subfolder outputs up to flat, canonically
 * named files beside the .ida files: the Run-0 trajectory ("viewer") file, and the SiteIDs.csv that
 * says which molecule and site each particle in it is.
 * <p>
 * This runs a short simulation rather than fabricating the subfolder, so it fails if the layout
 * MySystem writes to ever moves — otherwise the copy would silently find nothing and skip.
 */
public class CanonicalizeRunFilesTest {

    private static final String SIM_NAME = "sim";

    private final String inputFileContents =
            """
            Total time: 3.0E-4
            dt: 1.0E-8
            dt_data: 1.0E-4
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

            MOLECULE: "MT0" Intracellular Number 5 Site_Types 2 Total_Sites 2 Total_Links 1 is2D false
            {
                 TYPE: Name "Site0" Radius 1.00000 D 1.000 Color DARK_GRAY STATES "state0"
                 TYPE: Name "Site1" Radius 1.00000 D 1.000 Color BLUE STATES "state0"

                 SITE 0 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site0" Radius 1.00000 D 1.000 Color DARK_GRAY STATES "state0"
                      x 0.00000 y 0.00000 z 0.00000
                 SITE 1 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site1" Radius 1.00000 D 1.000 Color BLUE STATES "state0"
                      x 0.00000 y 0.00000 z 4.00000

                 LINK: Site 0 ::: Site 1

                 Initial_Positions: Random
            }

            *** MOLECULE FILES ***

            MOLECULE: MT0 null

            *** CREATION/DECAY REACTIONS ***

            'MT0' : kcreate  0  kdecay  0

            *** STATE TRANSITION REACTIONS ***

            *** ALLOSTERIC REACTIONS ***


            *** BIMOLECULAR BINDING REACTIONS ***

            *** MOLECULE COUNTERS ***

            'MT0' : Measure Total Free Bound

            *** STATE COUNTERS ***

            'MT0' : 'Site0' : 'state0' : Measure Total Free Bound
            'MT0' : 'Site1' : 'state0' : Measure Total Free Bound

            *** BOND COUNTERS ***

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

    @Test
    public void copiesRunZeroTrajectoryAndSiteIdsToFlatNames() throws IOException {
        Path tempDirectory = Files.createTempDirectory("test_canonicalize");
        try {
            ConsolidationPostprocessor cp = runShortSimulation(tempDirectory);

            Path siteIdsSource = tempDirectory.resolve(SIM_NAME + "_FOLDER")
                    .resolve("data").resolve("Run0").resolve("SiteIDs.csv");
            Assertions.assertTrue(Files.exists(siteIdsSource),
                    "the simulate step should have written " + siteIdsSource);

            cp.canonicalizeTrajectoryFile();
            cp.canonicalizeSiteIdsFile();

            Path flatViewer = tempDirectory.resolve(SIM_NAME + "_VIEW_Run0.txt");
            Path flatSiteIds = tempDirectory.resolve(SIM_NAME + "_SiteIDs_Run0.csv");
            Assertions.assertTrue(Files.exists(flatViewer), "missing " + flatViewer);
            Assertions.assertTrue(Files.exists(flatSiteIds), "missing " + flatSiteIds);
            Assertions.assertEquals(Files.readString(siteIdsSource), Files.readString(flatSiteIds),
                    "the flat copy should be byte-for-byte the original");

            // 5 molecules of 2 sites each, named "<molecule> Site <n> SiteType <type>"
            String siteIds = Files.readString(flatSiteIds);
            Assertions.assertEquals(10, siteIds.lines().filter(l -> !l.isBlank()).count());
            Assertions.assertTrue(siteIds.contains("MT0 Site 0 SiteType Site0"), siteIds);
            Assertions.assertTrue(siteIds.contains("MT0 Site 1 SiteType Site1"), siteIds);
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /** Re-running the copy must overwrite rather than fail. */
    @Test
    public void canonicalizingTwiceIsHarmless() throws IOException {
        Path tempDirectory = Files.createTempDirectory("test_canonicalize_twice");
        try {
            ConsolidationPostprocessor cp = runShortSimulation(tempDirectory);
            cp.canonicalizeSiteIdsFile();
            cp.canonicalizeSiteIdsFile();
            Assertions.assertTrue(Files.exists(tempDirectory.resolve(SIM_NAME + "_SiteIDs_Run0.csv")));
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    /**
     * A run that produced no subfolder output must not fail the postprocess — the primary results
     * are already written by the time these copies happen.
     */
    @Test
    public void missingSourceFilesAreSkippedNotFatal() throws IOException {
        Path tempDirectory = Files.createTempDirectory("test_canonicalize_missing");
        try {
            Path modelFile = tempDirectory.resolve(SIM_NAME + ".langevinInput");
            Files.writeString(modelFile, inputFileContents);
            Global g = new Global(modelFile.toFile());
            ConsolidationPostprocessor cp =
                    new ConsolidationPostprocessor(g, 1, false, new VCellMessagingLocal());

            // no simulation was run, so neither source exists
            Assertions.assertDoesNotThrow(cp::canonicalizeTrajectoryFile);
            Assertions.assertDoesNotThrow(cp::canonicalizeSiteIdsFile);
            Assertions.assertFalse(Files.exists(tempDirectory.resolve(SIM_NAME + "_VIEW_Run0.txt")));
            Assertions.assertFalse(Files.exists(tempDirectory.resolve(SIM_NAME + "_SiteIDs_Run0.csv")));
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }

    private ConsolidationPostprocessor runShortSimulation(Path tempDirectory) throws IOException {
        Path modelFile = tempDirectory.resolve(SIM_NAME + ".langevinInput");
        Path logFile = tempDirectory.resolve(SIM_NAME + ".log");
        Files.writeString(modelFile, inputFileContents);

        VCellMessaging vcellMessaging = new VCellMessagingLocal();
        Global g = new Global(modelFile.toFile(), logFile.toFile());
        new MySystem(g, 0, true, vcellMessaging).runSystem();
        return new ConsolidationPostprocessor(g, 1, true, vcellMessaging);
    }

    private void deleteDirectory(File directoryToBeDeleted) {
        File[] allContents = directoryToBeDeleted.listFiles();
        if (allContents != null) {
            for (File file : allContents) {
                deleteDirectory(file);
            }
        }
        directoryToBeDeleted.delete();
    }
}
