/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.counter;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.ArrayList;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import edu.uchc.cam.langevin.g.reaction.GBindingReaction;
import edu.uchc.cam.langevin.g.counter.GBondCounter;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Bond;

public class BondCounter {
    
    private final MySystem sys;
    
    // A hashmap between the bond's reactionName and its count.
    // I'm just going to count all of the bonds, but I won't write all of the data.
    private final HashMap<String, int[]> bondNumber;
    private final HashMap<String, Boolean> doCount;
    
    private final ArrayList<GBindingReaction> reactions;
            
    private final double [] time;
   
    private int counter =0;
    private final int totalCount;
    
    private int digits;
    
    public BondCounter(Global g, MySystem sys){
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
        
        reactions = g.getBindingReactions();
        
        doCount = new HashMap<>(3*reactions.size());
        bondNumber = new HashMap<>(3*reactions.size());
        
        for (GBindingReaction reaction : reactions) {
            String name = reaction.getName();
            GBondCounter data = reaction.getBondCounter();
            doCount.put(name, data.isCounted());
            int [] intArray = new int[totalCount];
            for(int j=0;j<intArray.length;j++){
                intArray[j] = 0;
            }
            bondNumber.put(name, intArray);
        }
    }
    
    /*******************************************************************\
     *                   METHOD TO GATHER THE DATA                     *
    \*******************************************************************/
    
    public void countBonds(){
        if(counter < time.length){
            // First get the time.
            time[counter] = sys.getTime();
            ArrayList<Bond> bonds = sys.getBonds();

            for(int i=bonds.size()-1;i>-1;i--){
                bondNumber.get(bonds.get(i).getName())[counter]++;
            }
            counter++;
        }
    }
    
    /******************************************************************\
     *                         WRITE DATA                             *
     * @param path                                                    *
    \******************************************************************/
    
    public void writeFullData(File path){
        String name;
        try(PrintWriter p = new PrintWriter(new FileWriter(path + "/FullBondData.csv"), true)){
            p.print("Time, ");
            for (GBindingReaction reaction : reactions) {
                name = reaction.getName();
                if(doCount.get(name)){
                    p.print(name + ",");
                }
            }
            p.println();
            // System.out.println("TotalCount = " + totalCount);
            for(int i=0;i<totalCount;i++){
                p.print(time[i] + ",");
                for (GBindingReaction reaction : reactions) {
                    name = reaction.getName();
                    if(doCount.get(name)){
                        p.print(bondNumber.get(name)[i] + ",");
                    }
                }
                p.println();
            }
        } catch (IOException e){
            e.printStackTrace(System.out);
        }
    }
    
    public void writePartialData(File path){
        String name;
        try(PrintWriter p = new PrintWriter(new FileWriter(path +
                "/Bond_Counts_At_Time_" + IOHelp.DF[digits].format(time[counter-1]) + "_s.txt"), true)){
            p.println("Time " + IOHelp.DF[digits].format(time[counter-1]));
            for (GBindingReaction reaction : reactions) {
                name = reaction.getName();
                if(doCount.get(name)){
                    p.println(name + " " + bondNumber.get(name)[counter-1]);
                }
            }
        }catch(IOException e){
            e.printStackTrace(System.out);
        }
    }
    
}
