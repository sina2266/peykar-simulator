/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.object;

import java.io.PrintWriter;
import java.util.Scanner;
import java.util.ArrayList;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GSiteType {

    public static final Logger lg = LogManager.getLogger(GSiteType.class);

    /* Example:
    the list of GSiteTypes belonging to a GMolwcule

                 TYPE: Name "Site0" Radius 1.00000 D 1.000 Color BLUE STATES "state0"
                 TYPE: Name "Site1" Radius 1.00000 D 1.000 Color RED STATES "state0" "state1"
 */
    private String name;
    
    private final GMolecule molecule;

    private int typeID; // Number between 1000000 and 3999999
    // Both radii in nm
    private double radius;
    private double reactionRadius;  // either 1.5x radius or r+2 nm, whichever is smaller, but not smaller than r+0.5 nm
    private double D;   // still in um^2/s! Gets converted to nm^2/s in the Site class and the BindingReaction class
    private String color;
    
    private ArrayList<GState> states = new ArrayList<>();
    
    
    // Static name for anchors
    public static String ANCHOR = "Anchor";

    
    public GSiteType(GMolecule molecule, String name){
        this.name = name;
        this.molecule = molecule;
    }
    
    // SET METHODS
    
    public void setName(String name){
        this.name = name;
    }
    
    public void setID(int id){
        typeID = id;
    }

    /*
     * geometric and numerical safety constraints that ensure molecules don’t react at a distance
     * or overlap unrealistically
     * enforce a lower bound (from geometry / molecules taking space)
     * enforce an upper bound (from physical plausibility)
     */
    public void setRadius(double r) {
        this.radius = r;
        // 0.5+r (nm) minimum separation between molecules to avoid overlaps, add 0.5nm as a buffer
        // 1.5*r (nm) prevents very large molecules from having unrealistically tiny reaction radii
        double minRadius = Math.max(0.5 + r, 1.5*r);

        // r+2 (nm) upper bound for the reaction radius, prevents reactions from occurring at unphysically large distances
        this.reactionRadius = Math.min(minRadius, r+2);
    }
    
    public void setD(double D){
        this.D = D;
    }
    
    public void setColor(String color){
        this.color = color;
    }
    
    public void setStateArray(ArrayList<GState> states){
        this.states = states;
    }
    
    // GET METHODS
    
    public String getName(){
        return name;
    }
    
    public GMolecule getMolecule(){
        return molecule;
    }
    
    public String getMoleculeName(){
        return molecule.getName();
    }
    
    public int getID(){
        return typeID;
    }
    
    public double getRadius(){
        return radius;
    }
    
    public double getReactionRadius(){
        return reactionRadius;
    }
    
    public double getD(){
        return D;
    }
    
    public String getColorName(){
        return color;
    }
    
    public ArrayList<GState> getStates(){
        return states;
    }
    
    public GState getState(String name){
        GState s = null;
        for (GState ts : states) {
            if(ts.getStateName().equals(name)){
                s = ts;
                break;
            }
        }
        return s;
    }
    
    
    /********************************************************************\
     *                        ASSIGN STATE IDS                          *
    \********************************************************************/
    
    // State ids can go up to 999, so use a padding of 3.
    public void assignStateIDs(){
        for(int i=0;i<states.size();i++){
            GState state = states.get(i);
            state.setID(IOHelp.appendNumber(typeID, i, 3));
        }
        
    }
    
    /* ******************************************************************\
     *                           FILE IO                                *
    \********************************************************************/
    
    public void writeType(PrintWriter p){
        p.print("TYPE: Name \"" + getName() + "\"");
        p.print(" Radius " + IOHelp.DF[3].format(getRadius()) + " D " + IOHelp.DF[3].format(getD()) + " Color " + getColorName());
        p.print(" STATES ");
        for (GState state : states) {
            p.print("\"" + state + "\"" + " ");
        }
        p.println();
    }
    
    public static GSiteType readType(GMolecule mol, String s){
        GSiteType tempType = new GSiteType(mol, "TempName");
        Scanner sc = new Scanner(s);
        if(!sc.next().equals("TYPE:")){
            lg.error("ERROR: Type line did not begin with \"TYPE:\"");
        }
        while(sc.hasNext()){
            String scnext = sc.next();
            switch(scnext){
                case "Name":{
                    tempType.setName(IOHelp.getNameInQuotes(sc));
                    break;
                }
                case "Radius":{
                    tempType.setRadius(Double.parseDouble(sc.next()));
                    break;
                }
                case "D":{
                    tempType.setD(Double.parseDouble(sc.next()));
                    break;
                }
                case "Color":{
                    tempType.setColor(sc.next());
                    break;
                }
                case "STATES":{
                    ArrayList<String> tempStrings = new ArrayList<>();
                    while(sc.hasNext()){
                        tempStrings.add(IOHelp.getNameInQuotes(sc));
                    }
                    ArrayList<GState> tempStates = new ArrayList<>();
                for (String tempString : tempStrings) {
                    tempStates.add(new GState(tempType, tempString));
                }
                    tempType.setStateArray(tempStates);
                    break;
                }
                default:{
                    lg.error("Type reader received unexpected input: " + scnext);
                }
            }
        }
        return tempType;
    }
    
}
