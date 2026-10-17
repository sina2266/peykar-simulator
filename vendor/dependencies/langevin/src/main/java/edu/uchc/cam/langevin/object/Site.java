/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.object;

import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.helpernovis.Location;
import edu.uchc.cam.langevin.langevinnovis01.MyVector;

public class Site {
    
    /** 
     * Each site will store its type name and type id.  Additionally, each site
     * will be given a unique identifier so I can easily track an individual
     * particle.
     */
    
    private final String typeName;
    private final int typeID;
    private final int id;
    
    /**
     * Each site stores the index of its corresponding GSite.
     */
    private final int gSiteIndex;
    
    /**
     * It will be useful if each site knows what molecule it belongs to.
     */
    
    private final Molecule molecule;
    
    /**
     * Each site knows its location.
     */
    private final int location;
    
    /**
     * Physical quantities defined by the site type. Physical radius, reaction 
     * radius, diffusion constant.
     */
    
    private final double r;
    private final double rrxn;
    private final double D;
    
    /**
     * Color for displaying. Just use a string.
     */
    
    private String color;
    
    // The scalar which must multiple the random force in order to get the correct diffusion rate.
    private double diffScale;
    
    // Position variables
    private final MyVector pos = new MyVector(0,0,0);
    // Purely for testing. Will comment out for real runs.
    // private final MyVector lastPos = new MyVector(0,0,0);
    // initial position
    private final MyVector initPos = new MyVector(0,0,0);
    
    // Force and velocity
    private final MyVector randForce = new MyVector();
    private final MyVector springForce = new MyVector();
    private final MyVector totalForce = new MyVector();
    private final MyVector velocity = new MyVector();
    
    // Site's current state
    private GState state;
    
    // Boundary information
    private double xmin;
    private double xmax;
    private double ymin;
    private double ymax;
    private double zmin;
    private double zmax;
    
    // Flag to indicate the site has been checked for collisions/reactions
    private boolean checked = false;
    
    // Store the current parition of this site
    private Partition partition;
    
    // Flags to indicate if this site is restricted to move in a 2D plane, or 
    // if the site is completely fixed in space.
    private boolean is2d = false;
    private boolean isFixed = false;
    
    // Flag to indicate if this site is bound
    private boolean bound = false;
    
    // A reference to the bond
    private Bond bond = null;
    
    // A reference to the site it is bound to
    private Site bindingPartner = null;
    
    public Site(GSiteType type, int id, int location, Molecule molecule, int siteIndex){
        this.typeName = type.getName();
        if(typeName.equals(GSiteType.ANCHOR)){
            is2d = true;
        }
        if(molecule.is2D()){
            is2d = true;
        }
        this.typeID = type.getID();
        this.color = type.getColorName();
        this.r = type.getRadius();
        this.rrxn = type.getReactionRadius();
        this.D = 1000000*type.getD(); //Convert D from um^2/s to nm^2/s
        this.id = id;
        this.location = location;
        this.molecule = molecule;
        this.gSiteIndex = siteIndex;
    }
    
    //*************       GET METHODS     *****************/
    
    public String getType(){
        return typeName;
    }
    
    public int getTypeID(){
        return typeID;
    }
    
    public int getID(){
        return id;
    }
    
    public Molecule getMolecule(){
        return molecule;
    }
    
    public GState getState(){
        return state;
    }
    
    public double getRadius(){
        return r;
    }
    
    public double getReactionRadius(){
        return rrxn;
    }
    
    public double getD(){
        return D;
    }
    
    public int getLocation(){
        return location;
    }
    
    public String getLocationName(){
        return Location.locationName[location];
    }
    
    public MyVector getPosition(){
        return pos;
    }
    
//    public MyVector getLastPosition(){
//        return lastPos;
//    }
    
    public double getX(){
        return pos.x;
    }
    public double getY(){
        return pos.y;
    }
    public double getZ(){
        return pos.z;
    }
    
    public MyVector getInitialPosition(){
        return initPos;
    }
    
    public String getColor(){
        return color;
    }
    
    public boolean isBound(){
        return bound;
    }
    
    public Site getBindingPartner(){
        return bindingPartner;
    }
    
    public Bond getBond(){
        return bond;
    }
    
    public int getGSiteIndex(){
        return gSiteIndex;
    }
     
    //*************       SET METHODS     *****************/
    

//    public void setRadius(double r){
//        this.r = r;
//    }
//    
//    public void setReactionRadius(double reactionRadius){
//        this.rrxn = reactionRadius;
//    }
//    
//    public void setD(double diffusionConstant){
//        this.D = diffusionConstant;
//    }
    
    
    public void setState(GState state){
        this.state = state;
    }
    
    public void setBound(boolean bool){
        bound = bool;
    }
    
    public void setColor(String color){
        this.color = color;
    }
    
//    public void setLastPosition(){
//        lastPos.x = pos.x;
//        lastPos.y = pos.y;
//        lastPos.z = pos.z;
//    }
    
    public void setPosition(MyVector pos){
        // setLastPosition();
        this.pos.x = pos.x;
        this.pos.y = pos.y;
        this.pos.z = pos.z;
    }
    
    public void setPosition(double x, double y, double z){
        // setLastPosition();
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }
    
    public void setPosition(double [] position){
        // setLastPosition();
        pos.x = position[0];
        pos.y = position[1];
        pos.z = position[2];
    }
    
    public void setBounds(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax){
        this.xmin = xmin; this.xmax = xmax;
        this.ymin = ymin; this.ymax = ymax;
        this.zmin = zmin; this.zmax = zmax;
    }
    
    public void setInitialPosition(double x, double y, double z){
        initPos.x = x;
        initPos.y = y;
        initPos.z = z;
    }
    
    public void setInitialPosition(MyVector pos){
        initPos.x = pos.x;
        initPos.y = pos.y;
        initPos.z = pos.z;
    }
    
    public void setInitialPosition(double [] position){
        initPos.x = position[0];
        initPos.y = position[1];
        initPos.z = position[2];
    }
    
    public void setDiffusionScale(double dt){
        diffScale = Math.sqrt(2*D/dt);
    }
    
    public void setBindingPartner(Site site){
        this.bindingPartner = site;
    }
    
    public void setBond(Bond bond){
        this.bond = bond;
    }
    
    /*/*****************************************************************\
     *          METHODS TO REPORT IF FIXED OR IF 2D                    * 
    \*******************************************************************/
    
    public boolean isFixed(){
        return isFixed;
    }
    
    public boolean is2D(){
        return is2d;
    }
    
    /*/*****************************************************************\
     *               METHODS TO MANAGE PARTITIONS                      * 
    \*******************************************************************/
    
    public Partition getPartition(){
        return partition;
    }
    
    public boolean getChecked(){
        return checked;
    }
    
    public void setPartition(Partition partition){
        this.partition = partition;
    }
    
    public void setChecked(boolean bool){
        checked = bool;
    }
    
    /*/****************************************************************\
     *               METHODS TO MANAGE FORCES                         *
    \******************************************************************/
    
    public MyVector getRandomForce(){
        return randForce;
    }
    
    public void setRandomForce(double fx, double fy, double fz){
        randForce.x = diffScale*fx;
        randForce.y = diffScale*fy;
        randForce.z = diffScale*fz;
    }
    
    public void clearSpringForce(){
        springForce.x = 0;
        springForce.y = 0;
        springForce.z = 0;
    }
    
    public void incrementSpringForce(double fx, double fy, double fz){
        if(!isFixed){
            springForce.x += fx;
            springForce.y += fy;
            if(!is2d){
                springForce.z += fz;
            }
        }
    }
    
    /*********************************************************************\
     *                   METHOD TO UPDATE POSITION                       * 
     * @param dt
    \*********************************************************************/
    
    public void updatePosition(double dt){
        // velocity = randForce.add(springForce);
        // System.out.println(this.getType());
        // System.out.println("Rand force: " + randForce.toString() + " Spring Force: " + springForce.toString());
        velocity.x = randForce.x + springForce.x;
        velocity.y = randForce.y + springForce.y;
        velocity.z = randForce.z + springForce.z;
        // System.out.println("Current velocity " + velocity.toString());
        //pos = pos.add(velocity.multiply(dt));
        // setLastPosition();
        pos.x += (dt*velocity.x);
        pos.y += (dt*velocity.y);
        pos.z += (dt*velocity.z);
        // System.out.println("position = " + pos.toString());
        // System.out.println("Current position: " + pos.toString());
        // Check to see if the site hit the system boundaries
        checkBoundaries();
    }
    
    public void checkBoundaries(){
        if(pos.x + r > xmax){
            // System.out.println("Overshot x: " + pos.x + " plus radius " + this.getRadius());
            // setLastPosition();
            pos.x = 2*(xmax-r) - pos.x;
            // System.out.println("New x: " + pos.x + " plus radius " + this.getRadius());
        } else if(pos.x - r < xmin){
            // setLastPosition();
            pos.x = 2*(xmin+r) - pos.x;
        }
        
        if(pos.y + r > ymax){
            // setLastPosition();
            pos.y = 2*(ymax-r) - pos.y;
        } else if(pos.y -r < ymin){
            // setLastPosition();
            pos.y = 2*(ymin+r) - pos.y;
        }
        
        if(pos.z + r > zmax){
            // setLastPosition();
            pos.z = 2*(zmax-r) - pos.z;
        } else if(pos.z - r < zmin){
            // setLastPosition();
            pos.z = 2*(zmin+r) - pos.z;
        }
    }
    
}
