/**
 * After much thought, I've decided that the information for all of the
 * bimolecular reactions will be stored in a single class.  This class will
 * have a hashmap between pairs of state ids (as strings) and a rate constant,
 * and will provide methods for determining if a reaction occurs.
 * It will also provide a method to build the new bond.
 *
 */

package edu.uchc.cam.langevin.reaction;

import edu.uchc.cam.langevin.counter.ReactionCounter;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.g.reaction.GBindingReaction;
import java.util.ArrayList;
import java.util.HashMap;
import edu.uchc.cam.langevin.helpernovis.Rand;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class BindingReactions {

    public static final Logger lg = LogManager.getLogger(BindingReactions.class);

    private final HashMap<String, Boolean> hasReaction;
    // I'm just going to store the product lambda*dt, which is the actual quantity of interest.
    private final HashMap<String, Double> onProbs;
    // As above, here I'll store koff*dt, which is the actual quantity of interest.
    private final HashMap<String, Double> offProbs;
    // Store the names of each reaction
    private final HashMap<String, String> reactionNames;
    // Store the bond lengths
    private final HashMap<String, Double> bondLengths;

    private ReactionCounter reactionCounter;

    /**
     * The constructor will be given arraylists of all the molecules and
     * all of the binding reactions.  The former will be used to make pairs
     * of all of the possible states, using the latter to decide if those
     * states actually react. If they do it adds to the reaction rate array.
     *
     * Why don't we just pass it the relevant global.  That's easier.
     * @param g The global class for this run.
     */

    public BindingReactions(Global g, MySystem sys) {
        ArrayList<GMolecule> gmolecules = g.getMolecules();
        ArrayList<GBindingReaction> reactions = g.getBindingReactions();
        double dt = g.getdt();
        // Make a list of every state in this system
        GMolecule mol;
        GSiteType type;
        GState state;
        ArrayList<GSiteType> allTypes = new ArrayList<>();
        ArrayList<GState> allStates = new ArrayList<>();
        for(int i=0;i<gmolecules.size();i++){
            mol = gmolecules.get(i);
            ArrayList<GSiteType> types = mol.getTypeArray();
            for(int j=0;j<types.size();j++){
                type = types.get(j);
                allTypes.add(type);
                ArrayList<GState> states = type.getStates();
                for(int k=0;k<states.size();k++){
                    state = states.get(k);
                    allStates.add(state);
                }
            }
        }

        int totalStates  = allStates.size();
        // We will have totalStates*totalStates possible pairs, so we should
        // initialize our hashmaps to be at least that size.  I'll initialize
        // them to be 10x that size. I'm only ever going to make one copy of
        // these hashmaps, and I'm not ever going to iterate over these hashmaps,
        // so their size won't affect performance. However, I want to get
        // info very quickly, so I want to really try to avoid collisions.

        hasReaction = new HashMap<>(10*totalStates*totalStates);

        // Actually, the rates hashmap only has to hold info proportational
        // to 2x the number of binding reactions, so we can initialize that
        // to the (usually) much smaller number of binding reactions.
        int totalReactions = reactions.size();
        onProbs = new HashMap<>(10*totalReactions);
        offProbs = new HashMap<>(10*totalReactions);
        reactionNames = new HashMap<>(10*totalReactions);
        bondLengths = new HashMap<>(10*totalReactions);
        // Now loop through all of the siteypes to see if they are involved
        // in any binding reactions.  We have to loop through the types first
        // because sometimes the "state" could be ANY_STATE_STRING, in which case we'd have
        // to add all of the states to the reaction hashmap.

        GSiteType type1;
        GSiteType type2;
        GState state1;
        GState state2;
        GBindingReaction reaction;
        for(int i=0;i<allTypes.size();i++){
            type1 = allTypes.get(i);
            ArrayList<GState> states1 = type1.getStates();
            for(int j=0;j<allTypes.size();j++){
                type2 = allTypes.get(j);
                ArrayList<GState> states2 = type2.getStates();
                for(int k=0;k<states1.size();k++){
                    state1 = states1.get(k);
                    String key1 = state1.getIdAsString();
                    for(int l=0;l<states2.size();l++){
                        state2 = states2.get(l);
                        String key2 = state2.getIdAsString();
                        // Now loop through the binding reactions
                        boolean foundReaction = false;
                        for(int m=0;m<reactions.size();m++){
                            reaction = reactions.get(m);
                            GSiteType [] reactionTypes = reaction.getTypes();

                            if(reactionTypes[0] == type1 && reactionTypes[1] == type2){
                                GState [] reactionState = reaction.getStates();
                                if(reactionState[0].getStateName().equals(GBindingReaction.ANY_STATE_STRING) && reactionState[1].getStateName().equals(GBindingReaction.ANY_STATE_STRING)){
                                    foundReaction = true;
                                    addReaction(key1,key2,reaction,dt);
                                    break;
                                }
                                else if(reactionState[0].getStateName().equals(GBindingReaction.ANY_STATE_STRING) && reactionState[1] == state2){
                                    foundReaction = true;
                                    addReaction(key1,key2,reaction,dt);
                                    break;
                                }
                                else if(reactionState[0] == state1 && reactionState[1].getStateName().equals(GBindingReaction.ANY_STATE_STRING)){
                                    foundReaction = true;
                                    addReaction(key1,key2,reaction,dt);
                                    break;
                                }
                                else if(reactionState[0] == state1 && reactionState[1] == state2){
                                    foundReaction = true;
                                    lg.trace("Found reaction between " + state1.getAbsoluteName() + " and " + state2.getAbsoluteName());
                                    lg.trace("State ids " + key1 + ", " + key2);
                                    addReaction(key1,key2,reaction,dt);
                                    break;
                                }
                            }
                        }
                        if(!foundReaction){
                            lg.trace("did not find reaction between " + state1.getAbsoluteName() + " and " + state2.getAbsoluteName());
                            // Because of the way I'm looping and assigning the
                            // reactions, it can occur that a reaction could
                            // get assigned here twice, which would override
                            // a value of true when it shouldn't.
                            Boolean bool = hasReaction.get(key1 + key2);
                            if(bool == null){
                                hasReaction.put(key1+key2, false);
                            }
                        }
                    }
                }
            }
        }
    }

    public void setReactionCounter(MySystem sys){
        this.reactionCounter = sys.getReactionCounter();
    }

    //  Helper function to add a reaction
    private void addReaction(String key1, String key2, GBindingReaction r, double dt){

        double onProbOld = r.getLambda()*dt;
        double onProbNew = 1.0 - Math.pow(Math.E, -(r.lambdaNew*dt));       // lambda dt -->  1 - e pow(-lambda dt)
        double offProbOld = r.getkoff()*dt;
        double offProbNew = 1.0 - Math.pow(Math.E, -(r.kOffIntrinsic*dt));
        lg.trace(r.getName() + ": onProbability: " + onProbOld + ", " + onProbNew);
        lg.trace(r.getName() + ": offProbability: " + offProbOld + ", " + offProbNew);
        if (Double.isNaN(offProbOld) || Double.isInfinite(offProbOld)) {    // not used, but we may want to investigate
            lg.debug("Invalid offProbability (old) " + offProbOld + " for reaction '" + r.getName() + "'");
        }
        if (Double.isNaN(offProbNew) || Double.isInfinite(offProbNew)) {
            throw new IllegalStateException("Invalid offProbability (new) " + offProbNew + " for reaction '" + r.getName());
        }

        hasReaction.put(key1+key2, Boolean.TRUE);
        // onProbs.put(key1+key2, r.getLambda()*dt);
        onProbs.put(key1+key2, onProbNew);
//        offProbs.put(key1+key2,r.getkoff()*dt);
        offProbs.put(key1+key2,offProbNew);
        reactionNames.put(key1+key2, r.getName());
        bondLengths.put(key1+key2, r.getBondLength());

        hasReaction.put(key2+key1, Boolean.TRUE);
//        onProbs.put(key2+key1, r.getLambda()*dt);
        onProbs.put(key2+key1, onProbNew);
//        offProbs.put(key2+key1,r.getkoff()*dt);
        offProbs.put(key2+key1,offProbNew);
        reactionNames.put(key2+key1, r.getName());
        bondLengths.put(key2+key1, r.getBondLength());
    }

    public boolean doReact(GState state1, GState state2){
        String key1 = state1.getIdAsString();
        String key2 = state2.getIdAsString();
        Boolean res = hasReaction.get(key1 + key2);
        if (res == null) {
            String fullName1 = state1.getAbsoluteNameBngl();
            String fullName2 = state2.getAbsoluteNameBngl();
            throw new IllegalStateException(
                    "Reaction lookup failed: no entry for key pair " + key1 + ", " + key2 + ". " +
                    "Full names: " + fullName1 + ", " + fullName2 + "."
            );
        }
        return res;
    }

    public double getOnProb(String key1, String key2){
        return onProbs.get(key1+key2);
    }

    public double getOffProb(String key1, String key2){
        Double d =  offProbs.get(key1+key2);
        // If the user did not define a dissociation rate between these two
        // states, then we assume they intended for them to not dissociate.
        if(d == null){
            return 0;
        } else {
            return d;
        }
    }

    public String getName(String key1, String key2){
        return reactionNames.get(key1+key2);
    }

    public Double getBondLength(String key1, String key2){
        return bondLengths.get(key1+key2);
    }

    //**  A function to check for a reaction between two states
    public boolean checkForReaction(String key1, String key2){
        double rand = Rand.randomPosDouble();
        return rand < onProbs.get(key1+key2);
    }


}
