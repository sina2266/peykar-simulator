/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.counter.*;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.g.reaction.GDecayReaction;
import edu.uchc.cam.langevin.helpernovis.EtaTracker;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import edu.uchc.cam.langevin.helpernovis.Location;
import edu.uchc.cam.langevin.helpernovis.Rand;
import edu.uchc.cam.langevin.object.*;
import edu.uchc.cam.langevin.reaction.AllostericReactions;
import edu.uchc.cam.langevin.reaction.BindingReactions;
import edu.uchc.cam.langevin.reaction.TransitionReactions;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.vcell.data.LangevinPostprocessor;
import org.vcell.messaging.VCellMessaging;
import org.vcell.messaging.WorkerEvent;

import java.io.*;
import java.math.BigInteger;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Random;

public class MySystem {

    public static final Logger lg = LogManager.getLogger(MySystem.class);

    public static final String IdaFileExtension = ".ida";
    public static final String ClustersFileExtension = ".json";

    // Source of the global variables
    Global g;
    Random rand;

    // Arrays of molecules, sites, links, and bonds.
    private final ArrayList<Molecule> molecules = new ArrayList<>();
    private final ArrayList<Site> sites = new ArrayList<>();
    private final ArrayList<Link> links = new ArrayList<>();
    private final ArrayList<Bond> bonds = new ArrayList<>();

    // In SpringSaLad the ratio k / (k_B * T) = 100 nm−2 is the same for all bonds and links;
    // this allows us to use time steps ~ 1-100 ns for biologically relevant diffusion coefficients
    // (where k_B is the Boltzmann constant, T is an absolute temperature, k is the spring constant)
    public final static int SpringConstant = 100;

    // To quickly add and remove molecules, it will help to have a hashmap which
    // maps Gmolecule ids to an arraylist containing just unbound molecules of
    // that type.  (Unbound because those are the only ones that can undergo
    // creation/decay reactions.)
    private final HashMap<Integer,ArrayList<Molecule>> freeMoleculeMap = new HashMap<>();

    // REACTION CLASSES
    private final ArrayList<GDecayReaction> decayReactions;
    private final BindingReactions bindingReactions;
    private final TransitionReactions transitionReactions;
    private final AllostericReactions allostericReactions;

    // DATA CLASSES
    private final MoleculeCounter moleculeCounter;
    private final StateCounter stateCounter;
    private final BondCounter bondCounter;
    private final ClusterCounter clusterCounter;
    private final boolean countingClusters;
    private final SitePropertyCounter sitePropertyCounter;
    private final LocationTracker locationTracker;
    private final ReactionCounter reactionCounter;

    // To assign molecule ids during creation reactions, it helps to have a
    // global molecule index.
    int molindex = 10000;
    // Keep track of the molecule and site ids so I can link the ids to molecules and types
    private final ArrayList<String> moleculeIDs = new ArrayList<>();
    private final ArrayList<String> siteIDs = new ArrayList<>();

    // Spatial system information.
    private final double xmin;
    private final double xmax;
    private final double ymin;
    private final double ymax;
    private final double zmin;
    private final double zmax;

    private final int npartx;
    private final int nparty;
    private final int npartz;

    // Temporal system information.
    private final double totalTime;     // simulation total time (time between begin time and end time)
    private final double dt;            // simulation time step default 1.0E-8
    private final double dtspring;      // spring interval default 1.0E-9
    private final double dtdata;        // data output interval (when we update the counters) default 1.0E-4
    private final double dtimage;       // image output interval default 1.0E-4

    // Current system time
    // starts at 0 and ends at totalTime, grows in increments of dt
    private double time = 0;

    // File information
    // Input file so we know where to read/write
    private File inputFile;
    // The name, not including ".txt".
    private String fileName;
    private File folder;
    // 2015-07-12 - Changing code to use a single viewer file. This will
    // dramatically cut down on storage space.
//    private File viewerFolder;
    private File viewerFile = null;
    private File dataFolder;

    // When launching from the front end GUI, I want the system updates to go
    // to pre-defined file.  Otherwise, just use the normal System.out
    private final boolean useOutputFile;

    // run counter for the viewer and data folders
    private final int runCounter;
    // Image counter for image file io
    private int imageCounter = 0;

    // Partition array
    private final Partition[][][] partition;
    private final double [] partitionSize = new double[3];
    private final ArrayList<Partition> activePartitions = new ArrayList<>();

    // Time we started the simulation and time it finished
    private long startTime; //  real system time: System.currentTimeMillis();
    private long stopTime;

    private final VCellMessaging vcellMessaging;
    private EtaTracker etaTracker;

    // simple pseudo random number generator using the LCG algorithm (Linear Congruential Generator)
    public class LangevinLCG {
        private long a = 1664525;               // multiplier
        private long c = 1013904223;            // increment
        private long m = 4294967296L;           // modulus (2^32)
        private long initialSeed = 1;           // initial seed
        private long currentSeed = initialSeed; // current seed

        public LangevinLCG(long seed) {
            this.initialSeed = seed;
            this.currentSeed = initialSeed;
        }
        public long next() {
            currentSeed = (a * currentSeed + c) % m;
            return currentSeed;
        }
        public String toString() {
            return(initialSeed + " " + currentSeed);
        }
    }
    /**********************************************************************\
     *                         CONSTRUCTOR                                *
     * The constructor just takes in the global class which was set up    *
     * by the input file.  The global object contains all of the data     *
     * needed to set up and run the system.                               *
     * @param g                                                           *
     * @param runCounter                                                  *
     * @param useOutputFile
    \**********************************************************************/

    public MySystem(Global g, int runCounter, boolean useOutputFile, VCellMessaging vcellMessaging) throws IOException {
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        // if an explicit start random seed was found in the input file, we use it and runCounter
        // // to generate a unique seed for this run, otherwise we just use the current time
        if(g.getStartSeed() == null) {
            // the granularity of system time may be larger than ms, depending on the OS
            // NOTE: the 2 calls to System.currentTimeMillis() on my Windows machine always returned the same number
            Rand.seedRand(System.currentTimeMillis());
            long seed = System.currentTimeMillis();
            rand = new Random(seed);
        } else {
            BigInteger bi = g.getStartSeed();
            long seed = bi.longValue();   // truncates if necessary
            LangevinLCG langevinLCG = new LangevinLCG(seed);
            for(int i=0; i <= getRunCounter(); i++) {
                seed = langevinLCG.next();
            }
            Rand.seedRand(seed);
            rand = new Random(seed);
        }
        lg.info("in solver, running: " + runCounter);

        this.g = g;
        this.runCounter = runCounter;
        this.useOutputFile = useOutputFile;
        this.vcellMessaging = vcellMessaging;
        this.etaTracker = new EtaTracker();

        vcellMessaging.sendWorkerEvent(WorkerEvent.startingEvent("Starting Simulation"), VCellMessaging.ThrowOnException.NO);

        this.decayReactions = g.getDecayReactions();
        bindingReactions = new BindingReactions(g, this);
        transitionReactions = new TransitionReactions(g, this, bindingReactions);
        allostericReactions = new AllostericReactions(g, this, bindingReactions);

        this.moleculeCounter = new MoleculeCounter(g, this);
        this.stateCounter = new StateCounter(g, this);
        this.bondCounter = new BondCounter(g, this);
        this.countingClusters = g.isCountingClusters();
        this.clusterCounter = new ClusterCounter(g, this);
        this.sitePropertyCounter = new SitePropertyCounter(g,this);
        this.locationTracker = new LocationTracker(g, this);
        this.reactionCounter = new ReactionCounter(g, this);

        setReactionCounter(this);

        // Spatial informartion.
        double xsize = g.getXsize();
        double ysize = g.getYsize();
        double zin = g.getZin();
        double zout = g.getZout();
        xmin = -xsize/2;
        xmax = xsize/2;
        ymin = -ysize/2;
        ymax = ysize/2;
        zmin = -zout;
        zmax = zin;
        npartx = g.getNpartx();
        nparty = g.getNparty();
        npartz = g.getNpartz();

        // Time information.
        totalTime = g.getTotalTime();
        dt = g.getdt();
        dtspring = g.getdtspring();
        dtdata = g.getdtdata();
        dtimage = g.getdtimage();

        // Partition setup
        partition = new Partition[npartx][nparty][npartz];
        createPartitions();

        // File information and file setup.
        folderSetup();

        // Initialize the Molecule hashmap arraylists
        ArrayList<GMolecule> gmolecules = g.getMolecules();
        for (GMolecule gmolecule : gmolecules) {
            freeMoleculeMap.put(gmolecule.getID(), new ArrayList<Molecule>());
        }

        makeMolecules();
        // </editor-fold>
    }

    private void setReactionCounter(MySystem mySystem) {
        bindingReactions.setReactionCounter(mySystem);
        transitionReactions.setReactionCounter(mySystem);
        allostericReactions.setReactionCounter(mySystem);
    }

    // **********************   GET METHODS **************************

    public ArrayList<Molecule> getMolecules(){
        return molecules;
    }
    public ArrayList<Bond> getBonds(){
        return bonds;
    }
    public ArrayList<Site> getSites(){
        return sites;
    }
    public double getTime(){
        return time;
    }
    public File getFolder(){
        return folder;
    }
    public int getRunCounter(){
        return runCounter;
    }
    public ReactionCounter getReactionCounter() {
        return reactionCounter;
    }
    public EtaTracker getEtaTracker() {
        return etaTracker;
    }


    // *********************  FOLDER MANAGEMENT ************************
    private void folderSetup() throws IOException {
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        // File information and file setup.
        inputFile = g.getInputFile();
        fileName = inputFile.getName();
        String filePath = inputFile.getAbsolutePath();
        // Strip the file name off of the path
        filePath = filePath.substring(0,filePath.length()-fileName.length());
        // Strip extension off the file name
        int dotIndex = fileName.lastIndexOf('.');
        if (dotIndex > 0) {
            fileName = fileName.substring(0, dotIndex);
        } else {
            // useless to also log error with same info
            throw new IllegalArgumentException("Input file must have an extension: " + fileName);
        }

        /* Check to see if the file is already in a folder named fileName_FOLDER
         * If it is, then we set folder to this folder. If it is not, then
         * we make this folder and we make a copy of the file in that folder.
         */
        String folderName = fileName + "_FOLDER";
        if(filePath.length() > folderName.length() && filePath.substring(folderName.length()-1).equals(folderName)){
            folder = new File(filePath);
        } else {
            folder = new File(filePath, folderName);
            // If this is the first run, then make a new directory and copy the
            // file into this directory.
            if(runCounter == 0){
                folder.mkdir();
                // Now make a copy of the input file in the new folder
                try{
                    Files.copy(inputFile.toPath(), folder.toPath().resolve(inputFile.toPath().getFileName()),StandardCopyOption.REPLACE_EXISTING );
                } catch(IOException e){
                    throw new IOException("Failed to copy input file to working folder", e);
                }
            }
        }
        // If this is the first run, then create some new folders
        // Now make folders to store the view files, the data files, video files, and image files
        if(runCounter == 0){
            try{
                Files.createDirectory(folder.toPath().resolve("images"));
            } catch(FileAlreadyExistsException fe){
                // Ignore this exception.
            } catch(IOException e){
                lg.warn("'images' folder creation failed in MySystem constructor.");
            }

            try{
                Files.createDirectory(folder.toPath().resolve("videos"));
            } catch(FileAlreadyExistsException fe){
                // Ignore this exception.
            } catch(IOException e){
                lg.warn("'videos' folder creation failed in MySystem constructor.");
            }

            try{
                Files.createDirectory(folder.toPath().resolve("viewer_files"));
            } catch(FileAlreadyExistsException fe){
                // Ignore this exception.
            } catch(IOException e){
                lg.warn("'videos' folder creation failed in MySystem constructor.");
            }
        }

//        // When we make the viewer and data files, we also want to determine which "run"
//        // folder we should create.
//        try{
//            viewerFolder = Files.createDirectories(folder.toPath().resolve("viewer_files/Run" + runCounter)).toFile();
//        } catch(FileAlreadyExistsException fe){
//            viewerFolder = new File(folder.toString() + "/viewer_files/Run" + runCounter);
//        } catch(IOException e){
//            lg.info("'viewer_files' folder creation failed in MySystem constructor.");
//            viewerFolder = null;
//        }

        try{
            dataFolder = Files.createDirectories(folder.toPath().resolve("data/Run" + runCounter)).toFile();
        } catch(FileAlreadyExistsException fe){
            dataFolder = new File(folder.toString() + "/data/Run" + runCounter);
        } catch(IOException e){
            dataFolder = null;
            throw new IOException("Failed to create data folder for Run " + runCounter, e);
        }

        // Now write the header file
        if(runCounter == 0){        // we only save movie for Run0
            viewerFile = new File(folder.toString() + "/viewer_files/", fileName + "_VIEW_Run" + runCounter + ".txt");
            writeViewerFileHeader();
        }
        // </editor-fold>
    }

    /*********************************************************************\
     *                    FREE MOLECULE MANAGEMENT                       *
     \*********************************************************************/

    private void addFreeMolecule(Molecule molecule){
        ArrayList<Molecule> freeMolecules = freeMoleculeMap.get(molecule.getGID());
        freeMolecules.add(molecule);
    }

    private void removeFreeMolecule(Molecule molecule){
        ArrayList<Molecule> freeMolecules = freeMoleculeMap.get(molecule.getGID());
        freeMolecules.remove(molecule);
    }

    /**************************************************************\
     *                  RANDOM POSITION                           *
     *  Makes a new random position based on the current system   *
     * bounds.                                                    *
     \**************************************************************/

    private MyVector randomVector(int location){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        double xrand = Rand.randomDouble(xmin, xmax);
        double yrand = Rand.randomDouble(ymin, ymax);
        double zrand;
        if(location == Location.INSIDE){
            zrand = Rand.randomDouble(0,zmax);
        } else if(location == Location.OUTSIDE){
            zrand = Rand.randomDouble(zmin, 0);
        } else {
            zrand = 0;
        }
        return new MyVector(xrand, yrand, zrand);
        // </editor-fold>
    }

    /*******************************************************************\
     *                COLLISION DETECTION DURING SETUP                 *
     * Checks for collisions with other particles AND with the         *
     * boundaries of the system.                                       *
     * This method is only used during system setup and so does not    *
     * bother using partitions to increase collision detection.  If    *
     * the systems get very very crowded I might need to change this.  *
     \*******************************************************************/

    private boolean siteOverlap(ArrayList<Site> newSites){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        boolean overlap = false;
        double r, rnew;
        for (Site newSite : newSites) {
            if(overlap){
                break;
            }
            Site tNewSite = newSite;
            MyVector newLoc = tNewSite.getPosition();
            rnew = tNewSite.getRadius();
            for (Site currSite : sites) {
                MyVector currLoc = currSite.getPosition();
                r = currSite.getRadius();
                double rtot = r + rnew;
                MyVector l = currLoc.subtract(newLoc);
                double len2 = l.length2();
                if(len2 < rtot*rtot){
                    overlap = true;
                    break;
                }
            }
            // Check against the walls
            if(!overlap){
                if(newLoc.x + rnew > xmax){
                    overlap = true;
                    break;
                }
                if(newLoc.x - rnew < xmin){
                    overlap = true;
                    break;
                }
                if(newLoc.y + rnew > ymax){
                    overlap = true;
                    break;
                }
                if(newLoc.y - rnew < ymin){
                    overlap = true;
                    break;
                }
                if(tNewSite.getLocation() == Location.INSIDE){
                    if(newLoc.z - rnew < 0){
                        overlap = true;
                        break;
                    }
                    if(newLoc.z + rnew > zmax){
                        overlap = true;
                        break;
                    }
                } else if(tNewSite.getLocation() == Location.OUTSIDE){
                    if(newLoc.z - rnew < zmin){
                        overlap = true;
                        break;
                    }
                    if(newLoc.z + rnew > 0){
                        overlap = true;
                        break;
                    }
                }
            }
        }
        return overlap;
        // </editor-fold>
    }

    /******************************************************************\
     *           COLLISION DETECTION DURING SIMULATION                *
     \******************************************************************/

    private void checkCollisions(Site site, ArrayList<Site> checkSites){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        double x = site.getX();
        double y = site.getY();
        double z = site.getZ();
        double r = site.getRadius();
        double rreact = site.getReactionRadius();
        MyVector vec = new MyVector();
        Site tempSite;
        for(int i=checkSites.size()-1;i>-1;i--){
            tempSite = checkSites.get(i);
            if(!tempSite.getChecked()){
                double tempx = tempSite.getX();
                double tempy = tempSite.getY();
                double tempz = tempSite.getZ();
                vec.x = x - tempx;
                vec.y = y - tempy;
                vec.z = z - tempz;
                double L2 = vec.length2();
                double sumRadii = r + tempSite.getRadius();
                double sumReactRadii = rreact + tempSite.getReactionRadius();
                // Check reaction radii first
                if(L2 < sumReactRadii*sumReactRadii){
                    // Look for collision before reaction, otherwise we could end
                    // up with bonds that are smaller then the sum of the site radii
                    if(L2 < sumRadii*sumRadii){
                        double L = Math.sqrt(L2);
                        double overlap = sumRadii - L;
                        if(site.isFixed()){
                            double newx = tempx - overlap*vec.x/L;
                            double newy = tempy - overlap*vec.y/L;
                            // The above two lines should really be modified if tempSite is 2D.  I'll get to that later.
                            if(tempSite.is2D()){
                                tempSite.setPosition(newx, newy, tempz);
                                tempSite.checkBoundaries();
                            } else {
                                tempSite.setPosition(newx, newy, tempz - overlap*vec.z/L);
                                tempSite.checkBoundaries();
                            }
                            // They can't both be fixed, or else they wouldn't have collided.
                        } else if(tempSite.isFixed()){
                            double newx1 = x + overlap*vec.x/L;
                            double newy1 = y + overlap*vec.y/L;
                            if(site.is2D()){
                                site.setPosition(newx1,newy1,z);
                                site.checkBoundaries();
                            } else {
                                site.setPosition(newx1, newy1, z + overlap*vec.z/L);
                                site.checkBoundaries();
                            }
                        } else {
                            double coefficient = 0.5*overlap/L;
                            double tx = tempx - coefficient*vec.x;
                            double ty = tempy - coefficient*vec.y;
                            double nx = x + coefficient*vec.x;
                            double ny = y + coefficient*vec.y;
                            if(tempSite.is2D()){
                                tempSite.setPosition(tx,ty,tempz);
                                tempSite.checkBoundaries();
                            } else {
                                tempSite.setPosition(tx,ty,tempz - coefficient*vec.z);
                                tempSite.checkBoundaries();
                            }
                            if(site.is2D()){
                                site.setPosition(nx,ny,z);
                                site.checkBoundaries();
                            } else {
                                site.setPosition(nx, ny, z+coefficient*vec.z);
                                site.checkBoundaries();
                            }
                        }
                    }
                    // DETERMINE IF A BIMOLECULE REACTION OCCURS
                    // First make sure both sites are unbound
                    if(!site.isBound() && !tempSite.isBound()){
                        GState state1 = site.getState();
                        GState state2 = tempSite.getState();
                        String key1 = state1.getIdAsString();
                        String key2 = state2.getIdAsString();
                        lg.trace("--- (state1, state2) = (" + state1.getStateName() + ", " + state2.getStateName() + ")");
                        lg.trace("(key1, key2) = (" + key1 + ", " + key2 + ")");

                        // Now make sure the sites actually react (if they don't
                        // then this prevents an unnecessary random number generation).
                        if(bindingReactions.doReact(state1, state2)){
                            lg.trace("Looking for binding reaction.");
                            if(bindingReactions.checkForReaction(key1, key2)){
                                reactionCounter.plusBindingReaction(bindingReactions.getName(key1, key2));
                                Bond newBond = new Bond(site, tempSite,
                                        SpringConstant, bindingReactions.getOffProb(key1, key2),
                                        bindingReactions.getName(key1, key2),
                                        bindingReactions.getBondLength(key1, key2));
                                bonds.add(newBond);
                                site.setBound(true);
                                site.setBindingPartner(tempSite);
                                site.setBond(newBond);
                                tempSite.setBound(true);
                                tempSite.setBindingPartner(site);
                                tempSite.setBond(newBond);
                                Molecule m0 = site.getMolecule();
                                Molecule m1 = tempSite.getMolecule();
//                                m0.addBindingPartner(m1);
//                                m1.addBindingPartner(m0);
                                if(!m0.isBound()){
                                    removeFreeMolecule(m0);
                                }
                                m0.plusBond();
                                if(m0 != m1){
                                    if(!m1.isBound()){
                                        removeFreeMolecule(m1);
                                    }
                                    m1.plusBond();
                                }
                                if(lg.getLevel() == org.apache.logging.log4j.Level.DEBUG) {
                                    String thisWho = m0.getGName() + "(" + site.getType() + "~" + site.getState().getStateName() + ")";
                                    String thatWho = m1.getGName() + "(" + tempSite.getType() + "~" + tempSite.getState().getStateName() + ")";
                                    lg.debug("Binding reaction : " + bindingReactions.getName(key1, key2) + " between " + thisWho + " + " + thatWho);
                                }
                            }
                        }
                    }
                }
            }
        }
        // </editor-fold>
    }

    /**********************************************************************\
     *                  ADD ONE MOLECULE TO SYSTEM                        *
     *  Method to add a single molecule to the system, given a gmolecule. *
     *  Makes sense to define this method since the same code is needed   *
     *  when we first initialize the molecules and when we add a new      *
     *  molecule to the system.                                           *
     \**********************************************************************/

    private void addMolecule(GMolecule gmolecule){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        Molecule tempMol;
        ArrayList<Molecule> mapArray = freeMoleculeMap.get(gmolecule.getID());
        boolean overlap;
        int counter = 0;
        do{
            tempMol = gmolecule.newInstance(molindex, randomVector(gmolecule.getLocation()));
            ArrayList<Site> newSites = tempMol.getSites();
            overlap = siteOverlap(newSites);
            counter++;
            if (counter > 100000) {
                throw new IllegalStateException("Failed to place molecule '" + gmolecule.getID() + "' after " + counter + " attempts.");
            }
        } while (overlap);
        molecules.add(tempMol);
        moleculeIDs.add(tempMol.getID() + "," + gmolecule.getName());
        mapArray.add(tempMol);
        ArrayList<Site> tSites = tempMol.getSites();
        // Record the site ids before we shuffle
        for(Site ts : tSites){
            siteIDs.add(ts.getID() + "," + gmolecule.getName() + " Site " + ts.getGSiteIndex() + " SiteType " + ts.getType());
        }
        // Randomize the order of the sites before assigning them to the site array.
        // Otherwise there are correlations in long molecules because of the order
        // of collision checking.
        Collections.shuffle((List)tSites);
        for (Site ts : tSites) {
            ts.setDiffusionScale(dt);
            if(ts.getLocation() == Location.INSIDE){
                ts.setBounds(xmin, xmax, ymin, ymax, 0, zmax);
            } else if(ts.getLocation() == Location.OUTSIDE){
                ts.setBounds(xmin, xmax, ymin, ymax, zmin, 0);
            } else {
                // z boundaries don't matter for membrane sites
                ts.setBounds(xmin, xmax, ymin, ymax, zmin, zmax);
            }
            sites.add(ts);
            assignPartition(ts);
        }
        molindex++;
        ArrayList<Link> tLinks = tempMol.getLinks();
        for (Link tLink : tLinks) {
            links.add(tLink);
        }
        // </editor-fold>
    }

    /*****************************************************************\
     *                       MAKE MOLECULES                          *
     *  We also create the hashmap between gmolecule ids and         *
     *  arraylists of specific molecules.                            *
     \*****************************************************************/

    private void makeMolecules(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        ArrayList<GMolecule> gmols = g.getMolecules();
        GMolecule gmol;
        Molecule tempMol;

        for (GMolecule gmol1 : gmols) {
            gmol = gmol1;
            for(int j=0;j<gmol.getNumber();j++){
                lg.trace("Adding molecule: '" + gmol.getName() + "', instance " + j + " of " + gmol.getNumber());
                addMolecule(gmol);
            }
        }
        // </editor-fold>
    }

    /*********************************************************************\
     *                      PARTITION MANAGEMENT                         *
     \*********************************************************************/

    private void createPartitions(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        // get the partition sizes
        double dx = (xmax-xmin)/npartx;
        double dy = (ymax-ymin)/nparty;
        double dz = (zmax-zmin)/npartz;

        partitionSize[0] = dx;
        partitionSize[1] = dy;
        partitionSize[2] = dz;

        /*
         * Use a temporary 3D array to add the partitions.  This will make it
         * easy to decide which arrays are neighbors. In the end I can throw
         * away that array and just use the 1D array to store them all. To
         * make the neighbor assignment much much easier, I'm going to use
         * an array which has extra entries at the border of the array.  Keep
         * these entries null and then I can use a simple if not null statement
         * below to assign the neighbors.
         */

        Partition [][][] tempPartition = new Partition[npartx+2][nparty+2][npartz+2];

        for(int i=0;i<npartx+2;i++){
            double x0,x1;
            x0 = xmin + (i-1)*dx;
            if(i != npartx){
                x1 = xmin + i*dx;
            } else {
                x1 = xmax;
            }
            double [] xs = {x0,x1};

            for(int j=0;j<nparty+2;j++){
                double y0, y1;
                y0 = ymin + (j-1)*dy;
                if(j != nparty){
                    y1 = ymin + j*dy;
                } else {
                    y1 = ymax;
                }
                double [] ys = {y0,y1};

                for(int k=0;k<npartz+2;k++){
                    double z0,z1;
                    z0 = zmin + (k-1)*dz;
                    if(k!=npartz){
                        z1 = zmin + k*dz;
                    } else {
                        z1 = zmax;
                    }
                    double [] zs = {z0,z1};

                    if(i!=0 && i!=npartx+1 && j!=0 && j!=nparty+1 && k!=0 && k!=npartz+1){
                        tempPartition[i][j][k] = new Partition(xs, ys, zs);
                        //lg.debug("Made partition with x range " + tempPartition[i][j][k].getXString());
                        lg.trace("Made partition with (i,j,k) = (" + i + ", " + j + ", " + k + ")");
                        partition[i-1][j-1][k-1] = tempPartition[i][j][k];
                    } else {
                        tempPartition[i][j][k] = null;
                    }
                }
            }
        }

        // Now loop through and assign neighbors. Each partition will have
        // 26 neighbors in 3D.
        for(int i=0;i<npartx+2;i++){
            for(int j=0;j<nparty+2;j++){
                for(int k=0;k<npartz+2;k++){
                    // First make sure it's not null
                    if(tempPartition[i][j][k] != null){
                        // Now just add all the neighbors, checking for nulls
                        for(int ii=-1;ii<=1;ii++){
                            for(int jj=-1;jj<=1;jj++){
                                for(int kk=-1;kk<=1;kk++){

                                    if(tempPartition[i+ii][j+jj][k+kk] != null){
                                        tempPartition[i][j][k].addPartition(tempPartition[i+ii][j+jj][k+kk]);

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // </editor-fold>
    }

    private void assignPartition(Site site){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        int nx = (int)((site.getX()-xmin)/partitionSize[0]);
        int ny = (int)((site.getY()-ymin)/partitionSize[1]);
        int nz = (int)((site.getZ()-zmin)/partitionSize[2]);
        Partition tempPart = null;
        try{
            tempPart = partition[nx][ny][nz];
        } catch(ArrayIndexOutOfBoundsException e){
            throw new ArrayIndexOutOfBoundsException("partition index out of bounds at (nx,ny,nz) " +
                    "= (" + nx + ", " + ny + ", " + nz + ")");
        }
        tempPart.addSite(site);
        site.setPartition(tempPart);
        if(!tempPart.isActive()){
            tempPart.setActive(true);
            activePartitions.add(tempPart);
        }
        // </editor-fold>
    }

    private void clearPartitions(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        for(int i=activePartitions.size()-1;i>-1;i--){
            activePartitions.get(i).clearSites();
        }
        activePartitions.clear();
        // </editor-fold>
    }

    /*********************************************************************\
     *                    DECAY REACTION UPDATE                          *
     *  Updates for the decay reactions can be logically separated from  *
     *  the other system updates, so they'll be updated in their own     *
     *  method.  We loop through all of the decay reactions, decide      *
     *  if a reaction occurs, and then update the free molecule map      *
     *  accordingly.  If we add a molecule, it is added to both the      *
     *  the full molecule list and the free molecule list.  If we        *
     *  destroy a molecule, it is nulled out and removed from the        *
     *  hashmap arraylist.  Only after we've gone through all reactions  *
     *  do we go through and remove any null molecules from the full     *
     *  molecule list.  There is probably a more efficient way of doing  *
     *  this, but right now I just want to get these reactions working.  *
     \*********************************************************************/

    private void checkDecayReactions(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        ArrayList<Molecule> freeMolecules;
        GDecayReaction decayReaction;
        GMolecule gmol;
        for(int i=decayReactions.size()-1;i>-1;i--){
            decayReaction = decayReactions.get(i);
            gmol = decayReaction.getMolecule();
            freeMolecules = freeMoleculeMap.get(gmol.getID());
            // reaction = 1 if we make a molecule, -1 to remove a molecule, 0 otherwise
            int reaction = decayReaction.getReaction(freeMolecules.size());
            if(reaction == 1){
                addMolecule(gmol);
                reactionCounter.plusCreationReaction(decayReaction.getName());
            } else if(reaction == -1){
                // If this reaction occurs the number of free molecules must
                // be greater than zero.
                int randIndex = rand.nextInt(freeMolecules.size());
                Molecule tmol = freeMolecules.remove(randIndex);
                // There must be a faster way to implement the next line.
                molecules.remove(tmol);
                sites.removeAll(tmol.getSites());
                links.removeAll(tmol.getLinks());
                reactionCounter.plusDecayReaction(decayReaction.getName());
            }
        }
        // </editor-fold>
    }

    /*******************************************************************\
     *                    SPRING RELAXATION METHOD                     *
     * @param n The number of relaxation steps to take.                *
    \*******************************************************************/

    private void relaxSprings(int n){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        Site tempSite1;
        int size = sites.size();
        // NEED TO CLEAR THE PARTITIONS
        clearPartitions();
        // Now clear all of the random forces
        for(int i=0;i<size;i++){
            sites.get(i).setRandomForce(0, 0, 0);
        }

        int counter = 0;
        while(counter < n){
            // NOW JUST UPDATE THE POSITIONS
            for(int i=0;i<size;i++){
                tempSite1 = sites.get(i);
                // Use the smaller dtspring
                tempSite1.updatePosition(dtspring);
                tempSite1.clearSpringForce();
            }

            for(int i=links.size()-1;i>-1;i--){
                links.get(i).updateOrientation();
                links.get(i).updateForces();
            }
            for(int i=bonds.size()-1;i>-1;i--){
                bonds.get(i).updateOrientation();
                bonds.get(i).updateForces();
            }
            counter++;
        }

        // Now reassign the partitions
        for(int i=0;i<size;i++){
            this.assignPartition(sites.get(i));
        }
        // </editor-fold>
    }

    /**********************************************************************\
     *                         SYSTEM UPDATE                              *
     * This is the workhorse method which actually takes the time step    *
     * by updating all of the positions and determining if any reactions  *
     * occurred.                                                          *
     \**********************************************************************/

    private void update(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        double randfx, randfy, randfz;
        Site site1;
        Link link;
        Bond bond;

        // Clear the partitions before updating the site positions
        clearPartitions();

        // Check for creation/decay reactions
        checkDecayReactions();

        int size = sites.size();
        // Now update the positions and assign partitions.
        for(int i=0;i<size;i++){
            site1 = sites.get(i);
            // Look to see if this site undergoes a transition reaction
            transitionReactions.tryReactions(site1);
            // Look to see if this site undergoes an allosteric reaction
            allostericReactions.tryReactions(site1);
            // IF WE FIX ANY PARTICLES OR RESTRICT MOTION TO 2D, THIS IS
            // WHERE THE CODE SHOULD GO. We don't want to calculate any
            // unnecessary Gaussian random variables.
            if(site1.is2D()){
                randfx = Rand.randomGaussian();
                randfy = Rand.randomGaussian();
                randfz = 0;
            } else {
                randfx = Rand.randomGaussian();
                randfy = Rand.randomGaussian();
                randfz = Rand.randomGaussian();
            }
            site1.setRandomForce(randfx, randfy, randfz);
            site1.updatePosition(dt);
            site1.clearSpringForce();
            // No need to assign the partition here. They get assigned
            assignPartition(site1);
            site1.setChecked(false);

        }

        /**                       RELAX SPRINGS HERE
         * I used to relax the springs in the run() method after the update
         * method, but I think it's better to relax them here, after the
         * sites have been moved but before we check for collisions and reactions.
         * For example, consider two molecules, each withe one reactive site
         * and one inactive site.  Imagine that the molecules are near each other,
         * and after we take a step the reactive sites reaction radii overlap.
         * But imagine that the time step was large, so that the springs in the
         * molecules are over-stretched, and when we relax the springs the
         * reaction radii no longer overlap.  In this case, checking for
         * a reaction before relaxing the springs could have led to a reaction
         * occurring while the spatial constraints imposed by the springs are
         * violated.  In other words, had we taken small enough time steps to
         * begin with, then the sites should never have had a chance to react.
         *
         * Of course, the exact order of these checks won't matter as long as
         * dt is small enough, but the whole point of using the relaxation
         * step is to allow us to take larger time steps than the springs could
         * tolerate.
         *
         *                          EDIT
         *
         * If we relax the springs here, then after we check for collisions
         * a spring can be stretched.  The next move update is done with the
         * full dt, and that can launch a site out of the system.  I guess
         * springs should be relaxed before and after checking for collisions,
         * but that seems like overkill.  I'm just going move the relax springs
         * method back to the run() method, after update().
         */
//        if(relaxationSteps > 2){
//            relaxSprings(relaxationSteps);
//        }

        // Loop through and check for collisions
        for(int i=0;i<size;i++){
            site1 = sites.get(i);
            // Mark this site as checked
            site1.setChecked(true);
            // Loop over the sites in neighboring partitions
            Partition tempPart1 = site1.getPartition();
            ArrayList<Partition> partitionList = tempPart1.getNeighbors();
            Partition tempPart2;
            for(int j=partitionList.size()-1;j>-1;j--){
                tempPart2 = partitionList.get(j);
                ArrayList<Site> siteList = tempPart2.getSites();
                checkCollisions(site1,siteList);
            }
        }

        // loop throught the links to determine new forces
        for(int i=links.size()-1;i>-1;i--){
            link = links.get(i);
            link.updateOrientation();
            link.updateForces();
        }
        // loop through bonds to determine new forces.  Also look to see if they dissociate!
        ArrayList<Bond> bondsToRemove = new ArrayList<>();
        for(int i=bonds.size()-1;i>-1;i--){
            bond = bonds.get(i);
            bond.updateOrientation();
            bond.updateForces();
            if(bond.dissociates()){
                if(lg.getLevel() == org.apache.logging.log4j.Level.DEBUG) {
                    Site s0 = bond.getSites()[0];
                    Site s1 = bond.getSites()[1];
                    String who = s0.getType() + ";" + s0.getState().getStateName() + " and " + s1.getType() + ";" + s1.getState().getStateName();
                    lg.debug("Bond " + bond.getName() + " dissociates: " + who);
                }
                reactionCounter.plusDissociationReaction(bond.getName());
                bondsToRemove.add(bond);
                Site [] bsite = bond.getSites();
                bsite[0].setBound(false);
                bsite[0].setBindingPartner(null);
                bsite[0].setBond(null);
                bsite[1].setBound(false);
                bsite[1].setBindingPartner(null);
                bsite[1].setBond(null);
                Molecule m0 = bsite[0].getMolecule();
                Molecule m1 = bsite[1].getMolecule();
                // This part of the code was wrong.  It was removing m0 as a
                // binding partner of m1, and vice versa, even if they were
                // still bound at another site.  This was messing up my
                // clustering code, and probably was introducing other errors
                // I wasn't aware of.  The right way to fix this is to keep
                // track of the multiplicity of each binding partner. I decided
                // to fix it by getting rid of the binding partner array and
                // just generating it each time I need to count clusters.
//                m0.removeBindingPartner(m1);
//                m1.removeBindingPartner(m0);
                m0.minusBond();
                if(!m0.isBound()){
                    addFreeMolecule(m0);
                }
                if(m0 != m1){
                    m1.minusBond();
                    if(!m1.isBound()){
                        addFreeMolecule(m1);
                    }
                }
            }
        }
        bonds.removeAll(bondsToRemove);
        // </editor-fold>
    }

    /********************************************************************\
     *                            RUN METHOD                            *
     * This is the method which implements the loop to run the system   *
     * simulation.                                                      *
     \********************************************************************/

    public void runSystem() throws IOException {
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        lg.debug("This stdout file is associated with run counter " + runCounter + ".");
        lg.info("Simulation started.");
        startTime = System.currentTimeMillis();
        etaTracker.initialize(startTime);

        // create fresh log file immediately at startup for all runs in the batch except for run #0
        if (useOutputFile && runCounter != 0) {
            try (PrintWriter p = new PrintWriter(new FileWriter(g.getOutputFile(), false))) {
                p.println("Simulation 0% complete. Elapsed time: " + IOHelp.formatTime(startTime, startTime));
            } catch (IOException ioe) {
                lg.warn("Could not create initial log file: " + g.getOutputFile(), ioe);
            }
        } else if(useOutputFile && runCounter == 0) {
            // we know for sure that the log file for run 0 was created at the start of the solver
            // and we need the special preamble for single runs, so we can just append to it
            try (PrintWriter p = new PrintWriter(new FileWriter(g.getOutputFile(), true))) {
                p.println("Simulation 0% complete. Elapsed time: " + IOHelp.formatTime(startTime, startTime));
            } catch (IOException ioe) {
                lg.warn("Could not create initial log file: " + g.getOutputFile(), ioe);
            }
        }

        /*
         * Meaningful simulation times, data points, image points, progress updates / logging
         */
        double nextRealTime = totalTime/100;
        int percentComplete = 0;
        double nextDataTime = dtdata;
        double nextImageTime = dtimage;
        int relaxationSteps = (int)(dt/dtspring);

        long totalSteps = (long)(totalTime / dt);

        // ------------------------------------------------------------------------------

        // GET THE DATA AT THE ZERO TIME POINT
        writePositions();
        moleculeCounter.countMolecules();
        // moleculeCounter.writePartialData(dataFolder);
        stateCounter.countStates();
        // stateCounter.writePartialData(dataFolder);
        bondCounter.countBonds();
        // bondCounter.writePartialData(dataFolder);
        if(countingClusters){
            clusterCounter.countClusters();
            clusterCounter.writeClusters(dataFolder);
        }
        sitePropertyCounter.countProperties();
//        locationTracker.initializeMaps();
//        locationTracker.trackPositions();

        lg.info("Number of steps: " + totalSteps);
        lg.info("Spring relaxation steps: " + relaxationSteps);

        // We go ever so slightly over the last time point to make sure we get data at the last time point.
        while(time < totalTime + dt){
            // lg.info("Simulation time: " + time + " of " + totalTime);    // this is too much output

            // ----- Look to see if we should output data
            if(time >= nextDataTime){
                moleculeCounter.countMolecules();
                stateCounter.countStates();
                bondCounter.countBonds();
                if(countingClusters){
                    clusterCounter.countClusters();
                    clusterCounter.writeClusters(dataFolder);
                }
                sitePropertyCounter.countProperties();
                reactionCounter.initDatapoint();    // we update in real time as reactions happen
                nextDataTime += dtdata;
            }

            // ----- Look to see if we should output an image
            if(time >= nextImageTime){
                writePositions();
                nextImageTime += dtimage;
            }

            // ----- Look to see if we should give the user an update
            if(time >= nextRealTime){
                long now = System.currentTimeMillis();
                percentComplete++;
                if(useOutputFile){
                    try(PrintWriter p = new PrintWriter(new FileWriter(g.getOutputFile(), true), true)){
                        p.println("Simulation " + percentComplete + "% complete. Elapsed time: " + IOHelp.formatTime(startTime, now));
                    } catch (IOException ioe) {
                        lg.warn("Could not write progress update to output file: " + g.getOutputFile(), ioe);
                    }
                } else {
                    lg.info("Simulation " + percentComplete + "% complete. Elapsed time: " + IOHelp.formatTime(startTime, now));
                }

                nextRealTime += (totalTime/100.0);
            }
            vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(time/(totalTime + dt), time), VCellMessaging.ThrowOnException.NO); // progress message are throttled by vcellMessaging

            // the expensive part of the simulation, update the system and relax the springs
            long iterStart = System.nanoTime();
            time += dt;
            update();
            if(relaxationSteps > 2){
                relaxSprings(relaxationSteps);
            }
            long iterEnd = System.nanoTime();
            long iterDuration = iterEnd - iterStart;

            // gather the special statistics for multiple runs, compute them only inside run 1
            if(runCounter == 1 && !etaTracker.isDisabled()) {
                etaTracker.updateIterationStats(iterDuration);
                etaTracker.maybeLog(System.currentTimeMillis(), totalSteps, lg);
            }

        }   // end of main simulation while loop

        stopTime = System.currentTimeMillis();
        try(PrintWriter pw = new PrintWriter(new FileWriter(new File(dataFolder, "RunningTime.txt")))){
            pw.println("Running Time: " + IOHelp.formatTime(startTime, stopTime));
        } catch (IOException e) {
            lg.error("Failed to write file 'RunningTime.txt' in: " + dataFolder.getAbsolutePath(), e);
        }

        // TODO: Write last ETA snapshot and real duration to file that will be used by watchdog
        //  use etaTracker.getLastSnapshot() and etaTracker.getTotalDuration()

        lg.info("Simulation finished. Writing more data.");
        this.writeMoleculeIDs();
        this.writeSiteIDs();
        // Write the data file
        moleculeCounter.writeFullData(dataFolder);
        // FileHelp.deleteAllFilesStartWith(dataFolder, "Mol_Counts_At_Time_");

        stateCounter.writeFullData(dataFolder);
        // FileHelp.deleteAllFilesStartWith(dataFolder, "State_Counts_At_Time_");

        bondCounter.writeFullData(dataFolder);
        // FileHelp.deleteAllFilesStartWith(dataFolder, "Bond_Counts_At_Time_");

        sitePropertyCounter.writeData(dataFolder);
//        locationTracker.writeData(dataFolder);

        reactionCounter.writeFullData(dataFolder);

        lg.info("Finished writing data.");
        // postprocess data
        // get file extension from inputFile
        String inputFileExtension = inputFile.getName().substring(inputFile.getName().lastIndexOf("."));

        File idaFile;
        File clustersFile;
        if(getRunCounter() == 0) {
            // we allow the ida file for run 0 to stay without run count suffix
            idaFile = new File(inputFile.getParentFile(), inputFile.getName().replace(inputFileExtension, IdaFileExtension));
            clustersFile = new File(inputFile.getParentFile(), inputFile.getName().replace(inputFileExtension, ClustersFileExtension));
        } else {
            // for run counts > 0 we add run count suffix
            String parentDirectory = inputFile.getParent();
            String fileNameWithoutExtension = inputFile.getName().split("\\.")[0];
            String newIdaFileName = fileNameWithoutExtension + "_" + getRunCounter() + IdaFileExtension;
            idaFile = new File(parentDirectory, newIdaFileName);
            String newClustersFileName = fileNameWithoutExtension + "_" + getRunCounter() + ClustersFileExtension;
            clustersFile = new File(parentDirectory, newClustersFileName);
        }
        LangevinPostprocessor.writeIdaFile(dataFolder.toPath(),idaFile.toPath());
        LangevinPostprocessor.writeClustersFile(dataFolder.toPath(),clustersFile.toPath());
        vcellMessaging.sendWorkerEvent(WorkerEvent.dataEvent(1.0, time), VCellMessaging.ThrowOnException.NO);
        vcellMessaging.sendWorkerEvent(WorkerEvent.progressEvent(1.0, time), VCellMessaging.ThrowOnException.NO);
        vcellMessaging.sendWorkerEvent(WorkerEvent.completedEvent(time), VCellMessaging.ThrowOnException.NO);
        // </editor-fold>
    }

    /**********************************************************************\
     *                       FILE IO METHODS                              *
     \**********************************************************************/

    private void writeViewerFileHeader(){
        if (viewerFile == null){
            return;
        }
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        try(PrintWriter p = new PrintWriter(new FileWriter(viewerFile), true)) {
            p.print("TotalTime\t" + totalTime + "\n");
            p.print("dtimage\t" + dtimage + "\n");
            p.print("xsize\t" + xmax + "\n");
            p.print("ysize\t" + ymax + "\n");
            p.print("z_outside\t" + (-zmin) + "\n");
            p.print("z_inside\t" + zmax + "\n\n");
        } catch (IOException e) {
            lg.error("Failed to write viewer header: " + viewerFile.getAbsolutePath(), e);
        }
        // </editor-fold>
    }

    private void writePositions(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        if(viewerFile == null){
            return;
        }
        try(PrintWriter p = new PrintWriter(new FileWriter(viewerFile, true), true)) {
            p.print("SCENE\n");
            p.print("SceneNumber\t" + imageCounter + "\tCurrentTime"
                    + "\t" + time + "\n");
            for (Site s : sites) {
                p.print("ID\t" + s.getID() + "\t" + s.getRadius() + "\t"
                        + s.getColor() + "\t"
                        + IOHelp.DF[6].format(s.getX()) + "\t"
                        + IOHelp.DF[6].format(s.getY()) + "\t"
                        + IOHelp.DF[6].format(s.getZ()) + "\n");
            }
            for (Link l : links) {
                p.print("Link\t" + l.getSite(0).getID() + "\t:\t"
                        + l.getSite(1).getID() + "\n");
            }
            for (Bond b : bonds) {
                p.print("Link\t" + b.getSite(0).getID() + "\t:\t"
                        + b.getSite(1).getID() + "\n");
            }
            p.print("\n");
        } catch (IOException e) {
            lg.error("Failed to write positions in: " + viewerFile.getAbsolutePath(), e);
        }
        imageCounter++;
        // </editor-fold>
    }

    private void writeMoleculeIDs(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        File file = new File(dataFolder, "MoleculeIDs.csv");
        try(PrintWriter p = new PrintWriter(new FileWriter(file), true)){
            for(String idString : moleculeIDs){
                p.println(idString);
            }
        } catch (IOException ioe) {
            lg.error("Failed to write MoleculeIDs.csv in folder: " + dataFolder.getAbsolutePath(), ioe);
        }
        // </editor-fold>
    }

    private void writeSiteIDs(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        File file = new File(dataFolder, "SiteIDs.csv");
        try(PrintWriter p = new PrintWriter(new FileWriter(file), true)){
            for(String idString : siteIDs){
                p.println(idString);
            }
        }catch(IOException ioe){
            lg.error("Failed to write SiteIDs.csv in folder: " + dataFolder.getAbsolutePath(), ioe);
        }
        // </editor-fold>
    }
}
