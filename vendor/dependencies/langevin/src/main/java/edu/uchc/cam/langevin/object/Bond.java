/*
 * While this class isn't absolutely necessary, it's nice to distinguish
 * between links which are bonds and those that aren't.
 */

package edu.uchc.cam.langevin.object;

import edu.uchc.cam.langevin.helpernovis.Rand;

public class Bond extends Link {
    
    /* **********  Name of reaction whose off-rate regulates this bond ******/
    // This can change if the binding partners change their internal states
    private String reactionName;
    
    /*  The off-rate from the reaction defined by the current site states ***/
    private double offProb;  // Probability of dissociation per time step.
    
    /* For testing it helps if each bond is given a unique id ****************/
    private final long id;
    public static long nextID = 0;

    public Bond(Site site1, Site site2, double offProb, String name, double bondLength){
        super(site1, site2);
        this.offProb = offProb;
        reactionName = name;
        // All new bonds have length 0.1 nm greater than the sum of the physical radii
        super.setL0(site1.getRadius() + site2.getRadius() + bondLength);
        
        this.id = nextID;
        nextID++;
    }
    
    public Bond(Site site1, Site site2, double springConstant, double offProb, String name, double bondLength){
        super(site1, site2, springConstant);
        this.offProb = offProb;
        this.reactionName = name;
        // All new bonds have length 0.1 nm greater than the sum of the physical radii
        super.setL0(site1.getRadius() + site2.getRadius() + bondLength);
        
        this.id = nextID;
        nextID++;
    }
    
    /* ******************* SET AND GET THE NAME ***************************/
    
    public String getName(){
        return reactionName;
    }
    
    public void setReactionName(String reactionName){
        this.reactionName = reactionName;
    }
    
    /* ****************** SET AND GET THE DISSOCIATION RATE **************/
    
    public void setOffProbability(double rate, double dt){
        offProb = rate*dt;
    }
    public void setOffProbability(double offProb){
        this.offProb = offProb;
    }
    public double getOffProbability() {
        return offProb;
    }
    
    /* ****************** SET THE BOND LENGTH ***********************/
    
    public void setBondLength(double length){
        Site [] site = super.getSites();
        super.setL0(site[0].getRadius() + site[1].getRadius() + length);
    }
    
    /* *****************  GET THE ID ********************************/
    public long getID(){
        return id;
    }
    
    /* ****************** LOOK TO SEE IF BOND DISSOCIATES ***************/
    
    public boolean dissociates(){
        double rand = Rand.randomPosDouble();
        return rand < offProb;
    }

    
}
