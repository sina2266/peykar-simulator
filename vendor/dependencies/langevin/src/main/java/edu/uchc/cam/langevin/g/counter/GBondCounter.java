/*
 * Right now this class is completely unnecessary.  It's more of a placeholder
 * in case I want to track more information about bonds in the future.
 */

package edu.uchc.cam.langevin.g.counter;

import edu.uchc.cam.langevin.g.reaction.GBindingReaction;
import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.io.PrintWriter;
import java.util.Scanner;
import edu.uchc.cam.langevin.langevinnovis01.Global;

public class GBondCounter {
    
    private final GBindingReaction reaction;
    private boolean counted;
    
    public static String COUNTED = "Counted";
    public static String NOT_COUNTED = "Not_Counted";
    
    public GBondCounter(GBindingReaction reaction){
        this.reaction = reaction;
        counted = false;
    }
    
    public void setCounted(boolean bool){
        counted = bool;
    }
    
    public boolean isCounted(){
        return counted;
    }
    
    public String reactionName(){
        return reaction.getName();
    }
    
    /* ***************  LOAD ALL COUNTERS *****************/
    // Since there is only a single data field to read in, it doesn't make 
    // sense to define a method to load a single counter.
    
    public static void loadCounters(Global g, Scanner dataScanner){
        while(dataScanner.hasNextLine()){
            String [] next = dataScanner.nextLine().split(":");
            GBindingReaction reaction = g.getGBindingReaction(IOHelp.getNameInQuotes(new Scanner(next[0])));
            GBondCounter counter = reaction.getBondCounter();
            counter.setCounted(next[1].trim().equals(COUNTED));
        }
    }
    
    /* *****************  WRITE COUNTER ********************/
    // For testing
    public void writeBondCounter(PrintWriter p){
        p.print("'" + reaction.getName() + "' : ");
        if(counted){
            p.println(COUNTED);
        } else {
            p.println(NOT_COUNTED);
        }
    }
    
}
