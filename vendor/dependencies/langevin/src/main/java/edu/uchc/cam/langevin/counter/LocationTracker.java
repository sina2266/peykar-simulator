/**
 * For now, only use this class when there are no creation/decay reactions.
 */

package edu.uchc.cam.langevin.counter;

import edu.uchc.cam.langevin.g.object.GMolecule;

import java.io.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Site;

public class LocationTracker {
    
    private final double DOES_NOT_EXIST = Double.NaN;

    private final MySystem sys;
    private final double [] time;
    // Create hashmap between site ids and a list storing the position of that site at a given time
    private final HashMap<Integer, double[]> xpos;
    private final HashMap<Integer, double[]> ypos;
    private final HashMap<Integer, double[]> zpos;
    
    // Save all of the site ids this class sees, so we can print them later.
    private final ArrayList<Integer> siteIDs = new ArrayList<>();
   
    private int counter =0;
    private final int totalCount;
    
    public LocationTracker(Global g, MySystem sys){
        this.sys = sys;
        totalCount = 1 + (int)Math.floor(g.getTotalTime()/g.getdtdata());
        time = new double[totalCount];
        
        int totalSites = 0;
        for(GMolecule gmolecule : g.getMolecules()){
            totalSites += gmolecule.siteNumber() * gmolecule.getNumber();
        }
        
        xpos = new HashMap<>(5*totalSites);
        ypos = new HashMap<>(5*totalSites);
        zpos = new HashMap<>(5*totalSites);
    }
    
    public void initializeMaps(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        double [] x;
        double [] y;
        double [] z;
        for(Site site : sys.getSites()){
            x = new double[totalCount];
            y = new double[totalCount];
            z = new double[totalCount];
            
            // Initialize array with NaN so that we can record if the site decays
            for(int i=0;i<totalCount;i++){
                x[i] = DOES_NOT_EXIST;
                y[i] = DOES_NOT_EXIST;
                z[i] = DOES_NOT_EXIST;
            }
            
            int id = site.getID();
            siteIDs.add(id);
            xpos.put(id, x);
            ypos.put(id, y);
            zpos.put(id, z);
        }
        // </editor-fold>
    }
    
    public void trackPositions(){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        if(counter < time.length){
            time[counter] = sys.getTime();
            
            for(Site site : sys.getSites()){
                double [] xtest = xpos.get(site.getID());
                if(xtest != null){
                    xpos.get(site.getID())[counter] = site.getX();
                    ypos.get(site.getID())[counter] = site.getY();
                    zpos.get(site.getID())[counter] = site.getZ();
                } else {
                    double [] x = new double[totalCount];
                    double [] y = new double[totalCount];
                    double [] z = new double[totalCount];
                    
                    // Initialize array with NaN so that we can record if the site decays
                    for(int i=0;i<totalCount;i++){
                        x[i] = DOES_NOT_EXIST;
                        y[i] = DOES_NOT_EXIST;
                        z[i] = DOES_NOT_EXIST;
                    }
                    
                    x[counter] = site.getX();
                    y[counter] = site.getY();
                    z[counter] = site.getZ();
                    
                    int id = site.getID();
                    siteIDs.add(id);
                    xpos.put(id, x);
                    ypos.put(id, y);
                    zpos.put(id, z);
                }
            }
            counter++;
        }
        // </editor-fold>
    }
    
    public void writeData(File path){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        // Before I write the location data I want to put the sites back "in order."
        // This way I can easily track the order of sites in long polymers without
        // needing to reference the site id list.
        Collections.sort((List<Integer>)siteIDs);
        try(PrintWriter p = new PrintWriter(new FileWriter(new File(path + "/LocationData.csv")), true)){
            p.print("Time,");
            for(Integer siteID : siteIDs){
                p.print(Integer.toString(siteID) + ",");
            }
            p.println();
            for(int i=0;i<totalCount;i++){
                p.print(time[i] + ",");
                for(Integer siteID : siteIDs){
                    p.print(xpos.get(siteID)[i] + " " + ypos.get(siteID)[i] + " " + zpos.get(siteID)[i] + ",");
                }
                p.println();
            }
        } catch(IOException ioe){
            ioe.printStackTrace(System.out);
        }
        // </editor-fold>
    }
}
