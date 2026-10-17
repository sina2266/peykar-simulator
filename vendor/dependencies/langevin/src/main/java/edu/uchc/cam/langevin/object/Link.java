/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.uchc.cam.langevin.object;

import edu.uchc.cam.langevin.langevinnovis01.MyVector;


public class Link {
    
    private final Site [] site = new Site[2];
    
    private double springConstant;
    // The equilibrium length of the spring. 
    private double L0;
    // Current length of spring.
    private double length;
    // Orientation vector points from site1 to site2
    private final MyVector orientation = new MyVector();
    // Normal vectors pointing AWAY from the center of the spring, from each site.
    private final MyVector [] normal = new MyVector[2];
    
    public Link(Site site1, Site site2){
        this.site[0] = site1;
        this.site[1] = site2;
        normal[0] = new MyVector();
        normal[1] = new MyVector();
        updateOrientation();
        L0 = length;
    }
    
    public Link(Site site1, Site site2, double k){
        this.site[0] = site1;
        this.site[1] = site2;
        normal[0] = new MyVector();
        normal[1] = new MyVector();
        this.springConstant = k;
        updateOrientation();
        L0 = length;
    }
    
    // SET METHODS
    public void setSpringConstant(double k){
        springConstant = k;
    }
    
    // GET METHODS
    public Site getSite(int i){
        return site[i];
    }
    
    public Site [] getSites(){
        return site;
    }
    
    public double getSpringConstant(){
        return springConstant;
    }
    
    public double getLength(){
        return length;
    }
    
    public void setL0(double L0){
        this.L0 = L0;
    }
    
    //************* Update orientation vector ****************************/
    
    public void updateOrientation(){
        orientation.x = site[1].getX() - site[0].getX();
        orientation.y = site[1].getY() - site[0].getY();
        orientation.z = site[1].getZ() - site[0].getZ();
        length = orientation.length();
        // I want to avoid the overhead of small for loops. Can't trust that every
        // compiler will do away with them.
        // Because of the way the normals are defined, it is normal[1] that is parallel
        // to the orientation vector.
        normal[1].x = orientation.x/length;
        normal[1].y = orientation.y/length;
        normal[1].z = orientation.z/length;
        normal[0].x = -normal[1].x;
        normal[0].y = -normal[1].y;
        normal[0].z = -normal[1].z;
    }
    
    public void updateForces(){
        /** If spring is compressed (length < L0), then magForce > 0.  Since 
         * the normals point away from the center of the spring, the actual
         * forces are determined using magForce directly..
         *
         * EDIT: dan vasilescu
         * force site now depends on the diffusion rate of the site
         * See Boris' comment on MySystem.SpringConstant
         */
        double magForceSite0 = springConstant*site[0].getD()*(L0-length);
        double magForceSite1 = springConstant*site[1].getD()*(L0-length);
        site[0].incrementSpringForce(magForceSite0*normal[0].x, magForceSite0*normal[0].y, magForceSite0*normal[0].z);
        site[1].incrementSpringForce(magForceSite1*normal[1].x, magForceSite1*normal[1].y, magForceSite1*normal[1].z);
    }
    
}
