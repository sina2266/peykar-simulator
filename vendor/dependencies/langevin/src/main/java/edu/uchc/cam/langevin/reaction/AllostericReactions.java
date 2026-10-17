/*
 * Model this class on the transition reaction class.  Here we can use a 
 * gmolecule id (as string) and site number to map to an allostericreaction, 
 * and then get the states, rate, etc from that allostericreaction reaction.
 * 
 * The key is always gmoleculeID + gsiteIndex (both as strings).
 */


package edu.uchc.cam.langevin.reaction;

import edu.uchc.cam.langevin.counter.ReactionCounter;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import edu.uchc.cam.langevin.object.Bond;
import edu.uchc.cam.langevin.object.Site;
import edu.uchc.cam.langevin.object.Molecule;
import edu.uchc.cam.langevin.g.object.GSite;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.g.reaction.GAllostericReaction;
import java.util.ArrayList;
import java.util.HashMap;
import edu.uchc.cam.langevin.helpernovis.Rand;
import edu.uchc.cam.langevin.langevinnovis01.Global;

public class AllostericReactions {

    // Check to see if we even have a reaction. As many states won't have a
    // this check should be faster than calling ArrayList.isEmpty. 
    private final HashMap<String, Boolean> hasReaction;
    
    private final HashMap<String, ArrayList<GAllostericReaction>> reactionMap;
    
    private final double dt;
    
    private final BindingReactions bindingReactions;

    private ReactionCounter reactionCounter;
    
    public AllostericReactions(Global g, MySystem sys, BindingReactions bindingReactions){
        ArrayList<GAllostericReaction> reactions = g.getAllostericReactions();
        ArrayList<GSite> allSites = new ArrayList<>();
        this.dt = g.getdt();
        this.bindingReactions = bindingReactions;
        
        for(GMolecule gmolecule: g.getMolecules()){
            for(GSite gsite : gmolecule.getSites()){
                allSites.add(gsite);
            }
        }
        
        hasReaction = new HashMap<>(10*allSites.size());
        reactionMap = new HashMap<>(10*allSites.size());
        
        // Populate maps with arraylists and set reaction flags to false
        for(GMolecule gmolecule : g.getMolecules()){
            String gmolID = Integer.toString(gmolecule.getID());
            for(GSite gsite : gmolecule.getSites()){
                String siteIndex = Integer.toString(gsite.getIndex());
                String key = gmolID + siteIndex;
                hasReaction.put(key, Boolean.FALSE);
                reactionMap.put(key, new ArrayList<>());
            }
        }
        
        // Now populate the maps based on the allosteric reactions
        for(GAllostericReaction reaction : reactions){
            String molID = Integer.toString(reaction.getGMolecule().getID());
            String siteIndex = Integer.toString(reaction.getGSite().getIndex());
            String key = molID + siteIndex;
            hasReaction.put(key, Boolean.TRUE);
            ArrayList<GAllostericReaction> rxnList = reactionMap.get(key);
            rxnList.add(reaction);
        }
    }

    public void setReactionCounter(MySystem sys){
        this.reactionCounter = sys.getReactionCounter();
    }

    public void tryReactions(Site site){
        Molecule molecule = site.getMolecule();
        String molID = Integer.toString(molecule.getGID());
        String siteIndex = Integer.toString(site.getGSiteIndex());
        String key = molID + siteIndex;
        if(hasReaction.get(key)){
            ArrayList<GAllostericReaction> reactions = reactionMap.get(key);
            // Now loop through the reactions
            for(GAllostericReaction reaction : reactions){
                
                if(site.getState() == reaction.getInitialState()){
                    int allostericSiteIndex = reaction.getAllostericSite().getIndex();
                    Site allostericSite = molecule.getSiteFromGSiteIndex(allostericSiteIndex);
                    GState currentState = allostericSite.getState();
                    GState allostericState = reaction.getAllostericState();
                    
                    if(currentState == allostericState){
                        if(reactionOccurs(reaction.getRate())){
                            reactionCounter.plusAllostericReaction(reaction.getName());
                            site.setState(reaction.getFinalState());
                            if(site.isBound()){
                                updateBondType(site);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    private boolean reactionOccurs(double rate){
        return Rand.randomPosDouble() < dt*rate;
    }
    
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
    }
    
}
