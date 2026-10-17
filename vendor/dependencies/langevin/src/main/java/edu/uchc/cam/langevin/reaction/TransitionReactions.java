/*
 * Model this class on the binding reaction class.  Here we can use a 
 * state id from the initial state to map to a transitionreaction, and then 
 * get the condition, rates, etc from that transition reaction.
 */

package edu.uchc.cam.langevin.reaction;

import edu.uchc.cam.langevin.counter.ReactionCounter;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Bond;
import edu.uchc.cam.langevin.object.Site;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.g.reaction.GTransitionReaction;
import java.util.ArrayList;
import java.util.HashMap;
import edu.uchc.cam.langevin.helpernovis.Rand;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class TransitionReactions {

    public static final Logger lg = LogManager.getLogger(TransitionReactions.class);

    // Check to see if we even have a reaction. As many states won't have a
    // this check should be faster than calling ArrayList.isEmpty. 
    
    private final HashMap<Integer, Boolean> hasReaction;
    
    private final HashMap<Integer, ArrayList<GTransitionReaction>> reactionMap;
    
    private final double dt;
    
    private final BindingReactions bindingReactions;

    private ReactionCounter reactionCounter;
    
    public TransitionReactions(Global g, MySystem sys, BindingReactions bindingReactions){
        ArrayList<GTransitionReaction> reactions = g.getTransitionReactions();
        ArrayList<GState> allStates = new ArrayList<>();
        this.dt = g.getdt();
        this.bindingReactions = bindingReactions;
        
        for(GMolecule gmolecule : g.getMolecules()){
            for(GSiteType gtype : gmolecule.getTypeArray()){
                for(GState gstate : gtype.getStates()){
                    allStates.add(gstate);
                }
            }
        }
        
        hasReaction = new HashMap<>(10*allStates.size());
        reactionMap = new HashMap<>(10*allStates.size());
        
        // First populate the reaction map with arraylists and set the reaction flags to false
        for(GState gstate : allStates){
            hasReaction.put(gstate.getID(), Boolean.FALSE);
            reactionMap.put(gstate.getID(), new ArrayList<GTransitionReaction>());
        }
        
        // Now populate the reaction arrays based on the initial states
        for(GTransitionReaction reaction : reactions){
            hasReaction.put(reaction.getInitialState().getID(), Boolean.TRUE);
            ArrayList<GTransitionReaction> tempReactions = reactionMap.get(reaction.getInitialState().getID());
            tempReactions.add(reaction);
        }
    }

    public void setReactionCounter(MySystem sys){
        this.reactionCounter = sys.getReactionCounter();
    }
    
    public void tryReactions(Site site){
        GState state = site.getState();
        if(hasReaction.get(state.getID())){
            ArrayList<GTransitionReaction> reactions = reactionMap.get(state.getID());
            // Now loop through the reactions
            boolean outerbreak = false;
            for(GTransitionReaction reaction : reactions){
                if(outerbreak){
                    break;
                }
                switch(reaction.getConditionID()){
                    // If there is no condition, then just try the reaction
                    case GTransitionReaction.NONE:{
                        if(reactionOccurs(reaction.getRate())){
                            reactionCounter.plusTransitionReaction(reaction.getName());
                            site.setState(reaction.getFinalState());
                            if(site.isBound()){
                                updateBondType(site);
                            }
                            outerbreak = true;
                        }
                        break;
                    }
                    // Now try the reactions with unbound (free) conditions
                    case GTransitionReaction.FREE:{
                        if(!site.isBound()){
                            if(reactionOccurs(reaction.getRate())){
                                reactionCounter.plusTransitionReaction(reaction.getName());
                                site.setState(reaction.getFinalState());
                                outerbreak = true;
                            }
                        }
                        break;
                    }
                    // Now try the reactions which only occur when a site is bound
                    case GTransitionReaction.BOUND:{
                        if(site.isBound()){
                            if(site.getBindingPartner().getTypeID() == reaction.getConditionalType().getID()){
                                // See if the reaction can occur regardless of the state of the binding partner
                                if(reaction.getConditionalState().getID() == GTransitionReaction.ANY_STATE_ID){
                                    if(reactionOccurs(reaction.getRate())){
                                        reactionCounter.plusTransitionReaction(reaction.getName());
                                        site.setState(reaction.getFinalState());
                                        updateBondType(site);
                                        outerbreak = true;
                                    }
                                }
                                // If the reaction needs a specific state, look to see if we have it
                                else if(site.getBindingPartner().getState().getID() == reaction.getConditionalState().getID()){
                                    if(reactionOccurs(reaction.getRate())){
                                        reactionCounter.plusTransitionReaction(reaction.getName());
                                        site.setState(reaction.getFinalState());
                                        updateBondType(site);
                                        outerbreak = true;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default:
                        // Do nothing
                }
            }
        }
    }

    // we changed the state of a site that is part of the bond
    // there may be another binding reaction (dissociation) for which this complex is now a participant
    // if there isn't any, we keep the same bond type (which is a lie, but it is better than using a null reaction name
    // which would crash the counter logic)
    private void updateBondType(Site site){
        String id = Integer.toString(site.getState().getID());
        String partnerID = Integer.toString(site.getBindingPartner().getState().getID());
        Bond bond = site.getBond();
        if(bindingReactions.getName(id, partnerID) != null) {
            bond.setReactionName(bindingReactions.getName(id, partnerID));
        }
        bond.setOffProbability(bindingReactions.getOffProb(id, partnerID));
        Double bondLength = bindingReactions.getBondLength(id, partnerID);
        if (bondLength != null) {
            bond.setBondLength(bondLength);
        }
        if (lg.isDebugEnabled()) {
            Site s0 = bond.getSites()[0];
            Site s1 = bond.getSites()[1];
            String who = s0.getType() + ";" + s0.getState().getStateName() +
                    " and " + s1.getType() + ";" + s1.getState().getStateName();
            double koff = bindingReactions.getOffProb(id, partnerID);
            lg.debug("Transition reaction occurred, new bond for " + bond.getName() + " koff: " + koff + " between " + who);
        }
    }
    
    private boolean reactionOccurs(double rate){
        return Rand.randomPosDouble() < dt*rate;
    }
    
}
