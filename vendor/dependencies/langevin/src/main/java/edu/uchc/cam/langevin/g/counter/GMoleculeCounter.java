/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.counter;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.io.PrintWriter;
import java.util.Scanner;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.langevinnovis01.Global;


public class GMoleculeCounter {

    private final GMolecule gmolecule;
    private boolean countTotal;
    private boolean countFree;
    private boolean countBound;
    
    public static final String FREE = "Free";
    public static final String BOUND = "Bound";
    public static final String TOTAL = "Total";
    public static final String NONE = "None";
    
    public GMoleculeCounter(GMolecule gmolecule){
        this.gmolecule = gmolecule;
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
    
    public String getGMoleculeName(){
        return gmolecule.getName();
    }
    
    public int getGMoleculeID(){
        return gmolecule.getID();
    }
    
    /* ************** WRITE COUNTER ******************************/
    
    public void writeMoleculeCounter(PrintWriter p){
        StringBuilder sb = new StringBuilder();
        sb.append("'").append(gmolecule.getName()).append("'");
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
    
    /* ************ SET SINGLE COUNTER **************************/
    
    public void loadCounter(Scanner dataScanner){
        // Skip the word "measure"
        dataScanner.next();
        while(dataScanner.hasNext()){
            this.setMeasurement(dataScanner.next(), true);
        }
    }
    
    /* *********** LOAD ALL MOLECULE COUNTERS ********************/
    
    public static void loadCounters(Global g, Scanner dataScanner){
        while(dataScanner.hasNextLine()){
            String [] next = dataScanner.nextLine().split(":");
            GMolecule mol = g.getGMolecule(IOHelp.getNameInQuotes(new Scanner(next[0])));
            GMoleculeCounter counter = mol.getGMoleculeCounter();
            counter.loadCounter(new Scanner(next[1].trim()));
        }
    }
}
