/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.counter;

import edu.uchc.cam.langevin.g.object.GSite;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.io.PrintWriter;
import java.util.Scanner;
import edu.uchc.cam.langevin.langevinnovis01.Global;

public class GSitePropertyCounter {

    private final GSite site;
    
    private boolean trackData;
    
    public GSitePropertyCounter(GSite site){
        this.site = site;
    }
    
    /* **********  SET THE TRACKDATA FIELD ***************/
    
    public void setTracked(boolean bool){
        trackData = bool;
    }
    
    public boolean isTracked(){
        return trackData;
    }
    
    /* ********** WRITE COUNTER *************************/
    // For testing
    public void writeSitePropertyCounter(PrintWriter p){
        StringBuilder sb = new StringBuilder();
        sb.append("'").append(site.getMoleculeName()).append("' Site ").append(site.getIndex());
        sb.append(" :  Track Properties ").append(trackData);
        p.println(sb.toString());
    }
    
    /* *********** LOAD COUNTERS *************************/
    /* ***************  LOAD ALL COUNTERS *****************/
    // Since there is only a single data field to read in, it doesn't make 
    // sense to define a method to load a single counter.
    public static void loadCounters(Global g, Scanner dataScanner){
        while(dataScanner.hasNextLine()){
            Scanner sc = new Scanner(dataScanner.nextLine());
            GMolecule molecule = g.getGMolecule(IOHelp.getNameInQuotes(sc));
            // Skip "Site"
            sc.next();
            int index = sc.nextInt();
            GSite mSite = molecule.getSite(index);
            GSitePropertyCounter propertyCounter = mSite.getPropertyCounter();
            // Skip ":"
            sc.next();
            // Skip "Track"
            sc.next();
            // Skip "Properties"
            sc.next();
            // Read in the boolean
            propertyCounter.setTracked(sc.nextBoolean());
        }
    }
    
}
