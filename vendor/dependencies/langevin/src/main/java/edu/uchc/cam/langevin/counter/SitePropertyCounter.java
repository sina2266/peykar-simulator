/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.counter;

import java.util.HashMap;
import java.util.ArrayList;
import java.io.*;
import edu.uchc.cam.langevin.g.reaction.GBindingReaction;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GSite;
import edu.uchc.cam.langevin.g.counter.GSitePropertyCounter;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Site;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class SitePropertyCounter {

    public static final Logger lg = LogManager.getLogger(SitePropertyCounter.class);

    private final MySystem sys;
    
    // Array to keep track of the time at which measurements were made
    private final double [] time;
    
    // A counter to tell us how many times we've taken data
    private int counter = 0;
    private final int totalCount;
    
    // Store the GSites
    private final ArrayList<GSite> sites = new ArrayList<>();
    
    /*
     * A map linking sites to a boolean telling if we should track the data.
     * The key is a string consisting of the gmolecule ID plus the gsite index.
     * Since the gmolecule ids are unique, this key will also be unique.
     */
    private final HashMap<String, Boolean> isTracked;
   
    /**
     * We keep track of the current state of the site, whether it is free or 
     * bound, and if bound, the state of its binding partner. For the state
     * we'll need a double hashmap, one going from the site to a second 
     * hashmap linking each possible state to an array counting the number
     * in that state.
     */
    
    private final HashMap<String, HashMap<Integer, int[]>> stateCounts;
    // Maps between site and array giving total number of free and bound sites.
    private final HashMap<String, int[]> freeCount;
    private final HashMap<String, int[]> boundCount;
    // Map between site and  possible bond types
    private final HashMap<String, HashMap<String, int[]>> bondCounts;
    // Map between site and possible binding reaction names
    private final HashMap<String, ArrayList<String>> reactionNames;
    
    public SitePropertyCounter(Global g, MySystem sys){
        this.sys = sys;
        totalCount = 1 + (int)Math.floor(g.getTotalTime()/g.getdtdata());
        time = new double[totalCount];
        
        // Calculate the total number of unique sites
        int totalSites = 0;
        for(GMolecule gmolecule : g.getMolecules()){
            totalSites += gmolecule.getSites().size();
        }
        
        isTracked = new HashMap<>(5*totalSites);
        freeCount = new HashMap<>(5*totalSites);
        boundCount = new HashMap<>(5*totalSites);
        stateCounts = new HashMap<>(5*totalSites);
        bondCounts = new HashMap<>(5*totalSites);
        reactionNames = new HashMap<>(5*totalSites);
        
        for(GMolecule gmolecule : g.getMolecules()){
            ArrayList<GSite> gsites = gmolecule.getSites();
            String molID = Integer.toString(gmolecule.getID());
            for(GSite gsite : gsites){
                sites.add(gsite);
                GSitePropertyCounter propertyCounter = gsite.getPropertyCounter();
                String siteIndex = Integer.toString(gsite.getIndex());
                // Create the isTracked hashmap
                isTracked.put(molID+siteIndex, propertyCounter.isTracked());
                // Create the state counts hashmap
                HashMap<Integer, int[]> mStateCounts = new HashMap<>(3*gsite.getType().getStates().size());
                for(GState gstate : gsite.getType().getStates()){
                    int [] counts = new int[time.length];
                    for(int i=0;i<counts.length;i++){
                        counts[i] = 0;
                    }
                    mStateCounts.put(gstate.getID(), counts);
                }
                stateCounts.put(molID+siteIndex, mStateCounts);
                // Create the free and bound count hashmaps
                int [] fcount = new int[time.length];
                int [] bcount = new int[time.length];
                for(int i=0;i<fcount.length;i++){
                    fcount[i] = 0;
                    bcount[i] = 0;
                }
                freeCount.put(molID+siteIndex, fcount);
                boundCount.put(molID+siteIndex, bcount);
                // Create the reaction name arraylist
                ArrayList<String> rNames = new ArrayList<>();
                // Create the bond type hashmap
                HashMap<String, int[]> possibleBonds = new HashMap<>(3*g.getBindingReactions().size());
                for(GBindingReaction reaction : g.getBindingReactions()){
                    if(reaction.getType(0) == gsite.getType() || reaction.getType(1) == gsite.getType()){
                        rNames.add(reaction.getName());
                        int [] bondCounter = new int[time.length];
                        for(int j=0;j<bondCounter.length;j++){
                            bondCounter[j] = 0;
                        }
                        possibleBonds.put(reaction.getName(), bondCounter);
                    }
                }
                reactionNames.put(molID+siteIndex, rNames);
                bondCounts.put(molID+siteIndex, possibleBonds);
            }
        }
    }
    
    /*******************************************************************\
     *                   METHOD TO GATHER THE DATA                     *
    \*******************************************************************/
    
    public void countProperties(){
        if(counter < time.length){
            // First get the time
            time[counter] = sys.getTime();
            
            for(Site site : sys.getSites()){
                String index = Integer.toString(site.getGSiteIndex());
                String molID = Integer.toString(site.getMolecule().getGID());
                // Count if the site is free or bound
                if(site.isBound()){
                    boundCount.get(molID+index)[counter]++;
                    // If it's bound we track its bond type
                    HashMap<String, int[]> mBondMap = bondCounts.get(molID + index);
                    String bondName = site.getBond().getName();
                    mBondMap.get(bondName)[counter]++;
                } else {
                    freeCount.get(molID+index)[counter]++;
                }
                // Now track its state
                HashMap<Integer, int[]> mStateMap = stateCounts.get(molID+index);
                mStateMap.get(site.getState().getID())[counter]++;
            }
            counter++;
        }
    }
    
    /******************************************************************\
     *                         WRITE DATA                             *
     * @param path                                                    *
    \******************************************************************/
    
    public void writeData(File path){
        try(PrintWriter p = new PrintWriter(new FileWriter(path + "/SitePropertyData.csv"), true)){
            for(GSite site : sites){
                String index = Integer.toString(site.getIndex());
                String molID = Integer.toString(site.getMolecule().getID());
                if(isTracked.get(molID + index)){
                    writeSingleGSite(p, site);
                    p.println();
                }
            }
        } catch (IOException ioe) {
            throw new IllegalStateException("Failed to write SitePropertyData.csv to path: " + path, ioe);
        }
    }
    
    // Write data for a single site
    private void writeSingleGSite(PrintWriter p, GSite gsite){
        String index = Integer.toString(gsite.getIndex());
        String molID = Integer.toString(gsite.getMolecule().getID());
        HashMap<Integer, int[]> mStateMap = stateCounts.get(molID + index);
        HashMap<String, int[]> mBondMap = bondCounts.get(molID + index);
        p.println("Molecule," + gsite.getMoleculeName() + ",Site Index,"
            + gsite.getIndex() + ",Site Type, " + gsite.getTypeName());
        p.print("Time, Free, Bound,");
        for(GState state : gsite.getType().getStates()){
            p.print(state.getStateName() + ",");
        }
        ArrayList<String> rNames = reactionNames.get(molID+index);
        for(String name : rNames){
            p.print(name + ",");
        }
        p.println();
        for(int i=0;i<time.length;i++){
            p.print(time[i] + ",");
            p.print(freeCount.get(molID + index)[i] + ",");
            p.print(boundCount.get(molID + index)[i] + ",");
            for(GState gstate : gsite.getType().getStates()){
                p.print(mStateMap.get(gstate.getID())[i] + ",");
            }
            for(String name : rNames){
                p.print(mBondMap.get(name)[i] + ",");
            }
            p.println();
        }
    }
}
