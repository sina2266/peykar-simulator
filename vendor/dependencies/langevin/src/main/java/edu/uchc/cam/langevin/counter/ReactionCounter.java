package edu.uchc.cam.langevin.counter;


import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.LinkedHashMap;

import edu.uchc.cam.langevin.g.reaction.*;
import edu.uchc.cam.langevin.langevinnovis01.Global;
import edu.uchc.cam.langevin.langevinnovis01.MySystem;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;


public class ReactionCounter {

    public static final Logger lg = LogManager.getLogger(ReactionCounter.class);

    private final MySystem sys;

    private final double dtdata;    // time interval between data points
    private final int totalCount;   // number of data points

    // Index of the datapoint that reactions are currently tallied into (arr[counter]).
    // Index 0 is the initial condition at t=0 and must stay reaction-free, so we begin
    // accumulating at index 1. It advances one step at each dtdata boundary (initDatapoint())
    // and is capped at the last datapoint, so reactions in the sub-dtdata tail after the final
    // boundary fold into the last datapoint instead of indexing past the arrays.
    private int counter = 1;
    private final double [] time;

    private final ArrayList<GReactionInterface> reactionList = new ArrayList<>();

    // key=reaction names, value= array of reaction occurrences for each datapoint
    private final HashMap<String, int[]> totals = new LinkedHashMap<>();
    private final HashMap<String, int[]> creations = new LinkedHashMap<>();
    private final HashMap<String, int[]> decays = new LinkedHashMap<>();
    private final HashMap<String, int[]> bindings = new LinkedHashMap<>();
    private final HashMap<String, int[]> dissociations = new LinkedHashMap<>();
    private final HashMap<String, int[]> transitions = new LinkedHashMap<>();
    private final HashMap<String, int[]> allosterics = new LinkedHashMap<>();

    public ReactionCounter(Global g, MySystem sys) {
        this.sys = sys;
        dtdata = g.getdtdata();
        totalCount = 1 + (int)Math.floor(g.getTotalTime() / g.getdtdata());
        time = new double[totalCount];
        for (int i = 0; i < totalCount; i++) {
            time[i] = i * dtdata;   // datapoint times are fixed by index; time[0] == 0 (the t=0 initial condition)
        }
        if (totalCount == 1) {
            counter = 0;            // degenerate case: only the t=0 datapoint exists, so accumulate there
        }

        for (GDecayReaction r : g.getDecayReactions()) {
            String name = r.getName();
            totals.put(name, new int[totalCount]);
            creations.put(name, new int[totalCount]);
            decays.put(name, new int[totalCount]);
        }
        for (GBindingReaction r : g.getBindingReactions()) {
            String name = r.getName();
            totals.put(name, new int[totalCount]);
            bindings.put(name, new int[totalCount]);
            dissociations.put(name, new int[totalCount]);
        }
        for (GTransitionReaction r : g.getTransitionReactions()) {
            String name = r.getName();
            totals.put(name, new int[totalCount]);
            transitions.put(name, new int[totalCount]);
        }
        for (GAllostericReaction r : g.getAllostericReactions()) {
            String name = r.getName();
            totals.put(name, new int[totalCount]);
            allosterics.put(name, new int[totalCount]);
        }
    }

    // Called at each dtdata boundary. Reactions are tallied into arr[counter] as they occur;
    // this advances counter to the next datapoint. It is capped at the final index so reactions
    // in the sub-dtdata tail after the last boundary fold into the last datapoint rather than
    // indexing past the end of the arrays.
    public void initDatapoint() {
        if (counter < totalCount - 1) {
            counter++;
        }
    }

    // utility method, we display the total number of occurrences for each reaction type at the end of the simulation
    public void printCounts() {
        lg.info("=== AllReactions ===");
        for (String name : totals.keySet()) {
            int[] arr = totals.get(name);
            int total = 0;
            for (int v : arr) {
                total += v;
            }
            lg.info(name + " " + total);
        }
    }
    public void printDetailedCounts() {
        lg.info("=== " + ReactionType.CREATION.longName + "s ===");
        for (String name : creations.keySet()) {
            int sum = 0;
            for (int v : creations.get(name)) sum += v;
            lg.info(name + " " + sum);
        }
        lg.info("=== " + ReactionType.DECAY.longName + "s ===");
        for (String name : decays.keySet()) {
            int sum = 0;
            for (int v : decays.get(name)) sum += v;
            lg.info(name + " " + sum);
        }
        lg.info("=== " + ReactionType.BINDING.longName + "s ===");
        for (String name : bindings.keySet()) {
            int sum = 0;
            for (int v : bindings.get(name)) sum += v;
            lg.info(name + " " + sum);
        }
        lg.info("=== " + ReactionType.DISSOCIATION.longName + "s ===");
        for (String name : dissociations.keySet()) {
            int sum = 0;
            for (int v : dissociations.get(name)) sum += v;
            lg.info(name + " " + sum);
        }
        lg.info("=== " + ReactionType.TRANSITION.longName + "s ===");
        for (String name : transitions.keySet()) {
            int sum = 0;
            for (int v : transitions.get(name)) sum += v;
            lg.info(name + " " + sum);
        }
        lg.info("=== " + ReactionType.ALLOSTERIC.longName + "s ===");
        for (String name : allosterics.keySet()) {
            int sum = 0;
            for (int v : allosterics.get(name)) sum += v;
            lg.info(name + " " + sum);
        }
    }


    public enum ReactionType {

        CREATION("CreationReaction", "CREATION"),
        DECAY("DecayReaction", "DECAY"),
        BINDING("BindingReaction", "BINDING"),
        DISSOCIATION("DissociationReaction", "UNBINDING"),
        TRANSITION("TransitionReaction", "TRANSITION"),
        ALLOSTERIC("AllostericReaction", "ALLOSTERIC");

        public final String longName;
        public final String shortName;
        ReactionType(String longName, String shortName) {
            this.longName = longName;
            this.shortName = shortName;
        }
        public String formatColumnName(String reactionName) {
            return shortName + "_" + reactionName;
        }
    }

    public void plusCreationReaction(String name) {
        lg.debug(" --------- " + ReactionType.CREATION.longName + ": " + name);
        totals.get(name)[counter]++;
        creations.get(name)[counter]++;
    }
    public void plusDecayReaction(String name) {
        lg.debug(" --------- " + ReactionType.DECAY.longName + ": " + name);
        totals.get(name)[counter]++;
        decays.get(name)[counter]++;
    }
    public void plusBindingReaction(String name) {
        lg.debug(" --------- " + ReactionType.BINDING.longName + ": " + name);
        totals.get(name)[counter]++;
        bindings.get(name)[counter]++;
    }
    public void plusDissociationReaction(String name) {
        lg.debug(" --------- " + ReactionType.DISSOCIATION.longName + ": " + name);
        totals.get(name)[counter]++;
        dissociations.get(name)[counter]++;
    }
    public void plusTransitionReaction(String name) {
        lg.debug(" --------- " + ReactionType.TRANSITION.longName + ": " + name);
        totals.get(name)[counter]++;
        transitions.get(name)[counter]++;
    }
    public void plusAllostericReaction(String name) {
        lg.debug(" --------- " + ReactionType.ALLOSTERIC.longName + ": " + name);
        totals.get(name)[counter]++;
        allosterics.get(name)[counter]++;
    }

    /******************************************************************\
     *                         WRITE DATA                             *
     * @param path                                                    *
    \******************************************************************/
    public void writeFullData(File path) {

        File out = new File(path, "FullTotalReactionsCountData.csv");
        try (PrintWriter p = new PrintWriter(new FileWriter(out), true)) {
            //
            // ---------- HEADER
            //
            p.print("Time,");
            for (String name : creations.keySet()) {
                p.print(ReactionType.CREATION.formatColumnName(name) + ",");
            }
            for (String name : decays.keySet()) {
                p.print(ReactionType.DECAY.formatColumnName(name) + ",");
            }
            for (String name : bindings.keySet()) {
                p.print(ReactionType.BINDING.formatColumnName(name) + ",");
            }
            for (String name : dissociations.keySet()) {
                p.print(ReactionType.DISSOCIATION.formatColumnName(name) + ",");
            }
            for (String name : transitions.keySet()) {
                p.print(ReactionType.TRANSITION.formatColumnName(name) + ",");
            }
            for (String name : allosterics.keySet()) {
                p.print(ReactionType.ALLOSTERIC.formatColumnName(name) + ",");
            }
            p.println();
            //
            // ---------- DATA ROWS
            //
            for (int i = 0; i < totalCount; i++) {

                double t = Math.round(time[i] * 1e12) / 1e12;   // Clean time rounding
                p.print(t + ",");

                for (String name : creations.keySet()) {
                    p.print(creations.get(name)[i] + ",");
                }
                for (String name : decays.keySet()) {
                    p.print(decays.get(name)[i] + ",");
                }
                for (String name : bindings.keySet()) {
                    p.print(bindings.get(name)[i] + ",");
                }
                for (String name : dissociations.keySet()) {
                    p.print(dissociations.get(name)[i] + ",");
                }
                for (String name : transitions.keySet()) {
                    p.print(transitions.get(name)[i] + ",");
                }
                for (String name : allosterics.keySet()) {
                    p.print(allosterics.get(name)[i] + ",");
                }
                p.println();
            }

        } catch (IOException e) {
            e.printStackTrace(System.out);
        }
    }



}
