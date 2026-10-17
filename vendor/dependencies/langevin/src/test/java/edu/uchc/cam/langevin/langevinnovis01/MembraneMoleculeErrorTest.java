package edu.uchc.cam.langevin.langevinnovis01;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.VCellMessagingLocal;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

/*
Bad input, using subdomain name, the solver expects one of the 3 reserved names Extracellular, Intracellular, Membrane
It categorizes the molecule as Intracellular, which results in failed initial location assignment
and out of bounds exception for partition index
MOLECULE: "Anchored" Extracellular_Intracellular_membrane Number 21 Site_Types 5 Total_Sites 5 Total_Links 4 is2D false

The vcell client should generate the line below, which seems to solve the problem
MOLECULE: "Anchored" Membrane Number 21 Site_Types 5 Total_Sites 5 Total_Links 4 is2D false
 */
public class MembraneMoleculeErrorTest {

    //             L_z_out: 0.010000000000000009
    String inputFileContents =
            """
            Total time: 1.00E-2
            dt: 1.00E-8
            dt_spring: 1.00E-9
            dt_data: 1.00E-4
            dt_image: 1.00E-4

            *** SYSTEM INFORMATION ***
            L_x: 0.1
            L_y: 0.1
            L_z_out: 0.01
            L_z_in: 0.09
            Partition Nx: 10
            Partition Ny: 10
            Partition Nz: 10

            *** MOLECULES ***

            MOLECULE: "Anchored" Membrane Number 21 Site_Types 5 Total_Sites 5 Total_Links 4 is2D false
            {
                TYPE: Name "Site0" Radius 1.00000 D 1.000 Color RED STATES "state0"
                TYPE: Name "Site1" Radius 1.00000 D 1.000 Color BLUE STATES "state0"
                TYPE: Name "Anchor" Radius 1.00000 D 1.000 Color DARK_GRAY STATES "Anchor"
                TYPE: Name "SitTran" Radius 1.00000 D 1.000 Color ORANGE STATES "State0" "State1"
                TYPE: Name "SitBin" Radius 1.00000 D 1.000 Color CYAN STATES "State0"

                SITE 0 : Extracellular : Initial State 'state0'
                    TYPE: Name "Site0" Radius 1.00000 D 1.000 Color RED STATES "state0"
                    x 0.00000 y 4.00000 z 4.00000\s
                SITE 1 : Extracellular : Initial State 'state0'
                    TYPE: Name "Site1" Radius 1.00000 D 1.000 Color BLUE STATES "state0"
                    x 0.00000 y 4.00000 z 8.00000\s
                SITE 2 : Membrane : Initial State 'Anchor'
                    TYPE: Name "Anchor" Radius 1.00000 D 1.000 Color DARK_GRAY STATES "Anchor"
                    x 0.00000 y 4.00000 z 12.00000\s
                SITE 3 : Intracellular : Initial State 'State0'
                    TYPE: Name "SitTran" Radius 1.00000 D 1.000 Color ORANGE STATES "State0" "State1"
                    x 0.00000 y 4.00000 z 16.00000\s
                SITE 4 : Intracellular : Initial State 'State0'
                    TYPE: Name "SitBin" Radius 1.00000 D 1.000 Color CYAN STATES "State0"
                    x 0.00000 y 4.00000 z 20.00000

                LINK: Site 0 ::: Site 1
                LINK: Site 1 ::: Site 2
                LINK: Site 2 ::: Site 3
                LINK: Site 3 ::: Site 4

                Initial_Positions: Random
            }
                            
            *** MOLECULE FILES ***

            MOLECULE: Anchored null

            *** CREATION/DECAY REACTIONS ***

            'Anchored' : kcreate  0  kdecay  0

            *** STATE TRANSITION REACTIONS ***

            'r0' ::     'Anchored' : 'SitTran' : 'State0' --> 'State1'  Rate 20.0  Condition None

            *** ALLOSTERIC REACTIONS ***


            *** BIMOLECULAR BINDING REACTIONS ***


            *** MOLECULE COUNTERS ***

            'Anchored' : Measure Total Free Bound

            *** STATE COUNTERS ***

            'Anchored' : 'Site0' : 'state0' : Measure Total Free Bound
            'Anchored' : 'Site1' : 'state0' : Measure Total Free Bound
            'Anchored' : 'Anchor' : 'Anchor' : Measure Total Free Bound
            'Anchored' : 'SitTran' : 'State0' : Measure Total Free Bound
            'Anchored' : 'SitTran' : 'State1' : Measure Total Free Bound
            'Anchored' : 'SitBin' : 'State0' : Measure Total Free Bound

            *** BOND COUNTERS ***


            *** SITE PROPERTY COUNTERS ***

            'Anchored' Site 0 : Track Properties true
            'Anchored' Site 1 : Track Properties true
            'Anchored' Site 2 : Track Properties true
            'Anchored' Site 3 : Track Properties true
            'Anchored' Site 4 : Track Properties true

            *** CLUSTER COUNTERS ***

            Track_Clusters: true

            *** SYSTEM ANNOTATIONS ***


            *** MOLECULE ANNOTATIONS ***


            *** REACTION ANNOTATIONS ***


            *** SIMULATION OPTIONS ***


            """;

    String expected_idafile_contents_2_lines = """
            t:r1:TOTAL_MT0:FREE_MT0:BOUND_MT0:TOTAL_MT0__Site0__state0:FREE_MT0__Site0__state0:BOUND_MT0__Site0__state0:TOTAL_MT0__Site1__state0:FREE_MT0__Site1__state0:BOUND_MT0__Site1__state0:TOTAL_MT0__Site1__state1:FREE_MT0__Site1__state1:BOUND_MT0__Site1__state1
            0.0 0 20 20 0 20 20 0 20 20 0 0 0 0
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
    public void testExtraMemIntraMolecule() throws IOException {
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
//            Assertions.assertTrue(idaFileContents.startsWith(expected_idafile_contents_2_lines));
//            Assertions.assertTrue(g.getStartSeed() != null, "startSeed must be not null");
//            Assertions.assertEquals(g.getStartSeed().longValue(),-1820505376029002863L, "Unexpected startSeed value");
            System.out.println(idaFileContents.substring(0,500)+"\n...\n"+idaFileContents.substring(idaFileContents.length()-500));
        } finally {
            deleteDirectory(tempDirectory.toFile());
        }
    }
}
