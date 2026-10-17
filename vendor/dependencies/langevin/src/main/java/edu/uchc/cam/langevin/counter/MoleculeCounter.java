/**
 * This class will be called whenever we want to count the molecules in the 
 * system. We'll give this object a reference to the system, and when collecting
 * data we'll get a reference to the current molecule array.  We'll then loop
 * through that array, looking at the molecule's gId to get it's type. We'll
 * use that to tell us what type of counts we should be keeping and update
 * those counts accordingly.  Like many parts of this program, I'm building this
 * on the fly and there will probably be many places were I could make this 
 * counting more efficient.
 */

package edu.uchc.cam.langevin.counter;

import edu.uchc.cam.langevin.g.counter.GMoleculeCounter;
import java.util.HashMap;
import java.util.ArrayList;
import java.io.*;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.object.Molecule;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;

public class MoleculeCounter {
    
    private final MySystem sys;
    
    // Use hashmaps to determine what kind of data we need
    private final HashMap<Integer, Boolean>countTotal = new HashMap<>();
    private final HashMap<Integer, Boolean>countFree = new HashMap<>();
    private final HashMap<Integer, Boolean>countBound = new HashMap<>();
    
    // Use hashmaps to find the arraylist associated with each molecule type.
    private final HashMap<Integer, int[]> total = new HashMap<>();
    private final HashMap<Integer, int[]> free = new HashMap<>();
    private final HashMap<Integer, int[]> bound = new HashMap<>();
    
    // Array to keep track of the time at which measurements were made
    private final double [] time;
    
    // A counter to tell us how many times we've taken data
    private int counter = 0;
    private final int totalCount;
    
    // Let's remember all of the GMolecule names and ids
    private final ArrayList<String> gmoleculeNames = new ArrayList<>();
    private final ArrayList<Integer> gmoleculeIDs = new ArrayList<>();
    
    public MoleculeCounter(Global g, MySystem sys){
        this.sys = sys;
        // System.out.println("total time = " + g.getTotalTime() + ", dt = " + g.getdt());
        totalCount = 1 + (int)Math.floor(g.getTotalTime()/g.getdtdata());
        // System.out.println("ratio = " + totalCount);
        time = new double[totalCount];

        
        for (GMolecule gmolecule : g.getMolecules()) {
            GMoleculeCounter gMoleculeCounter = gmolecule.getGMoleculeCounter();
            int gcid = gMoleculeCounter.getGMoleculeID();
            gmoleculeNames.add(gMoleculeCounter.getGMoleculeName());
            gmoleculeIDs.add(gcid);
            // System.out.println("Molecule id " + gcid + ". Count total? " + gMoleculeCounter.countTotal());
            countTotal.put(gcid, gMoleculeCounter.countTotal());
            countFree.put(gcid, gMoleculeCounter.countFree());
            countBound.put(gcid, gMoleculeCounter.countBound());
            
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
    
    /*******************************************************************\
     *                   METHOD TO GATHER THE DATA                     *
    \*******************************************************************/
    
    public void countMolecules(){
        if(counter < time.length){
            // First get the time.
            time[counter] = sys.getTime();
            ArrayList<Molecule> molecules = sys.getMolecules();
            int id;
            Molecule mol;
            StringBuilder sb = new StringBuilder();
            for(int i=molecules.size()-1;i>-1;i--){
                mol = molecules.get(i);
                id = mol.getGID();
                // System.out.println("Molecule id " + id + ". Count total? " + countTotal.get(id));
                if(countTotal.get(id)){
                    total.get(id)[counter]++;
                }
                if(countFree.get(id) && !mol.isBound()){
                    free.get(id)[counter]++;
                }
                if(countBound.get(id) && mol.isBound()){
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
        try(PrintWriter p = new PrintWriter(new FileWriter(path + "/FullCountData.csv"), true)){
            p.print("Time, ");
            for(int i=0;i<gmoleculeIDs.size();i++){
                int id = gmoleculeIDs.get(i);
                String name = gmoleculeNames.get(i);
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
                for (Integer gmoleculeID : gmoleculeIDs) {
                    int id = gmoleculeID;
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
    
}
