/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.object;

import edu.uchc.cam.langevin.g.counter.GStateCounter;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GState {

    public static final Logger lg = LogManager.getLogger(GState.class);

    private final String stateName;
    private final GSiteType siteType;
    
    private int stateID; // In range 1,000,000,000 to 3,999,999,999
    
    private final GStateCounter stateCounter;
    
    public GState(GSiteType gSiteType, String stateName){
        this.stateName = stateName;
        this.siteType = gSiteType;
        stateCounter = new GStateCounter(this);
    }
    
    // GET METHODS
    public String getStateName(){
        return stateName;
    }
    
    public String getAbsoluteName(){
        StringBuilder sb = new StringBuilder();
        sb.append(siteType.getMoleculeName()).append(" : ");
        sb.append(siteType.getName()).append(" : ");
        sb.append(stateName);
        return sb.toString();
    }
    public String getAbsoluteNameBngl(){
        StringBuilder sb = new StringBuilder();
        sb.append(siteType.getMoleculeName()).append("(");
        sb.append(siteType.getName()).append("~");
        sb.append(stateName);
        sb.append(")");
        return sb.toString();
    }
    
    public int getID(){
        return stateID;
    }
    
    public String getIdAsString(){
        return Integer.toString(stateID);
    }
    
    public GSiteType getSiteType(){
        return siteType;
    }
    
    public String getSiteTypeName(){
        return siteType.getName();
    }
    
    public GMolecule getMolecule(){
        return siteType.getMolecule();
    }
    
    public String getMoleculeName(){
        return siteType.getMoleculeName();
    }
    
    // SET METHODS
    
    public void setID(int id){
        this.stateID = id;
    }
    
    // Override toString()
    @Override
    public String toString(){
        return stateName;
    }
    
    public GStateCounter getGStateCounter(){
        return stateCounter;
    }
    
}
