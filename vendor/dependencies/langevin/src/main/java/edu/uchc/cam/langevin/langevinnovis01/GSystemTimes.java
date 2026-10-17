/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.langevinnovis01;

import edu.uchc.cam.langevin.helpernovis.IOHelp;
import java.io.PrintWriter;
import java.util.Scanner;


public class GSystemTimes {
    
    /* ********** Strings for system IO ****************************/
    public static final String TOTALTIME = "Total time";
    public static final String DT = "dt";
    public static final String DTSPRING = "dt_spring";
    public static final String DTDATA = "dt_data";
    public static final String DTIMAGE = "dt_image";

    /* *********** All times stored as double values, in seconds ********/
    private double totalTime;
    private double dt;
    private double dtspring;
    private double dtdata;
    private double dtimage;
    
    /* *********** Constructor just sets some defaults *****************/
    public GSystemTimes(){
        totalTime = 1e-2;
        dt = 1e-8;
        dtspring = 1e-9;
        dtdata = 1e-4;
        dtimage = 1e-4;
    }
    
    /* ************ GET METHODS  ********************/
    
    public double getTotalTime(){
        return totalTime;
    }
    
    public double getdt(){
        return dt;
    }
    
    public double getdtspring(){
        return dtspring;
    }
    
    public double getdtdata(){
        return dtdata;
    }
    
    public double getdtimage(){
        return dtimage;
    }
    
    /* ************* LOAD DATA ************************/
    
    public void loadData(String dataString){
        // System.out.println(dataString);
        Scanner sc = new Scanner(dataString);
        while(sc.hasNextLine()){
            String [] next = sc.nextLine().split(":");
            switch(next[0]){
                case TOTALTIME:
                    totalTime = Double.parseDouble(next[1]);
                    break;
                case DT:
                    dt = Double.parseDouble(next[1]);
                    break;
                case DTSPRING:
                    dtspring = Double.parseDouble(next[1]);
                    break;
                case DTDATA:
                    dtdata = Double.parseDouble(next[1]);
                    break;
                case DTIMAGE:
                    dtimage = Double.parseDouble(next[1]);
                    break;
                default:
                    System.out.println("SystemTimes loadData received"
                        + " unexpected input line. "
                            + "Input = " + next[0] + " : " + next[1] );
            }
        }
    }
    
     /* ************** WRITE DATA ************************/
    
    public void writeData(PrintWriter p){

        p.println(TOTALTIME + ": " + IOHelp.scientificFormat.format(totalTime));
        p.println(DT + ": " + IOHelp.scientificFormat.format(dt));
        p.println(DTSPRING + ": " + IOHelp.scientificFormat.format(dtspring));
        p.println(DTDATA + ": " + IOHelp.scientificFormat.format(dtdata));
        p.println(DTIMAGE + ": " + IOHelp.scientificFormat.format(dtimage));

    }
    
}
