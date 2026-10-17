/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.g.object;

import java.io.PrintWriter;


public class GLink {
    
    private final GSite site1;
    private final GSite site2;
    
    private double springConstant;
    
    public GLink(GSite site1, GSite site2){
        this.site1 = site1;
        this.site2 = site2;
    }
    
    public GLink(GSite site1, GSite site2, double k){
        this.site1 = site1;
        this.site2 = site2;
        this.springConstant = k;
    }
    
    // SET METHODS
    public void setSpringConstant(double k){
        springConstant = k;
    }
    
    // GET METHODS
    public GSite getSite1(){
        return site1;
    }
    
    public GSite getSite2(){
        return site2;
    }
    
    public double getSpringConstant(){
        return springConstant;
    }
    
    public double getLength(){
        double x1=site1.getX(); double y1=site1.getY(); double z1=site1.getZ();
        double x2=site2.getX(); double y2=site2.getY(); double z2=site2.getZ();
        return Math.sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
    }
    
    /* ***************************************************************\
     *              FILE IO METHODS                                  *
     *   I won't have a readGLink() method.  It'll be easiest to     *
     *   construct the links when I construct the whole molecule,    *
     *   because then I'll have the references to the sites on hand. *
    \*****************************************************************/
    
    public void writeLink(PrintWriter p){
        if(site1 == null){
            System.out.println("Site 1 is null.");
        }
        if(site2 == null){
            System.out.println("Site 2 is null.");
        }
        p.println("LINK: Site " + site1.getIndex() + " ::: Site " + site2.getIndex());
    }
    
}
