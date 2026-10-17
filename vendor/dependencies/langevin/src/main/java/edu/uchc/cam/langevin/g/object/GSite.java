/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.object;

import edu.uchc.cam.langevin.g.counter.GSitePropertyCounter;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.io.PrintWriter;
import java.util.ArrayList;
import edu.uchc.cam.langevin.helpernovis.Location;
import edu.uchc.cam.langevin.object.Molecule;
import edu.uchc.cam.langevin.object.Site;

public class GSite {
    /* Example:
        a GMolecule may have multiple GSites of the same GSiteType:

                 SITE 0 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site0" Radius 1.00000 D 1.000 Color BLUE STATES "state0"
                      x 0.00000 y 0.00000 z 0.00000
                 SITE 1 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site1" Radius 1.00000 D 1.000 Color RED STATES "state0" "state1"
                      x 0.00000 y 0.00000 z 0.00000
                 SITE 2 : Intracellular : Initial State 'state0'
                      TYPE: Name "Site1" Radius 1.00000 D 1.000 Color RED STATES "state0" "state1"
                      x 0.00000 y 0.00000 z 0.00000
     */
    
    private final GMolecule gmolecule;
    private final GSiteType type;
    
    // index to identify the site in the molecule
    private int index;
    
    private double x;
    private double y;
    private double z;
    
    private final ArrayList<GSite> connectedSites = new ArrayList<>();

    private int location;
    private GState initialState;

    private final GSitePropertyCounter sitePropertyCounter;
    
    public GSite(GMolecule gmolecule, GSiteType type){
        this.gmolecule = gmolecule;
        this.type = type;
        this.sitePropertyCounter = new GSitePropertyCounter(this);
        x=0;
        y=0;
        z=0;
    }
    
    // SET METHODS
    public void setIndex(int i){
        index = i;
    }
    public void setX(double x){
        this.x = x;
    }
    public void setY(double y){
        this.y = y;
    }
    public void setZ(double z){
        this.z = z;
    }
    public void setPosition(double x, double y, double z){
        this.x = x;
        this.y = y;
        this.z = z;
    }
    
    public void setLocation(int i){
        location = i;
    }
    
    public void setLocation(String loc){

        for(int i=0;i<Location.locationName.length;i++){
            if(loc.equals(Location.locationName[i])){
                location = i;
                break;
            }
        }

    }
    
    public void setInitialState(GState state){
        initialState = state;
    }
    
    // GET METHODS
    public int getIndex(){
        return index;
    }
    
    public double getX(){
        return x;
    }
    
    public double getY(){
        return y;
    }
    
    public double getZ(){
        return z;
    }
    
    public double [] getPosition(){
        return new double[]{x,y,z};
    }
    
    public GSiteType getType(){
        return type;
    }
    
    public double getRadius(){
        return type.getRadius();
    }
    
    public double getD(){
        return type.getD();
    }
    
    public String getTypeName(){
        return type.getName();
    }
    
    public String getColor(){
        return type.getColorName();
    }
    
    public String getLocationName(){
        return Location.locationName[location];
    }
    
    public int getLocation(){
        return location;
    }
    
    public GState getInitialState(){
        return initialState;
    }
    
    public GMolecule getMolecule(){
        return gmolecule;
    }
    
    public String getMoleculeName(){
        return gmolecule.getName();
    }
    
    public GSitePropertyCounter getPropertyCounter(){
        return sitePropertyCounter;
    }
    
    /* ******************************************************************\
     *                   Create instance method                         *
    \********************************************************************/
    
    public Site newInstance(int id, Molecule molecule){
        return new Site(type, id, location, molecule, index);
    }
    
    /* *******************************************************************\
     *                      FILE IO METHODS                              *
    \*********************************************************************/
    
    public void writeSite(PrintWriter p){
        p.println("SITE " + this.getIndex() + " : " + getLocationName() + " : Initial State '" + initialState + "'");
        p.print("          ");
        this.getType().writeType(p);
        p.println("          " + "x " + IOHelp.DF[3].format(getX()) + " y " + IOHelp.DF[3].format(getY()) + " z " + IOHelp.DF[3].format(getZ()) + " ");
    }
    
    
}
