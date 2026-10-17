/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.reaction;

import edu.uchc.cam.langevin.g.counter.GBondCounter;
import edu.uchc.cam.langevin.helpernovis.OnRateSolver;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.util.Scanner;
import java.util.ArrayList;

import edu.uchc.cam.langevin.g.object.GMolecule;
import edu.uchc.cam.langevin.g.object.GSiteType;
import edu.uchc.cam.langevin.g.object.GState;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class GBindingReaction implements GReactionInterface{

    public static final Logger lg = LogManager.getLogger(GBindingReaction.class);

    // Might want to name the reactions
    private String name;

    private final GMolecule [] molecule = new GMolecule[2];
    private final GSiteType [] type = new GSiteType[2];
    private final GState [] state = new GState[2];

    private double kon = 0.0; // Units uM-1.s-1
    private double lambdaOld = 0.0; // Units s-1. This is the rate used to actually compute the probability of reacting.
    public double lambdaNew = 0.0;
    private double koff = 0.0;  // Units s-1.  This is given directly to the bond.
    public double kOffIntrinsic = 0.0;

    // The bond length is the distance between the margins of the two sites when they are bound.
    // The effective distance between the centers of the two sites when they are bound
    // is bondLength + type[0].getRadius() + type[1].getRadius()
    private double bondLength = 0.5; // Units nm

    public final static String ANY_STATE_STRING = "Any_State";
    public final static GState ANY_STATE = new GState(null, ANY_STATE_STRING);
    public final static int ANY_STATE_ID = -10;



    // It really helps to be able to get the BondData object directly from the
    // binding reaction
    private final GBondCounter bondData;

    public GBindingReaction(){
        name = null;
        bondData = new GBondCounter(this);
        for(int i=0;i<2;i++){
            molecule[i] = null;
            type[i] = null;
            state[i] = null;
        }
    }

    public GBindingReaction(String name){
        this.name = name;
        bondData = new GBondCounter(this);
        for(int i=0;i<2;i++){
            molecule[i] = null;
            type[i] = null;
            state[i] = null;
        }
    }

    /* *************************************************************\
     *             DATA SPECIFIC TO THE REACTION                   *
     *  Reaction name and rates.                                   *
    \***************************************************************/

    public void setName(String name){
        this.name = name;
    }

    @Override
    public String getName(){
        return name;
    }

    public void setkon(double kon){
        this.kon = kon;
        setLambda();
    }

    public double getkon(){
        return kon;
    }

    public void setkoff(double koff){
        this.koff = koff;
    }

    public double getkoff(){
        return koff;
    }

    public double getLambda(){
        return lambdaNew;
    }

    private void setLambda() {
        lg.trace("BindingReaction: " + name + ", setLambda() called.");
        // conversion when going from concentration‑based kinetics to particle‑based / spatial stochastic kinetics
        // rescale kon from 1/(uM*s) to nm^3/s
        // 1uM = 10^-6 molecules/l, multiply by NA = 6.022e23 molecules/mol to get 6.022e17 molecules/liter
        // 1 liter = 10e24 nm^3 so 1uM = 6.022e17 / 1e24 = 6.022e-7 molecules/nm^3
        // 1/
        double rescalekon = kon*1660000.0;

        double p = type[0].getRadius() + type[1].getRadius();
        double R = type[0].getReactionRadius() + type[1].getReactionRadius();
        // Rescale site type diffusion rate D so it's in nm^2/s (from um^2/s)
        double D = 1000000.0 * (type[0].getD() + type[1].getD());

        // old formula, will keep the code for now, just in case
        // When a site reacts its own type, the rate should be rescaled to 2*kon
        if(type[0] != type[1]){
            if(koff == 0) {
                lambdaOld = OnRateSolver.getrootIrreversible(p, R, D, rescalekon);
            } else {
                lambdaOld = OnRateSolver.getrootReversible(p, p+bondLength, R, D, rescalekon);
            }
        } else {
            if(koff == 0) {
                lambdaOld = OnRateSolver.getrootIrreversible(p, R, D, 2.0 * rescalekon);
            } else {
                lambdaOld = OnRateSolver.getrootReversible(p, p+bondLength, R, D, 2.0 * rescalekon);
            }
        }

        // ----------------------------------------------------------------------------------------
        if(type[0] == type[1]) {
            rescalekon *= 2.0;
        }
        double volReact = 4.0 * Math.PI * (Math.pow(R,3) - Math.pow(p,3)) / 3.0;    // R=reaction radius, p=site radius
        double kD = 4 * Math.PI * R * D;        // R=reaction radius, D=diffusion rate
        double kOnIntrinsic = (rescalekon * kD) / (kD - rescalekon);    // may be negative if rescalekon > kD, which is non-physical
        if(kOnIntrinsic < 0.0) {
            double t0r=type[0].getRadius();
            double t1r=type[1].getRadius();
            double t0R=type[0].getReactionRadius();
            double t1R=type[1].getReactionRadius();
            double t0D=type[0].getD();
            double t1D=type[1].getD();

            throw new RuntimeException("Kon is too large, leading to non-physical negative intrinsic on-rate!\n"
                    + "Molecule 1: " + molecule[0].getName() + ", Type: " + type[0].getName() + ", Radius: " + t0r + " nm, Reaction Radius: " + t0R + " nm, D: " + t0D + " um^2/s\n"
                    + "Molecule 2: " + molecule[1].getName() + ", Type: " + type[1].getName() + ", Radius: " + t1r + " nm, Reaction Radius: " + t1R + " nm, D: " + t1D + " um^2/s\n"
                    + "Kon: " + kon + " uM^-1 s^-1");
        }
        lambdaNew = kOnIntrinsic / volReact;
        lg.trace("  Old Lambda = " + lambdaOld + ", New Lambda = " + lambdaNew);

        // if kon is 0 (pure dissociation), then the normal formula for kOffIntrinsic will give NaN
        // because we divide by rescalekon == 0.  In that case, we just set kOffIntrinsic = koff.
        if(kon == 0.0) {
            kOffIntrinsic = koff;
        } else {
            kOffIntrinsic = koff * kOnIntrinsic / rescalekon;
        }
    }

    public double getBondLength(){
        return bondLength;
    }

    public void setBondLength(double length){
        bondLength = length;
    }

    /* ********  GET AND SET THE MOLECULES ********************/

    public GMolecule [] getMolecules(){
        return molecule;
    }

    public GMolecule getMolecule(int i){
        return molecule[i];
    }

    public void setMolecules(GMolecule mol1, GMolecule mol2){
        molecule[0] = mol1;
        molecule[1] = mol2;
    }

    public void setMolecule(int i, GMolecule mol){
        molecule[i] = mol;
    }

    /* ********** GET AND SET THE SITE TYPES *****************/

    public GSiteType [] getTypes(){
        return type;
    }

    public GSiteType getType(int i){
        return type[i];
    }

    public void setTypes(GSiteType type1, GSiteType type2){
        type[0] = type1;
        type[1] = type2;
    }

    public void setType(int i, GSiteType type){
        this.type[i] = type;
    }

    /* ********* GET AND SET THE STATES *********************/

    public GState [] getStates(){
        return state;
    }

    public GState getState(int i){
        return state[i];
    }

    public void setStates(GState state1, GState state2){
        state[0] = state1;
        state[1] = state2;
    }

    public void setState(int i, GState state){
        this.state[i] = state;
    }

    /* ***** GET THE BONDDATA OBJECT ASSOCIATED WITH THIS REACTION ****/

    public GBondCounter getBondCounter(){
        return bondData;
    }

    /* ***************** LOAD SINGLE REACTION *************************/

    public void loadReaction(Global g, Scanner dataScanner){
        // <editor-fold defaultstate="collapsed" desc="Method Code">
        name = IOHelp.getNameInQuotes(dataScanner);
        molecule[0] = g.getGMolecule(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        type[0] = molecule[0].getType(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        String state0 = IOHelp.getNameInQuotes(dataScanner);
        if(state0.equals(GBindingReaction.ANY_STATE_STRING)){
            state[0] = GBindingReaction.ANY_STATE;
        } else {
            state[0] = type[0].getState(state0);
        }
        // Skip '+'
        dataScanner.next();
        molecule[1] = g.getGMolecule(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        type[1] = molecule[1].getType(IOHelp.getNameInQuotes(dataScanner));
        dataScanner.next();
        String state1 = IOHelp.getNameInQuotes(dataScanner);
        if(state1.equals(GBindingReaction.ANY_STATE_STRING)){
            state[1] = GBindingReaction.ANY_STATE;
        } else {
            state[1] = type[1].getState(state1);
        }
        // Skip 'kon'
        dataScanner.next();
        kon = dataScanner.nextDouble();
        // Skip 'koff'
        dataScanner.next();
        koff = dataScanner.nextDouble();
        // Skip "Bond_Length"
        dataScanner.next();
        bondLength = dataScanner.nextDouble();

        // MUST SET LAMBDA HERE.  I FORGOT TO DO THIS AT FIRST!
        setLambda();
        // </editor-fold>
    }

    /* ***************** LOAD FULL ARRAY *******************************/

    public static ArrayList<GBindingReaction> loadReactions(Global g, Scanner sc){
        ArrayList<GBindingReaction> reactions = new ArrayList<>();
        GBindingReaction reaction;
        while(sc.hasNextLine()){
            reaction = new GBindingReaction();
            reaction.loadReaction(g, new Scanner(sc.nextLine()));
            reactions.add(reaction);
        }
        return reactions;
    }

    /* ******   WRITE REACTION    ***********************/
    // I only need this for testing, I think.
    @Override
    public String toString(){
        return name;
    }

    public String writeReaction(){
        StringBuilder sb = new StringBuilder();
        sb.append("'").append(name).append("'       ");
        if(molecule[0] != null && molecule[1] != null){
            sb.append("'").append(molecule[0].getName()).append("' : '")
                    .append(type[0].getName()).append("' : '")
                    .append(state[0].toString());
            sb.append("'  -->  '");
            sb.append(molecule[1].getName()).append("' : '")
                    .append(type[1].getName()).append("' : '")
                    .append(state[1].toString());
            sb.append("'  kon  ").append(Double.toString(kon));
            sb.append("  koff ").append(Double.toString(koff));
            sb.append("  Bond_Length ").append(Double.toString(bondLength));
        }
        return sb.toString();
    }

}
