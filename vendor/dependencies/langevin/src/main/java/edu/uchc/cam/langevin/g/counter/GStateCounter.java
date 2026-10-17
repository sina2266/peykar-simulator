/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.counter;

import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.io.PrintWriter;
import java.util.Scanner;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;


public class GStateCounter {

    public static final Logger lg = LogManager.getLogger(GStateCounter.class);

    private final GState gstate;
    private boolean countTotal;
    private boolean countFree;
    private boolean countBound;
    
    public static final String FREE = "Free";
    public static final String BOUND = "Bound";
    public static final String TOTAL = "Total";
    public static final String NONE = "None";
    
    public GStateCounter(GState state){
        this.gstate = state;
        // System.out.println("Making GMoleculeCounter for " + gmolecule.getName() + ".");
        countTotal = false;
        countFree = false;
        countBound = false;
    }
    
    // SET METHODS
    
    public void setMeasurement(String type, boolean bool){
        switch(type){
            case FREE:{
                countFree = bool;
                break;
            }
            case BOUND:{
                countBound = bool;
                break;
            }
            case TOTAL:{
                countTotal = bool;
                break;
            }
            case NONE:{
                countFree = false;
                countBound = false;
                countTotal = false;
                break;
            }
            default:{
                System.out.println("CountData setMeasurement() received the following unexpected input: " + type);
            }
        }
    }
    
    // GET METHODS
    
    public boolean countTotal(){
        return countTotal;
    }
    public boolean countFree(){
        return countFree;
    }
    public boolean countBound(){
        return countBound;
    }
    
    public String getGStateName(){
        return gstate.getStateName();
    }
    
    public int getGStateID(){
        return gstate.getID();
    }
    
    /* ******** LOAD SINGLE STATE COUNTER ******************************/
    
    public void loadCounter(Scanner sc){
        sc.next();
        while(sc.hasNext()){
            this.setMeasurement(sc.next(), true);
        }
    }
    
    /* ******* LOAD ALL STATE COUNTERS **********************************/
    
    public static void loadCounters(Global g, Scanner dataScanner){
        while(dataScanner.hasNextLine()){
            Scanner sc = new Scanner(dataScanner.nextLine());
            GMolecule mol = g.getGMolecule(IOHelp.getNameInQuotes(sc));
            sc.next();
            GSiteType type = mol.getType(IOHelp.getNameInQuotes(sc));
            sc.next();
            GState state = type.getState(IOHelp.getNameInQuotes(sc));
            sc.next();
            state.getGStateCounter().loadCounter(sc);
        }
    }
    
    /* ********  WRITE STATE COUNTER ***********************************/

    public void writeStateCounter(PrintWriter p){
        StringBuilder sb = new StringBuilder();
        sb.append("'").append(gstate.getMoleculeName()).append("' : '");
        sb.append(gstate.getSiteTypeName()).append("' : '");
        sb.append(gstate.getStateName()).append("'");
        sb.append(" : Measure ");
        if(!countBound && !countFree && !countTotal){
            sb.append(NONE).append(" ");
        } else {
            if(countTotal){
                sb.append(TOTAL).append(" ");
            }
            if(countFree){
                sb.append(FREE).append(" ");
            }
            if(countBound){
                sb.append(BOUND).append(" ");
            }
        }
        p.println(sb.toString());
    }
    
}
