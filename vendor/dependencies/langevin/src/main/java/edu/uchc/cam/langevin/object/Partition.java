/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.object;

import java.util.ArrayList;

public class Partition {
    
    // A partition stores the list of sites within its boundaries
    private final ArrayList<Site> site = new ArrayList<>();
    
    // A partition knows it's neighboring partitions.  NOTE THAT THE PARTITION
    // ITSELF IS ONE OF ITS NEIGHBORS.  THIS MAKES THE LOOPING STATEMENTS
    // MUCH SIMPLER.
    private final ArrayList<Partition> partitions = new ArrayList<>();
    
    // A partition knows it's beginning and ending x,y,and z coordinates
    private double [] x = new double[2];
    private double [] y = new double[2];
    private double [] z = new double[2];
    
    // A flag, "active", to tell us if the partition has any sites in it.
    private boolean active = false;
    
    public Partition(double [] x, double [] y, double [] z){
        this.x = x;
        this.y = y;
        this.z = z;
    }
    
    public void addPartition(Partition p){
        partitions.add(p);
    }
    
    public void addSite(Site site){
        this.site.add(site);
    }
    
    public void clearSites(){
        active = false;
        this.site.clear();
    }
    
    public ArrayList<Site> getSites(){
        return site;
    }
    
    public ArrayList<Partition> getNeighbors(){
        return partitions;
    }
    
    public double [] getXLimits(){
        return x;
    }
    public double [] getYLimits(){
        return y;
    }
    public double [] getZLimits(){
        return z;
    }
    
    public void setActive(boolean bool){
        active = bool;
    }
    
    public boolean isActive(){
        return active;
    }
    
}
