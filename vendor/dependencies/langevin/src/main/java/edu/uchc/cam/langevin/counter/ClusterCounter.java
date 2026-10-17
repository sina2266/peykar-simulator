/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.counter;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.util.ArrayList;
import java.util.HashMap;
import java.io.*;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.object.Molecule;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;

public class ClusterCounter {

    private final Global g;
    private final MySystem sys;
    private final double [] time;
    // The clusters at a given time
    private ArrayList<ArrayList<Molecule>> clusters = new ArrayList<>();
    
    long clusterCounter;
    // An integer telling us how many digits to print out for the names of the 
    // partial data files
    int digits;
    
    public ClusterCounter(Global g, MySystem sys){
        this.g = g;
        this.sys = sys;
        clusterCounter = 0;
        int totalCount = 1 + (int)Math.floor(g.getTotalTime()/g.getdtdata());
        time = new double[totalCount];
        digits = 0;
        while(g.getdtdata() < Math.pow(10,-digits)){
            digits++;
            if(digits == 8){
                break;
            }
        }
    }
    
    public void countClusters(){
        assignClusters();
    }
    
    private void assignClusters(){
        clusters.clear();
        clusterCounter = -1;
        // Clear the cluster indices
        for(Molecule molecule : sys.getMolecules()){
            molecule.setClusterIndex(clusterCounter);
        }

        for(Molecule molecule : sys.getMolecules()){
            if(molecule.getClusterIndex() == -1){
                clusterCounter++;
                ArrayList<Molecule> cluster = new ArrayList<>();
                clusters.add(cluster);
                addAndCheckPartners(cluster, molecule);
            }
        }
    }
    
    private void addAndCheckPartners(ArrayList<Molecule> cluster, Molecule molecule){
        molecule.setClusterIndex(clusterCounter);
        cluster.add(molecule);
        for(Molecule partner : molecule.getBindingPartners()){
            if(partner.getClusterIndex() == -1){
                addAndCheckPartners(cluster, partner);
            }
        }
    }
    
    // Return a hashmap mapping GMolecule ids and the number of those molecules in a given cluster
    private HashMap<Integer, Integer> moleculeCounts(ArrayList<Molecule> cluster){
        HashMap<Integer, Integer> counts = new HashMap<>(3*g.getMolecules().size());
        for(GMolecule gmolecule : g.getMolecules()){
            counts.put(gmolecule.getID(), 0);
        }
        
        for(Molecule molecule : cluster){
            int c = counts.get(molecule.getGID());
            c++;
            counts.put(molecule.getGID(), c);
        }
        
        return counts;
    }
    
    public void writeClusters(File path){
        try(PrintWriter p = new PrintWriter(new FileWriter(path + 
            "/Clusters_Time_" + IOHelp.DF[digits].format(sys.getTime()) + ".csv"))){
            p.println("Total clusters, " + clusters.size());
            p.println();
            for(ArrayList<Molecule> cluster : clusters){
                if(cluster.size() > 1){
                    p.println("Cluster Index, " + cluster.get(0).getClusterIndex());
                    p.println("Size, " + cluster.size());
                    HashMap<Integer, Integer> counts = moleculeCounts(cluster);
                    for(GMolecule gmolecule : g.getMolecules()){
                        p.println(gmolecule.getName() + ", " + counts.get(gmolecule.getID()));
                    }
                    p.println();
                }
            }
            
        } catch(IOException ioe){
            ioe.printStackTrace(System.out);
        }
    }
    
}
