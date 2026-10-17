/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.reaction;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.util.Scanner;
import java.util.ArrayList;
import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GTransitionReaction implements  GReactionInterface {

    public static final Logger lg = LogManager.getLogger(GTransitionReaction.class);

    private String name;
    
    private GMolecule molecule;
    private GSiteType type;
    private GState initialState;
    private GState finalState;
    
    // Conditional variables
    private GMolecule conditionalMolecule;
    private GSiteType conditionalType;
    private GState conditionalState;
    
    // The conditional state could be any state of the conditional type
    public final static String ANY_STATE_STRING = "Any_State";
    public final static GState ANY_STATE = new GState(null, ANY_STATE_STRING);
    public final static int ANY_STATE_ID = -100;
    static {
        ANY_STATE.setID(ANY_STATE_ID);
    }
    
    // There might be no conditions on the reaction, or maybe it must be free, etc.
    public final static String NO_CONDITION = "None";
    public final static String FREE_CONDITION = "Free";
    public final static String BOUND_CONDITION = "Bound";
    
    // The condition on this reaction
    private String condition; 
    
    // Give a separate representation of the condition as an integer
    public final static int NONE = 0;
    public final static int FREE = 1;
    public final static int BOUND = 2;
    
    private int conditionID;
    
    // Each transition reaction has a single rate
    private double rate;  // Units s-1
    
    public GTransitionReaction(){
        this.name = null;
        this.molecule = null;
        this.type = null;
        
        conditionalMolecule = null;
        conditionalType = null;
        conditionalState = null;
        condition = NO_CONDITION;
    }
    
    /* ********* GET AND SET THE REACTION NAME ***************/
    
    public void setName(String name){
        this.name = name;
    }

    @Override
    public String getName(){
        return name;
    }
    
    @Override
    public String toString(){
        return name;
    }
    
    /* *********   GET AND SET THE MOLECULE *****************/
    
    public void setMolecule(GMolecule molecule){
        this.molecule = molecule;
    }
    
    public GMolecule getMolecule(){
        return molecule;
    }

    /* ********** GET AND SET THE SITE TYPE *********************/
    
    public void setType(GSiteType type){
        this.type = type;
    }
    
    public GSiteType getType(){
        return type;
    }
    
    /* ********* GET AND SET THE INITIAL AND FINAL STATES ***********/
    
    public void setInitialState(GState state){
        initialState = state;
    }
    
    public void setFinalState(GState state){
        finalState = state;
    }
    
    public GState getInitialState(){
        return initialState;
    }
    
    public GState getFinalState(){
        return finalState;
    }
    
    /* ******** GET AND SET THE CONDITIONALS ************************/
    
    public void setConditionalMolecule(GMolecule molecule){
        conditionalMolecule = molecule;
    }
    
    public void setConditionalType(GSiteType type){
        conditionalType = type;
    }
    
    public void setConditionalState(GState state){
        conditionalState = state;
    }
    
    public GMolecule getConditionalMolecule(){
        return conditionalMolecule;
    }
    
    public GSiteType getConditionalType(){
        return conditionalType;
    }
    
    public GState getConditionalState(){
        return conditionalState;
    }
    
    
    /* ********* GET AND SET THE REACTION CONDITION *********************/
    
    public void setCondition(String condition){
        switch(condition){
            case NO_CONDITION:{
                this.condition = condition;
                conditionID = NONE;
                break;
            }
            case FREE_CONDITION:{
                this.condition = condition;
                conditionID = FREE;
                break;
            }
            case BOUND_CONDITION:{
                this.condition = condition;
                conditionID = BOUND;
                break;
            }
            default:
                throw new IllegalArgumentException("GTransitionReaction.setCondition received unexpected input: " + condition);
        }
        lg.trace("Set condition to " + this.condition + ", with ID " + conditionID);
    }
    
    public void setConditionID(){
        switch(condition){
            case NO_CONDITION:
                conditionID = NONE;
                break;
            case FREE_CONDITION:
                conditionID = FREE;
                break;
            case BOUND_CONDITION:
                conditionID = BOUND;
                break;
            default:
                throw new IllegalArgumentException(
                        "GTransitionReaction.setCondition received unexpected input: " + condition
                );
        }
    }
    
    public String getCondition(){
        return condition;
    }
    
    public int getConditionID(){
        return conditionID;
    }
       
    /* ************ GET AND SET THE REACTION RATE **********************/
    
    public void setRate(double rate){
        this.rate = rate;
    }
    
    public double getRate(){
        return rate;
    }
    
    /* **************** LOAD SINGLE REACTION ****************************/
    
    public void loadReaction(Global g, Scanner dataScanner){
        name = IOHelp.getNameInQuotes(dataScanner);
        dataScanner.next();
        molecule = g.getGMolecule(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        type = molecule.getType(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        initialState = type.getState(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        finalState = type.getState(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        rate = dataScanner.nextDouble();
        dataScanner.next();
        condition = dataScanner.next();
        setConditionID();
        if(!condition.equals(BOUND_CONDITION)){
            conditionalMolecule = null;
            conditionalType = null;
            conditionalState = null;
        } else {
            conditionalMolecule = g.getGMolecule(IOHelp.getNameInQuotes(dataScanner));
            dataScanner.next();
            conditionalType = conditionalMolecule.getType(IOHelp.getNameInQuotes(dataScanner));
            dataScanner.next();
            String condState = IOHelp.getNameInQuotes(dataScanner);
            if(condState.equals(GTransitionReaction.ANY_STATE_STRING)){
                conditionalState = GTransitionReaction.ANY_STATE;
            } else {
                conditionalState = conditionalType.getState(condState);
            }
        }
    }
    
    /* *************** LOAD FULL ARRAY *********************************/
    
    public static ArrayList<GTransitionReaction> loadReactions(Global g, Scanner sc){
        ArrayList<GTransitionReaction> transitionReactions = new ArrayList<>();
        GTransitionReaction reaction;
        while(sc.hasNextLine()){
            reaction = new GTransitionReaction();
            reaction.loadReaction(g, new Scanner(sc.nextLine()));
            transitionReactions.add(reaction);
        }
        return transitionReactions;
    }
    
    /* ***************** PRINT A REACTION REPRESENTATION ******************/
    
    public String writeReaction(){
        StringBuilder sb = new StringBuilder();
        sb.append("'").append(name).append("' ::     ");
        if(molecule != null && type != null && initialState != null && finalState != null){
            sb.append("'").append(molecule.getName()).append("' : '");
            sb.append(type.getName()).append("' : '");
            sb.append(initialState.getStateName()).append("'");
            sb.append(" --> ");
            sb.append("'").append(finalState.getStateName()).append("' ");
            sb.append(" Rate ").append(Double.toString(rate)).append(" ");
            sb.append(" Condition ").append(condition);
            if(condition.equals(BOUND_CONDITION)){
                sb.append(" '").append(conditionalMolecule.getName()).append("' : '");
                sb.append(conditionalType.getName()).append("' : '");
                sb.append(conditionalState.getStateName()).append("'");
            }
        }
        return sb.toString();
    }

}