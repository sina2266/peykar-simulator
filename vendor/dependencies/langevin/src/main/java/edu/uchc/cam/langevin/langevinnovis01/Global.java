/**
 * This class store all of the information that we read in from the input 
 * file.  This class will not have any "set" methods because none of the
 * variables should ever be modified after initial setup.  I will make the
 * variables private.  Static fields would work, but I always think that code
 * looks cleaner when you're not using static fields all over the place.
 */

package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.g.counter.GStateCounter;
import edu.uchc.cam.langevin.g.counter.GSitePropertyCounter;
import edu.uchc.cam.langevin.g.counter.GBondCounter;
import edu.uchc.cam.langevin.g.counter.GMoleculeCounter;
import edu.uchc.cam.langevin.g.object.GSite;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.g.reaction.GAllostericReaction;
import edu.uchc.cam.langevin.g.reaction.GBindingReaction;
import edu.uchc.cam.langevin.g.reaction.GTransitionReaction;
import edu.uchc.cam.langevin.g.reaction.GDecayReaction;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.math.BigInteger;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.HashMap;
import java.io.*;

public class Global {

    public static final Logger lg = LogManager.getLogger(Global.class);

    /* ********** Strings to represent the different object categories ****/
    public final static String SPATIAL_INFORMATION = "SYSTEM INFORMATION";
    public final static String TIME_INFORMATION = "TIME INFORMATION";
    public final static String MOLECULES = "MOLECULES";
    public final static String DECAY_REACTIONS = "CREATION/DECAY REACTIONS";
    public final static String TRANSITION_REACTIONS = "STATE TRANSITION REACTIONS";
    public final static String ALLOSTERIC_REACTIONS = "ALLOSTERIC REACTIONS";
    public final static String BINDING_REACTIONS = "BIMOLECULAR BINDING REACTIONS";
    public final static String MOLECULE_COUNTERS = "MOLECULE COUNTERS";
    public final static String STATE_COUNTERS = "STATE COUNTERS";
    public final static String BOND_COUNTERS = "BOND COUNTERS";
    public final static String SITE_PROPERTY_COUNTERS = "SITE PROPERTY COUNTERS";
    public final static String CLUSTER_COUNTERS = "CLUSTER COUNTERS";
    public final static String SIMULATION_OPTIONS = "SIMULATION OPTIONS";

    public final static String RandomSeed = "RandomSeed";

    // The time data
    private final GSystemTimes systemTimes;
    
    // System size info.  
    private final GBoxGeometry boxGeometry;
    
    // The types of molecules in the sytstem
    private ArrayList<GMolecule> molecules = new ArrayList<>();
    
    // Binding reactions
    private ArrayList<GBindingReaction> bindingReactions = new ArrayList<>();
    // Transition Reactions
    private ArrayList<GTransitionReaction> transitionReactions = new ArrayList<>();
    // Allosteric reactions
    private ArrayList<GAllostericReaction> allostericReactions = new ArrayList<>();
    
    // HashMaps to link IDs to the names of molecules, types, and states.
    
    private final HashMap<Integer, String> moleculeNames = new HashMap<>();
    private final HashMap<Integer, String> typeNames = new HashMap<>();
    private final HashMap<Integer, String> stateNames = new HashMap<>();
    
    /**
     * This is boolean to tell us if file reading was successful.  I'm making it
     * a global so that several methods have access to its value. For example,
     * in the checkValue methods, we don't want to output additional error 
     * messages if we've already found that one of the inputs was bad.
     */
    private File defaultFolder;
    private final File inputFile;
    private final File outputFile;  // The file we write the updates to.
    
    private boolean countClusters;
    private BigInteger startSeed = null;
    
    public Global(File inFile) throws IOException {
        
        systemTimes = new GSystemTimes();
        boxGeometry = new GBoxGeometry();

        Path f2 = Paths.get(inFile.getAbsolutePath());
        Path f3 = f2.getParent();
        defaultFolder = f3.toFile();

        inputFile = inFile;
        outputFile = null;
        
        // Default
        countClusters = false;
        try {
            readFile(inputFile.toString());
        } catch (IOException e) {
            throw new IOException("Failed to read input file: " + inputFile.getAbsolutePath(), e);
        }
    }
    
    public Global(File inFile, File outFile) throws IOException {

        systemTimes = new GSystemTimes();
        boxGeometry = new GBoxGeometry();
        
        defaultFolder = new File(inFile.getParent());

        this.inputFile = inFile;
        this.outputFile = outFile;

        try{
            readFile(inputFile.toString());
            
        } catch(IOException e){
            throw new IOException("Failed to read input file: " + inputFile.getAbsolutePath(), e);
        } 
    }
    
    /* *************** GET THE OUTPUT FILE *******************************/
    public File getOutputFile(){
        return outputFile;
    }
    
    /* ************* COUNT CLUSTERS BOOLEAN *******************************/
    public boolean isCountingClusters(){
        return countClusters;
    }

    /* ************* START SEED BIG INTEGER *******************************/
    public BigInteger getStartSeed() {
        return startSeed;
    }
    
    /* *************** GET THE SYTEM TIMES *********************************/
    
    public double getTotalTime(){
        return systemTimes.getTotalTime();
    }
    
    public double getdt(){
        return systemTimes.getdt();
    }
    
    public double getdtspring(){
        return systemTimes.getdtspring();
    }
    
    public double getdtdata(){
        return systemTimes.getdtdata();
    }
    
    public double getdtimage(){
        return systemTimes.getdtimage();
    }
    
    /* ************* GET THE SYSTEM SIZE **********************************/
    
    public double getXsize(){
        return boxGeometry.getX();
    }
    
    public double getYsize(){
        return boxGeometry.getY();
    }
    
    public double getZin(){
        return boxGeometry.getZin();
    }
 
    public double getZout(){
        return boxGeometry.getZout();
    }
    
    public double getZtot(){
        return getZout() + getZin();
    }
    
    public double getVin(){
        return getXsize()*getYsize()*getZin();
    }
    
    public double getVout(){
        return getXsize()*getYsize()*getZout();
    }
    
    public double getVtot(){
        return getXsize()*getYsize()*(getZout() + getZin());
    }
    
    /* ************   GET PARTITION NUMBER **************************/
    
    public int getNpartx(){
        return boxGeometry.getNpart(0);
    }
    public int getNparty(){
        return boxGeometry.getNpart(1);
    }
    public int getNpartz(){
        return boxGeometry.getNpart(2);
    }
    
    /* ************** GET MOLECULE AND REACTION ARRAYS ***************/
    
    public ArrayList<GMolecule> getMolecules(){
        return molecules;
    }
    
    public ArrayList<GBindingReaction> getBindingReactions(){
        return bindingReactions;
    }
    
    public ArrayList<GTransitionReaction> getTransitionReactions(){
        return transitionReactions;
    }
    
    public ArrayList<GAllostericReaction> getAllostericReactions(){
        return allostericReactions;
    }
    
    public ArrayList<GDecayReaction> getDecayReactions(){
        ArrayList<GDecayReaction> reactions = new ArrayList<>();
        for(GMolecule molecule : molecules){
            reactions.add(molecule.getGDecayReaction());
        }
        return reactions;
    }
    
    public File getInputFile(){
        return inputFile;
    }

    
    /* ******************************************************************\
     *                   METHODS TO HANDLE NAME HASHMAPS                *
    \********************************************************************/
    
    public String getMoleculeName(int id){
        return moleculeNames.get(id);
    }
    
    public String getTypeName(int id){
        return typeNames.get(id);
    }
    
    public String getStateName(int id){
        return stateNames.get(id);
    }
    
    public void putMoleculeName(int id, String name){
        moleculeNames.put(id, name);
    }
    
    public void putTypeName(int id, String name){
        typeNames.put(id, name);
    }
    
    public void putStateName(int id, String name){
        stateNames.put(id, name);
    }
    
    private void assignMoleculeIDs(){
        int moleculeID = 1000;
        for(GMolecule gmolecule : molecules){
            gmolecule.setID(moleculeID);
            this.putMoleculeName(moleculeID, gmolecule.getName());
            gmolecule.assignTypeandStateIDs();
            ArrayList<GSiteType> ttypes = gmolecule.getTypeArray();
            for (GSiteType ttype : ttypes) {
                this.putTypeName(ttype.getID(), ttype.getName());
                ArrayList<GState> tstates = ttype.getStates();
                    for (GState tstate : tstates) {
                        this.putStateName(tstate.getID(), tstate.getStateName());
                    }
            }
            moleculeID++;
            if (moleculeID > 4000) {
                throw new IllegalStateException("Molecule ID exceeds supported range (max 3000). Received moleculeID = " + moleculeID);
            }
        }
    }
    
    /******************************************************************\
     *             METHOD TO GET MOLECULE REFERENCE BY NAME           *
     * @param name                                                    *
     * @return                                                        *
    \******************************************************************/
    
    public GMolecule getGMolecule(String name){
        GMolecule gmol = null;
        for (GMolecule molecule : molecules) {
            if (molecule.getName().equals(name)) {
                gmol = molecule;
                break;
            }
        }
        return gmol;
    }
    
    public GBindingReaction getGBindingReaction(String name){
        GBindingReaction brxn = null;
        for (GBindingReaction bindingReaction : bindingReactions) {
            if (bindingReaction.getName().equals(name)) {
                brxn = bindingReaction;
                break;
            }
        }
        return brxn;
    }
    
    public GTransitionReaction getGTransitionReaction(String name){
        GTransitionReaction trxn = null;
        for(GTransitionReaction transitionReaction : transitionReactions){
            if(transitionReaction.getName().equals(name)){
                trxn = transitionReaction;
                break;
            }
        }
        return trxn;
    }
    
    public GAllostericReaction getAllostericReaction(String name){
        GAllostericReaction arxn = null;
        for(GAllostericReaction reaction : allostericReactions){
            if(reaction.getName().equals(name)){
                arxn = reaction;
                break;
            }
        }
        return arxn;
    }
    
    
    /******************************************************************\
     *                       READ FILE METHOD                         *
     * @param filename                                                *
     * @return                                                        *
     * @throws IOException                                            *
    \******************************************************************/
    
    private void readFile(String filename) throws IOException{
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        File file = new File(filename);

        BufferedReader br = null;
        try{
            br = new BufferedReader(new FileReader(file));
        } catch (FileNotFoundException e){
            throw new RuntimeException("file " + filename + " not found");
        }
        
        Scanner sc = new Scanner(br);
        sc.useDelimiter("\\*\\*\\*");
        
        while(sc.hasNext()){
        String next = sc.next().trim();
            switch(next){
                case TIME_INFORMATION:
                    systemTimes.loadData(sc.next().trim());
                    break;
                case SPATIAL_INFORMATION:
                    boxGeometry.loadData(sc.next().trim());
                    break;
                case MOLECULES:
                    molecules = GMolecule.loadMolecules(sc.next().trim());
                    assignMoleculeIDs();
                    break;
                case DECAY_REACTIONS:
                    GDecayReaction.loadReactions(this, new Scanner(sc.next().trim()));
                    break;
                case TRANSITION_REACTIONS:
                    transitionReactions = GTransitionReaction.loadReactions(this, new Scanner(sc.next().trim()));
                    break;
                case ALLOSTERIC_REACTIONS:
                    allostericReactions = GAllostericReaction.loadReactions(this, new Scanner(sc.next().trim()));
                    break;
                case BINDING_REACTIONS:
                    bindingReactions = GBindingReaction.loadReactions(this, new Scanner(sc.next().trim()));
                    break;
                case MOLECULE_COUNTERS:
                    GMoleculeCounter.loadCounters(this, new Scanner(sc.next().trim()));
                    break;
                case STATE_COUNTERS:
                    GStateCounter.loadCounters(this, new Scanner(sc.next().trim()));
                    break;
                case BOND_COUNTERS:
                    GBondCounter.loadCounters(this, new Scanner(sc.next().trim()));
                    break;
                case SITE_PROPERTY_COUNTERS:
                    GSitePropertyCounter.loadCounters(this, new Scanner(sc.next().trim()));
                    break;
                case CLUSTER_COUNTERS: {
                    Scanner scanner = new Scanner(sc.next().trim());
                    scanner.next();
                    countClusters = scanner.nextBoolean();
                    break;
                }
                case SIMULATION_OPTIONS: {
                    Scanner scanner = new Scanner(sc.next().trim());
                    while(scanner.hasNextLine()) {
                        String [] innerNext = scanner.nextLine().split(":");
                        switch(innerNext[0]) {
                            case RandomSeed:
                                String randomSeed = innerNext[1].trim();
                                try {
                                    startSeed = new BigInteger(randomSeed);
                                } catch(NumberFormatException ex) {
                                    throw new RuntimeException("Invalid RandomSeed: " + randomSeed);
                                }
                                break;
                            default:
                                throw new IllegalArgumentException("SimulationOptions.loadData received unexpected input line: " +
                                                innerNext[0] + " : " + innerNext[1]);
                        }
                    }
                }
            }
            
        }
        // </editor-fold>
    }

    /* ***************************************************************\
     *                 WRITE SYSTEM DATA TO FILE                     *
     *  This copies the method from Langevin setup global, but here  *
     *  I just use it for testing to make sure that the input file   * 
     *  is being read correctly.  I should get a file out identical  *
     *  to the one I read in. 
    \*****************************************************************/
    
    
    public void writeData(String filename){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        try(PrintWriter p = new PrintWriter(new FileWriter(new File(defaultFolder.getAbsolutePath(), filename)), true)){
            /* ********* BEGIN BY WRITING THE TIMES *********/
            p.println("*** " + TIME_INFORMATION + " ***");
            systemTimes.writeData(p);
            p.println();
            
            /* ********* WRITE THE SPATIAL INFORMATION **********/
            p.println("*** " + SPATIAL_INFORMATION + " ***");
            boxGeometry.writeData(p);
            p.println();
            
            /* ******* WRITE THE SPECIES INFORMATION ***********/
            p.println("*** " + MOLECULES + " ***");
            p.println();
            for(GMolecule molecule : molecules){
                molecule.writeMolecule(p);
            }
            
            /* ******* WRITE THE DECAY REACTIONS ***************/
            p.println("*** " + DECAY_REACTIONS + " ***");
            p.println();
            for(GMolecule molecule : molecules){
                molecule.getGDecayReaction().writeReaction(p);
            }
            p.println();
            
            /* ******* WRITE THE TRANSITION REACTIONS **********/
            p.println("*** " + TRANSITION_REACTIONS + " ***");
            p.println();
            for(GTransitionReaction reaction : transitionReactions){
                p.println(reaction.writeReaction());
            }
            p.println();
            
            /* ******* WRITE THE ALLOSTERIC REACTIONS **********/
            p.println("*** " + ALLOSTERIC_REACTIONS + " ***");
            p.println();
            for(GAllostericReaction reaction : allostericReactions){
                p.println(reaction.writeReaction());
            }
            p.println();
            
            /* ******* WRITE THE BINDING REACTIONS ************/
            p.println("*** " + BINDING_REACTIONS + " ***");
            p.println();
            for(GBindingReaction reaction : bindingReactions){
                p.println(reaction.writeReaction());
            }
            p.println();
            
            /* ****** WRITE THE MOLECULE COUNTERS **********/
            p.println("*** " + MOLECULE_COUNTERS + " ***");
            p.println();
            for(GMolecule molecule: molecules){
                molecule.getGMoleculeCounter().writeMoleculeCounter(p);
            }
            p.println();
            
            /* ******  WRITE THE STATE COUNTERS *************/
            p.println("*** " + STATE_COUNTERS + " ***");
            p.println();
            for(GMolecule molecule : molecules){
                for(GSiteType type : molecule.getTypeArray()){
                    for(GState state : type.getStates()){
                        state.getGStateCounter().writeStateCounter(p);
                    }
                }
            }
            p.println();
            
            /* ***** WRITE THE BOND COUNTERS ***************/
            p.println("*** " + BOND_COUNTERS + " ***");
            p.println();
            for(GBindingReaction reaction: bindingReactions){
                reaction.getBondCounter().writeBondCounter(p);
            }
            p.println();
            
            /* ******** WRITE THE SITE PROPERTY COUNTERS **********/
            p.println("*** " + SITE_PROPERTY_COUNTERS + " ***");
            p.println();
            for(GMolecule gmolecule : molecules){
                ArrayList<GSite> gsites = gmolecule.getSites();
                for(GSite gsite : gsites){
                    gsite.getPropertyCounter().writeSitePropertyCounter(p);
                }
            }
            
        } catch(IOException ioe){
            ioe.printStackTrace(System.out);
        }
        // </editor-fold>
    }
    

}
