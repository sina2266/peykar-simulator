/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.counter;

import java.util.HashMap;
import java.util.ArrayList;

import edu.uchc.cam.langevin.helpernovis.IOHelp;

import java.io.*;

import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.g.counter.GStateCounter;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Site;

public class StateCounter {
    
    private final MySystem sys;
    
    // Use hashmaps to determine what kind of data we need
    private final HashMap<Integer, Boolean>countTotal = new HashMap<>();
    private final HashMap<Integer, Boolean>countFree = new HashMap<>();
    private final HashMap<Integer, Boolean>countBound = new HashMap<>();
    
    // Use hashmaps to find the arraylist associated with each state type.
    private final HashMap<Integer, int[]> total = new HashMap<>();
    private final HashMap<Integer, int[]> free = new HashMap<>();
    private final HashMap<Integer, int[]> bound = new HashMap<>();
    
    // Array to keep track of the time at which measurements were made
    private final double [] time;
    
    // A counter to tell us how many times we've taken data
    private int counter = 0;
    private final int totalCount;
    
    // Let's remember all of the GMolecule names and ids
    private final ArrayList<String> gstateFullNames = new ArrayList<>();    // ex: MT0:Site1:state0
    private final ArrayList<Integer> gstateIDs = new ArrayList<>();
    
    // An integer telling us how many digits to print out for the names of the 
    // partial data files
    int digits;
    
    public StateCounter(Global g, MySystem sys){
        this.sys = sys;
        totalCount = 1 + (int)Math.floor(g.getTotalTime()/g.getdtdata());
        time = new double[totalCount];
        digits = 0;
        while(g.getdtdata() < Math.pow(10,-digits)){
            digits++;
            if(digits == 8){
                break;
            }
        }
        
        for (GMolecule gmolecule : g.getMolecules()) {
            for (GSiteType gtype : gmolecule.getTypeArray()){
                for(GState gstate : gtype.getStates()){
                    GStateCounter gStateCounter = gstate.getGStateCounter();
                    int gcid = gstate.getID();
                    gstateFullNames.add(gstate.getAbsoluteName());
                    gstateIDs.add(gcid);
                    // System.out.println("Molecule id " + gcid + ". Count total? " + gMoleculeCounter.countTotal());
                    countTotal.put(gcid, gStateCounter.countTotal());
                    countFree.put(gcid, gStateCounter.countFree());
                    countBound.put(gcid, gStateCounter.countBound());

                    int [] tCount = new int[totalCount];
                    int [] fCount = new int[totalCount];
                    int [] bCount = new int[totalCount];
                    for(int j=0;j<totalCount;j++){
                        tCount[j] = 0;
                        fCount[j] = 0;
                        bCount[j] = 0;
                    }

                    total.put(gcid, tCount);
                    free.put(gcid, fCount);
                    bound.put(gcid, bCount);
                }
            }
        }
    }
    
    /*******************************************************************\
     *                   METHOD TO GATHER THE DATA                     *
    \*******************************************************************/
    
    public void countStates(){
        if(counter < time.length){
            // First get the time.
            time[counter] = sys.getTime();
            for(Site site : sys.getSites()){
                int id = site.getState().getID();
                // System.out.println("Molecule id " + id + ". Count total? " + countTotal.get(id));
                if(countTotal.get(id)){
                    total.get(id)[counter]++;
                }
                if(countFree.get(id) && !site.isBound()){
                    free.get(id)[counter]++;

                }
                if(countBound.get(id) && site.isBound()){
                    bound.get(id)[counter]++;
                }
            }
            counter++;
        }
    }
    
    /******************************************************************\
     *                         WRITE DATA                             *
     * @param path                                                    *
    \******************************************************************/
    
    public void writeFullData(File path){
        try(PrintWriter p = new PrintWriter(new FileWriter(path + "/FullStateCountData.csv"), true)){
            p.print("Time, ");
            for(int i=0;i<gstateIDs.size();i++){
                int id = gstateIDs.get(i);
                String name = gstateFullNames.get(i);
                if(countTotal.get(id)){
                    p.print("TOTAL " + name + ",");
                }
                if(countFree.get(id)){
                    p.print("FREE " + name + ",");
                }
                if(countBound.get(id)){
                    p.print("BOUND " + name + ",");
                }
            }
            p.println();
            // System.out.println("TotalCount = " + totalCount);
            for(int i=0;i<totalCount;i++){
                p.print(time[i] + ",");
                for (Integer gstateID : gstateIDs) {
                    int id = gstateID;
                    if(countTotal.get(id)){
                        p.print(total.get(id)[i] + ",");
                    }
                    if(countFree.get(id)){
                        p.print(free.get(id)[i] + ",");
                    }
                    if(countBound.get(id)){
                        p.print(bound.get(id)[i] + ",");
                    }
                }
                p.println();
            }
        } catch (IOException e){
            e.printStackTrace(System.out);
        }
    }
    
    public void writePartialData(File path){

        try(PrintWriter p = new PrintWriter(new FileWriter(path +
                "/State_Counts_At_Time_" + IOHelp.DF[digits].format(time[counter-1]) + "_s.txt"), true)){
            p.println("Time " + IOHelp.DF[digits].format(time[counter-1]));
            for(int i=0;i<gstateIDs.size();i++){
                int id = gstateIDs.get(i);
                String name = gstateFullNames.get(i);
                if(countTotal.get(id)){
                    p.println(name + " Total " + total.get(id)[counter-1]);
                }
                if(countFree.get(id)){
                    p.println(name + " Free " + free.get(id)[counter-1]);
                }
                if(countBound.get(id)){
                    p.println(name + " Bound " + bound.get(id)[counter-1]);
                }
            }
        }catch(IOException e){
            e.printStackTrace(System.out);
        }
    }
    
}
