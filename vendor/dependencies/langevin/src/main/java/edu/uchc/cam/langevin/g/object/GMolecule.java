/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.object;

import edu.uchc.cam.langevin.g.counter.GMoleculeCounter;
import edu.uchc.cam.langevin.g.counter.GStateCounter;
import edu.uchc.cam.langevin.g.reaction.GDecayReaction;

import java.util.ArrayList;
import java.util.Scanner;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import edu.uchc.cam.langevin.helpernovis.Location;
import java.io.PrintWriter;

import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Link;
import edu.uchc.cam.langevin.object.Molecule;
import edu.uchc.cam.langevin.langevinnovis01.MyVector;
import edu.uchc.cam.langevin.object.Site;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GMolecule {

    public static final Logger lg = LogManager.getLogger(GMolecule.class);
    /* Example:
                MOLECULE: "MT0" Intracellular Number 20 Site_Types 2 Total_Sites 3 Total_Links 2 is2D false

                TYPE: Name "Site0" ...
                TYPE: Name "Site1" ...

                SITE 0 : ...
                SITE 1 : ...
                SITE 2 : ...

     */
    
    // Strings to indicate random or defined initial positions
    public final static String RANDOM = "Random";
    public final static String SET = "Set";
    
    private String name;
    private int gmolID;  // Should be a number between 1000 and 3999.
    
    private ArrayList<GSiteType> typeArray = new ArrayList<>();
    private ArrayList<GSite> siteArray = new ArrayList<>();
    private ArrayList<GLink> linkArray = new ArrayList<>();
    
    private final GDecayReaction decayReaction;
    
    private final GMoleculeCounter moleculeCounter;
    
    private int number;
    
    private int location;
    
    private boolean is2d = false;
    
    // Initial position arrays
    private final ArrayList<Double> xIC;
    private final ArrayList<Double> yIC;
    private final ArrayList<Double> zIC;
    // Use an initial condition counter to determine if we still have ICs to use.
    // This means I can just slightly modify the getInstance code, and I don't 
    // have to worry if we're creating new molecules later in the simulation.
    private int initialConditionToUse = 0;
    
    public GMolecule(String name){
        this.name = name;
        this.decayReaction = new GDecayReaction(this);
        this.moleculeCounter = new GMoleculeCounter(this);
        this.xIC = new ArrayList<>();
        this.yIC = new ArrayList<>();
        this.zIC = new ArrayList<>();
    }
    
    // SET METHODS
    public void setTypeArray(ArrayList<GSiteType> types){
        this.typeArray = types;
    }
    
    public void setSiteArray(ArrayList<GSite> sites){
        this.siteArray = sites;
    }
    
    public void setLinkArray(ArrayList<GLink> links){
        this.linkArray = links;
    }
    
    public void setName(String name){
        this.name = name;
    }
    
    public void setID(int id){
        gmolID = id;
    }
    
    public void set2D(boolean bool){
        is2d = bool;
    }
    
    public void setNumber(int number){
        this.number = number;
    }
    
    
    public void setLocation(int i){
        location = i;
    }

    public void setLocation(String loc) {
        boolean found = false;
        for(int i = 0; i < Location.locationName.length;i++) {
            if(loc.equals(Location.locationName[i])){
                location = i;
                found = true;
                break;
            }
        }
        if(!found) {
            throw new RuntimeException("Location must be Intracellular, Membrane or Extracellular. Found: '" + loc + "'.");
        }
    }
    
    // GET METHODS
    public String getName(){
        return name;
    }
    
    public int getID(){
        return gmolID;
    }
    
    public ArrayList<GSiteType> getTypeArray(){
        return typeArray;
    }
    
    public GSiteType getType(String typeName){
        GSiteType tempType = null;
        for (GSiteType typeArray1 : typeArray) {
            if (typeArray1.getName().equals(typeName)) {
                tempType = typeArray1;
                break;
            }
        }
        if (tempType == null) {
            throw new IllegalStateException("GMolecule.getType() failed: no type found for typeName = " +
                            (typeName == null ? "null" : "'" + typeName + "'"));
        }
        return tempType;
    }
    
    public ArrayList<GSite> getSites(){
        return siteArray;
    }
    
    public GSite getSite(int index){
        return siteArray.get(index);
    }
    
    public ArrayList<GLink> getLinks(){
        return linkArray;
    }
    
    public String getLocationName(){
        return Location.locationName[location];
    }
    
    public GDecayReaction getGDecayReaction(){
        return decayReaction;
    }
    
    public GMoleculeCounter getGMoleculeCounter(){
        return this.moleculeCounter;
    }
    
    public int getLocation(){
        return location;
    }
    
    public boolean is2D(){
        return is2d;
    }
    
    public int getNumber(){
        return number;
    }
    
    public int typeNumber(){
        return typeArray.size();
    }
    
    public int siteNumber(){
        return siteArray.size();
    }
    
    public int linkNumber(){
        return linkArray.size();
    }
    
    /* *****************  METHODS RELATED TO INITIAL POSITIONS *************/
    
    public boolean hasInitialPositions(){
        return !xIC.isEmpty();
    }
    
    public void setAllInitialPositions(String [] xs, String [] ys, String [] zs){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        if(xs.length != ys.length || ys.length != zs.length){
            throw new RuntimeException("Unequal number of x,y,z initial positions, xs="+xs.length+", ys="+ys.length+", zs="+zs.length);
        } else {
            try{
                for(int i=0;i<xs.length;i++){
                    xIC.add(Double.parseDouble(xs[i]));
                    yIC.add(Double.parseDouble(ys[i]));
                    zIC.add(Double.parseDouble(zs[i]));
                }
            } catch(NumberFormatException nfe){
                throw new RuntimeException("Could not interpret all initial conditions as doubles.", nfe);
            }
        }
        // </editor-fold>
    }
    
    
    /********************************************************************\
     *                   CREATE NEW INSTANCE                            *
     * The instance is created at the specified position.               *
     * @param id_mol                                                    *
     * @param pos                                                       *
     * @return                                                          *
     \*******************************************************************/
    
    public Molecule newInstance(int id_mol, MyVector pos){
        // <editor-fold defaultstate="collapsed" desc="Method Code">  
        Molecule mol =  new Molecule(id_mol, gmolID, name, location, is2d);
        // The coordinates of membrane molecules must be shifted so that the 
        // anchors lie on the membrane. 
        double zshift = 0;
        // For positioning, might as well set one of the molecules sites at x=0, y=0,
        // otherwise we'd have to shift the range of the random vector generator.
        double yshift = 0;
        double xshift = 0;
        if(this.getLocation() == Location.MEMBRANE){
            for (GSite gs : siteArray) {
                if(gs.getLocation() == Location.MEMBRANE){
                    zshift = gs.getZ();
                    yshift = gs.getY();
                    xshift = gs.getX();
                }
            }
        } else {
            // Even if it's not on the membrane it helps to shift one of the 
            // sites so it's centered on the origin.  This makes it more 
            // likely that the random vector generator will place the sites uniformly
            // throughout the system.  Just grab site at index 0.
            zshift = siteArray.get(0).getZ();
            yshift = siteArray.get(0).getY();
            xshift = siteArray.get(0).getX();
        }
        for(int i=0;i<siteArray.size();i++){
            GSite gsite = siteArray.get(i);
            // lg.info("In gmolecule newInstance(): gsite initial state is " + gsite.getInitialState() + " with id " + gsite.getInitialState().getIdAsString());
            // The site id will be the molecule id appended with three digits
            // THIS LIMITS MOLECULES TO NO MORE THAN 1000 SITES, WHICH IS 
            // SUFFICIENT FOR THE FORESEEABLE FUTURE.
            
            Site site = gsite.newInstance(IOHelp.appendNumber(id_mol, i, 4), mol);
            site.setState(gsite.getInitialState());
            
            double tx;
            double ty;
            double tz;
            if(initialConditionToUse < xIC.size()){
                tx = xIC.get(initialConditionToUse) + gsite.getX() - xshift;
                ty = yIC.get(initialConditionToUse) + gsite.getY() - yshift;
                tz = zIC.get(initialConditionToUse) + gsite.getZ() - zshift;
            } else {
                tx = pos.x + gsite.getX() - xshift;
                ty = pos.y + gsite.getY() - yshift;
                tz = pos.z + gsite.getZ() - zshift;
            }
            site.setPosition(tx, ty, tz);
            site.setInitialPosition(tx, ty, tz);
            site.setState(gsite.getInitialState());
            mol.addSite(site);
        }
        initialConditionToUse++;
        
        for (GLink glink : linkArray) {
            GSite gs1 = glink.getSite1();
            GSite gs2 = glink.getSite2();
            int i1 = siteArray.indexOf(gs1);
            int i2 = siteArray.indexOf(gs2);
            Site s1 = mol.getSite(i1);
            Site s2 = mol.getSite(i2);
            Link link = new Link(s1, s2);
            link.setSpringConstant(MySystem.SpringConstant);  // was Math.pow(10,8), corrected for D
            mol.addLink(link);
        }
        return mol;
        // </editor-fold>
    }
    
    /*******************************************************************\
     *                     METHOD TO ASSIGN TYPE IDS                  *
    \*******************************************************************/
   
    public void assignTypeandStateIDs(){
        // Sitetype ids can go up to 999, so use a padding of 3.
        for(int i=0;i<typeArray.size();i++){
            GSiteType type = typeArray.get(i);
            type.setID(IOHelp.appendNumber(gmolID, i, 3));
            
            type.assignStateIDs();
        }
    }
    
    
    /* ***************************************************************\
     *                    FILE IO METHODS                            *
    \*****************************************************************/


    public static GMolecule readMolecule(Scanner sc){
        // <editor-fold defaultstate="collapsed" desc="Method Code">  
        GMolecule tempMol = new GMolecule("TempName");
        // Read all of the lines into an arraylist
        ArrayList<String> line = new ArrayList<>();
        String checkLine = sc.nextLine();
        while(!checkLine.equals("}")){
            line.add(checkLine);
            checkLine = sc.nextLine();
        }
        for(int i=0;i<line.size();i++){
            lg.trace("Line " + i + ": " + line.get(i));
        }
        
        Scanner sc0 = new Scanner(line.get(0)); 
        // check input line
//        if(!sc0.next().equals("MOLECULE:")){
//            // we'll throw some exception eventually but for now we want to log as much as possible of what went wrong
//            lg.error("ERROR: Molecule scanner does not begin with \"MOLECULE:\"");
//        }
        
        tempMol.setName(IOHelp.getNameInQuotes(sc0));
        tempMol.setLocation(sc0.next());
        
        // Now get the total number of these molecules
        if(sc0.next().equals("Number")){
            tempMol.setNumber(sc0.nextInt());
        } else {
            lg.error("Could not read the total number of molecules.");
        }
        // Now get the total types, sites, and links
        int totalTypes = -1;
        if(sc0.next().equals("Site_Types")){
            totalTypes = sc0.nextInt();
        } else {
            lg.error("Could not read total types.");
        }
        int totalSites = -1;
        if(sc0.next().equals("Total_Sites")){
            totalSites = sc0.nextInt();
        } else {
            lg.error("Could not read number of total sites.");
        }
        int totalLinks = -1;
        if(sc0.next().equals("Total_Links")){
            totalLinks = sc0.nextInt();
        } else {
            lg.error("Could not read total number of links.");
        }
        if(sc0.next().equals("is2D")){
            tempMol.set2D(sc0.nextBoolean());
        } else {
            lg.error("Could not read is2D flag.");
        }
        // check input file one last time
        if(!line.get(1).equals("{")){
            lg.error("ERROR: Molecule scanner did not find opening \"{\"");
        }
        
        // now read in the site types
        ArrayList<GSiteType> types = new ArrayList<>();
        for(int i=0;i<totalTypes;i++){
            types.add(GSiteType.readType(tempMol, line.get(i+2)));
        }
        for(int i=0;i<types.size();i++){
            lg.trace(types.get(i).getName());
        }
        
        // Now read in the sites
        ArrayList<GSite> sites = new ArrayList<>();
        GSite tempSite = null;
        int siteIndex;
        Scanner sc1;
        String tempName;
        String [] siteString = new String[3];
        for(int i=3+totalTypes;i<3+totalTypes+3*totalSites;i+=3){
            siteString[0] = line.get(i);
            siteString[1] = line.get(i+1);
            siteString[2] = line.get(i+2);
            for(int j=0;j<3;j++){
                lg.trace("Grabbing line " + i);
                lg.trace("siteString [" + j + "] = " + siteString[j]);
            }
            // Get the site index
            sc1 = new Scanner(siteString[0]);
            sc1.next();
            siteIndex = sc1.nextInt();
            // Get the location
            sc1.next();
            String loc = sc1.next();
            // Get the initial state
            sc1.next(); sc1.next(); sc1.next();
            String initialStateName = IOHelp.getNameInQuotes(sc1);
            lg.trace("siteIndex = " + siteIndex);
            // Get the type name
            sc1 = new Scanner(siteString[1]);
            sc1.next();
            sc1.next();
            
            tempName = IOHelp.getNameInQuotes(sc1);
            for (GSiteType type : types) {
                String tName = type.getName();
                //lg.trace("types[" + j + "] has name " + tName);
                lg.trace("Temp name is " + tempName);
                boolean match = tName.equals(tempName);
                lg.trace("The name match? " + match);
                if (tempName.equals(tName)) {
                    tempSite = new GSite(tempMol, type);
                    break;
                }
            }
            // Make sure the tempsite has been initialized at this point
            if(tempSite == null){
                lg.error("ERROR: Did not initialize the site with a type!");
                break;
            }
            tempSite.setLocation(loc);
            tempSite.setInitialState(tempSite.getType().getState(initialStateName));
            // Now get the x,y, and z coordinates of the site
            sc1 = new Scanner(siteString[2]);
            while(sc1.hasNext()){
                String var = sc1.next();
                switch(var){
                    case "x":{ tempSite.setX(sc1.nextDouble());
                    break;
                    }
                    case "y":{ tempSite.setY(sc1.nextDouble());
                    break;
                    }
                    case "z":{ tempSite.setZ(sc1.nextDouble());
                    break;
                    }
                    default:{ lg.error("ERROR: Couldn't read (x,y,z) values.");
                    }
                }
            }
            tempSite.setIndex(siteIndex);
            // Now add the site to the site array
            sites.add(tempSite);
            
        }
        for(int k = 0;k<sites.size();k++){
            lg.trace("Index of site " + k + " is " + sites.get(k).getIndex());
        }
        
        // Finally, we add the links
        ArrayList<GLink> links = new ArrayList<>();
        GLink tempLink;
        GSite site1;
        GSite site2;
        Scanner sc2;
        int i1;
        int i2;
        for(int i=4+totalTypes+3*totalSites;i<4+totalTypes + 3*totalSites + totalLinks; i++){
            site1 = null;
            site2 = null;
            sc2 = new Scanner(line.get(i));
            sc2.next();
            sc2.next();
            i1 = sc2.nextInt();
            sc2.next();
            sc2.next();
            i2 = sc2.nextInt();
            for (GSite site : sites) {
                //lg.trace("GSite " + j + " index is " + sites.get(j).getIndex() );
                if (i1 == site.getIndex()) {
                    site1 = site;
                }
                if (i2 == site.getIndex()) {
                    site2 = site;
                }
                if(site1 != null && site2 != null){
                    break;
                }
            }
            tempLink = new GLink(site1,site2);
            links.add(tempLink);
        }
        
        // Now we add  all of this information to the molecule!
        tempMol.setLinkArray(links);
        tempMol.setSiteArray(sites);
        tempMol.setTypeArray(types);
        
        int initPosLine = 5+totalTypes + 3*totalSites + totalLinks;
        // For backwards compatibility, we check if we actually have the initial
        // position flag.
        if(line.size() > initPosLine){
            Scanner ipScanner = new Scanner(line.get(initPosLine));
            ipScanner.next();
            if(ipScanner.next().equals(RANDOM)){
                // Do nothing here
            } else {
                Scanner xScanner = new Scanner(line.get(initPosLine+1));
                xScanner.useDelimiter(":");
                xScanner.next();
                String [] xs = xScanner.next().trim().split(" ");
                xScanner.close();

                Scanner yScanner = new Scanner(line.get(initPosLine+2));
                yScanner.useDelimiter(":");
                yScanner.next();
                String [] ys = yScanner.next().trim().split(" ");
                yScanner.close();

                Scanner zScanner = new Scanner(line.get(initPosLine+3));
                zScanner.useDelimiter(":");
                zScanner.next();
                String [] zs = zScanner.next().trim().split(" ");
                zScanner.close();

                tempMol.setAllInitialPositions(xs, ys, zs);
            }
            ipScanner.close();
        } else {
            // Do nothing
        }
        
        return tempMol;
        // </editor-fold>
    }
    
    public static ArrayList<GMolecule> loadMolecules(String moleculeData){
        // <editor-fold defaultstate="collapsed" desc="Method Code">  
        // BREAK UP THE STRING AT THE WORD 'MOLECULE'
        Scanner sc = new Scanner(moleculeData);
        sc.useDelimiter("MOLECULE:");
        ArrayList<String> moleculeStrings = new ArrayList<>();
        while(sc.hasNext()){
            moleculeStrings.add(sc.next());
        }
        ArrayList<GMolecule> molecules = new ArrayList<>();
        for(String data : moleculeStrings){
            GMolecule mol = GMolecule.readMolecule(new Scanner(data));
            molecules.add(mol);
        }
        return molecules;
        // </editor-fold>
    }
    
    public void writeMolecule(PrintWriter p){
        // <editor-fold defaultstate="collapsed" desc="Method Code">   
        p.println("MOLECULE: \"" + this.getName() + "\" " + location + " Number " + this.getNumber() + " Site_Types " + this.typeNumber() + " Total"
                + "_Sites " + this.siteNumber() + " Total_Links " + this.linkNumber());
        p.println("{");
        for (GSiteType typeArray1 : typeArray) {
            p.print("     ");
            typeArray1.writeType(p);
        }
        p.println();
        for (GSite siteArray1 : siteArray) {
            p.print("     ");
            siteArray1.writeSite(p);
        }
        p.println();
        for (GLink linkArray1 : linkArray) {
            p.print("     ");
            linkArray1.writeLink(p);
        }
        p.println();
        p.print("     Initial_Positions: ");
        if(xIC.isEmpty()){
            p.println(RANDOM);
        } else {
            p.println(SET);
            p.println("     x: " + IOHelp.printArrayList(xIC, 3));
            p.println("     y: " + IOHelp.printArrayList(yIC, 3));
            p.println("     z: " + IOHelp.printArrayList(zIC, 3));
        }
        p.println("}");
        p.println();
        // </editor-fold> 
    }
    
}
