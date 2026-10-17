
package edu.uchc.cam.langevin.g.reaction;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.util.Scanner;
import java.util.ArrayList;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GSite;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GAllostericReaction implements GReactionInterface {

    public static final Logger lg = LogManager.getLogger(GAllostericReaction.class);

    private String name;
    
    private GMolecule molecule;
    private GSite site;
    private GState initialState;
    private GState finalState;
    
    // Allosteric site
    private GSite allostericSite;
    private GState allostericState;
    
    // Each allosteric reaction has a single rate
    private double rate;  // Units s-1
    
    public GAllostericReaction(){
        name = null;
        molecule = null;
        site = null;
        initialState = null;
        finalState = null;
        allostericSite = null;
        allostericState = null;
        rate = 0;
    }
    
    /* ********* GET AND SET THE REACTION NAME ***************/
    
    public void setName(String name){
        this.name = name;
    }

    @Override
    public String getName(){
        return name;
    }
    
    @Override
    public String toString(){
        return name;
    }
    
    /* *********   GET AND SET THE MOLECULE *****************/
    
    public void setGMolecule(GMolecule molecule){
        this.molecule = molecule;
    }
    
    public GMolecule getGMolecule(){
        return molecule;
    }
    
    /* ********** GET AND SET THE SITE *********************/
    
    public void setGSite(GSite site){
        this.site = site;
    }
    
    public GSite getGSite(){
        return site;
    }
    
    /* ********* GET AND SET THE INITIAL AND FINAL STATES ***********/
    
    public void setInitialState(GState state){
        initialState = state;
    }
    
    public void setFinalState(GState state){
        finalState = state;
    }
    
    public GState getInitialState(){
        return initialState;
    }
    
    public GState getFinalState(){
        return finalState;
    }
    
    /* ********** GET AND SET THE ALLOSTERIC SITE AND STATE ***********/
    
    public void setAllostericSite(GSite site){
        this.allostericSite = site;
    }
    
    public void setAllostericState(GState state){
        this.allostericState = state;
    }
    
    public GSite getAllostericSite(){
        return allostericSite;
    }
    
    public GState getAllostericState(){
        return allostericState;
    }
    
    /* ************ GET AND SET THE REACTION RATE **********************/
    
    public void setRate(double rate){
        this.rate = rate;
    }
    
    public double getRate(){
        return rate;
    }
    
    /* ***************** PRINT A REACTION REPRESENTATION ******************/
    
    public String writeReaction(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        StringBuilder sb = new StringBuilder();
        sb.append("'").append(name).append("' ::     ");
        if(molecule != null && site != null && initialState != null && finalState != null){
            sb.append("'").append(molecule.getName()).append("' : ");
            sb.append("Site ").append(site.getIndex()).append(" : '");
            sb.append(initialState.getStateName()).append("'");
            sb.append(" --> ");
            sb.append("'").append(finalState.getStateName()).append("' ");
            sb.append(" Rate ").append(Double.toString(rate));
            sb.append(" Allosteric_Site ").append(allostericSite.getIndex());
            sb.append(" State '").append(allostericState.getStateName()).append("'");
        }
        return sb.toString();
        // </editor-fold>
    }
    
    /* **************** LOAD SINGLE REACTION ****************************/
    
    public void loadReaction(Global g, Scanner sc){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        name = IOHelp.getNameInQuotes(sc);
        sc.next();
        molecule = g.getGMolecule(IOHelp.getNameInQuotes(sc));
        sc.next();
        sc.next();
        site = molecule.getSite(sc.nextInt());
        sc.next();
        GSiteType type = site.getType();
        initialState = type.getState(IOHelp.getNameInQuotes(sc));
        sc.next();
        finalState = type.getState(IOHelp.getNameInQuotes(sc));
        sc.next();
        rate = sc.nextDouble();
        sc.next();
        allostericSite = molecule.getSite(sc.nextInt());
        sc.next();
        GSiteType alloType = allostericSite.getType();
        allostericState = alloType.getState(IOHelp.getNameInQuotes(sc));
        // </editor-fold>
    }
    
    /* *************** LOAD FULL ARRAY ***********************************/
    
    public static ArrayList<GAllostericReaction> loadReactions(Global g, Scanner sc){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        ArrayList<GAllostericReaction> allostericReactions = new ArrayList<>();
        GAllostericReaction reaction;
        while(sc.hasNextLine()){
            reaction= new GAllostericReaction();
            reaction.loadReaction(g, new Scanner(sc.nextLine()));
            allostericReactions.add(reaction);
        }
        return allostericReactions;
        // </editor-fold>
    }
    
}
